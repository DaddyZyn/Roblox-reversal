#pragma once
#include <Windows.h>

// roblox obfuscates gc fields per build. each field gets one wrapper type.
// which wrapper a field uses rotates every update. re-check per build.

template<typename T>
struct VMValue0
{
private:
    T Storage;
public:
    operator const T() const { return Storage; }
    void operator=(const T& Value) { Storage = Value; }
    const T operator->() const { return Storage; }
    T Get() const { return Storage; }
    void Set(const T& Value) { Storage = Value; }
};

template<typename T>
struct VMValue1
{
private:
    T Storage;
public:
    operator const T() const { return (T)((uintptr_t)Storage - (uintptr_t)this); }
    void operator=(const T& Value) { Storage = (T)((uintptr_t)Value + (uintptr_t)this); }
    const T operator->() const { return operator const T(); }
    T Get() const { return operator const T(); }
    void Set(const T& Value) { operator=(Value); }
};

template<typename T>
struct VMValue2
{
private:
    T Storage;
public:
    operator const T() const { return (T)((uintptr_t)this - (uintptr_t)Storage); }
    void operator=(const T& Value) { Storage = (T)((uintptr_t)this - (uintptr_t)Value); }
    const T operator->() const { return operator const T(); }
    T Get() const { return operator const T(); }
    void Set(const T& Value) { operator=(Value); }
};

template<typename T>
struct VMValue3
{
private:
    T Storage;
public:
    operator const T() const { return (T)((uintptr_t)this ^ (uintptr_t)Storage); }
    void operator=(const T& Value) { Storage = (T)((uintptr_t)Value ^ (uintptr_t)this); }
    const T operator->() const { return operator const T(); }
    T Get() const { return operator const T(); }
    void Set(const T& Value) { Storage = Value; }
};

template<typename T>
struct VMValue4
{
private:
    T Storage;
public:
    operator const T() const { return (T)((uintptr_t)this + (uintptr_t)Storage); }
    void operator=(const T& Value) { Storage = (T)((uintptr_t)Value - (uintptr_t)this); }
    const T operator->() const { return operator const T(); }
    T Get() const { return operator const T(); }
    void Set(const T& Value) { operator=(Value); }
};

// field enc mappings
#define CLOSURE_CONT_ENC VMValue4
#define CLOSURE_DEBUGNAME_DEPRECATED_ENC VMValue2
#define LSTATE_STACKSIZE_ENC VMValue2
#define PROTO_ABSLINEINFO_ENC VMValue1
#define PROTO_DEBUGINSN_ENC VMValue1
#define PROTO_DEBUGNAME_ENC VMValue4
#define PROTO_LINEINFO_ENC VMValue3
#define PROTO_LOCVARS_ENC VMValue3
#define PROTO_SOURCE_ENC VMValue1
#define PROTO_TYPEINFO_ENC VMValue2
#define PROTO_UPVALUES_ENC VMValue4
#define PROTO_USERDATA_ENC VMValue2
#define PROTO_K_ENC VMValue1
#define PROTO_CODE_ENC VMValue1
#define PROTO_P_ENC VMValue1
#define TSTRING_HASH_ENC VMValue4
#define UDATA_META_ENC VMValue4
