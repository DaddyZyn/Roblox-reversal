# Finding Every Offset — IDA + CE Guide

How to locate every offset in this repo on any build. Written for IDA Pro 9.x and Cheat Engine 7.x.

---

## Setup

The shipped exe is packed. The entire `.text` section is encrypted and section headers are mangled. You need a runtime dump.

1. Launch the client, wait for login screen (Hyperion unpacks at init)
2. Read the main module's memory via RPM
3. Write the carved image to disk
4. Walk `.reloc` (DIR64 fixups) to rebase to `0x140000000`
5. Normalize section table (raw offset = VA, FileAlignment = 0x1000)
6. Load in IDA as PE, let autoanalysis finish (~38k functions)

For CE: attach to the running client, memory scanner + hex editor only. 
---

## VM Core

### LuaV_Execute

Sort functions by size in IDA. The largest function (~48KB) is the interp. Confirm by checking the first 64 bytes for the dispatch signature:

```
movzx eax, byte ptr [r15]        ; opcode fetch
cmp rax, 0xFB                    ; bounds
movzx eax, byte ptr [table + rax]
mov ecx, dword ptr [table2 + rax*4]
add rcx, base
jmp rcx
```

The two tables referenced here are `optab` (op -> index) and `hdtab` (index -> handler). Extract both and you have the complete opcode map. Remember: opcode values shuffle every build, so always re-extract.

### Dispatch Tables

From the dispatch loop, follow the RIP-relative addresses. The optab is in .rdata as 252 bytes (one per opcode slot). The hdtab is 252 int32s (absolute handler RVAs). Unassigned slots route to the illegal handler.

### Opcode Handlers

Each handler is identified by its helper calls:

- Handlers calling the same arith helper are the same family. The arith helpers self-identify by SIMD: `addsd` = ADD, `subsd` = SUB, `mulsd` = MUL, `divsd` = DIV, fmod call = MOD, pow call = POW
- NEWTABLE: allocates 112/48 byte structures, increments count at +0x34
- NAMECALL: has a nested sub-switch (namecall cache dispatch)
- FORNPREP: checks step == 0.0
- JMPX: 25-bit jump displacement packing
- MOVE: copies a full 16-byte TValue (OWORD move)

---

## Lua Globals

### luaO_nilobject

16 all-zero bytes in .rdata, referenced ~224 times (60+ from the interp). Search IDA for 16-byte zero runs in .rdata, check xref count. The real one has the most traffic.

### luaH_dummynode

32+ zero bytes in .rdata near the nilobject. ~70 references. Same search method.

### LuaT_Eventnames

Array of `char*` pointers to the metamethod names. First entry points to `"__namecall"`, second to `"__call"`, etc. (18 entries total through `"__metatable"`).

**String search in IDA:** `Alt+T` for `__namecall`. The string is in .rdata. Find the array that contains a pointer to it — that array is `luaT_eventnames`.

### LuaT_typenames

Same approach. Search for `"nil"`, `"boolean"`, `"number"` strings. The array containing pointers to all of them in order (nil, boolean, userdata, number, integer, vector, string, table, function, userdata, thread, buffer, class, object) is `luaT_typenames`. 14 entries.

---

## luaD Family

### LuaD_Throw

Called from error paths in the VM. In IDA, find functions called from `luaV_execute`'s error handling that have the `lua_exception: not enough memory` string nearby.

### LuaD_GrowStack

Called when the stack index is out of bounds. Find `luaA_index2adr` first (it handles upvalue pseudo-index `idx*16 - 0x10`), then look at what it calls when the index is out of range. That callee is growstack.

### LuaD_Call family

Scan .text for E8 (call) instructions targeting `luaV_execute`. Each caller is a member of the luaD_call family. The one with the most call sites is `luaD_callnoyield`.

---

## Arith Helpers

From the arith opcode handlers in `luaV_execute`, each one calls a slow-path helper. Disassemble the helper — it self-identifies by SIMD:

| Instruction | Opcode |
|-------------|--------|
| `addsd` | ADD |
| `subsd` | SUB |
| `mulsd` | MUL |
| `divsd` | DIV |
| fmod call | MOD |
| pow call | POW |

The helper containing `addsd` is the ADD slow path, etc.

---

## Luau C API

These live in a contiguous code region (~0x84D000 to ~0x1615000). They all take `lua_State*` as first arg (rcx) and access TValue tags at stack offsets.

### lua_pushnil

Sets tag at +0xC to 0. Tiny function, no calls.

### lua_pushnumber

Sets tag at +0xC to 3 and stores a double at +0x0.

### lua_createtable

Allocates a Table (112/48 byte structs), sets tag to 6 (LUA_TTABLE).

### lua_settop

Adjusts the stack pointer by a signed delta. Contains stack arith.

### lua_getfield

Calls the table-get helper. References the `__index` metamethod string via the eventnames table.

### lua_setfield

Calls the table-set helper plus a write barrier.

### String search shortcuts

These strings in .rdata point to their containing functions:
- `"stack overflow"` → near pushstring/settop family
- `"not enough memory"` → near allocation functions
- `"expected, got"` → near luaL_argerror family

---

## reflect System

### Property lookup failure

String search: `'%s' is not a valid member of %s`. Has exactly one xref — that's `PropLookupFail`.

### Unable to cast

String search: `Unable to cast %s to %s`. One xref — the cast function.

### reflect registry

At a fixed .rdata address: `{class_name*, method_name*}` pairs (16 bytes each). Walk it to enumerate every script API method. The function pointers are in separate `BoundFuncDesc` objects (found via RTTI).

---

## Identity System

### GetIdentityStructure

A .data global that contains a non-zero pointer (the identity struct). Check that the address contains a valid heap pointer.

### GetCapabilities

Near the identity functions. Reads the capabilities bitmask from the identity struct at offset +0x28.

---

## Signal Emitters

Found through EventDesc RTTI templates. Each `EventDesc<void(Sig)>` has a vftable where slot [1] is the emitter.

1. Search RTTI for `EventDesc@VClickDetector` — the mangled name encodes the signature
2. Get the TypeDescriptor RVA
3. Find the COL pointing to it (self-RVA validated at COL+0x14)
4. Find the vftable pointing to the COL
5. Vtable slot [1] = fire function

Same-signature events share one emitter. `void(shared_ptr<Player>)` is used by both ClickDetector MouseClick and ProximityPrompt Triggered.

---

## Task Scheduler

### TaskSchedulerImpl

RTTI: `_Ref_count_obj2<UFrame@TaskSchedulerImpl::step>` confirms `step()` exists. The frame vftable is the make_shared allocation inside step.

Find the scheduler object at runtime: scan for the vftable pointer in the live process, then dump the object's fields.

### RawScheduler

A .data global containing a heap pointer. Verify: read the qword, deref, check the target has a valid vftable.

### TargetFps

A double in .data. Cheat Engine method:
1. Change FPS cap in settings
2. Scan for the new value as double
3. Change again, narrow
4. Stable address = TargetFps

Note: reads 0.0 when no custom cap is set.

---

## FakeDataModelPointer

A .data global pointing to a fake DataModel shell. Field +0x1D0 on the fake points to the real DataModel.

**IMPORTANT:** null during gameplay. Only set during loading phase (login → game transition). To capture:
1. Attach before the client finishes loading
2. Read the global while loading screen is visible
3. By the time you're in-game it's null

Alternative: find the real DataModel by scanning for the ScriptContext vftable, then reading +0x68 (parent field).

---

## DataModel / Instance Offsets

### Parent (+0x68)

Every Service object holds its parent DataModel pointer at +0x68. Find by:
1. Locate the DataModel at runtime (scan for vftable)
2. Scan the heap for references to it
3. Walk back from each reference to find the containing object's vftable
4. Every service class shows the same offset

### Session State (+0x5D0)

Non-zero when in a game, zero on login screen.

### Children (+0x78)

Children vector on the DataModel object. On this build, children use handle tables (not raw pointers), so the vector contents are handles, not Instance pointers.

---

## integrity Catalog (staticintegrity)

524 tables in .rdata with stride-5 entries `{u32 fnRVA, u8 tag}`. Covers ~24k functions (63% of all functions, biased toward small ones).

Extract by scanning .rdata for stride-5 runs of valid function RVAs. Each table covers one module/cluster. The catalog IS the integrity check manifest — these are the functions that get checksummed.

---

## Cheat Engine Workflows

### Finding runtime globals

1. Find the current value (exact scan for floats/doubles)
2. Change it in-game
3. Re-scan with new value
4. Repeat until 1-2 addresses remain
5. Check if the address is static (.data) or heap

### Walking pointer chains

1. Add the known static global address
2. Read its value (points to heap)
3. Dump the object, look for vftable pointers
4. Match vftable against RTTI class names
5. Follow fields to reach the target

### Verifying struct offsets

1. Get a live object address
2. Dump 0x100 bytes
3. For each qword: vftable? pointer to known object? known tag pattern?
4. Build layout from observed patterns

---

## Common mistakes

- Don't xref the Luau error strings ("attempt to index nil with..."). Dead literals, real error paths are mutated. You'll waste hours.
- Don't use other builds' ExtraSpace offsets. They move every update. Scan for the displacement values in the current binary.
- Don't try to find the DataModel pointer in the DataModel itself. It's handle-table based on this build.
- Don't attach a debugger to the running client. hyperion has 5,464 rdtsc checks that detect it within one heartbeat.
- The on-disk binary is useless for code analysis. Everything in .text is encrypted.
