/* konka-tv.h - Keytable for konka_tv Remote Controller
 *
 * keymap imported from ir-keymaps.c
 *
 * Copyright (c) 2010 by Mauro Carvalho Chehab <mchehab@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <media/rc-core.h>

/*
 * Jimmy Hsu <jimmy.hsu@mstarsemi.com>
 * this is the remote control that comes with the konka smart tv
 * which based on STAOS standard.
 */

static struct rc_map_table konka_tv[] = {
#if 1
    // 1st IR controller. (Internal)
    { 0x000B, KEY_POWER },
    { 0x0000, KEY_0 },
    { 0x0001, KEY_1 },
    { 0x0002, KEY_2 },
    { 0x0003, KEY_3 },
    { 0x0004, KEY_4 },
    { 0x0005, KEY_5 },
    { 0x0006, KEY_6 },
    { 0x0007, KEY_7 },
    { 0x0008, KEY_8 },
    { 0x0009, KEY_9 },
    { 0x001A, KEY_RED },
    { 0x001F, KEY_GREEN },
    { 0x000C, KEY_YELLOW },
    { 0x0016, KEY_BLUE },
    { 0x002B, KEY_UP },
    { 0x002C, KEY_DOWN },
    { 0x002D, KEY_LEFT },
    { 0x002E, KEY_RIGHT },
    { 0x002F, KEY_ENTER },
    { 0x0011, KEY_CHANNELUP },
    { 0x0010, KEY_CHANNELDOWN },
    { 0x0013, KEY_VOLUMEUP },
    { 0x0012, KEY_VOLUMEDOWN },
    { 0x0027, KEY_PAGEUP },
    { 0x0024, KEY_PAGEDOWN },
    { 0x0038, KEY_HOME },
    { 0x0015, KEY_MENU },
    { 0x0030, KEY_BACK },
    { 0x0014, KEY_MUTE },       // VOLUME_MUTE
    { 0x000A, KEY_INFO },
    { 0x0032, KEY_TV },
    { 0x0025, KEY_SEARCH },
    { 0x000E, KEY_DELETE },     // DEL
    { 0x001C, KEY_KP0 },        // TV_INPUT
    { 0x0036, KEY_KP1 },        // 3D MODE
    { 0x0033, KEY_KP2 },        // AVR_POWER
    { 0x000D, KEY_AUDIO },      // (C)SOUND_MODE
    { 0x001D, KEY_CAMERA },     // (C)PICTURE_MODE
    { 0x0022, KEY_EPG },        // (C)EPG
    { 0x002A, KEY_LIST },       // (C)LIST
    { 0x0019, KEY_FN_F2 },      // (C)FREEZE
    { 0x0031, KEY_FN_F4 },      // (C)HDMI
    { 0x001E, KEY_FN_F5 },      // (C)DISPLAY_MODE
    { 0x0034, KEY_F1 },         // KEYCODE_KONKA_YPBPR
#else
    // 1st IR controller. (Overseas)
    { 0x000B, KEY_POWER },
    { 0x0000, KEY_0 },
    { 0x0001, KEY_1 },
    { 0x0002, KEY_2 },
    { 0x0003, KEY_3 },
    { 0x0004, KEY_4 },
    { 0x0005, KEY_5 },
    { 0x0006, KEY_6 },
    { 0x0007, KEY_7 },
    { 0x0008, KEY_8 },
    { 0x0009, KEY_9 },
    { 0x001A, KEY_RED },
    { 0x001F, KEY_GREEN },
    { 0x000C, KEY_YELLOW },
    { 0x0016, KEY_BLUE },
    { 0x002B, KEY_UP },
    { 0x002C, KEY_DOWN },
    { 0x002D, KEY_LEFT },
    { 0x002E, KEY_RIGHT },
    { 0x002F, KEY_ENTER },
    { 0x0011, KEY_CHANNELUP },
    { 0x0010, KEY_CHANNELDOWN },
    { 0x0013, KEY_VOLUMEUP },
    { 0x0012, KEY_VOLUMEDOWN },
    { 0x0034, KEY_HOME },
    { 0x0015, KEY_MENU },
    { 0x0030, KEY_BACK },
    { 0x0014, KEY_MUTE },       // VOLUME_MUTE
    { 0x000A, KEY_INFO },
    { 0x000E, KEY_DELETE },     // DEL
    { 0x001C, KEY_KP0 },        // TV_INPUT
    { 0x0036, KEY_KP1 },        // 3D MODE
    { 0x000F, KEY_AUDIO },      // (C)SOUND_MODE
    { 0x0023, KEY_CAMERA },     // (C)PICTURE_MODE
    { 0x0025, KEY_SLEEP },      // (C)SLEEP
    { 0x0036, KEY_EPG },        // (C)EPG
    { 0x0027, KEY_LIST },       // (C)LIST
    { 0x0018, KEY_SUBTITLE },   // (C)SUBTITLE
    { 0x0024, KEY_FAVORITES },  // (C)FAVORITE
    { 0x0022, KEY_FN_F1 },      // (C)MTS
    { 0x002A, KEY_FN_F2 },      // (C)FREEZE
    { 0x001E, KEY_FN_F3 },      // (C)TTX
    { 0x0031, KEY_FN_F5 },      // (C)DISPLAY_MODE
    { 0x0034, KEY_F1 },         // KEYCODE_KONKA_YPBPR
#endif

    // 2nd IR controller.
};

static struct rc_map_list konka_tv_map = {
	.map = {
		.scan    = konka_tv,
		.size    = ARRAY_SIZE(konka_tv),
		.rc_type = RC_TYPE_UNKNOWN,	/* Legacy IR type */
		.name    = RC_MAP_KONKA_TV,
	}
};

static int __init init_rc_map_konka_tv(void)
{
	return rc_map_register(&konka_tv_map);
}

static void __exit exit_rc_map_konka_tv(void)
{
	rc_map_unregister(&konka_tv_map);
}

module_init(init_rc_map_konka_tv)
module_exit(exit_rc_map_konka_tv)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mauro Carvalho Chehab <mchehab@redhat.com>");
