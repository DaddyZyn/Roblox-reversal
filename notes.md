# How the Roblox Windows client works (build f5a60436d48947d3)

Findings from live-memory reverse engineering of `RobloxPlayerBeta.exe`
(145.5 MB unpacked image, 38,136 functions) and `RobloxPlayerBeta.dll`
(21 MB anti-tamper module). All RVAs assume preferred image base
`0x140000000` (see `include/offsets.h`).

---

## 1. Binary architecture

### 1.1 Two modules, two problems

- **RobloxPlayerBeta.exe** — the entire engine: Luau VM, reflect system,
  renderers, content pipeline, OpenSSL 3.x (statically linked). This is what
  you want to analyze.
- **RobloxPlayerBeta.dll** — the Byfron/Hyperion anti-tamper runtime. One giant
  `.byfron` section (~19.3 MB of the 21 MB file). Contains the integrity
  engine, the protected-pool allocator, and its own crypto.

### 1.2 Packing state (the thing that kills naive static analysis)

- The on-disk exe has **100% encrypted `.text`** (entropy 8.0 across all
  blocks). Section headers on disk are deliberately mangled; the real section
  table only exists in memory after Hyperion rebuilds it during init.
- ~31.5k exception-directory entries and ~31.5k RTTI type descriptors survive
  in plaintext on disk — a full class inventory is available statically, but
  no code is.
- The DLL is the opposite: mostly **plaintext code** with its section names
  mangled. Its export `run` (RVA `0x2C460`) is visibly mutated
  (constant-soup imul/rol/xor chains).

### 1.3 Runtime state

After init, `.text` is fully decrypted in memory (0 high-entropy pages out of
24,443). A ReadProcessMemory snapshot at that point captures a complete
decrypted image. Hyperion does not strip external read handles on this build
(OpenProcess + RPM works from an unprivileged process; verified repeatedly).

Entry point: RVA `0x9175004` — a 20-byte `tempest` stub section.

---

## 2. The Luau VM

`luaV_execute` lives at RVA `0x26CA540` (47.8 KB, the largest function in the
binary). Modern Luau, heavily customized.

### 2.1 Instruction encoding (unchanged from upstream)

```
u32 instruction:
  bits [0:8)   opcode
  bits [8:16)  A
  bits [16:24) C
  bits [24:32) B
sBx = signed 16-bit at bytes [2:4]
pc advances 4 bytes per instruction; jump targets: pc += sBx*4 + 4
```

### 2.2 VM state (register map inside luaV_execute)

| Register | Role |
|----------|------|
| `r15` | pc (bytecode instruction pointer) |
| `r12` | base (Lua stack of TValues) |
| `[rsp+0x48]` | constants pointer |
| `xmm10/xmm11/xmm12` | preloaded 0.5 / 3.0 / 2.0 (arith fast paths) |

Dispatch (inlined at the top of the function and re-inlined at handler tails):

```
movzx eax, byte ptr [r15]              ; opcode
cmp   rax, 0xFB                        ; bounds: 252 slots
movzx eax, byte ptr [optab + rax]      ; op -> index (0x26D65BC)
mov   ecx, dword ptr [hdtab + rax*4]   ; index -> handler (0x26D6450)
jmp   rcx                              ; handlers are absolute RVAs
```

~120 real opcodes; unassigned slots route to an "illegal" handler at
`0x26D5F30` (dispatch index 90).

### 2.3 Opcode shuffling

**The bytecode opcode numbering is shuffled per build.** The `op` values that
appear in compiled bytecode map through `optab` to a stable dispatch index.
A client update changes the shuffle; the mapping must be re-derived from
`optab`/`hdtab` each build (scriptable in ~20 minutes from a memory dump —
see `include/vm.h` for the extracted table of this build).

### 2.4 Handler identification highlights

- arith: seven dedicated slow-path helpers fingerprinted by their SIMD
  semantics (addsd/subsd/mulsd/divsd/fmod/pow), used by ADD/SUB/MUL/DIV/MOD/POW
  and their K-variants.
- `POWK` (dispatch idx 0) has exponent fast paths: 2 → square, 0.5 → `sqrtpd`
  (negative base branches to a fallback helper), 3 → cube, else generic `pow`.
- `NAMECALL` (idx 87) carries a nested sub-switch — the namecall cache dispatch.
- `FORNPREP` (idx 51) validates step `== 0.0` and resets loop state.
- GET/SETTABLE family calls a **Vector3 equality helper outside the VM**
  (`0x85F060`) — Roblox tables accept vector keys, and that call site is the
  Lua→engine boundary landmark.

### 2.5 The error-string dead end (important time-saver)

All upstream Luau error literals ("attempt to index %s with '%s'",
"attempt to perform arith (%s) on %s", ...) sit in `.rdata` with **zero
references of any kind** — no LEA, no SSE copies, no pointer tables, no heap
copies anywhere in 2 GB of live process memory. The VM error layer is mutated
into protected code; those literals are dead leftovers. Do not waste time
anchoring on them — anchor on the metamethod names ("__index" etc.), which
are alive via `luaT_init`-style std::string builders at `0x7419C0`/`0x741D70`,
or on the Roblox reflect error strings ("'%s' is not a valid member of %s"
→ `0x3FE1EA0`), which are plainly referenced.

---

## 3. reflect & metadata

- The Lua reflect registry (class/method name pairs, 16-byte stride) sits
  at `0x6949C00` — ContentProvider's encrypted-asset methods
  (`RegisterEncryptedAsset`, `RegisterSessionEncryptedAsset`,
  `RegisterDefaultEncryptionKey`, `ListEncryptedAssets`, ...) are enumerated
  there, but function pointers live in separate packed tables.
- **Packed function catalog**: 524 tables in `.rdata` with stride-5 entries
  `{u32 fnRVA, u8 tag}` covering 23,995 functions (63% of all functions,
  biased toward small ones; the biggest functions — the VM, crypto, allocator
  machinery — are excluded). Semantics not fully determined (integrity
  manifest vs OTA-patch metadata), but it's a ready-made module map.
- 31,564 RTTI class descriptors survive in `.rdata` — the full class inventory,
  including the `reflect@RBX` layer where mangled names carry complete
  Lua API signatures.

---

## 4. Asset pipeline & cache

### 4.1 Content cache (`%LOCALAPPDATA%\Roblox\rbx-storage`)

~21k files, hash-named (`MD5`-style, bucketed by first 2 hex chars), three
container formats — **all plaintext**:

1. **RBXH v2 binary variant** (~97%): 37-byte header
   (`"RBXH"` + u32 ver=2 + `0x00`@0x08 + zeros + `0xC8`@0x0D + u32
   payload_size@0x19 + u32 hash@0x1D + 4 zero bytes) → raw asset
   (KTX2 textures, Roblox meshes v2/v4/v5/v7 incl. COREMESH+Draco, PNG, OGG,
   RBXM, TTF).
2. **RBXH v2 URL variant**: `url_len` byte @0x08 + `tr.rbxcdn.com` URL +
   `00 C8` + size/hash fields → carved WebP (avatar thumbnails).
3. **zstd container** (~3%): u32 hash + u16s + zstd stream (raw-block heavy)
   → CSGPHS physics, wrapped meshes.

### 4.2 The ChaCha20 asset-encryption path (dormant)

The client ships a complete encrypted-asset subsystem: HKDF + ChaCha20-Poly1305
via OpenSSL EVP, session-scoped keys (`RegisterSessionEncryptedAsset`), and a
20-byte seed literal at `0x6F248B0` (`d1ce3a4c10b7885d704014a45eb39b630468b9be`,
stored as hex-ASCII). During normal gameplay on this build, **zero encrypted
assets were observed** — every fetched asset cached plaintext. The path exists
but appears gated off for standard content. Any tooling that decrypts "Roblox's
encrypted assets" on a build like this is solving a problem the client isn't
currently throwing.

---

## 5. Protection (hyperion / Byfron / Hyperion)

- **Code encryption**: exe `.text` fully encrypted at rest, decrypted in place
  at init (see 1.2).
- **String obfuscation**: selective; sensitive literals either stripped or
  left as dead decoys while consumers reference them from mutated code.
- **Mutation VMs (the DLL core)**: two flattened dispatch blobs —
  `sub_180226D40` (2.4 MB) and `sub_1811DA680` (489 KB) — self-calling,
  reached via `.rdata` function-pointer tables, with anti-debug tripwires
  (`pushfq`/trap-flag checks, TEB magic compares via `gs:[computed]`) and
  garbage-byte interleave. ChaCha20 lives inside them (sigma at DLL RVA
  `0x1297450`, 96+ byte key table at `0x1297490`).
- **Protected-pool allocator**: Hyperion allocates game memory through a
  ChaCha20-encrypted pool (init machinery at exe `0x7A6340`, 1 GB-threshold
  allocations, ENOMEM handling, TLS indices 0x441/0x590). Live memory shows
  hundreds of per-region guard contexts whose nonces embed their own address.
- **Runtime rekey**: at least one active subsystem (`0x872EBC0` context)
  rekeys on game join.
- Passive external reads (RPM) were never detected/blocked on this build;
  handle stripping was not active.

---

## 6. How I got all of this

For anyone repeating this on the next update:

1. **Dump the client in memory.** The shipped exe is fully packed, so I let
   Hyperion unpack its own sections at launch and carved the main module out
   of the live process from the login screen (no injection needed, external
   reads were never blocked on this build).
2. **Rebuild the image.** The carved memory is a PE with live pointers, so I
   walked the `.reloc` directory (534k DIR64 fixups), re-added runtime
   pointers the loader wrote outside the reloc coverage, and set the base
   back to `0x140000000`. After that it loads in IDA like a normal binary.
3. **Re-anchor analysis.** Autoanalysis on the decrypted image finds all
   38,136 functions, and the surviving RTTI gives every class a name. From
   there it's ordinary RE.
4. **Find the VM.** The interp gives itself away: an opcode byte fetch,
   a two-level table dispatch, an indirect jmp. Extracting the two tables
   gives the full opcode map (remember: op values are shuffled per build).
5. **Name everything else.** Helper functions fall out of the call graph —
   the arith helpers are self-identifying by their SIMD ops, table/call/for
   helpers cluster by shared callees, and the reflect metadata ties
   function pointers to API names.

The same order works on the next client update without redoing the thinking.