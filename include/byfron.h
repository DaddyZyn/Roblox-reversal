#pragma once
#include <cstdint>

// dll .byfron section layout
// code range: 0x10000 - 0x128A516 (19.3MB)

namespace byfron
{
	// mutation vms
	const uintptr_t vm1 = 0x226D40;         // 2.4MB integrity engine
	const uintptr_t vm2 = 0x11DA680;        // 489KB secondary
	const uintptr_t vm2_callee = 0x061DDF0; // only real fn inside vm2

	// entry
	const uintptr_t export_run = 0x2C460;

	// chacha
	const uintptr_t chacha_sigma = 0x1297450;
	const uintptr_t chacha_keys = 0x1297490;   // 128 bytes, 4x32B keys

	// syscall stubs (only 3 standalone, rest inline)
	const uintptr_t syscall_1 = 0x6894C3;
	const uintptr_t syscall_2 = 0x6BBCDC;
	const uintptr_t syscall_3 = 0x737002;

	// fn ptr tables in .rdata (hold live heap ptrs to vm entries)
	const uintptr_t vm1_fntable = 0x144A2A4;
	const uintptr_t vm2_fntable = 0x14588CC;

	// veh registration sites
	const uintptr_t veh_handlers[] = {
		0x127D3F0, 0x092CC0, 0x0C0F958,
		0x11DA220, 0x02D418, 0x1089894,
		0x11ACFE0, 0x0052DCC,
	};

	// heartbeat delay sites (mov edx, 0x3E8 = sleep 1000ms)
	const uintptr_t heartbeat_1 = 0x01F6A2;
	const uintptr_t heartbeat_2 = 0x497017;

	// integrity comparison sites (rep cmpsb)
	const uintptr_t cmpsb_sites[] = {
		0x076C2F, 0x08AC3C, 0x0BF0D0, 0x10D810,
		0x133976, 0x164E55, 0x1CDE06, 0x1D07E4,
		0x1E9D83, 0x2340A7,
	};

	// rdtsc density blocks (top)
	// 0x440000: 62, 0x3F0000: 60, 0x3D0000: 58, 0x080000: 56

	// debug register access
	// 20+ sites, dr0-dr7 read/write scattered through code

	// tls indices
	const uintptr_t tls_idx_1 = 0x441;
	const uintptr_t tls_idx_2 = 0x590;
}
