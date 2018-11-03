/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifdef BUILD_LK
    #include <platform/mt_gpio.h>
    #include <string.h>
#else
    #include <linux/string.h>
    #ifdef BUILD_UBOOT
        #include <asm/arch/mt_gpio.h>
    #else
        #include <mach/mt_gpio.h>
    #endif
#endif

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH          (480)
#define FRAME_HEIGHT         (854)

#define LCM_ID_BITLAND       (0) // Main   source: Bitland BT045TN06
#define LCM_ID_TRULY           (1) // Second source: TRULY
#define LCM_ID               LCM_ID_BITLAND // This is main source LCM driver

#define LCM_DSI_CMD_MODE     0 // 0:video; 1:command mode

#define BUF_SIZE             5 // for debug
#define ENABLE_DEBUG         1 // 0:disable; 1:enable

#ifndef TRUE
    #define TRUE  1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

unsigned int lcm_esd_test = FALSE; // only for ESD test

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v) (lcm_util.set_reset_pin((v)))

#define UDELAY(n)        (lcm_util.udelay(n))
#define MDELAY(n)        (lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)    lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                   lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)               lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg                                         lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)            lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#if (ENABLE_DEBUG)
    #if defined(BUILD_LK)
        #include <printf.h>
        #define LCM_DEBUG(format, ...)  printf("[LK][ili9806e_fwvga_dsi_vdo_bitland]" format "\n", ## __VA_ARGS__)
    #elif defined(BUILD_UBOOT)
        #include <printf.h> // or #include <linux/printk.h> ?
        #define LCM_DEBUG(format, ...)  printf("[UBOOT][ili9806e_fwvga_dsi_vdo_bitland]" format "\n", ## __VA_ARGS__)
    #else
        #include <linux/printk.h>
        #define LCM_DEBUG(format, ...)  printk("[kernel][ili9806e_fwvga_dsi_vdo_bitland]" format "\n", ## __VA_ARGS__)
    #endif
#endif

/*
// Debug only in Linux kernel
#if (ENABLE_DEBUG)
    #if defined(BUILD_LK)
    #elif defined(BUILD_UBOOT)
    #else
        static void lcm_read(void);
    #endif
#endif
*/

static struct LCM_setting_table lcm_initialization_setting[] = {
	/**
	 * Note:
	 * Data ID will depends on the following rule:
	 *     count of parameters > 1 => Data ID = 0x39
	 *     count of parameters = 1 => Data ID = 0x15
	 *     count of parameters = 0 => Data ID = 0x05
	 * Structure Format:
	 *     {DCS command, count of parameters, {parameter list}}
	 *     {REGFLAG_DELAY, milliseconds of time, {}},
	 *     ...
	 *     Setting ending by predefined flag
	 *     {REGFLAG_END_OF_TABLE, 0x00, {}}
	 */

        {0xFF, 5, {0xFF, 0x98, 0x06, 0x04, 0x01}}, // Change to Page 1
        {REGFLAG_DELAY, 10, {}},
        {0x08, 1, {0x10}},                         // output SDA
        {REGFLAG_DELAY, 10, {}},
        {0x21, 1, {0x01}},                         // DE = 1 Active
        {REGFLAG_DELAY, 10, {}},
        {0x30, 1, {0x01}},                         // 480 X 854
        {REGFLAG_DELAY, 10, {}},
        {0x31, 1, {0x02}},                         // 2-dot Inversion
        {REGFLAG_DELAY, 10, {}},
        {0x40, 1, {0x15}},                         // BT
        {REGFLAG_DELAY, 10, {}},
        {0x41, 1, {0x33}},                         // DDVDH/DDVDL 33
        {REGFLAG_DELAY, 10, {}},
        {0x42, 1, {0x02}},                         // VGH/VGL
        {REGFLAG_DELAY, 10, {}},
        {0x43, 1, {0x09}},                         // VGH_CP_OFF
        {REGFLAG_DELAY, 10, {}},
        {0x44, 1, {0x09}},                         // VGL_CP_OFF
        {REGFLAG_DELAY, 10, {}},
        {0x50, 1, {0x78}},                         // VGMP
        {REGFLAG_DELAY, 10, {}},
        {0x51, 1, {0x78}},                         // VGMN
        {REGFLAG_DELAY, 10, {}},
        {0x52, 1, {0x00}},                         //Flicker MSB
        {REGFLAG_DELAY, 10, {}},
        {0x53, 1, {0x56}},                         //Flicker LSB (by Panel )
        {REGFLAG_DELAY, 10, {}},
        {0x57, 1, {0x50}},
        {REGFLAG_DELAY, 10, {}},
        {0x60, 1, {0x07}},                         // SDTI
        {REGFLAG_DELAY, 10, {}},
        {0x61, 1, {0x00}},                         // CRTI
        {REGFLAG_DELAY, 10, {}},
        {0x62, 1, {0x08}},                         // EQTI
        {REGFLAG_DELAY, 10, {}},
        {0x63, 1, {0x00}},                         // PCTI
        {REGFLAG_DELAY, 10, {}},
        //++++++++++++++++++ Gamma Setting ++++++++++++++++++
        {0xFF, 5, {0xFF, 0x98, 0x06, 0x04, 0x01}}, // Change to Page 1
        {REGFLAG_DELAY, 10, {}},
        {0xA0, 1, {0x00}},                         // Gamma 0
        {REGFLAG_DELAY, 10, {}},
        {0xA1, 1, {0x04}},                         // Gamma 4
        {REGFLAG_DELAY, 10, {}},
        {0xA2, 1, {0x10}},                         // Gamma 8
        {REGFLAG_DELAY, 10, {}},
        {0xA3, 1, {0x0E}},                         // Gamma 16
        {REGFLAG_DELAY, 10, {}},
        {0xA4, 1, {0x0B}},                         // Gamma 24
        {REGFLAG_DELAY, 10, {}},
        {0xA5, 1, {0x1A}},                         // Gamma 52
        {REGFLAG_DELAY, 10, {}},
        {0xA6, 1, {0x0D}},                         // Gamma 80
        {REGFLAG_DELAY, 10, {}},
        {0xA7, 1, {0x07}},                         // Gamma 108
        {REGFLAG_DELAY, 10, {}},
        {0xA8, 1, {0x01}},                         // Gamma 147
        {REGFLAG_DELAY, 10, {}},
        {0xA9, 1, {0x06}},                         // Gamma 175
        {REGFLAG_DELAY, 10, {}},
        {0xAA, 1, {0x0A}},                         // Gamma 203
        {REGFLAG_DELAY, 10, {}},
        {0xAB, 1, {0x05}},                         // Gamma 231
        {REGFLAG_DELAY, 10, {}},
        {0xAC, 1, {0x0B}},                         // Gamma 239
        {REGFLAG_DELAY, 10, {}},
        {0xAD, 1, {0x32}},                         // Gamma 247
        {REGFLAG_DELAY, 10, {}},
        {0xAE, 1, {0x27}},                         // Gamma 251
        {REGFLAG_DELAY, 10, {}},
        {0xAF, 1, {0x00}},                         // Gamma 255
        {REGFLAG_DELAY, 10, {}},
        //================== Nagitive ======================
        {0xC0, 1, {0x00}},                         // Gamma 0
        {REGFLAG_DELAY, 10, {}},
        {0xC1, 1, {0x07}},                         // Gamma 4
        {REGFLAG_DELAY, 10, {}},
        {0xC2, 1, {0x0F}},                         // Gamma 8
        {REGFLAG_DELAY, 10, {}},
        {0xC3, 1, {0x12}},                         // Gamma 16
        {REGFLAG_DELAY, 10, {}},
        {0xC4, 1, {0x09}},                         // Gamma 24
        {REGFLAG_DELAY, 10, {}},
        {0xC5, 1, {0x17}},                         // Gamma 52
        {REGFLAG_DELAY, 10, {}},
        {0xC6, 1, {0x07}},                         // Gamma 80
        {REGFLAG_DELAY, 10, {}},
        {0xC7, 1, {0x08}},                         // Gamma 108
        {REGFLAG_DELAY, 10, {}},
        {0xC8, 1, {0x05}},                         // Gamma 147
        {REGFLAG_DELAY, 10, {}},
        {0xC9, 1, {0x0B}},                         // Gamma 175
        {REGFLAG_DELAY, 10, {}},
        {0xCA, 1, {0x03}},                         // Gamma 203
        {REGFLAG_DELAY, 10, {}},
        {0xCB, 1, {0x06}},                         // Gamma 231
        {REGFLAG_DELAY, 10, {}},
        {0xCC, 1, {0x0F}},                         // Gamma 239
        {REGFLAG_DELAY, 10, {}},
        {0xCD, 1, {0x26}},                         // Gamma 247
        {REGFLAG_DELAY, 10, {}},
        {0xCE, 1, {0x27}},                         // Gamma 251
        {REGFLAG_DELAY, 10, {}},
        {0xCF, 1, {0x00}},                         // Gamma 255
        {REGFLAG_DELAY, 10, {}},
        //******************** Page 6 Command GIP Timing Setting *********************
        {0xFF, 5, {0xFF, 0x98, 0x06, 0x04, 0x06}},
        {REGFLAG_DELAY, 10, {}},
        {0x00, 1, {0x21}},
        {REGFLAG_DELAY, 10, {}},
        {0x01, 1, {0x06}},
        {REGFLAG_DELAY, 10, {}},
        {0x02, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x03, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x04, 1, {0x01}},
        {REGFLAG_DELAY, 10, {}},
        {0x05, 1, {0x01}},
        {REGFLAG_DELAY, 10, {}},
        {0x06, 1, {0x80}},
        {REGFLAG_DELAY, 10, {}},
        {0x07, 1, {0x02}},
        {REGFLAG_DELAY, 10, {}},
        {0x08, 1, {0x05}},
        {REGFLAG_DELAY, 10, {}},
        {0x09, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x0A, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x0B, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x0C, 1, {0x01}},
        {REGFLAG_DELAY, 10, {}},
        {0x0D, 1, {0x01}},
        {REGFLAG_DELAY, 10, {}},
        {0x0E, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x0F, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x10, 1, {0xF0}},
        {REGFLAG_DELAY, 10, {}},
        {0x11, 1, {0xF4}},
        {REGFLAG_DELAY, 10, {}},
        {0x12, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x13, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x14, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x15, 1, {0xC0}},
        {REGFLAG_DELAY, 10, {}},
        {0x16, 1, {0x08}},
        {REGFLAG_DELAY, 10, {}},
        {0x17, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x18, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x19, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x1A, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x1B, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x1C, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x1D, 1, {0x00}},
        {REGFLAG_DELAY, 10, {}},
        {0x20, 1, {0x02}},
        {REGFLAG_DELAY, 10, {}},
        {0x21, 1, {0x13}},
        {REGFLAG_DELAY, 10, {}},
        {0x22, 1, {0x45}},
        {REGFLAG_DELAY, 10, {}},
        {0x23, 1, {0x67}},
        {REGFLAG_DELAY, 10, {}},
        {0x24, 1, {0x01}},
        {REGFLAG_DELAY, 10, {}},
        {0x25, 1, {0x23}},
        {REGFLAG_DELAY, 10, {}},
        {0x26, 1, {0x45}},
        {REGFLAG_DELAY, 10, {}},
        {0x27, 1, {0x67}},
        {REGFLAG_DELAY, 10, {}},
        {0x30, 1, {0x13}},
        {REGFLAG_DELAY, 10, {}},
        {0x31, 1, {0x22}},
        {REGFLAG_DELAY, 10, {}},
        {0x32, 1, {0x22}},
        {REGFLAG_DELAY, 10, {}},
        {0x33, 1, {0x22}},
        {REGFLAG_DELAY, 10, {}},
        {0x34, 1, {0x22}},
        {REGFLAG_DELAY, 10, {}},
        {0x35, 1, {0xBB}},
        {REGFLAG_DELAY, 10, {}},
        {0x36, 1, {0xAA}},
        {REGFLAG_DELAY, 10, {}},
        {0x37, 1, {0xDD}},
        {REGFLAG_DELAY, 10, {}},
        {0x38, 1, {0xCC}},
        {REGFLAG_DELAY, 10, {}},
        {0x39, 1, {0x66}},
        {REGFLAG_DELAY, 10, {}},
        {0x3A, 1, {0x77}},
        {REGFLAG_DELAY, 10, {}},
        {0x3B, 1, {0x22}},
        {REGFLAG_DELAY, 10, {}},
        {0x3C, 1, {0x22}},
        {REGFLAG_DELAY, 10, {}},
        {0x3D, 1, {0x22}},
        {REGFLAG_DELAY, 10, {}},
        {0x3E, 1, {0x22}},
        {REGFLAG_DELAY, 10, {}},
        {0x3F, 1, {0x22}},
        {REGFLAG_DELAY, 10, {}},
        {0x40, 1, {0x22}},
        {REGFLAG_DELAY, 10, {}},

        {0x52, 1, {0x10}},
        {REGFLAG_DELAY, 10, {}},
        {0x53, 1, {0x10}},
        {REGFLAG_DELAY, 10, {}},

        //****************************************************************************
        {0xFF, 5, {0xFF, 0x98, 0x06, 0x04, 0x07}},
        {REGFLAG_DELAY, 10, {}},
        {0x17, 1, {0x22}},
        {REGFLAG_DELAY, 10, {}},
        {0x02, 1, {0x77}},
        {REGFLAG_DELAY, 10, {}},
        {0xE1, 1, {0x79}},
        {REGFLAG_DELAY, 10, {}},
        {0x68, 1, {0x80}},
        {REGFLAG_DELAY, 10, {}},


        {0xFF, 5, {0xFF, 0x98, 0x06, 0x04, 0x00}},
        {REGFLAG_DELAY, 10, {}},

	// Note:
	// Strongly recommend not to set Sleep Out / Display On here.
	// That will cause messed frame to be shown as later the backlight is on.

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


/*
static struct LCM_setting_table lcm_set_window[] = {
	{0x2A, 4, {0x00, 0x00, (FRAME_WIDTH  >> 8), (FRAME_WIDTH  & 0xFF)}},
	{0x2B, 4, {0x00, 0x00, (FRAME_HEIGHT >> 8), (FRAME_HEIGHT & 0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/


static struct LCM_setting_table lcm_sleep_out_setting[] = {
	// Sleep out
	{0x35, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},

	// Sleep out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	// Display on
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},

        // Change to Page 8 CMD
        {0xFF, 5, {0xFF, 0x98, 0x06, 0x04, 0x08}},
        {REGFLAG_DELAY, 10, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
        // Change to Page 0 CMD
        {0xFF, 5, {0xFF, 0x98, 0x06, 0x04, 0x00}},

        {REGFLAG_DELAY, 1, {}},

        // Change to Page 0 CMD
        {0xFF, 5, {0xFF, 0x98, 0x06, 0x04, 0x00}},

	// Display off
	{0x28, 1, {0x00}},

	{REGFLAG_DELAY, 10, {}},

	// Sleep in
	{0x10, 1, {0x00}},

	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


#if (LCM_DSI_CMD_MODE)
static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for (i=0; i<count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {
		case REGFLAG_DELAY:
			MDELAY(table[i].count);
			break;
		case REGFLAG_END_OF_TABLE :
			break;
		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
#else
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
#endif

	// DSI
	// Command mode setting
	params->dsi.LANE_NUM                = LCM_TWO_LANE;
	// The following defined the format for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size = 256;

	// Video mode setting
	params->dsi.intermediat_buffer_num = 0;
	// Because DSI/DPI HW design change, this parameter should be 0
	// when video mode in MT658X, or it will memory leakage.

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 2;
	params->dsi.vertical_backporch   = 20;
	params->dsi.vertical_frontporch  = 20;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active  = 10;
	params->dsi.horizontal_backporch    = 60;
	params->dsi.horizontal_frontporch   = 80;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	// Required BRPL (Bit Rate per Lane):
	// = (2+20+20+854)*(10+60+200+480)*RGB24bit*60fps / 2 lane
	// = 483,840,000 bit/s

	// Bit rate calculation
	params->dsi.PLL_CLOCK = 247;
	// fvco=2*PLL_CLOCK
        //params->dsi.ssc_range = 3;
        params->dsi.ssc_disable = 1;

}


static void lcm_init(void)
{
	SET_RESET_PIN(1);
	MDELAY(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);

	/*
	// Debug only in Linux kernel
	#if (ENABLE_DEBUG)
	    #if defined(BUILD_LK)
	    #elif defined(BUILD_UBOOT)
	    #else
	        lcm_read();
	    #endif
	#endif
	*/

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);

	/*
	// Debug only in Linux kernel
	#if (ENABLE_DEBUG)
	    #if defined(BUILD_LK)
	    #elif defined(BUILD_UBOOT)
	    #else
	        lcm_read();
	    #endif
	#endif
	*/
}


static void lcm_suspend(void)
{
	//SET_RESET_PIN(1);
	//SET_RESET_PIN(0);
	//MDELAY(10);
	//SET_RESET_PIN(1);
	//MDELAY(20);

	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
	//lcm_init();
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width  - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = ( x0       & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = ( x1       & 0xFF);
	unsigned char y0_MSB = ((y0 >> 8) & 0xFF);
	unsigned char y0_LSB = ( y0       & 0xFF);
	unsigned char y1_MSB = ((y1 >> 8) & 0xFF);
	unsigned char y1_LSB = ( y1       & 0xFF);

	unsigned int data_array[16];

	data_array[0] = 0x00053902;
	data_array[1] = (x1_MSB<<24) | (x0_LSB<<16) | (x0_MSB<<8) | 0x2a;
	data_array[2] = (x1_LSB);
	data_array[3] = 0x00053902;
	data_array[4] = (y1_MSB<<24) | (y0_LSB<<16) | (y0_MSB<<8) | 0x2b;
	data_array[5] = (y1_LSB);
	data_array[6] = 0x002c3909;

	dsi_set_cmdq(data_array, 7, 0);
}
#endif


#if (LCM_DSI_CMD_MODE)
static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 145;
	unsigned int mapped_level  = 0;

	//for LGE backlight IC mapping table
	if (level > 255)
		level = 255;

	if (level >0)
		mapped_level = default_level + (level)*(255-default_level)/(255);
	else
		mapped_level=0;

	// Refresh value of backlight level
	lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}
#endif



static unsigned int lcm_compare_id(void)
{
	int  array[4];
	char buffer[5];
	char id_high = 0;
	char id_low  = 0;
	int  id      = 0;

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(200);

	array[0] = 0x00053700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xa1, buffer, 5);

	id_high = buffer[2];
	id_low  = buffer[3];
	id      = (id_high << 8) | id_low;

/*#ifdef BUILD_UBOOT
	printf("ili9806e_fwvga_dsi_vdo uboot %s \n", __func__);
	printf("%s id = 0x%08x \n", __func__, id);
#else
	printk("ili9806e_fwvga_dsi_vdo kernel %s \n", __func__);
	printk("%s id = 0x%08x \n", __func__, id);
#endif*/

	id = 0;
	mt_set_gpio_mode(GPIO_LCM_ID, GPIO_LCM_ID_M_GPIO); // GPIO_LCM_ID_M_GPIO or GPIO_LCM_ID_M_CLK
	mt_set_gpio_dir(GPIO_LCM_ID, GPIO_DIR_IN); // GPIO_DIR_IN or GPIO_DIR_OUT
	mt_set_gpio_pull_enable(GPIO_LCM_ID, GPIO_PULL_DISABLE); // GPIO_PULL_DISABLE or GPIO_PULL_ENABLE
	//mt_set_gpio_pull_select(GPIO_LCM_ID, GPIO_PULL_DOWN); // GPIO_PULL_DOWN or GPIO_PULL_UP
	id = mt_get_gpio_in(GPIO_LCM_ID);
	LCM_DEBUG("[%s] LCM_ID: %d", __func__, id); // Debug

	return (id == LCM_ID) ? 1 : 0;
}


/*
// Debug only in Linux kernel
#if (ENABLE_DEBUG)
    #if defined(BUILD_LK)
    #elif defined(BUILD_UBOOT)
    #else
static void lcm_read(void)
{
	unsigned char buf[BUF_SIZE]={0};

	// Get display id
	memset(buf, 0xFF, BUF_SIZE);
	read_reg_v2(0x04, buf, 3);
	LCM_DEBUG("[%s] display id: (0x%02X, 0x%02X, 0x%02X)", __func__, buf[0], buf[1], buf[2]);

	// Get display status
	memset(buf, 0xFF, BUF_SIZE);
	read_reg_v2(0x09, buf, 4);
	LCM_DEBUG("[%s] display status: (0x%02X, 0x%02X, 0x%02X, 0x%02X)", __func__, buf[0], buf[1], buf[2], buf[3]);

	// Get power mode
	memset(buf, 0xFF, BUF_SIZE);
	read_reg_v2(0x0A, buf, 1);
	LCM_DEBUG("[%s] power mode: (0x%02X)", __func__, buf[0]);

	// Get display mode
	memset(buf, 0xFF, BUF_SIZE);
	read_reg_v2(0x0D, buf, 2);
	LCM_DEBUG("[%s] display mode: (0x%02X, 0x%02X)", __func__, buf[0], buf[1]);

	// Get signal mode
	memset(buf, 0xFF, BUF_SIZE);
	read_reg_v2(0x0E, buf, 2);
	LCM_DEBUG("[%s] signal mode: (0x%02X, 0x%02X)", __func__, buf[0], buf[1]);

	// Get diagnostic result
	memset(buf, 0xFF, BUF_SIZE);
	read_reg_v2(0x0F, buf, 2);
	LCM_DEBUG("[%s] diagnostic result: (0x%02X, 0x%02X)", __func__, buf[0], buf[1]);
}
    #endif
#endif
*/

static unsigned int lcm_esd_check(void)
{
   unsigned int ret = FALSE;
#ifndef BUILD_LK
   char buf[6];
   int  array[4];

   if (lcm_esd_test) {
	lcm_esd_test = FALSE;
	return TRUE;
   }

   UDELAY(600); // avoid splashing occasionally

   /*
   // Change to Page 7 CMD
   array[0] = 0x00063902;
   array[1] = 0x0698ffff;
   array[2] = 0x00000704;
   dsi_set_cmdq(array, 3, 1);

   // Change from HS to LP mode
   array[0] = 0x00681500;
   dsi_set_cmdq(array, 1, 1);
   */

   // Change to Page 0 CMD
   array[0] = 0x00063902;
   array[1] = 0x0698ffff;
   array[2] = 0x00000004;
   dsi_set_cmdq(array, 3, 1);

   // Set maximal return packet length = 1 (byte)
   array[0] = 0x00013700;
   dsi_set_cmdq(array, 1, 1);

   read_reg_v2(0x0A, buf, 2);
   printk("[ili9806e_fwvga_dsi_vdo] %s: buf0=%02X, buf1=%02X\n", __func__, buf[0], buf[1]);
   if (buf[0] == 0x9C) {
	ret = FALSE;
   } else {
	ret = TRUE;
   }

   // Change to Page 8 CMD
   array[0] = 0x00063902;
   array[1] = 0x0698ffff;
   array[2] = 0x00000804;
   dsi_set_cmdq(array, 3, 1);
#endif
   return ret;
}


static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK
   lcm_init();
   printk("[ili9806e_fwvga_dsi_vdo_bitland] %s: do lcm_init!\n", __func__);
#endif
   return TRUE;
}

LCM_DRIVER ili9806e_fwvga_dsi_vdo_drv_lx8x =
{
	.name           = "ili9806e_fwvga_dsi_vdo_lx8x",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
        .esd_check      = lcm_esd_check,
        .esd_recover    = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
	.set_backlight  = lcm_setbacklight,
	.update         = lcm_update,
#endif
};
