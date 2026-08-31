#pragma once
#include <cstdint>
#include "enc.h"

// gc header on everything
struct GCHeader
{
    void* next;
    uint8_t tt;
    uint8_t marked;
    uint16_t _pad;
};

// tt values
enum : int
{
    TNIL = 0,
    TBOOLEAN = 1,
    TLIGHTUSERDATA = 2,
    TNUMBER = 3,
    TVECTOR = 4,
    TSTRING = 5,
    TTABLE = 6,
    TFUNCTION = 7,
    TUSERDATA = 8,
    TTHREAD = 9,
    TBUFFER = 10,
};

// 16 bytes, tag at +0x0C
struct TValue
{
    union
    {
        double n;
        void* gc;
        int32_t b;
        float v[3];
        uint64_t raw;
    } value;
    uint32_t pad;
    int32_t tt;
};

struct TString : GCHeader
{
    TSTRING_HASH_ENC uint32_t hash;
    uint32_t len;
    // char data
};

struct UpVal : GCHeader
{
    uint32_t flag;
    union
    {
        TValue closed;
        TValue* open;
    };
};

struct Table : GCHeader
{
    uint32_t flags;
    uint8_t lsizenode;
    uint8_t _pad[3];
    // +0x14 array part
    // +0x1C node array
    // +0x28 metatable related
    // +0x34 element count
};

struct Proto : GCHeader
{
    uint8_t numparams;
    uint8_t flags;
    uint8_t _pad[2];

    PROTO_K_ENC void* k;
    PROTO_CODE_ENC void* code;
    PROTO_P_ENC void** protos;
    PROTO_LINEINFO_ENC int* lineinfo;
    PROTO_ABSLINEINFO_ENC void* abslineinfo;
    PROTO_SOURCE_ENC TString* source;
    PROTO_DEBUGNAME_ENC TString* debugname;
    PROTO_LOCVARS_ENC void* locvars;
    PROTO_UPVALUES_ENC void* upvalues;
    PROTO_DEBUGINSN_ENC void* debuginsn;
    PROTO_TYPEINFO_ENC void* typeinfo;
    PROTO_USERDATA_ENC void* userdata;
};

struct Closure : GCHeader
{
    uint8_t stacksize;
    uint8_t nupvalues;
    uint8_t flags;

    CLOSURE_CONT_ENC void* cont;
    CLOSURE_DEBUGNAME_DEPRECATED_ENC TString* debugname;
};

struct UserData : GCHeader
{
    UDATA_META_ENC void* metatable;
    // usertype data follows
};

struct CallInfo
{
    TValue* func;
    TValue* top;
    TValue* base;
    void* savedpc;
    uint32_t flags;
    int32_t nresults;
    // far slots for CALL/RETURN at +1280/+1320
};

struct lua_State : GCHeader
{
    uint8_t status;
    uint8_t allowthread;
    uint8_t activememcat;
    uint8_t busy;
    // +0x18 top (hot)
    // +0x20 global
    // +0x28 ci (hot)
    // +0x30 stack_last / stack
    // +0x40 base
    LSTATE_STACKSIZE_ENC uintptr_t stacksize;
};
