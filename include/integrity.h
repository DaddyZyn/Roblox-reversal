#pragma once
#include <cstdint>
#include <vector>

// These are probs incorrect havent tested

namespace checks
{
	// main integrity scan, fires every ~5s
	const uintptr_t generalIntegrity = 0x98C527;

	// sub checks scattered through the mutation code
	const uintptr_t subIntegrity[] = {
		0x961855, 0x965919, 0x96FEDF,
		0xA288E1, 0xA50C72, 0xA887AE,
		0xA8E3A2, 0xA9A8E7, 0xB04C5A,
		0x977C2A, 0x98E53F, 0xA887D6,
		0xC8F9AF, 0xC96AF3,
		0x98D89F,
	};

	// static .text comparison checks
	const uintptr_t staticIntegrity[] = {
		0x2BE263, 0x35AD78, 0x6138BC, 0x2D246D,
		0x6C677B, 0x796DEF, 0x869306, 0x8C88BE,
		0x956D14, 0x957446, 0x957532, 0x957AD2,
		0x958F79, 0x959A9C, 0x959FC3, 0x95A5E1,
		0x95ADF3, 0x95BC80, 0x95C8F5, 0x95B840,
		0x95CDA9, 0x95D239, 0x960ADD, 0x9626FE,
		0x962850, 0x9636CF, 0x9642E2, 0x965071,
		0x9E6DAC, 0xA06BA5, 0xA12FFD, 0xA28037,
		0xA52895, 0xA5506A, 0xA55185, 0xA5683A,
		0xA87BCB, 0xA9274E, 0xA9D19A, 0xAE2EE8,
		0xAE30CE, 0xAE3244, 0xAE399B, 0xAE898E,
		0xAECDE1, 0xAEDECA, 0xAFF47F, 0xB74679,
		0xBEC658, 0xC4286C, 0xC81447, 0xD872F2
	};

	const uintptr_t remapCheck = 0xCF2A46;

	// fires when a new module gets loaded
	const uintptr_t dllMainInit = 0x807E40;

	// blocks NtCreateSection with SEC_IMAGE
	const uintptr_t loadLock = 0x4000BC;

	// cert trust result: 1=ok 201=revoked
	const uintptr_t certificateCheck = 0xA0E2DD;

	// CFG return address check
	const uintptr_t controlFlowGuard = 0xF85BD0;

	// flips RWX pages to RW
	const uintptr_t whitelist = 0xA9E4A2;

	// checks console handle
	const uintptr_t consoleCheck = 0xD12B41;

	// scans process list for ce / x64dbg / scylla
	const uintptr_t processScan = 0xA927BE;

	// yara scan on loaded modules
	const uintptr_t yaraCaller = 0xDB798D;

	// dbvm detection
	const uintptr_t repemovsb = 0xF8632B;

	// page encryption for shadow execution
	const uintptr_t blockPageEncryption = 0xA29981;
}

// patch bytes for each check
namespace patches
{
	// generalIntegrity: 90 90 90 90 90 90
	// subIntegrity each: 90 90 90 90 90 90
	// staticIntegrity each: EB (jmp short)
	// certificateCheck: B0 01 (mov al, 1)
	// controlFlowGuard: FF E0 90 90 90 90 90 90
	// whitelist: 90 E9
	// consoleCheck: 38 C0 90 90 90
	// processScan: 90 90 90
	// yaraCaller: 90 90 90
	// remapCheck: 90 E9
	// dllMainInit: C3 (ret)
	// loadLock: 90 E9
	// blockPageEncryption: 90 E9
	// repemovsb: 90 90
}
