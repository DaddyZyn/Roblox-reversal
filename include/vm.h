#pragma once
#include <cstdint>

// luau vm decode for f5a60436d48947d3.
// the interp (luaV_execute) sits at rva 0x26CA540, 0xBF0D bytes.

union rbx_instr
{
    uint32_t u32;
    struct
    {
        uint8_t op;
        uint8_t a;
        uint8_t c;
        uint8_t b;
    };
    struct
    {
        uint16_t lo;
        int16_t sbx;
    };
};

// vm register roles inside the interp:
//   r15 = pc, r12 = base (stack of TValue), [rsp+0x48] = constants
//   xmm10/11/12 = 0.5 / 3.0 / 2.0 preloaded for the arith fastpaths
// dispatch: op -> optab[op] -> hdtab[idx] -> jmp rcx
// unassigned ops go to the illegal handler (idx 90)

enum rbx_dispatch_idx
{
    // stable dispatch slots we identified (idx = position in hdtab)
    IDX_POWK = 0,
    IDX_CONCATK = 7,
    IDX_GETTABLEKS = 6,
    IDX_CALL = 24,
    IDX_NEWTABLE = 9,
    IDX_SUB = 12,
    IDX_NOT = 14,
    IDX_LOADNIL = 19,
    IDX_CALL_PREP = 13,
    IDX_JUMPIFLT_K = 27,
    IDX_SUBK = 28,
    IDX_JUMPIFEQK = 29,
    IDX_JUMPIFEQ = 30,
    IDX_NEWTABLE2 = 32,
    IDX_DIV = 33,
    IDX_FORNPREP = 51,
    IDX_DIVK = 52,
    IDX_JUMPIFEQK_NUM = 56,
    IDX_JMPX = 64,
    IDX_LOADB = 67,
    IDX_JUMPIFLT = 46,
    IDX_JUMPIFLE = 72,
    IDX_MOVE = 76,
    IDX_POW = 79,
    IDX_JUMPIFEQ2 = 80,
    IDX_MODK = 81,
    IDX_ADD = 85,
    IDX_ADDK = 86,
    IDX_NAMECALL = 87,
    IDX_JMP = 89,
    IDX_ILLEGAL = 90,
};

// full table for this build: idx, bytecode op value, name
// conf: h = fingerprinted, m = decompile read, l = family guess
struct rbx_op_entry { int idx; int op; const char* name; char conf; };

static const rbx_op_entry rbx_op_map[] = {
    {  0,   0, "POWK",              'h' },
    {  1,   2, "LOADN",             'm' },
    {  2,   6, "arith_slow_a",      'l' },
    {  3,   7, "CONCATK",           'h' },
    {  4,   9, "CONCAT",            'h' },
    {  5,  15, "SETTABLE",          'm' },
    {  6,  21, "GETTABLEKS",        'm' },
    {  7,  24, "CALL",              'h' },
    {  8,  27, "SETUPVAL",          'm' },
    {  9,  32, "NEWTABLE",          'h' },
    { 10,  36, "GETTABLEKS_V",      'm' },
    { 11,  37, "MISC_I",            'm' },
    { 12,  42, "SUB",               'h' },
    { 13,  45, "CALL_PREP",         'm' },
    { 14,  47, "NOT",               'h' },
    { 15,  49, "SETTABLE",          'm' },
    { 16,  50, "MOVE",              'h' },
    { 17,  52, "JUMPIF_fam",        'm' },
    { 18,  53, "JUMPIF_fam",        'm' },
    { 19,  54, "LOADNIL",           'h' },
    { 20,  55, "JUMPIFLT_LE_pair",  'm' },
    { 21,  58, "FORN_fam",          'm' },
    { 22,  62, "SETLIST",           'm' },
    { 23,  70, "GETTABLE_big",      'm' },
    { 24,  71, "CALL_RET",          'm' },
    { 25,  73, "CALL_v",            'h' },
    { 26,  75, "JUMPIFNOT",         'm' },
    { 27,  79, "JUMPIFLT_K",        'm' },
    { 28,  82, "SUBK",              'h' },
    { 29,  83, "JUMPIFEQK",         'm' },
    { 30,  84, "JUMPIFEQ",          'm' },
    { 31,  88, "MISC_E",            'm' },
    { 32,  89, "NEWTABLE_v",        'h' },
    { 33,  90, "DIV",               'h' },
    { 34,  92, "GETIMPORT",         'm' },
    { 35,  93, "GETTABLE",          'm' },
    { 36, 102, "FORN_fam",          'm' },
    { 37, 107, "JUMPIF_fam",        'm' },
    { 38, 109, "JUMPIFLE",          'm' },
    { 39, 117, "SETTABLE",          'm' },
    { 40, 122, "error_tail",        'l' },
    { 41, 123, "SETLIST",           'm' },
    { 42, 126, "GETIMPORT",         'm' },
    { 43, 129, "SETUPVAL",          'm' },
    { 44, 131, "SUBK2",             'm' },
    { 45,  45, "MOD",               'h' },
    { 46, 137, "JUMPIFLT",          'm' },
    { 47, 138, "GETTABLE",          'm' },
    { 48, 139, "SETTABLE",          'm' },
    { 49, 143, "MISC_G",            'm' },
    { 50, 144, "GETTABLE",          'm' },
    { 51, 147, "FORNPREP",          'h' },
    { 52, 148, "DIVK",              'm' },
    { 53, 149, "RETURN_tail",       'm' },
    { 54, 150, "GETIMPORT",         'm' },
    { 55, 152, "JUMPIF_fam",        'm' },
    { 56, 156, "JUMPIFEQK_num",     'm' },
    { 57, 157, "TAG16_op",          'l' },
    { 58, 159, "shared_tail",       'l' },
    { 59, 160, "SETUPVAL",          'm' },
    { 60, 161, "CALL_RET",          'm' },
    { 61, 163, "shared_tail",       'l' },
    { 62, 166, "SETLIST",           'm' },
    { 63, 171, "SETUPVAL",          'm' },
    { 64, 174, "JMPX",              'h' },
    { 65, 178, "GETIMPORT",         'm' },
    { 66, 183, "SETLIST",           'm' },
    { 67, 184, "LOADB",             'h' },
    { 68, 196, "SETTABLE",          'm' },
    { 69, 198, "RETURN_tail",       'm' },
    { 70, 202, "MUL",               'h' },
    { 71, 204, "arith_tail",        'l' },
    { 72, 205, "JUMPIFLE",          'm' },
    { 73, 208, "FORN_fam",          'm' },
    { 74, 209, "FORN_fam",          'm' },
    { 75, 210, "ARITH7K",           'm' },
    { 76, 212, "MOVE_V",            'm' },
    { 77, 217, "JUMPIF_fam",        'm' },
    { 78, 218, "SETUPVAL",          'm' },
    { 79, 223, "POW",               'h' },
    { 80, 224, "JUMPIFEQ2",         'm' },
    { 81, 228, "MODK",              'm' },
    { 82, 231, "arith_slow_b",      'l' },
    { 83, 235, "MISC_K",            'm' },
    { 84, 237, "RETURN_tail",       'm' },
    { 85, 240, "ADD",               'h' },
    { 86, 241, "ADDK",              'h' },
    { 87, 243, "NAMECALL",          'h' },
    { 88, 248, "CALL_RET",          'm' },
    { 89, 251, "JMP",               'h' },
};
