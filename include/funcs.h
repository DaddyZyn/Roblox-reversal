#pragma once
#include <cstdint>

// complete luau/engine function map for f5a60436d48947d3
// call shape, (g)uess. all rva.

namespace luau
{
    // ---- vm core
    constexpr uintptr_t luaV_execute_rva   = 0x26CA540;   // (c) dispatch signature
    constexpr uintptr_t vm_optab_rva       = 0x26D65BC;   // (c) op -> idx
    constexpr uintptr_t vm_hdtab_rva       = 0x26D6450;   // (c) idx -> handler
    constexpr uintptr_t vm_illegal_rva     = 0x026D5F30;  // (c) idx 90

    // ---- lvm: arith slow paths (simd fingerprinted, (c))
    constexpr uintptr_t luaV_add           = 0x026B86C0;
    constexpr uintptr_t luaV_sub           = 0x026B88A0;
    constexpr uintptr_t luaV_mul           = 0x026B8A80;
    constexpr uintptr_t luaV_div           = 0x026B8D00;
    constexpr uintptr_t luaV_mod           = 0x026B8F80;
    constexpr uintptr_t luaV_pow           = 0x026B9280;
    constexpr uintptr_t luaV_arith7        = 0x026B9430;  // 7th arith variant
    constexpr uintptr_t crt_pow            = 0x059AE460;  // msvc pow
    constexpr uintptr_t pow_negpath        = 0x05C1A2C0;  // POWK neg-base branch

    // ---- lvm: table / compare / misc handlers (l)
    constexpr uintptr_t vm_tableget_a      = 0x026B36E0;
    constexpr uintptr_t vm_tableget_b      = 0x026B4570;
    constexpr uintptr_t vm_tableset        = 0x026BCCC0;
    constexpr uintptr_t vm_concat          = 0x026E0520;
    constexpr uintptr_t vm_setlist         = 0x026BCE30;
    constexpr uintptr_t vm_cmpeq           = 0x0597DBF0;
    constexpr uintptr_t vm_cmple           = 0x026E56F0;
    constexpr uintptr_t vm_cmplt           = 0x026E57A0;
    constexpr uintptr_t vm_vector3_eq      = 0x0085F060;  // vector keys

    // ---- ldo: call / throw / grow (c) via resume + error path analysis
    constexpr uintptr_t luaD_growstack     = 0x0269BC50;  // (c) called on stack bound fail
    constexpr uintptr_t luaD_throw         = 0x026ADAD0;  // (c) handler error paths
    // luaD_call family - all call luaV_execute. exact upstream names tbd:
    constexpr uintptr_t luaD_call_a        = 0x0269D730;
    constexpr uintptr_t luaD_call_b        = 0x026ADBB0;
    constexpr uintptr_t luaD_call_c        = 0x026AF410;
    constexpr uintptr_t luaD_call_d        = 0x026AFFD0;
    constexpr uintptr_t luaD_callnoyield   = 0x026B1940;  // (l) hottest, api entry
    constexpr uintptr_t luaD_call_f        = 0x026B2B00;
    constexpr uintptr_t vm_callmach        = 0x026D8D10;  // CALL handler fast path
    constexpr uintptr_t callmach2          = 0x026E58B0;
    constexpr uintptr_t retmach            = 0x026E5940;
    constexpr uintptr_t callmach4          = 0x0597D560;

    // ---- lvm: for loop helpers (l)
    constexpr uintptr_t for_prep           = 0x026F1F70;
    constexpr uintptr_t for_step           = 0x026BDA40;
    constexpr uintptr_t for_loop           = 0x026B8590;

    // ---- lgc: write barriers (l) called on every table/upval store
    constexpr uintptr_t luaC_barrier_a     = 0x026B1890;
    constexpr uintptr_t luaC_barrier_b     = 0x026BC760;

    constexpr uintptr_t luaA_index2adr     = 0x03F96B90;

    // ---- lstr: metamethod name builders (l) build "__index" etc
    constexpr uintptr_t luaS_mm_init_a     = 0x0007419C0;
    constexpr uintptr_t luaS_mm_init_b     = 0x000741D70;

    // ---- scheduler / scriptcontext bridge (l) one of these is resume
    constexpr uintptr_t sc_resume_1        = 0x03F903B0;
    constexpr uintptr_t sc_resume_2        = 0x03F929C0;
    constexpr uintptr_t sc_resume_3        = 0x03F93510;
    constexpr uintptr_t sc_resume_4        = 0x03F96B90;  // strong (runs scripts)
    constexpr uintptr_t sc_resume_5        = 0x03F97570;
    constexpr uintptr_t sc_resume_6        = 0x03F98160;
    constexpr uintptr_t sc_resume_7        = 0x03F985E0;  // strong (runs scripts)
    constexpr uintptr_t sc_resume_8        = 0x04028AA0;

    // ---- stdlib print: roblox impl + vm wrapper (l)
    constexpr uintptr_t luaB_print_impl    = 0x040D2A20;
    constexpr uintptr_t luaB_print_vmwrap  = 0x026E63C0;

    // ---- reflect
    constexpr uintptr_t refl_registry      = 0x006949C00;
    constexpr uintptr_t refl_propfail      = 0x003FE1EA0;
    constexpr uintptr_t refl_unable_cast   = 0x00852420;

    // ---- class vftables (exact rtti match)
    constexpr uintptr_t vft_scriptcontext  = 0x006BA2788;
    constexpr uintptr_t vft_datamodel      = 0x006B9E2F8;
    constexpr uintptr_t vft_instance       = 0x006ABD980;
    constexpr uintptr_t vft_clickdetector  = 0x006B4BDC0;
    constexpr uintptr_t vft_proximityprompt= 0x006B06298;
    constexpr uintptr_t vft_touchtransmitter = 0x006B4C110;

    // ---- signal emitters (EventDesc<void(Sig)> vtable slot 1) (c)
    constexpr uintptr_t sig_fire_player    = 0x00B78B10;  // mouseclick/triggered
    constexpr uintptr_t sig_fire_remoteint = 0x00B79080;
    constexpr uintptr_t sig_fire_void      = 0x008A5B70;  // triggered/ended
    constexpr uintptr_t sig_fire_instance  = 0x00CE8B60;  // prompt shown
    constexpr uintptr_t sig_fire_inputtype = 0x00D2B450;

    // ---- instance layout (obs)
    constexpr uintptr_t instance_parent_off = 0x68;       // services hold DM here
    constexpr uintptr_t dm_scriptcontext_off = -1;        // not recovered (deep/encoded)

    // ---- asset crypto anchors
    constexpr uintptr_t asset_seed_sigma   = 0x006F248B0; // "expand 32-byte k"
}
