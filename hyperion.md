# hyperion (Byfron/Hyperion) Anti-Cheat Architecture

Complete reverse engineering of the hyperion protection layer in
`RobloxPlayerBeta.dll` (19.3MB `.byfron` section) and its integration
with `RobloxPlayerBeta.exe`.

All RVAs relative to `RobloxPlayerBeta.dll` base unless noted.

---

## 1. Detection Layers

### 1.1 Timing checks (rdtsc)

**5,464 `rdtsc` instructions** distributed across the entire DLL.
Every 64KB code block contains 50-62 instances. Each check:
1. Executes `rdtsc` to read the cycle counter
2. Runs a check operation
3. Executes `rdtsc` again
4. Compares the delta against a threshold

If the delta exceeds the threshold, a debugger or analysis tool was
detected (single-stepping, trace, or heavy instrumentation).

The checks are uniform — no single "master timing check." Every code
region independently validates its own execution time.

### 1.2 Hardware breakpoint detection

**20+ sites** accessing debug registers DR0-DR7:

| Register | Access | Purpose |
|----------|--------|---------|
| DR0 | read + write | hardware BP #0 |
| DR1 | read + write | hardware BP #1 |
| DR2 | read + write | hardware BP #2 |
| DR4 | read + write | aliased to DR0 (debugger test) |
| DR6 | read | breakpoint status |
| DR7 | read + write | breakpoint enable/control |

DR4/DR5 accesses are notable: on x86, these alias to DR0/DR1 in real
mode. In protected mode they cause #UD. hyperion uses them to detect
certain debugger behaviors.

### 1.3 Trap flag detection

**30,074 `pushfq` instructions.** Each one preserves flags, and many
are followed by a test of the trap flag (bit 8 of EFLAGS). Any
single-step operation sets TF, which is detected on the next pushfq
check.

The sheer volume (one pushfq per ~400 bytes of code) means every
code path is TF-checked.

### 1.4 Debug exception checks (INT 2D)

**39 `int 2d` instructions.** When a debugger is attached:
- The debug event is consumed by the debugger
- Execution continues normally (or at a different address)
- The check code after INT 2D detects the consumption

When no debugger:
- INT 2D raises an exception
- hyperion's vectored exception handler (VEH) catches it
- Execution continues correctly

If the VEH doesn't fire (debugger ate it), detection triggers.

### 1.5 VM/hypervisor detection

**17 `inb al, 0x64` instructions** reading the PS/2 keyboard
controller status port. VMware, VirtualBox, and QEMU respond
differently to port 0x64 reads than bare metal. Used to detect
sandboxed analysis environments.

### 1.6 Direct syscalls (usermode hook bypass)

**7,643 `syscall` instructions.** The DLL never calls ntdll exports
for sensitive operations. Instead, it:
1. Resolves the syscall number at runtime (SSN resolution)
2. Executes `syscall` directly from the DLL's code

This bypasses ALL usermode API hooks:
- IAT hooks (no IAT calls for sensitive ops)
- Inline hooks on ntdll (never called through ntdll)
- ETW-based monitoring (direct syscall, no telemetry)

Syscall density is uniform (72-102 per 64KB block) — every code
region independently makes direct syscalls.

### 1.7 Code integrity / patch detection

**55 `rep cmpsb` instructions** performing byte-by-byte memory
comparisons. These compare code sections against expected values.

Combined with the ChaCha20-based integrity system (see section 3),
this forms the memory patch detection layer. Any modification to
protected code or data pages triggers detection.

**74 `rep movsb` instructions** for memory copying — used for code
decryption, state transfer between check passes, and response
dispatch.

---

## 2. Architecture

### 2.1 Module layout

```
RobloxPlayerBeta.dll (21 MB)
├── .byfron section: 0x10000 - 0x128A516 (19.3 MB, R+X)
│   ├── 0x226D40 - 0x470CB8: Mutation VM #1 (2.4 MB)
│   ├── 0x11DA680 - 0x12529CA: Mutation VM #2 (489 KB)
│   └── scattered: ~7,600 syscall stubs, ~5,400 rdtsc checks
├── .rdata: 0x1290000 - 0x128DEC84
│   └── 0x1297450: ChaCha20 sigma + key table (96+ bytes)
├── .data: 0x1470000
└── export "run": 0x2C460 (mutated entry point)
```

### 2.2 Mutation VMs

Two flattened code blocks that are not normal functions:

- **VM #1** (`0x226D40`, 2.4 MB): the main integrity engine.
  Self-calling (10+ internal dispatch sites). Contains the ChaCha20
  references and the primary integrity check loops.
- **VM #2** (`0x11DA680`, 489 KB): secondary engine, 33 self-call
  sites, one real callee (`sub_18061DDF0`, 0x66D bytes — a normal
  function buried inside the mutation).

Both are reached via function-pointer tables in `.rdata` (found at
`0x18144A2A4` and `0x1814588CC` — the pointers are live heap values,
not static RVAs).

Each VM uses:
- Computed jumps (`jmp rcx` from calculated targets)
- Constant-soup arith chains (imul/rol/xor/add with random immediates)
- Nibble permutation dances (0xF0F0/0x3333/0x5555 masking)
- Flag-based opaque predicates with garbage fallback paths
- `cpuid` leaf 0x8000001B for environment fingerprinting
- TLS access via computed `gs:[offset]` with magic value validation

### 2.3 Anti-debug tripwires

**Export `run` (0x2C460)** contains the canonical example:

```asm
pushfq                     ; save flags
pop     r8                 ; r8 = EFLAGS
test    r8d, 100h          ; check trap flag
jz      continue           ; no debugger
jmp     [corrupted_addr]   ; TF set = crash into garbage

mov     r10d, [computed]   ; obfuscated TLS offset
mov     rax, gs:[r10]      ; read TLS via computed gs offset
cmp     rax, 0x527762C441382211  ; magic validation
jnz     detected           ; wrong value = tampering
...
hlt                        ; if we reach here, something's wrong
```

Tripwire types:
- **TF check**: `pushfq; pop r8; test r8d, 100h` → if trap flag set, crash
- **TLS magic**: computed `gs:[offset]` must match a hardcoded constant
- **Garbage landing**: jump targets land in the middle of instructions,
  producing undefined behavior
- **Stack canary**: `mov rax, [security_cookie]; xor rax, rsp` checked
  at function exit

### 2.4 Syscall mechanism

hyperion resolves syscall numbers at runtime and issues them directly:

```
mov eax, <SSN>           ; syscall number (resolved per build)
mov r10, rcx             ; Windows x64 convention
syscall                  ; direct kernel transition
```

The SSNs are obfuscated in the mutation VMs and decoded per-call.
This means:
- No ntdll import is needed for sensitive operations
- API monitoring tools see nothing
- Inline hooks on ntdll are completely bypassed

Known syscall operations used by hyperion (inferred from behavior):
- `NtQueryInformationProcess` (debug port, debug object handle)
- `NtQuerySystemInformation` (process/thread enumeration)
- `NtReadVirtualMemory` (self-read for integrity)
- `NtProtectVirtualMemory` (page protection changes)
- `NtOpenThread` (thread enumeration and inspection)
- `NtSetInformationThread` (thread hide from debugger)

---

## 3. integrity system

### 3.1 Memory integrity

The `rep cmpsb` instructions perform byte-by-byte comparison of code
regions. Combined with the ChaCha20 encryption:

- Protected pages are encrypted at rest
- On access, they're decrypted into a shadow buffer
- The shadow buffer is compared against expected values
- Any mismatch = patch detected

The protected-pool allocator (exe `0x7A6340`, chacha ctx at `0x7B06860`)
manages encrypted memory regions. Each allocation is encrypted with a
per-region key derived from the master key + address.

### 3.2 integrity verification loop

The heartbeat loop runs at **1000ms intervals** (two sites found at
dll+`0x1F6A2` and dll+`0x497017`, both with `mov edx, 0x3E8` before
a timer/syscall call).

Each heartbeat:
1. Enumerates threads (looking for unknown threads)
2. Checks memory integrity (rep cmpsb against shadow)
3. Validates TLS magic values
4. Checks debug registers
5. Runs timing consistency checks (rdtsc deltas)
6. Verifies the protected pool integrity

---

## 4. Detection response

When any check fails, the response is immediate and non-recoverable:

- **Crash via garbage jump**: `jmp` to a corrupted/misaligned address
  causes an access violation at an unpredictable address
- **Stack corruption**: `pop rsp; mov esp, <garbage>` destroys the stack
- **HLT**: `hlt` instruction in usermode causes an access violation
- **Infinite loop**: the thread enters a tight loop, effectively hanging

There is no "detection flag" or "grace period." Every check that fails
results in immediate process termination through hardware-level
corruption that cannot be caught by exception handlers.

---

## 5. Thread architecture

555 total threads in the client process. The anti-cheat threads are:

- **Priority 16 thread** (TID 17220): highest priority, likely the
  main integrity heartbeat
- **Priority 1 thread** (TID 24612): above-normal, likely the
  memory scanner
- ~10 TLS-active threads (TLS array populated) handle various
  check dispatch roles

The remaining ~540 threads are Roblox engine workers (rendering,
physics, audio, network, scripting coroutines).

---

## 6. Integration points

### 6.1 Protected-pool allocator

The exe-side allocator (at `0x7A6340`) provides encrypted memory:
- ChaCha20 context at exe `.data` offset `0x7B06860`
- Key: `19a4746e8df7b1dab671cdb166484a8942709a04c9f42f61b86521a38dbb9238`
- Counter resets on game join (observed 80 → 1)
- Regions allocated above 1GB threshold use this pool

### 6.2 TLS integration

hyperion uses two TLS slots (indices 0x441 and 0x590) for per-thread state:
- Slot 0x441: thread identity / validation state
- Slot 0x590: context pointer chain

Access pattern: `gs:[0x58]` → TLS array → `[slot * 8]` → TLS block
→ field at fixed offset within the block

### 6.3 Init-time protections

The DLL's export `run` sets up:
1. VEH (vectored exception handler) for INT 2D handling
2. TLS callbacks (additional entry before main)
3. Page protection elevation for .byfron section
4. Direct syscall table initialization
5. Heartbeat thread creation

---

## 6.5 Two-tier integrity architecture

The integrity catalog (524 tables, 24k entries) covers only **small functions**
(avg 150 bytes = 63% of all functions). The remaining 37% — the large, complex
functions including the VM itself, crypto, allocator, and all high-value targets —
are UNCOVERED by the catalog.

This means protection is two-tier:

**Tier 1: Catalog integrity (63% of functions)**
- Small functions checked by simple hash/comparison against the catalog
- Fast, runs every heartbeat (1000ms)
- Detects any byte-level modification

**Tier 2: Runtime mutation protection (37% of functions)**
- Large functions protected by the mutation VMs, ChaCha20 encryption,
  5,464 rdtsc timing checks, and 30,074 TF checks
- The interp, crypto, allocator, and all high-value code
- Cannot be patched without defeating the mutation VM

The uncovered set includes:
- luaV_execute (the VM itself)
- All arith/table/concat/compare helpers
- Print and the stdlib bridge
- The protected-pool allocator
- All crypto functions
- The check dispatcher

This is deliberate: the small functions are "known good" and hash-verifiable.
The large functions require runtime obfuscation because they're too complex
to hash-check efficiently.

---

## 6.8 VEH and check function addresses

VEH registration sites found in the DLL (the `mov ecx,1; call` pattern).
The handler at 0x11DA220 is a calloc wrapper (false positive from generic
pattern match). The real VEH handler requires ntdll VEH list enumeration
at runtime.

Confirmed check function addresses:
- 0x11DA220: allocation wrapper (false VEH positive)
- 0x2D418: exception code checker (C runtime, not anti-cheat)
- 55 rep cmpsb sites: the actual memory integrity comparisons
- 5464 rdtsc sites: timing checks (uniform distribution)

All the check logic is inside the two mutation VMs. The `rep cmpsb` sites
are at these RVAs (dll relative):
0x76C2F, 0x8AC3C, 0xBF0D0, 0x10D810, 0x133976, 0x164E55, 0x1CDE06,
0x1D07E4, 0x1E9D83, 0x2340A7, and 45 more

The heartbeat delay (Sleep(1000)) is at dll+0x1F6A2 and dll+0x497017.
The heartbeat function containing the 1000ms sleep starts near those sites.

---

## 7. Summary of check types

| Check type | Count | What it detects |
|-----------|-------|-----------------|
| rdtsc timing | 5,464 | single-step, trace, heavy instrumentation |
| syscall (direct) | 7,643 | bypasses all usermode hooks |
| pushfq (TF check) | 30,074 | single-step operation |
| cpuid | 450 | CPU features, VM detection |
| int 2d | 39 | debugger presence |
| DR access | 20+ | hardware breakpoints |
| PS/2 port read | 17 | VM/hypervisor environment |
| rep cmpsb | 55 | memory patch detection |
| rep movsb | 74 | code/data transfer (decryption) |
| pushfq total | 30,074 | flag preservation + TF |

The volume is the defense: with 5,400+ timing checks and 30,000+ flag
preservations spread uniformly, there is no "safe region" to analyze in.
Every 400-500 bytes of code has an independent check.
