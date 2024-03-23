/*
 * Copyright (C) 2021-2024 Colin Ian King
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "stress-ng.h"

static const stress_help_t help[] = {
	{ NULL,	"goto N",		"start N workers that exercise heavy branching" },
	{ NULL, "goto-direction D",	"select goto direction forward, backward, random" },
	{ NULL,	"goto-ops N",		"stop after 1024 x N goto bogo operations" },
	{ NULL,	NULL,			NULL }
};

#define STRESS_GOTO_FORWARD	(1)
#define STRESS_GOTO_BACKWARD	(2)
#define STRESS_GOTO_RANDOM	(3)

typedef struct {
	const char *option;
	const int  direction;
} stress_goto_direction_t;

static int stress_set_goto_direction(const char *opts)
{
	size_t i;

	static const stress_goto_direction_t stress_goto_direction[] = {
		{ "forward",	STRESS_GOTO_FORWARD },
		{ "backward",	STRESS_GOTO_BACKWARD },
		{ "random",	STRESS_GOTO_RANDOM },
	};

	for (i = 0; i < SIZEOF_ARRAY(stress_goto_direction); i++) {
		if (!strcmp(opts, stress_goto_direction[i].option)) {
			return stress_set_setting("goto-direction", TYPE_ID_INT, &stress_goto_direction[i].direction);
		}
	}
	(void)fprintf(stderr, "goto-option option '%s' not known, options are:", opts);
	for (i = 0; i < SIZEOF_ARRAY(stress_goto_direction); i++)
		(void)fprintf(stderr, "%s %s", i == 0 ? "" : ",", stress_goto_direction[i].option);
	(void)fprintf(stderr, "\n");

	return -1;
}

static const stress_opt_set_func_t opt_set_funcs[] = {
	{ OPT_goto_direction,	stress_set_goto_direction },
	{ 0,			NULL },
};

#define MAX_LABELS	(0x400)

#if defined(HAVE_LABEL_AS_VALUE) &&	\
    !defined(HAVE_COMPILER_PCC)

#define G(n) L ## n:			\
{					\
	if ((n & 0x3f) == 0)		\
		counters[n >> 6]++;	\
	goto *labels[n];		\
}

/*
 *  Intel icx can take hours optimizating the code,
 *  so workaround this by defaulting it to -O0 until
 *  this is resolved
 */
#if defined(HAVE_COMPILER_ICX)
#define OPTIMIZE_GOTO	OPTIMIZE0
#else
#define OPTIMIZE_GOTO	OPTIMIZE3
#endif

/*
 *  stress_goto()
 *	stress instruction goto prediction
 */
static int OPTIMIZE_GOTO stress_goto(stress_args_t *args)
{
	size_t i;
	int rc = EXIT_SUCCESS, goto_direction;
	double t1, t2, duration, rate;
	uint64_t lo, hi, bogo_counter;

	static const void ALIGN64 *default_labels[MAX_LABELS] = {
		&&L0x000, &&L0x001, &&L0x002, &&L0x003, &&L0x004, &&L0x005, &&L0x006, &&L0x007,
		&&L0x008, &&L0x009, &&L0x00a, &&L0x00b, &&L0x00c, &&L0x00d, &&L0x00e, &&L0x00f,
		&&L0x010, &&L0x011, &&L0x012, &&L0x013, &&L0x014, &&L0x015, &&L0x016, &&L0x017,
		&&L0x018, &&L0x019, &&L0x01a, &&L0x01b, &&L0x01c, &&L0x01d, &&L0x01e, &&L0x01f,
		&&L0x020, &&L0x021, &&L0x022, &&L0x023, &&L0x024, &&L0x025, &&L0x026, &&L0x027,
		&&L0x028, &&L0x029, &&L0x02a, &&L0x02b, &&L0x02c, &&L0x02d, &&L0x02e, &&L0x02f,
		&&L0x030, &&L0x031, &&L0x032, &&L0x033, &&L0x034, &&L0x035, &&L0x036, &&L0x037,
		&&L0x038, &&L0x039, &&L0x03a, &&L0x03b, &&L0x03c, &&L0x03d, &&L0x03e, &&L0x03f,

		&&L0x040, &&L0x041, &&L0x042, &&L0x043, &&L0x044, &&L0x045, &&L0x046, &&L0x047,
		&&L0x048, &&L0x049, &&L0x04a, &&L0x04b, &&L0x04c, &&L0x04d, &&L0x04e, &&L0x04f,
		&&L0x050, &&L0x051, &&L0x052, &&L0x053, &&L0x054, &&L0x055, &&L0x056, &&L0x057,
		&&L0x058, &&L0x059, &&L0x05a, &&L0x05b, &&L0x05c, &&L0x05d, &&L0x05e, &&L0x05f,
		&&L0x060, &&L0x061, &&L0x062, &&L0x063, &&L0x064, &&L0x065, &&L0x066, &&L0x067,
		&&L0x068, &&L0x069, &&L0x06a, &&L0x06b, &&L0x06c, &&L0x06d, &&L0x06e, &&L0x06f,
		&&L0x070, &&L0x071, &&L0x072, &&L0x073, &&L0x074, &&L0x075, &&L0x076, &&L0x077,
		&&L0x078, &&L0x079, &&L0x07a, &&L0x07b, &&L0x07c, &&L0x07d, &&L0x07e, &&L0x07f,

		&&L0x080, &&L0x081, &&L0x082, &&L0x083, &&L0x084, &&L0x085, &&L0x086, &&L0x087,
		&&L0x088, &&L0x089, &&L0x08a, &&L0x08b, &&L0x08c, &&L0x08d, &&L0x08e, &&L0x08f,
		&&L0x090, &&L0x091, &&L0x092, &&L0x093, &&L0x094, &&L0x095, &&L0x096, &&L0x097,
		&&L0x098, &&L0x099, &&L0x09a, &&L0x09b, &&L0x09c, &&L0x09d, &&L0x09e, &&L0x09f,
		&&L0x0a0, &&L0x0a1, &&L0x0a2, &&L0x0a3, &&L0x0a4, &&L0x0a5, &&L0x0a6, &&L0x0a7,
		&&L0x0a8, &&L0x0a9, &&L0x0aa, &&L0x0ab, &&L0x0ac, &&L0x0ad, &&L0x0ae, &&L0x0af,
		&&L0x0b0, &&L0x0b1, &&L0x0b2, &&L0x0b3, &&L0x0b4, &&L0x0b5, &&L0x0b6, &&L0x0b7,
		&&L0x0b8, &&L0x0b9, &&L0x0ba, &&L0x0bb, &&L0x0bc, &&L0x0bd, &&L0x0be, &&L0x0bf,

		&&L0x0c0, &&L0x0c1, &&L0x0c2, &&L0x0c3, &&L0x0c4, &&L0x0c5, &&L0x0c6, &&L0x0c7,
		&&L0x0c8, &&L0x0c9, &&L0x0ca, &&L0x0cb, &&L0x0cc, &&L0x0cd, &&L0x0ce, &&L0x0cf,
		&&L0x0d0, &&L0x0d1, &&L0x0d2, &&L0x0d3, &&L0x0d4, &&L0x0d5, &&L0x0d6, &&L0x0d7,
		&&L0x0d8, &&L0x0d9, &&L0x0da, &&L0x0db, &&L0x0dc, &&L0x0dd, &&L0x0de, &&L0x0df,
		&&L0x0e0, &&L0x0e1, &&L0x0e2, &&L0x0e3, &&L0x0e4, &&L0x0e5, &&L0x0e6, &&L0x0e7,
		&&L0x0e8, &&L0x0e9, &&L0x0ea, &&L0x0eb, &&L0x0ec, &&L0x0ed, &&L0x0ee, &&L0x0ef,
		&&L0x0f0, &&L0x0f1, &&L0x0f2, &&L0x0f3, &&L0x0f4, &&L0x0f5, &&L0x0f6, &&L0x0f7,
		&&L0x0f8, &&L0x0f9, &&L0x0fa, &&L0x0fb, &&L0x0fc, &&L0x0fd, &&L0x0fe, &&L0x0ff,

		&&L0x100, &&L0x101, &&L0x102, &&L0x103, &&L0x104, &&L0x105, &&L0x106, &&L0x107,
		&&L0x108, &&L0x109, &&L0x10a, &&L0x10b, &&L0x10c, &&L0x10d, &&L0x10e, &&L0x10f,
		&&L0x110, &&L0x111, &&L0x112, &&L0x113, &&L0x114, &&L0x115, &&L0x116, &&L0x117,
		&&L0x118, &&L0x119, &&L0x11a, &&L0x11b, &&L0x11c, &&L0x11d, &&L0x11e, &&L0x11f,
		&&L0x120, &&L0x121, &&L0x122, &&L0x123, &&L0x124, &&L0x125, &&L0x126, &&L0x127,
		&&L0x128, &&L0x129, &&L0x12a, &&L0x12b, &&L0x12c, &&L0x12d, &&L0x12e, &&L0x12f,
		&&L0x130, &&L0x131, &&L0x132, &&L0x133, &&L0x134, &&L0x135, &&L0x136, &&L0x137,
		&&L0x138, &&L0x139, &&L0x13a, &&L0x13b, &&L0x13c, &&L0x13d, &&L0x13e, &&L0x13f,

		&&L0x140, &&L0x141, &&L0x142, &&L0x143, &&L0x144, &&L0x145, &&L0x146, &&L0x147,
		&&L0x148, &&L0x149, &&L0x14a, &&L0x14b, &&L0x14c, &&L0x14d, &&L0x14e, &&L0x14f,
		&&L0x150, &&L0x151, &&L0x152, &&L0x153, &&L0x154, &&L0x155, &&L0x156, &&L0x157,
		&&L0x158, &&L0x159, &&L0x15a, &&L0x15b, &&L0x15c, &&L0x15d, &&L0x15e, &&L0x15f,
		&&L0x160, &&L0x161, &&L0x162, &&L0x163, &&L0x164, &&L0x165, &&L0x166, &&L0x167,
		&&L0x168, &&L0x169, &&L0x16a, &&L0x16b, &&L0x16c, &&L0x16d, &&L0x16e, &&L0x16f,
		&&L0x170, &&L0x171, &&L0x172, &&L0x173, &&L0x174, &&L0x175, &&L0x176, &&L0x177,
		&&L0x178, &&L0x179, &&L0x17a, &&L0x17b, &&L0x17c, &&L0x17d, &&L0x17e, &&L0x17f,

		&&L0x180, &&L0x181, &&L0x182, &&L0x183, &&L0x184, &&L0x185, &&L0x186, &&L0x187,
		&&L0x188, &&L0x189, &&L0x18a, &&L0x18b, &&L0x18c, &&L0x18d, &&L0x18e, &&L0x18f,
		&&L0x190, &&L0x191, &&L0x192, &&L0x193, &&L0x194, &&L0x195, &&L0x196, &&L0x197,
		&&L0x198, &&L0x199, &&L0x19a, &&L0x19b, &&L0x19c, &&L0x19d, &&L0x19e, &&L0x19f,
		&&L0x1a0, &&L0x1a1, &&L0x1a2, &&L0x1a3, &&L0x1a4, &&L0x1a5, &&L0x1a6, &&L0x1a7,
		&&L0x1a8, &&L0x1a9, &&L0x1aa, &&L0x1ab, &&L0x1ac, &&L0x1ad, &&L0x1ae, &&L0x1af,
		&&L0x1b0, &&L0x1b1, &&L0x1b2, &&L0x1b3, &&L0x1b4, &&L0x1b5, &&L0x1b6, &&L0x1b7,
		&&L0x1b8, &&L0x1b9, &&L0x1ba, &&L0x1bb, &&L0x1bc, &&L0x1bd, &&L0x1be, &&L0x1bf,

		&&L0x1c0, &&L0x1c1, &&L0x1c2, &&L0x1c3, &&L0x1c4, &&L0x1c5, &&L0x1c6, &&L0x1c7,
		&&L0x1c8, &&L0x1c9, &&L0x1ca, &&L0x1cb, &&L0x1cc, &&L0x1cd, &&L0x1ce, &&L0x1cf,
		&&L0x1d0, &&L0x1d1, &&L0x1d2, &&L0x1d3, &&L0x1d4, &&L0x1d5, &&L0x1d6, &&L0x1d7,
		&&L0x1d8, &&L0x1d9, &&L0x1da, &&L0x1db, &&L0x1dc, &&L0x1dd, &&L0x1de, &&L0x1df,
		&&L0x1e0, &&L0x1e1, &&L0x1e2, &&L0x1e3, &&L0x1e4, &&L0x1e5, &&L0x1e6, &&L0x1e7,
		&&L0x1e8, &&L0x1e9, &&L0x1ea, &&L0x1eb, &&L0x1ec, &&L0x1ed, &&L0x1ee, &&L0x1ef,
		&&L0x1f0, &&L0x1f1, &&L0x1f2, &&L0x1f3, &&L0x1f4, &&L0x1f5, &&L0x1f6, &&L0x1f7,
		&&L0x1f8, &&L0x1f9, &&L0x1fa, &&L0x1fb, &&L0x1fc, &&L0x1fd, &&L0x1fe, &&L0x1ff,

		&&L0x200, &&L0x201, &&L0x202, &&L0x203, &&L0x204, &&L0x205, &&L0x206, &&L0x207,
		&&L0x208, &&L0x209, &&L0x20a, &&L0x20b, &&L0x20c, &&L0x20d, &&L0x20e, &&L0x20f,
		&&L0x210, &&L0x211, &&L0x212, &&L0x213, &&L0x214, &&L0x215, &&L0x216, &&L0x217,
		&&L0x218, &&L0x219, &&L0x21a, &&L0x21b, &&L0x21c, &&L0x21d, &&L0x21e, &&L0x21f,
		&&L0x220, &&L0x221, &&L0x222, &&L0x223, &&L0x224, &&L0x225, &&L0x226, &&L0x227,
		&&L0x228, &&L0x229, &&L0x22a, &&L0x22b, &&L0x22c, &&L0x22d, &&L0x22e, &&L0x22f,
		&&L0x230, &&L0x231, &&L0x232, &&L0x233, &&L0x234, &&L0x235, &&L0x236, &&L0x237,
		&&L0x238, &&L0x239, &&L0x23a, &&L0x23b, &&L0x23c, &&L0x23d, &&L0x23e, &&L0x23f,

		&&L0x240, &&L0x241, &&L0x242, &&L0x243, &&L0x244, &&L0x245, &&L0x246, &&L0x247,
		&&L0x248, &&L0x249, &&L0x24a, &&L0x24b, &&L0x24c, &&L0x24d, &&L0x24e, &&L0x24f,
		&&L0x250, &&L0x251, &&L0x252, &&L0x253, &&L0x254, &&L0x255, &&L0x256, &&L0x257,
		&&L0x258, &&L0x259, &&L0x25a, &&L0x25b, &&L0x25c, &&L0x25d, &&L0x25e, &&L0x25f,
		&&L0x260, &&L0x261, &&L0x262, &&L0x263, &&L0x264, &&L0x265, &&L0x266, &&L0x267,
		&&L0x268, &&L0x269, &&L0x26a, &&L0x26b, &&L0x26c, &&L0x26d, &&L0x26e, &&L0x26f,
		&&L0x270, &&L0x271, &&L0x272, &&L0x273, &&L0x274, &&L0x275, &&L0x276, &&L0x277,
		&&L0x278, &&L0x279, &&L0x27a, &&L0x27b, &&L0x27c, &&L0x27d, &&L0x27e, &&L0x27f,

		&&L0x280, &&L0x281, &&L0x282, &&L0x283, &&L0x284, &&L0x285, &&L0x286, &&L0x287,
		&&L0x288, &&L0x289, &&L0x28a, &&L0x28b, &&L0x28c, &&L0x28d, &&L0x28e, &&L0x28f,
		&&L0x290, &&L0x291, &&L0x292, &&L0x293, &&L0x294, &&L0x295, &&L0x296, &&L0x297,
		&&L0x298, &&L0x299, &&L0x29a, &&L0x29b, &&L0x29c, &&L0x29d, &&L0x29e, &&L0x29f,
		&&L0x2a0, &&L0x2a1, &&L0x2a2, &&L0x2a3, &&L0x2a4, &&L0x2a5, &&L0x2a6, &&L0x2a7,
		&&L0x2a8, &&L0x2a9, &&L0x2aa, &&L0x2ab, &&L0x2ac, &&L0x2ad, &&L0x2ae, &&L0x2af,
		&&L0x2b0, &&L0x2b1, &&L0x2b2, &&L0x2b3, &&L0x2b4, &&L0x2b5, &&L0x2b6, &&L0x2b7,
		&&L0x2b8, &&L0x2b9, &&L0x2ba, &&L0x2bb, &&L0x2bc, &&L0x2bd, &&L0x2be, &&L0x2bf,

		&&L0x2c0, &&L0x2c1, &&L0x2c2, &&L0x2c3, &&L0x2c4, &&L0x2c5, &&L0x2c6, &&L0x2c7,
		&&L0x2c8, &&L0x2c9, &&L0x2ca, &&L0x2cb, &&L0x2cc, &&L0x2cd, &&L0x2ce, &&L0x2cf,
		&&L0x2d0, &&L0x2d1, &&L0x2d2, &&L0x2d3, &&L0x2d4, &&L0x2d5, &&L0x2d6, &&L0x2d7,
		&&L0x2d8, &&L0x2d9, &&L0x2da, &&L0x2db, &&L0x2dc, &&L0x2dd, &&L0x2de, &&L0x2df,
		&&L0x2e0, &&L0x2e1, &&L0x2e2, &&L0x2e3, &&L0x2e4, &&L0x2e5, &&L0x2e6, &&L0x2e7,
		&&L0x2e8, &&L0x2e9, &&L0x2ea, &&L0x2eb, &&L0x2ec, &&L0x2ed, &&L0x2ee, &&L0x2ef,
		&&L0x2f0, &&L0x2f1, &&L0x2f2, &&L0x2f3, &&L0x2f4, &&L0x2f5, &&L0x2f6, &&L0x2f7,
		&&L0x2f8, &&L0x2f9, &&L0x2fa, &&L0x2fb, &&L0x2fc, &&L0x2fd, &&L0x2fe, &&L0x2ff,

		&&L0x300, &&L0x301, &&L0x302, &&L0x303, &&L0x304, &&L0x305, &&L0x306, &&L0x307,
		&&L0x308, &&L0x309, &&L0x30a, &&L0x30b, &&L0x30c, &&L0x30d, &&L0x30e, &&L0x30f,
		&&L0x310, &&L0x311, &&L0x312, &&L0x313, &&L0x314, &&L0x315, &&L0x316, &&L0x317,
		&&L0x318, &&L0x319, &&L0x31a, &&L0x31b, &&L0x31c, &&L0x31d, &&L0x31e, &&L0x31f,
		&&L0x320, &&L0x321, &&L0x322, &&L0x323, &&L0x324, &&L0x325, &&L0x326, &&L0x327,
		&&L0x328, &&L0x329, &&L0x32a, &&L0x32b, &&L0x32c, &&L0x32d, &&L0x32e, &&L0x32f,
		&&L0x330, &&L0x331, &&L0x332, &&L0x333, &&L0x334, &&L0x335, &&L0x336, &&L0x337,
		&&L0x338, &&L0x339, &&L0x33a, &&L0x33b, &&L0x33c, &&L0x33d, &&L0x33e, &&L0x33f,

		&&L0x340, &&L0x341, &&L0x342, &&L0x343, &&L0x344, &&L0x345, &&L0x346, &&L0x347,
		&&L0x348, &&L0x349, &&L0x34a, &&L0x34b, &&L0x34c, &&L0x34d, &&L0x34e, &&L0x34f,
		&&L0x350, &&L0x351, &&L0x352, &&L0x353, &&L0x354, &&L0x355, &&L0x356, &&L0x357,
		&&L0x358, &&L0x359, &&L0x35a, &&L0x35b, &&L0x35c, &&L0x35d, &&L0x35e, &&L0x35f,
		&&L0x360, &&L0x361, &&L0x362, &&L0x363, &&L0x364, &&L0x365, &&L0x366, &&L0x367,
		&&L0x368, &&L0x369, &&L0x36a, &&L0x36b, &&L0x36c, &&L0x36d, &&L0x36e, &&L0x36f,
		&&L0x370, &&L0x371, &&L0x372, &&L0x373, &&L0x374, &&L0x375, &&L0x376, &&L0x377,
		&&L0x378, &&L0x379, &&L0x37a, &&L0x37b, &&L0x37c, &&L0x37d, &&L0x37e, &&L0x37f,

		&&L0x380, &&L0x381, &&L0x382, &&L0x383, &&L0x384, &&L0x385, &&L0x386, &&L0x387,
		&&L0x388, &&L0x389, &&L0x38a, &&L0x38b, &&L0x38c, &&L0x38d, &&L0x38e, &&L0x38f,
		&&L0x390, &&L0x391, &&L0x392, &&L0x393, &&L0x394, &&L0x395, &&L0x396, &&L0x397,
		&&L0x398, &&L0x399, &&L0x39a, &&L0x39b, &&L0x39c, &&L0x39d, &&L0x39e, &&L0x39f,
		&&L0x3a0, &&L0x3a1, &&L0x3a2, &&L0x3a3, &&L0x3a4, &&L0x3a5, &&L0x3a6, &&L0x3a7,
		&&L0x3a8, &&L0x3a9, &&L0x3aa, &&L0x3ab, &&L0x3ac, &&L0x3ad, &&L0x3ae, &&L0x3af,
		&&L0x3b0, &&L0x3b1, &&L0x3b2, &&L0x3b3, &&L0x3b4, &&L0x3b5, &&L0x3b6, &&L0x3b7,
		&&L0x3b8, &&L0x3b9, &&L0x3ba, &&L0x3bb, &&L0x3bc, &&L0x3bd, &&L0x3be, &&L0x3bf,

		&&L0x3c0, &&L0x3c1, &&L0x3c2, &&L0x3c3, &&L0x3c4, &&L0x3c5, &&L0x3c6, &&L0x3c7,
		&&L0x3c8, &&L0x3c9, &&L0x3ca, &&L0x3cb, &&L0x3cc, &&L0x3cd, &&L0x3ce, &&L0x3cf,
		&&L0x3d0, &&L0x3d1, &&L0x3d2, &&L0x3d3, &&L0x3d4, &&L0x3d5, &&L0x3d6, &&L0x3d7,
		&&L0x3d8, &&L0x3d9, &&L0x3da, &&L0x3db, &&L0x3dc, &&L0x3dd, &&L0x3de, &&L0x3df,
		&&L0x3e0, &&L0x3e1, &&L0x3e2, &&L0x3e3, &&L0x3e4, &&L0x3e5, &&L0x3e6, &&L0x3e7,
		&&L0x3e8, &&L0x3e9, &&L0x3ea, &&L0x3eb, &&L0x3ec, &&L0x3ed, &&L0x3ee, &&L0x3ef,
		&&L0x3f0, &&L0x3f1, &&L0x3f2, &&L0x3f3, &&L0x3f4, &&L0x3f5, &&L0x3f6, &&L0x3f7,
		&&L0x3f8, &&L0x3f9, &&L0x3fa, &&L0x3fb, &&L0x3fc, &&L0x3fd, &&L0x3fe, &&L0x3ff,
	};

	static uint64_t ALIGN64 counters[MAX_LABELS >> 6];
	static const void ALIGN64 *labels_forward[MAX_LABELS];
	static const void ALIGN64 *labels_backward[MAX_LABELS];
	const void **labels = labels_forward;

	for (i = 0; i < MAX_LABELS; i++) {
		labels_forward[i] = default_labels[(i + 1) % MAX_LABELS];
		labels_backward[i] = default_labels[(MAX_LABELS + i - 1) % MAX_LABELS];
	}

	goto_direction = STRESS_GOTO_RANDOM;
	(void)stress_get_setting("goto-direction", &goto_direction);

	stress_set_proc_state(args->name, STRESS_STATE_RUN);

	switch (goto_direction) {
	case STRESS_GOTO_FORWARD:
		labels = labels_forward;
		break;
	case STRESS_GOTO_BACKWARD:
		labels = labels_backward;
		break;
	case STRESS_GOTO_RANDOM:
		labels = labels_forward;
		break;
	}

	t1 = stress_time_now();
	for (;;) {
L0x000:
		if (!stress_continue(args))
			break;
		if (goto_direction == STRESS_GOTO_RANDOM)
			labels = stress_mwc1() ? labels_backward : labels_forward;
		stress_bogo_inc(args);
		counters[0]++;
		goto *labels[0];

			 G(0x001) G(0x002) G(0x003) G(0x004) G(0x005) G(0x006) G(0x007)
		G(0x008) G(0x009) G(0x00a) G(0x00b) G(0x00c) G(0x00d) G(0x00e) G(0x00f)
		G(0x010) G(0x011) G(0x012) G(0x013) G(0x014) G(0x015) G(0x016) G(0x017)
		G(0x018) G(0x019) G(0x01a) G(0x01b) G(0x01c) G(0x01d) G(0x01e) G(0x01f)
		G(0x020) G(0x021) G(0x022) G(0x023) G(0x024) G(0x025) G(0x026) G(0x027)
		G(0x028) G(0x029) G(0x02a) G(0x02b) G(0x02c) G(0x02d) G(0x02e) G(0x02f)
		G(0x030) G(0x031) G(0x032) G(0x033) G(0x034) G(0x035) G(0x036) G(0x037)
		G(0x038) G(0x039) G(0x03a) G(0x03b) G(0x03c) G(0x03d) G(0x03e) G(0x03f)

		G(0x040) G(0x041) G(0x042) G(0x043) G(0x044) G(0x045) G(0x046) G(0x047)
		G(0x048) G(0x049) G(0x04a) G(0x04b) G(0x04c) G(0x04d) G(0x04e) G(0x04f)
		G(0x050) G(0x051) G(0x052) G(0x053) G(0x054) G(0x055) G(0x056) G(0x057)
		G(0x058) G(0x059) G(0x05a) G(0x05b) G(0x05c) G(0x05d) G(0x05e) G(0x05f)
		G(0x060) G(0x061) G(0x062) G(0x063) G(0x064) G(0x065) G(0x066) G(0x067)
		G(0x068) G(0x069) G(0x06a) G(0x06b) G(0x06c) G(0x06d) G(0x06e) G(0x06f)
		G(0x070) G(0x071) G(0x072) G(0x073) G(0x074) G(0x075) G(0x076) G(0x077)
		G(0x078) G(0x079) G(0x07a) G(0x07b) G(0x07c) G(0x07d) G(0x07e) G(0x07f)

		G(0x080) G(0x081) G(0x082) G(0x083) G(0x084) G(0x085) G(0x086) G(0x087)
		G(0x088) G(0x089) G(0x08a) G(0x08b) G(0x08c) G(0x08d) G(0x08e) G(0x08f)
		G(0x090) G(0x091) G(0x092) G(0x093) G(0x094) G(0x095) G(0x096) G(0x097)
		G(0x098) G(0x099) G(0x09a) G(0x09b) G(0x09c) G(0x09d) G(0x09e) G(0x09f)
		G(0x0a0) G(0x0a1) G(0x0a2) G(0x0a3) G(0x0a4) G(0x0a5) G(0x0a6) G(0x0a7)
		G(0x0a8) G(0x0a9) G(0x0aa) G(0x0ab) G(0x0ac) G(0x0ad) G(0x0ae) G(0x0af)
		G(0x0b0) G(0x0b1) G(0x0b2) G(0x0b3) G(0x0b4) G(0x0b5) G(0x0b6) G(0x0b7)
		G(0x0b8) G(0x0b9) G(0x0ba) G(0x0bb) G(0x0bc) G(0x0bd) G(0x0be) G(0x0bf)

		G(0x0c0) G(0x0c1) G(0x0c2) G(0x0c3) G(0x0c4) G(0x0c5) G(0x0c6) G(0x0c7)
		G(0x0c8) G(0x0c9) G(0x0ca) G(0x0cb) G(0x0cc) G(0x0cd) G(0x0ce) G(0x0cf)
		G(0x0d0) G(0x0d1) G(0x0d2) G(0x0d3) G(0x0d4) G(0x0d5) G(0x0d6) G(0x0d7)
		G(0x0d8) G(0x0d9) G(0x0da) G(0x0db) G(0x0dc) G(0x0dd) G(0x0de) G(0x0df)
		G(0x0e0) G(0x0e1) G(0x0e2) G(0x0e3) G(0x0e4) G(0x0e5) G(0x0e6) G(0x0e7)
		G(0x0e8) G(0x0e9) G(0x0ea) G(0x0eb) G(0x0ec) G(0x0ed) G(0x0ee) G(0x0ef)
		G(0x0f0) G(0x0f1) G(0x0f2) G(0x0f3) G(0x0f4) G(0x0f5) G(0x0f6) G(0x0f7)
		G(0x0f8) G(0x0f9) G(0x0fa) G(0x0fb) G(0x0fc) G(0x0fd) G(0x0fe) G(0x0ff)

		G(0x100) G(0x101) G(0x102) G(0x103) G(0x104) G(0x105) G(0x106) G(0x107)
		G(0x108) G(0x109) G(0x10a) G(0x10b) G(0x10c) G(0x10d) G(0x10e) G(0x10f)
		G(0x110) G(0x111) G(0x112) G(0x113) G(0x114) G(0x115) G(0x116) G(0x117)
		G(0x118) G(0x119) G(0x11a) G(0x11b) G(0x11c) G(0x11d) G(0x11e) G(0x11f)
		G(0x120) G(0x121) G(0x122) G(0x123) G(0x124) G(0x125) G(0x126) G(0x127)
		G(0x128) G(0x129) G(0x12a) G(0x12b) G(0x12c) G(0x12d) G(0x12e) G(0x12f)
		G(0x130) G(0x131) G(0x132) G(0x133) G(0x134) G(0x135) G(0x136) G(0x137)
		G(0x138) G(0x139) G(0x13a) G(0x13b) G(0x13c) G(0x13d) G(0x13e) G(0x13f)

		G(0x140) G(0x141) G(0x142) G(0x143) G(0x144) G(0x145) G(0x146) G(0x147)
		G(0x148) G(0x149) G(0x14a) G(0x14b) G(0x14c) G(0x14d) G(0x14e) G(0x14f)
		G(0x150) G(0x151) G(0x152) G(0x153) G(0x154) G(0x155) G(0x156) G(0x157)
		G(0x158) G(0x159) G(0x15a) G(0x15b) G(0x15c) G(0x15d) G(0x15e) G(0x15f)
		G(0x160) G(0x161) G(0x162) G(0x163) G(0x164) G(0x165) G(0x166) G(0x167)
		G(0x168) G(0x169) G(0x16a) G(0x16b) G(0x16c) G(0x16d) G(0x16e) G(0x16f)
		G(0x170) G(0x171) G(0x172) G(0x173) G(0x174) G(0x175) G(0x176) G(0x177)
		G(0x178) G(0x179) G(0x17a) G(0x17b) G(0x17c) G(0x17d) G(0x17e) G(0x17f)

		G(0x180) G(0x181) G(0x182) G(0x183) G(0x184) G(0x185) G(0x186) G(0x187)
		G(0x188) G(0x189) G(0x18a) G(0x18b) G(0x18c) G(0x18d) G(0x18e) G(0x18f)
		G(0x190) G(0x191) G(0x192) G(0x193) G(0x194) G(0x195) G(0x196) G(0x197)
		G(0x198) G(0x199) G(0x19a) G(0x19b) G(0x19c) G(0x19d) G(0x19e) G(0x19f)
		G(0x1a0) G(0x1a1) G(0x1a2) G(0x1a3) G(0x1a4) G(0x1a5) G(0x1a6) G(0x1a7)
		G(0x1a8) G(0x1a9) G(0x1aa) G(0x1ab) G(0x1ac) G(0x1ad) G(0x1ae) G(0x1af)
		G(0x1b0) G(0x1b1) G(0x1b2) G(0x1b3) G(0x1b4) G(0x1b5) G(0x1b6) G(0x1b7)
		G(0x1b8) G(0x1b9) G(0x1ba) G(0x1bb) G(0x1bc) G(0x1bd) G(0x1be) G(0x1bf)

		G(0x1c0) G(0x1c1) G(0x1c2) G(0x1c3) G(0x1c4) G(0x1c5) G(0x1c6) G(0x1c7)
		G(0x1c8) G(0x1c9) G(0x1ca) G(0x1cb) G(0x1cc) G(0x1cd) G(0x1ce) G(0x1cf)
		G(0x1d0) G(0x1d1) G(0x1d2) G(0x1d3) G(0x1d4) G(0x1d5) G(0x1d6) G(0x1d7)
		G(0x1d8) G(0x1d9) G(0x1da) G(0x1db) G(0x1dc) G(0x1dd) G(0x1de) G(0x1df)
		G(0x1e0) G(0x1e1) G(0x1e2) G(0x1e3) G(0x1e4) G(0x1e5) G(0x1e6) G(0x1e7)
		G(0x1e8) G(0x1e9) G(0x1ea) G(0x1eb) G(0x1ec) G(0x1ed) G(0x1ee) G(0x1ef)
		G(0x1f0) G(0x1f1) G(0x1f2) G(0x1f3) G(0x1f4) G(0x1f5) G(0x1f6) G(0x1f7)
		G(0x1f8) G(0x1f9) G(0x1fa) G(0x1fb) G(0x1fc) G(0x1fd) G(0x1fe) G(0x1ff)

		G(0x200) G(0x201) G(0x202) G(0x203) G(0x204) G(0x205) G(0x206) G(0x207)
		G(0x208) G(0x209) G(0x20a) G(0x20b) G(0x20c) G(0x20d) G(0x20e) G(0x20f)
		G(0x210) G(0x211) G(0x212) G(0x213) G(0x214) G(0x215) G(0x216) G(0x217)
		G(0x218) G(0x219) G(0x21a) G(0x21b) G(0x21c) G(0x21d) G(0x21e) G(0x21f)
		G(0x220) G(0x221) G(0x222) G(0x223) G(0x224) G(0x225) G(0x226) G(0x227)
		G(0x228) G(0x229) G(0x22a) G(0x22b) G(0x22c) G(0x22d) G(0x22e) G(0x22f)
		G(0x230) G(0x231) G(0x232) G(0x233) G(0x234) G(0x235) G(0x236) G(0x237)
		G(0x238) G(0x239) G(0x23a) G(0x23b) G(0x23c) G(0x23d) G(0x23e) G(0x23f)

		G(0x240) G(0x241) G(0x242) G(0x243) G(0x244) G(0x245) G(0x246) G(0x247)
		G(0x248) G(0x249) G(0x24a) G(0x24b) G(0x24c) G(0x24d) G(0x24e) G(0x24f)
		G(0x250) G(0x251) G(0x252) G(0x253) G(0x254) G(0x255) G(0x256) G(0x257)
		G(0x258) G(0x259) G(0x25a) G(0x25b) G(0x25c) G(0x25d) G(0x25e) G(0x25f)
		G(0x260) G(0x261) G(0x262) G(0x263) G(0x264) G(0x265) G(0x266) G(0x267)
		G(0x268) G(0x269) G(0x26a) G(0x26b) G(0x26c) G(0x26d) G(0x26e) G(0x26f)
		G(0x270) G(0x271) G(0x272) G(0x273) G(0x274) G(0x275) G(0x276) G(0x277)
		G(0x278) G(0x279) G(0x27a) G(0x27b) G(0x27c) G(0x27d) G(0x27e) G(0x27f)

		G(0x280) G(0x281) G(0x282) G(0x283) G(0x284) G(0x285) G(0x286) G(0x287)
		G(0x288) G(0x289) G(0x28a) G(0x28b) G(0x28c) G(0x28d) G(0x28e) G(0x28f)
		G(0x290) G(0x291) G(0x292) G(0x293) G(0x294) G(0x295) G(0x296) G(0x297)
		G(0x298) G(0x299) G(0x29a) G(0x29b) G(0x29c) G(0x29d) G(0x29e) G(0x29f)
		G(0x2a0) G(0x2a1) G(0x2a2) G(0x2a3) G(0x2a4) G(0x2a5) G(0x2a6) G(0x2a7)
		G(0x2a8) G(0x2a9) G(0x2aa) G(0x2ab) G(0x2ac) G(0x2ad) G(0x2ae) G(0x2af)
		G(0x2b0) G(0x2b1) G(0x2b2) G(0x2b3) G(0x2b4) G(0x2b5) G(0x2b6) G(0x2b7)
		G(0x2b8) G(0x2b9) G(0x2ba) G(0x2bb) G(0x2bc) G(0x2bd) G(0x2be) G(0x2bf)

		G(0x2c0) G(0x2c1) G(0x2c2) G(0x2c3) G(0x2c4) G(0x2c5) G(0x2c6) G(0x2c7)
		G(0x2c8) G(0x2c9) G(0x2ca) G(0x2cb) G(0x2cc) G(0x2cd) G(0x2ce) G(0x2cf)
		G(0x2d0) G(0x2d1) G(0x2d2) G(0x2d3) G(0x2d4) G(0x2d5) G(0x2d6) G(0x2d7)
		G(0x2d8) G(0x2d9) G(0x2da) G(0x2db) G(0x2dc) G(0x2dd) G(0x2de) G(0x2df)
		G(0x2e0) G(0x2e1) G(0x2e2) G(0x2e3) G(0x2e4) G(0x2e5) G(0x2e6) G(0x2e7)
		G(0x2e8) G(0x2e9) G(0x2ea) G(0x2eb) G(0x2ec) G(0x2ed) G(0x2ee) G(0x2ef)
		G(0x2f0) G(0x2f1) G(0x2f2) G(0x2f3) G(0x2f4) G(0x2f5) G(0x2f6) G(0x2f7)
		G(0x2f8) G(0x2f9) G(0x2fa) G(0x2fb) G(0x2fc) G(0x2fd) G(0x2fe) G(0x2ff)

		G(0x300) G(0x301) G(0x302) G(0x303) G(0x304) G(0x305) G(0x306) G(0x307)
		G(0x308) G(0x309) G(0x30a) G(0x30b) G(0x30c) G(0x30d) G(0x30e) G(0x30f)
		G(0x310) G(0x311) G(0x312) G(0x313) G(0x314) G(0x315) G(0x316) G(0x317)
		G(0x318) G(0x319) G(0x31a) G(0x31b) G(0x31c) G(0x31d) G(0x31e) G(0x31f)
		G(0x320) G(0x321) G(0x322) G(0x323) G(0x324) G(0x325) G(0x326) G(0x327)
		G(0x328) G(0x329) G(0x32a) G(0x32b) G(0x32c) G(0x32d) G(0x32e) G(0x32f)
		G(0x330) G(0x331) G(0x332) G(0x333) G(0x334) G(0x335) G(0x336) G(0x337)
		G(0x338) G(0x339) G(0x33a) G(0x33b) G(0x33c) G(0x33d) G(0x33e) G(0x33f)

		G(0x340) G(0x341) G(0x342) G(0x343) G(0x344) G(0x345) G(0x346) G(0x347)
		G(0x348) G(0x349) G(0x34a) G(0x34b) G(0x34c) G(0x34d) G(0x34e) G(0x34f)
		G(0x350) G(0x351) G(0x352) G(0x353) G(0x354) G(0x355) G(0x356) G(0x357)
		G(0x358) G(0x359) G(0x35a) G(0x35b) G(0x35c) G(0x35d) G(0x35e) G(0x35f)
		G(0x360) G(0x361) G(0x362) G(0x363) G(0x364) G(0x365) G(0x366) G(0x367)
		G(0x368) G(0x369) G(0x36a) G(0x36b) G(0x36c) G(0x36d) G(0x36e) G(0x36f)
		G(0x370) G(0x371) G(0x372) G(0x373) G(0x374) G(0x375) G(0x376) G(0x377)
		G(0x378) G(0x379) G(0x37a) G(0x37b) G(0x37c) G(0x37d) G(0x37e) G(0x37f)

		G(0x380) G(0x381) G(0x382) G(0x383) G(0x384) G(0x385) G(0x386) G(0x387)
		G(0x388) G(0x389) G(0x38a) G(0x38b) G(0x38c) G(0x38d) G(0x38e) G(0x38f)
		G(0x390) G(0x391) G(0x392) G(0x393) G(0x394) G(0x395) G(0x396) G(0x397)
		G(0x398) G(0x399) G(0x39a) G(0x39b) G(0x39c) G(0x39d) G(0x39e) G(0x39f)
		G(0x3a0) G(0x3a1) G(0x3a2) G(0x3a3) G(0x3a4) G(0x3a5) G(0x3a6) G(0x3a7)
		G(0x3a8) G(0x3a9) G(0x3aa) G(0x3ab) G(0x3ac) G(0x3ad) G(0x3ae) G(0x3af)
		G(0x3b0) G(0x3b1) G(0x3b2) G(0x3b3) G(0x3b4) G(0x3b5) G(0x3b6) G(0x3b7)
		G(0x3b8) G(0x3b9) G(0x3ba) G(0x3bb) G(0x3bc) G(0x3bd) G(0x3be) G(0x3bf)

		G(0x3c0) G(0x3c1) G(0x3c2) G(0x3c3) G(0x3c4) G(0x3c5) G(0x3c6) G(0x3c7)
		G(0x3c8) G(0x3c9) G(0x3ca) G(0x3cb) G(0x3cc) G(0x3cd) G(0x3ce) G(0x3cf)
		G(0x3d0) G(0x3d1) G(0x3d2) G(0x3d3) G(0x3d4) G(0x3d5) G(0x3d6) G(0x3d7)
		G(0x3d8) G(0x3d9) G(0x3da) G(0x3db) G(0x3dc) G(0x3dd) G(0x3de) G(0x3df)
		G(0x3e0) G(0x3e1) G(0x3e2) G(0x3e3) G(0x3e4) G(0x3e5) G(0x3e6) G(0x3e7)
		G(0x3e8) G(0x3e9) G(0x3ea) G(0x3eb) G(0x3ec) G(0x3ed) G(0x3ee) G(0x3ef)
		G(0x3f0) G(0x3f1) G(0x3f2) G(0x3f3) G(0x3f4) G(0x3f5) G(0x3f6) G(0x3f7)
		G(0x3f8) G(0x3f9) G(0x3fa) G(0x3fb) G(0x3fc) G(0x3fd) G(0x3fe) G(0x3ff)
	}
	t2 = stress_time_now();

	bogo_counter = stress_bogo_get(args);
	lo = bogo_counter - 1;
	hi = bogo_counter + 1;

	/*
	 *  sanity check that every 64th goto got a correct number
	 *  of execution hits.
	 */
	for (i = 0; i < SIZEOF_ARRAY(counters); i++) {
		if ((counters[i] < lo) || (counters[i] > hi)) {
			pr_fail("%s: goto label %zd execution count out by more than +/-1, "
				"got %" PRIu64 ", expected between %" PRIu64 " and %" PRIu64 "\n",
				args->name, i * 64, counters[i], lo, hi);
			rc = EXIT_FAILURE;
		}
	}

	duration = t2 - t1;
	rate = (duration > 0.0) ? (1024.0 * (double)stress_bogo_get(args)) / duration : 0.0;
	stress_metrics_set(args, 0, "million gotos per sec",
		rate / 1000000.0, STRESS_HARMONIC_MEAN);

	stress_set_proc_state(args->name, STRESS_STATE_DEINIT);

	return rc;
}

stressor_info_t stress_goto_info = {
	.stressor = stress_goto,
	.class = CLASS_CPU,
	.opt_set_funcs = opt_set_funcs,
	.verify = VERIFY_ALWAYS,
	.help = help
};
#else
stressor_info_t stress_goto_info = {
	.stressor = stress_unimplemented,
	.class = CLASS_CPU,
	.opt_set_funcs = opt_set_funcs,
	.verify = VERIFY_ALWAYS,
	.help = help,
	.unimplemented_reason = "built without compiler support gcc style 'labels as values' feature"
};
#endif