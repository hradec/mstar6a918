/* hisense-tv.h - Keytable for hisense_tv Remote Controller
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
 * this is the remote control that comes with the hisense smart tv
 * which based on STAOS standard.
 */

static struct rc_map_table hisense_tv[] = {
    // 1st IR controller.
    { 0xBF0D, KEY_POWER },
    { 0xBF0E, KEY_MUTE },
    { 0xBF1A, KEY_SLEEP },
    { 0xBF11, KEY_AUDIO },      // SOUND_MODE
    { 0xBF10, KEY_CAMERA },     // PICTURE_MODE
    { 0xBF13, KEY_ZOOM },       // ASPECT_RATIO
    { 0xBF0B, KEY_CHANNEL },    // CHANNEL_RETURN
    { 0xBF00, KEY_0 },
    { 0xBF01, KEY_1 },
    { 0xBF02, KEY_2 },
    { 0xBF03, KEY_3 },
    { 0xBF04, KEY_4 },
    { 0xBF05, KEY_5 },
    { 0xBF06, KEY_6 },
    { 0xBF07, KEY_7 },
    { 0xBF08, KEY_8 },
    { 0xBF09, KEY_9 },
    { 0xBF52, KEY_RED },
    { 0xBF53, KEY_GREEN },
    { 0xBF54, KEY_YELLOW },
    { 0xBF55, KEY_BLUE },
    { 0xBF14, KEY_MENU },
    { 0xBF48, KEY_BACK },
    { 0xBF1D, KEY_EPG },
    { 0xBF16, KEY_UP },
    { 0xBF17, KEY_DOWN },
    { 0xBF18, KEY_RIGHT },
    { 0xBF19, KEY_LEFT },
    { 0xBF15, KEY_ENTER },
    { 0xBF4A, KEY_CHANNELUP },
    { 0xBF4B, KEY_CHANNELDOWN },
    { 0xBF44, KEY_VOLUMEUP },
    { 0xBF43, KEY_VOLUMEDOWN },
    { 0xBF5C, KEY_BACK },
    { 0xBF94, KEY_HOME },
    { 0xBF1F, KEY_SUBTITLE },
    { 0xBF5E, KEY_RECORD },     // DVR
//    { 0xBF0C, KEY_INFO },
    { 0xBF95, KEY_INFO },
    { 0xBF1B, KEY_TV },
    { 0xBF5B, KEY_SEARCH },
    { 0xBF1E, KEY_FAVORITES },
    { 0xBF57, KEY_REWIND },
    { 0xBF56, KEY_FORWARD },
    { 0xBF58, KEY_PREVIOUSSONG },
    { 0xBF59, KEY_NEXTSONG },
    { 0xBF5A, KEY_STOP },
    { 0xBF4E, KEY_PLAYPAUSE },
    { 0xBF4C, KEY_FN_F1 },      // MTS
    { 0xBF0F, KEY_FN_F2 },      // FREEZE
    { 0xBF12, KEY_FN_F6 },      // TV_INPUT
    { 0xBF5F, KEY_FN_F7 },      // 3D_MODE

    // Hisense extended
    { 0xBF1C, KEY_F1 },         // HISENSE_SAVEMODE
    { 0xBF49, KEY_F2 },         // HISENSE_AUDIO_TRACK
    { 0xBF5D, KEY_F3 },         // HISENSE_BROADCAST


    // 2nd IR cotroller.
    { 0x0F18, 530 },            // A
    { 0x0F19, 531 },
    { 0x0F1A, 532 },
    { 0x0F1B, 533 },
    { 0x0F1C, 534 },
    { 0x0F0A, 535 },
    { 0x0F2A, 536 },
    { 0x0F2B, 537 },
    { 0x0F2C, 538 },
    { 0x0F2D, 539 },
    { 0x0F2E, 540 },
    { 0x0F2F, 541 },
    { 0x0F31, 542 },
    { 0x0F32, 543 },
    { 0x0F33, 544 },
    { 0x0F34, 545 },
    { 0x0F35, 546 },
    { 0x0F36, 547 },
    { 0x0F37, 548 },
    { 0x0F38, 549 },
    { 0x0F39, 550 },
    { 0x0F3A, 551 },
    { 0x0F3B, 552 },
    { 0x0F3C, 553 },
    { 0x0F3D, 554 },
    { 0x0F3E, 555 },             // Z
    { 0x0F20, 556 },             // NUMPAD_0
    { 0x0F21, 557 },
    { 0x0F22, 558 },
    { 0x0F23, 559 },
    { 0x0F24, 560 },
    { 0x0F25, 561 },
    { 0x0F26, 562 },
    { 0x0F27, 563 },
    { 0x0F28, 564 },
    { 0x0F29, 565 },             // NUMPAD_9
    { 0x0F1F, 566 },             // HISENSE_PRODUCT_SCAN_START
    { 0x0F30, 567 },             // HISENSE_PRODUCT_SCAN_OVER
    { 0x0F00, 591 },             // HISENSE_TEST_BROAD_TV
    { 0x0F01, 592 },             // HISENSE_TEST_BROAD_DTV
    { 0x0F02, 593 },             // HISENSE_TEST_BROAD_AV1
    { 0x0F03, 594 },             // HISENSE_TEST_BROAD_AV2
    { 0x0F04, 595 },             // HISENSE_TEST_BROAD_AV3
    { 0x0F05, 596 },             // HISENSE_TEST_BROAD_SVIDEO1
    { 0x0F06, 597 },             // HISENSE_TEST_BROAD_SVIDEO2
    { 0x0F07, 598 },             // HISENSE_TEST_BROAD_SVIDEO3
    { 0x0F08, 599 },             // HISENSE_TEST_BROAD_SCART1
    { 0x0F09, 600 },             // HISENSE_TEST_BROAD_SCART2
    { 0x0F0A, 601 },             // HISENSE_TEST_BROAD_SCART3
    { 0x0F0B, 602 },             // HISENSE_TEST_BOARD_YPbPr1
    { 0x0F0C, 603 },             // HISENSE_TEST_BOARD_YPbPr2
    { 0x0F0D, 604 },             // HISENSE_TEST_BOARD_YPbPr3
    { 0x0F0E, 605 },             // HISENSE_TEST_BOARD_VGA
    { 0x0F0F, 606 },             // HISENSE_TEST_BOARD_HDMI1
    { 0x0F10, 607 },             // HISENSE_TEST_BOARD_HDMI2
    { 0x0F11, 608 },             // HISENSE_TEST_BOARD_HDMI3
    { 0x0F12, 609 },             // HISENSE_TEST_BOARD_HDMI4
    { 0x0F13, 610 },             // HISENSE_TEST_BOARD_HDMI5
    { 0x0F14, 611 },             // HISENSE_TEST_BOARD_DMP
    { 0x0F15, 612 },             // HISENSE_TEST_BOARD_EMP
    { 0x0F16, 613 },             // HISENSE_TEST_BOARD_AUTOCOLOR
    { 0x0F17, 614 },             // HISENSE_TEST_BOARD_SAVE
    { 0x0F18, 615 },             // HISENSE_TEST_BOARD_TELETEXT
    { 0x0F19, 616 },             // HISENSE_TEST_BOARD_SAPL
    { 0x0F1A, 617 },             // HISENSE_TEST_BOARD_VCHIP
    { 0x0F1B, 618 },             // HISENSE_TEST_BOARD_CCD
    { 0x0F1C, 619 },             // HISENSE_TEST_BOARD_BTSC
    { 0x0F1D, 620 },             // HISENSE_TEST_BOARD_FAC_OK

	{ 0xFC50, 621 },//KEY_FN_RDRV_PLUS }, 
	{ 0xFC46, 622 },//KEY_FN_RDRV_INC }, 
	{ 0xFC4C, 623 },//KEY_FN_GDRV_PLUS }, 
	{ 0xFC5A, 624 },//KEY_FN_GDRV_INC }, 
	{ 0xFC49, 625 },//KEY_FN_BDRV_PLUS }, 
	{ 0xFC4A, 626 },//KEY_FN_BDRV_INC }, 
	{ 0xFC44, 627 },//KEY_FN_RCUT_PLUS }, 
	{ 0xFC41, 628 },//KEY_FN_RCUT_INC }, 
	{ 0xFC4B, 629 },//KEY_FN_GCUT_PLUS }, 
	{ 0xFC51, 630 },//KEY_FN_GCUT_INC }, 
	{ 0xFC08, 631 },//KEY_FN_BCUT_PLUS }, 
	{ 0xFC45, 641 },//KEY_FN_BCUT_INC }, 
	{ 0xBF4D, 642 },//    KEYCODE_MEDIA_SONG_SYSTEM }, 
	{ 0xBF4E, 643 },//KEYCODE_MEDIA_PLAY_PAUSE }, 
	{ 0xBF5A, 644 },//KEYCODE_MEDIA_STOP }, 

    // 3th IR cotroller.
    { 0xFC5D, 568 },             // HISENSE_FAC_NEC_F1
    { 0xFC42, 569 },             // HISENSE_FAC_NEC_F2
    { 0xFC56, 570 },             // HISENSE_FAC_NEC_F3
    { 0xFC48, 571 },             // HISENSE_FAC_NEC_F4
    { 0xFC53, 572 },             // HISENSE_FAC_NEC_F5
    { 0xFC1D, 573 },             // HISENSE_FAC_NEC_F6
    { 0xFC4F, 574 },             // HISENSE_FAC_NEC_F7
    { 0xFC47, 575 },             // HISENSE_FAC_NEC_OK
    { 0xFC0A, 576 },             // HISENSE_FAC_NEC_MAC
    { 0xFC4E, 577 },             // HISENSE_FAC_NEC_IP
    { 0xFC00, 578 },             // HISENSE_FAC_NEC_M
    { 0xFC05, 579 },             // HISENSE_FAC_NEC_AV
    { 0xFC18, 580 },             // HISENSE_FAC_NEC_PC
    { 0xFC52, 581 },             // HISENSE_FAC_NEC_HDMI
    { 0xFC01, 582 },             // HISENSE_FAC_NEC_YPBPR
    { 0xFC06, 586 },             // HISENSE_FAC_NEC_BALANCE
    { 0xFC0D, 588 },             // HISENSE_FAC_NEC_LOGO
    { 0xFC19, 590 },             // HISNESE_FAC_NEC_PANEL
#if (HISENSE_USE_BOX==0)
	{ 0xFC09, 583 },             // HISENSE_FAC_NEC_SAVE
    { 0xFC58, 584 },             // HISENSE_FAC_NEC_PATTERN
    { 0xFC15, 585 },             // HISENSE_FAC_NEC_AGING
	{ 0xFC40, 587 },             // HISENSE_FAC_NEC_ADC
	{ 0xFC1C, 589 },             // HISENSE_FAC_NEC_3D
#else
	{ 0xFC09, KEY_LEFT },     //box factory ir  
    { 0xFC58, KEY_UP },            
    { 0xFC15, KEY_RIGHT },           
    { 0xFC40, KEY_DOWN},             
    { 0xFC1C, KEY_ENTER},            
#endif
	// box ir
	{ 0xBC0D, KEY_POWER },
    { 0xBC0E, KEY_MUTE },
    { 0xBC11, KEY_AUDIO },      // SOUND_MODE
    { 0xBC13, KEY_ZOOM },       // ASPECT_RATIO
    { 0xBC0B, KEY_CHANNEL },    // CHANNEL_RETURN
    { 0xBC00, KEY_0 },
    { 0xBC01, KEY_1 },
    { 0xBC02, KEY_2 },
    { 0xBC03, KEY_3 },
    { 0xBC04, KEY_4 },
    { 0xBC05, KEY_5 },
    { 0xBC06, KEY_6 },
    { 0xBC07, KEY_7 },
    { 0xBC08, KEY_8 },
    { 0xBC09, KEY_9 },
    { 0xBC14, KEY_MENU },
    { 0xBC48, KEY_BACK },
    { 0xBC16, KEY_UP },
    { 0xBC17, KEY_DOWN },
    { 0xBC18, KEY_RIGHT },
    { 0xBC19, KEY_LEFT },
    { 0xBC15, KEY_ENTER },
    { 0xBC4A, KEY_CHANNELUP },
    { 0xBC4B, KEY_CHANNELDOWN },
    { 0xBC44, KEY_VOLUMEUP },
    { 0xBC43, KEY_VOLUMEDOWN },
    { 0xBC5C, KEY_HOME},
    { 0xBC0C, KEY_INFO },
    { 0xBC1B, KEY_TV },
    { 0xBC5B, KEY_SEARCH },
    { 0xBC1E, KEY_FAVORITES },
    { 0xBC57, KEY_REWIND },
    { 0xBC56, KEY_FORWARD },
    { 0xBC58, KEY_PREVIOUSSONG },
    { 0xBC59, KEY_NEXTSONG },
    { 0xBC5A, KEY_STOP },
    { 0xBC4E, KEY_PLAYPAUSE },
    { 0xBC4C, KEY_FN_F1 },      // MTS
    { 0xBC0F, KEY_FN_F2 },      // FREEZE
    { 0xBC12, KEY_FN_F6 },      // TV_INPUT
    { 0xBC5F, KEY_FN_F7 },      // 3D_MODE
};

static struct rc_map_list hisense_tv_map = {
	.map = {
		.scan    = hisense_tv,
		.size    = ARRAY_SIZE(hisense_tv),
		.rc_type = RC_TYPE_UNKNOWN,	/* Legacy IR type */
		.name    = RC_MAP_HISENSE_TV,
	}
};

static int __init init_rc_map_hisense_tv(void)
{
	return rc_map_register(&hisense_tv_map);
}

static void __exit exit_rc_map_hisense_tv(void)
{
	rc_map_unregister(&hisense_tv_map);
}

module_init(init_rc_map_hisense_tv)
module_exit(exit_rc_map_hisense_tv)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mauro Carvalho Chehab <mchehab@redhat.com>");
