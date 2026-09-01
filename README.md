# rbx reversal

f5a60436d48947d3 (0.736.0.7361348). dumped from live memory after hyperion unpacks.

## files

- `guide.md` — how to find each offset with ida + ce
- `notes.md` — session findings
- `hyperion.md` — byfron internals
- `include/offsets.h` — all exe rvas (vm, c api, reflect, signals, net, task)
- `include/vm.h` — instruction format, opcode table, register roles
- `include/types.h` — TValue, TString, Table, Proto, Closure, CallInfo, lua_State
- `include/funcs.h` — engine function map
- `include/enc.h` — obfuscated field wrappers (VMValue0-4)
- `include/byfron.h` — dll layout, mutation vms, chacha, veh, heartbeat
- `api_map.txt` — 2252 reflected methods across 347 classes

## quick numbers

```
luaV_execute       0x26CA540
optab              0x26D65BC
hdtab              0x26D6450
luaO_nilobject     0x62F7418
luaH_dummynode     0x62F6EC8
luaD_throw         0x26ADAD0
luaD_growstack     0x269BC50
Print              0x40D2A20
ScriptContextResume 0x40CBD40
GetLuaState        0x405A360
```

opcode values shuffle every build. re-extract optab/hdtab from memory.
