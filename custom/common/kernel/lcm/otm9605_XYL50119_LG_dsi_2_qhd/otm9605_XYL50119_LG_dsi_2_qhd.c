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
#include <platform/mt_pmic.h>
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
    #include <linux/string.h>
   
#endif
#include "lcm_drv.h"
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (540)
#define FRAME_HEIGHT (960)

#define LCM_DSI_CMD_MODE		0
#define LCM_ID       (0x9605)



// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define REGFLAG_DELAY             							0XFFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)     

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};
static unsigned int lcm_compare_id(void);
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},

	{REGFLAG_DELAY, 50, {}},
	{0x4F, 1, {0x01}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_initialization_setting[] = {

//  LG500+OTM9605-MIPI-VIDEO-V1.0

{0x00,1,{0x00}},
{0xFF,3,{0x96,0x05,0x01}},
	
{0x00,1,{0x80}},
{0xFF,2,{0x96,0x05}},
	
{0x00,1,{0x92}}, // mipi 2 lane
{0xFF,2,{0x10,0x02}},
	
{0x00,1,{0xB4}},	
{0xC0,1,{0x10}},//inversion  50

//{0x00,1,{0xB5}},	
//{0xC0,1,{0x48}},//inversion  50

{0x00,1,{0x80}},
{0xC1,2,{0x36,0x66}}, //70Hz  77
	
{0x00,1,{0x89}},
{0xC0,1,{0x01}},// TCON OSC turbo mode
	
{0x00,1,{0xA0}},
{0xC1,1,{0x00}},//02
{0x00,1,{0x80}},
{0xC5,4,{0x08,0x00,0xA0,0x11}},
	
{0x00,1,{0x90}},
{0xC5,3,{0xD6,0x57,0x01}},	//VGH=14V, VGL=-11V

{0x00,1,{0xB0}},
{0xC5,2,{0x05,0xac}}, //05 28

{0x00,1,{0x00}},	//GVDD=5.2V/NGVDD=-5.2V
{0xD8,2,{0xa7,0xa7}},//85  85

{0x00,1,{0x00}},	//Vcom setting
{0xD9,1,{0x64}},//47  64  5a  56//************

{0x00,1,{0x80}},//************ 
{0xC4,1,{0x9C}},

{0x00,1,{0x87}},//**************** 
{0xC4,1,{0x40}},

{0x00,1,{0xA6}},
{0xC1,1,{0x01}},

{0x00,1,{0xA2}}, // pl_width, pch_dri_pch_nop
{0xC0,3,{0x0C,0x05,0x02}},
	
{0x00,1,{0x80}},	//GOA mapping
{0xCB,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x90}},	//GOA mapping
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},    

{0x00,1,{0xA0}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},    

{0x00,1,{0xB0}},	
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0xC0}},
{0xCB,15,{0x04,0x04,0x04,0x04,0x08,0x04,0x08,0x04,0x08,0x04,0x08,0x04,0x04,0x04,0x08}},   

{0x00,1,{0xD0}},	
{0xCB,15,{0x08,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x08,0x04,0x08,0x04,0x08,0x04}},    

{0x00,1,{0xE0}}, 
{0xCB,10,{0x08,0x04,0x04,0x04,0x08,0x08,0x00,0x00,0x00,0x00}},

{0x00,1,{0xF0}}, 
{0xCB,10,{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}},

{0x00,1,{0x80}},
{0xCC,10,{0x26,0x25,0x21,0x22,0x00,0x0F,0x00,0x0D,0x00,0x0B}},

{0x00,1,{0x90}},
{0xCC,15,{0x00,0x09,0x01,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x25,0x21,0x22,0x00}},    

{0x00,1,{0xA0}},
{0xCC,15,{0x10,0x00,0x0E,0x00,0x0C,0x00,0x0A,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00}},

{0x00,1,{0x80}},//GOA VST Setting
{0xCE,12,{0x8B,0x03,0x06,0x8A,0x03,0x06,0x89,0x03,0x06,0x88,0x03,0x06}},   								

{0x00,1,{0x90}}, //GOA VEND and Group Setting
{0xCE,14,{0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0x00,0x00}},    			

{0x00,1,{0xA0}},	//GOA CLK1 and GOA CLK2 Setting
{0xCE,14,{0x38,0x03,0x03,0xC0,0x00,0x06,0x00,0x38,0x02,0x03,0xC1,0x00,0x06,0x00}},   			

{0x00,1,{0xB0}},	//GOA CLK3 and GOA CLK4 Setting/Address Shift
{0xCE,14,{0x38,0x01,0x03,0xC2,0x00,0x06,0x00,0x38,0x00,0x03,0xC3,0x00,0x06,0x00}},    			

{0x00,1,{0xC0}},	//GOA CLKB1 and GOA CLKB2 Setting
{0xCE,14,{0x38,0x07,0x03,0xBC,0x00,0x06,0x00,0x38,0x06,0x03,0xBD,0x00,0x06,0x00}},   			

{0x00,1,{0xD0}},	//GOA CLKB3 and GOA CLKB4 Setting
{0xCE,14,{0x38,0x05,0x03,0xBE,0x00,0x06,0x00,0x38,0x04,0x03,0xBF,0x00,0x06,0x00}},    			

{0x00,1,{0x80}},	//GOA CLKC1 and GOA CLKC2 Setting
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},   			

{0x00,1,{0x90}},	//GOA CLKC3 and GOA CLKC4 Setting
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},    			

{0x00,1,{0xA0}},	//GOA CLKD1 and GOA CLKD2 Setting
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},    			

{0x00,1,{0xB0}},	//GOA CLKD3 and GOA CLKD4 Setting
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},    			

{0x00,1,{0xC0}},	//GOA ECLK Setting and GOA Other Options1 and GOA Signal Toggle Option Setting
{0xCF,10,{0x02,0x02,0x20,0x20,0x00,0x00,0x01,0x02,0x00,0x02}},

{0x00,1,{0x00}},
{0xE1,16,{0x00,0x0B,0x11,0x10,0x07,0x0E,0x09,0x07,0x05,0x06,0x12,0x08,0x0f,0x0d,0x08,0x03}},//G2.2 POS				

{0x00,1,{0x00}},
{0xE2,16,{0x00,0x0B,0x11,0x10,0x07,0x0E,0x09,0x07,0x05,0x06,0x12,0x08,0x0f,0x0d,0x03,0x03}},//G2.2 POS					
	


{0x00,1,{0xb1}}, 
{0xC5,1,{0x28}},//VDD18 

{0x00,1,{0xB2}},
{0xF5,4,{0x15,0x00,0x15,0x00}},//VRGH disable 

{0x00,1,{0xC0}},
{0xC5,1,{0x00}},//thermo disable 

{0x00,1,{0x80}},//Source output levels during porch and non-display area 
{0xC4,1,{0x9C}}, 

{0x00,1,{0x00}}, 
{0xff,3,{0xff,0xff,0xff}},

{0x36,1,{0xc0}},
//{REGFLAG_DELAY,20,{}}, 

{0x11,1,{0x00}},//SLEEP OUT
{REGFLAG_DELAY,120,{}},
                                 				                                                                                
{0x29,1,{0x00}},//Display ON 
{REGFLAG_DELAY,20,{}},
};

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

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 2;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 4;//2
		params->dsi.vertical_backporch					= 16;//50
		params->dsi.vertical_frontporch					= 15;//20
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;//2
		params->dsi.horizontal_backporch				= 64;//100
		params->dsi.horizontal_frontporch				= 64;//100
		params->dsi.horizontal_blanking_pixel			= 60;   //add
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
    params->dsi.pll_div1=1;
    params->dsi.pll_div2=1;
		 params->dsi.fbk_sel=1;	//add
    params->dsi.fbk_div=30;
}


static void lcm_init(void)
{

#if defined(BUILD_LK)
	upmu_set_rg_vgp2_vosel(5);
	upmu_set_rg_vgp2_en(1);

	upmu_set_rg_vgp3_vosel(3);
	upmu_set_rg_vgp3_en(1);	
#else
	hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_2800, "Lance_LCM");
	hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_1800, "Lance_LCM");
#endif

    SET_RESET_PIN(1);
    MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(30);
    SET_RESET_PIN(1);
    MDELAY(120);

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{

    push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);

}

static void lcm_resume(void)
{
#if 1
    lcm_init();
#else
    push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}
static unsigned int lcm_compare_id(void)
{
		int   array[4];
		char  buffer[5];
		unsigned int id=0;

#if defined(BUILD_LK)
	 upmu_set_rg_vgp2_vosel(5);
	 upmu_set_rg_vgp2_en(1);

	 upmu_set_rg_vgp3_vosel(3);
	 upmu_set_rg_vgp3_en(1);	
#else
	 hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_2800, "Lance_LCM");
	 hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_1800, "Lance_LCM");
#endif

	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(30);
	SET_RESET_PIN(1);
	MDELAY(120);
		
	array[0] = 0x00083700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xA1,buffer,4);
	id=(buffer[2]<<8)+buffer[3];
#ifdef BUILD_LK
	printf("mycat lcd id: 0x%08x\n", id);
#else
	printk("mycat lcd id: 0x%08x\n", id);
#endif        
	 return (LCM_ID == id)?1:0;
}


LCM_DRIVER otm9605_XYL50119_LG_dsi_2_qhd_lcm_drv = 
{
    .name			= "otm9605_XYL50119_LG_dsi_2_qhd",
	.set_util_funcs = lcm_set_util_funcs,
	.compare_id     = lcm_compare_id,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
//    .esd_check      = lcm_esd_check,
//    .esd_recover    = lcm_esd_recover,
#endif
//		.check_status = lcm_check_status

    };