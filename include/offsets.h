#pragma once
#include <cstdint>
#include <Windows.h>

#define REBASE(addr)(addr+reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)))

namespace fn
{
	const uintptr_t Print = REBASE(0x1C68FE0);
	const uintptr_t GetLuaState = REBASE(0x405A360);
	const uintptr_t LuauExecute = REBASE(0x26CA540);
	const uintptr_t LuaDThrow = REBASE(0x26ADAD0);
	const uintptr_t LuaDGrowStack = REBASE(0x269BC50);
	const uintptr_t LuaDCallNoYield = REBASE(0x26B1940);
	const uintptr_t Optable = REBASE(0x26D65BC);
	const uintptr_t HandlerTable = REBASE(0x26D6450);
	const uintptr_t IllegalHandler = REBASE(0x26D5F30);
	const uintptr_t SCResume = REBASE(0x40CBD40);
	const uintptr_t GetVMState = REBASE(0x479CE50);
	const uintptr_t GetModuleFromVMStateMap = REBASE(0x40D9DA0);
	const uintptr_t GetCurrentThreadId = REBASE(0x4781660);
	const uintptr_t Index2Adr = REBASE(0x3F96B90);
	const uintptr_t Vector3Eq = REBASE(0x85F060);
	const uintptr_t luaC_step = REBASE(0xEB4D90);
	const uintptr_t WriteLock = REBASE(0x5979AF0);
	const uintptr_t LoadStringStrict = REBASE(0x40D1DA0);
	const uintptr_t RbxSpawn = REBASE(0x40D3520);
	const uintptr_t EnableLoadModule = REBASE(0x40D0CB0);
}

namespace arith
{
	const uintptr_t Add = REBASE(0x26B86C0);
	const uintptr_t Sub = REBASE(0x26B88A0);
	const uintptr_t Mul = REBASE(0x26B8A80);
	const uintptr_t Div = REBASE(0x26B8D00);
	const uintptr_t Mod = REBASE(0x26B8F80);
	const uintptr_t Pow = REBASE(0x26B9280);
	const uintptr_t GenericPow = REBASE(0x59AE460);
}

namespace globals
{
	const uintptr_t NilObject = REBASE(0x62F7418);
	const uintptr_t DummyNode = REBASE(0x62F6EC8);
	const uintptr_t Eventnames = REBASE(0x6313748);
	const uintptr_t Typenames = REBASE(0x63136C0);
}

namespace reflect
{
	const uintptr_t GetProperty = REBASE(0x1CA2B00);
	const uintptr_t GetPropertyData = REBASE(0x2AAD770);
	const uintptr_t KTable = REBASE(0x7B92F70);
	const uintptr_t CastArgs = REBASE(0x3F93510);
	const uintptr_t GetValues = REBASE(0x3F93BF0);
	const uintptr_t PropFail = REBASE(0x3FE1EA0);
	const uintptr_t UnableToCast = REBASE(0x852420);
	const uintptr_t Registry = REBASE(0x6949C00);
}

namespace identity
{
	const uintptr_t GetCapabilities = REBASE(0x1CA46D0);
	const uintptr_t GetIdentityStruct = REBASE(0x7AFE900);
	const uintptr_t GetIdentity = REBASE(0x40D2A40);
	const uintptr_t IdentityPtr = REBASE(0x6E8FC08);
	const uintptr_t CapabilitiesOffset = 0x28;
}

namespace signals
{
	const uintptr_t FirePlayerArg = REBASE(0xB78B10);
	const uintptr_t FireRemoteInt = REBASE(0xB79080);
	const uintptr_t FireVoid = REBASE(0x8A5B70);
	const uintptr_t FireInstanceArg = REBASE(0xCE8B60);
	const uintptr_t FireInputType = REBASE(0xD2B450);
	const uintptr_t Disconnect = REBASE(0x40B3BA0);
}

namespace net
{
	const uintptr_t FireServer = REBASE(0x33EF8B0);
	const uintptr_t InvokeServer = REBASE(0x3451FB0);
	const uintptr_t FireAllClients = REBASE(0x33EFC30);
	const uintptr_t InvokeClient = REBASE(0x34522D0);
	const uintptr_t ResumeWaitingScripts = REBASE(0x40F3FD0);
	const uintptr_t EnableLoadModule = REBASE(0x40D0CB0);
	const uintptr_t ValidateUrl = REBASE(0x3AD9F80);
	const uintptr_t ProcessPacket = REBASE(0x288AD80);
	const uintptr_t ReportError = REBASE(0x45866D0);
}

namespace capi
{
	const uintptr_t pushnil = REBASE(0x84D310);
	const uintptr_t pushnumber = REBASE(0x84FC20);
	const uintptr_t createtable = REBASE(0x855950);
	const uintptr_t pushcclosure = REBASE(0x865E10);
	const uintptr_t pushlstring = REBASE(0x865E10);
	const uintptr_t pushvalue = REBASE(0x87B680);
	const uintptr_t settop = REBASE(0x9D3BA0);
	const uintptr_t setfield = REBASE(0xB50740);
	const uintptr_t getfield = REBASE(0xB50800);
	const uintptr_t getfieldmeta = REBASE(0xB50A80);
	const uintptr_t concat = REBASE(0xB50B40);
	const uintptr_t Vsettable = REBASE(0xBD5950);
	const uintptr_t pcall = REBASE(0x1614F80);
}

namespace luau_x
{
	const uintptr_t LuaOpenBase = REBASE(0x40B3BA0);
	const uintptr_t Impersonator = REBASE(0x55F03A0);
	const uintptr_t LuaBAssert = REBASE(0x26EBB40);
	const uintptr_t SetErrorObj = REBASE(0x269FF30);
	const uintptr_t HashLookup = REBASE(0x10AAC10);
	const uintptr_t IndexError2 = REBASE(0x26DFDA0);
	const uintptr_t OpenLibsCore = REBASE(0x26EDD90);
}

namespace task
{
	const uintptr_t Delay = REBASE(0x4187180);
	const uintptr_t Desync = REBASE(0x4185DB0);
	const uintptr_t Spawn = REBASE(0x4186E10);
	const uintptr_t Sync = REBASE(0x41859A0);
	const uintptr_t Wait = REBASE(0x4187480);
}

namespace lock
{
	const uintptr_t WriteLock = REBASE(0x5979AF0);
}

namespace physics
{
	const uintptr_t SenderMaxBandwidthBps = REBASE(0x7B9FB88);
}
