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

#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (854)

#define REGFLAG_DELAY             							0xAB
#define REGFLAG_END_OF_TABLE      							0xAA   // END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define LCM_ID       (0x55)

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
static unsigned int lcm_compare_id(void);
struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    
    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 10, {}},
    
    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};
static struct LCM_setting_table lcm_initialization_setting[] = {

//ENABLE CMD2 Page 1//
{ 0xF0 ,5,{0x55,0xAA,0x52,0x08,0x01}},

//#VGMP/VGMN/VOOM Setting, VGMP=3.9V  #VGSP=0.6125V
{ 0xBC,3,{0x00,0x48,0x1A}},
{ 0xBD,3,{0x00,0x48,0x1A}},

//#VCOM=
{ 0xBE,2,{0x00,0x31 }},

//# R+            
{ 0xD1,52,{0x00,0x00,0x00,0x23,0x00,0x3C,0x00,0x58,0x00,0x73,0x00,0xA1,0x00,0xC6,0x01,0x02,0x01,0x33,0x01,0x71,0x01,0xA4,0x01,0xF1,0x02,0x2E,0x02,0x2F,0x02,0x65,0x02,0x9C,0x02,0xBA,0x02,0xDF,0x02,0xF5,0x03,0x12,0x03,0x24,0x03,0x3C,0x03,0x4D,0x03,0x64,0x03,0x9A,0x03,0xFF}},

//#G +            
{ 0xD2,52,{0x00,0x00,0x00,0x23,0x00,0x3C,0x00,0x58,0x00,0x73,0x00,0xA1,0x00,0xC6,0x01,0x02,0x01,0x33,0x01,0x71,0x01,0xA4,0x01,0xF1,0x02,0x2E,0x02,0x2F,0x02,0x65,0x02,0x9C,0x02,0xBA,0x02,0xDF,0x02,0xF5,0x03,0x12,0x03,0x24,0x03,0x3C,0x03,0x4D,0x03,0x64,0x03,0x9A,0x03,0xFF}},
                 
//#B +            
{ 0xD3,52,{0x00,0x00,0x00,0x23,0x00,0x3C,0x00,0x58,0x00,0x73,0x00,0xA1,0x00,0xC6,0x01,0x02,0x01,0x33,0x01,0x71,0x01,0xA4,0x01,0xF1,0x02,0x2E,0x02,0x2F,0x02,0x65,0x02,0x9C,0x02,0xBA,0x02,0xDF,0x02,0xF5,0x03,0x12,0x03,0x24,0x03,0x3C,0x03,0x4D,0x03,0x64,0x03,0x9A,0x03,0xFF}},

//# R-            
{ 0xD4,52,{0x00,0x00,0x00,0x23,0x00,0x3C,0x00,0x58,0x00,0x73,0x00,0xA1,0x00,0xC6,0x01,0x02,0x01,0x33,0x01,0x71,0x01,0xA4,0x01,0xF1,0x02,0x2E,0x02,0x2F,0x02,0x65,0x02,0x9C,0x02,0xBA,0x02,0xDF,0x02,0xF5,0x03,0x12,0x03,0x24,0x03,0x3C,0x03,0x4D,0x03,0x64,0x03,0x9A,0x03,0xFF}},
                 
//#G -            
{ 0xD5,52,{0x00,0x00,0x00,0x23,0x00,0x3C,0x00,0x58,0x00,0x73,0x00,0xA1,0x00,0xC6,0x01,0x02,0x01,0x33,0x01,0x71,0x01,0xA4,0x01,0xF1,0x02,0x2E,0x02,0x2F,0x02,0x65,0x02,0x9C,0x02,0xBA,0x02,0xDF,0x02,0xF5,0x03,0x12,0x03,0x24,0x03,0x3C,0x03,0x4D,0x03,0x64,0x03,0x9A,0x03,0xFF}},
                 
//#B -           
{ 0xD6,52,{0x00,0x00,0x00,0x23,0x00,0x3C,0x00,0x58,0x00,0x73,0x00,0xA1,0x00,0xC6,0x01,0x02,0x01,0x33,0x01,0x71,0x01,0xA4,0x01,0xF1,0x02,0x2E,0x02,0x2F,0x02,0x65,0x02,0x9C,0x02,0xBA,0x02,0xDF,0x02,0xF5,0x03,0x12,0x03,0x24,0x03,0x3C,0x03,0x4D,0x03,0x64,0x03,0x9A,0x03,0xFF}},

//Set AVDD Voltage//
{ 0xB0,3,{0x02,0x02,0x02}},

//Set AVEE voltage//
{ 0xB1,3,{0x00,0x00,0x00}},
  
//AVDD=2.5xVDCI//
{ 0xB6,3,{0x36,0x36,0x36 }},   

//AVEE= -2.5xVDCI//
{ 0xB7,3,{0x26,0x26,0x26}},
 
{ 0xB8,3,{0x26,0x26,0x26 }},                          

//Set VGH//
//#VGH, Set//  
{ 0xB9,3,{0x34,0x34,0x34}},

//Set VGL_REG=                             
//#VGLX, Set VGL// 
{ 0xBA,3,{0x16,0x16,0x16}},
                  
//########################
//#ENABLE CMD2 Page 0               
{ 0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
//########################       

//#RAM Keep                    
{ 0xB1,1,{0xFC}},             

//#Vivid Color
{ 0xB4,1,{0x10}},       

//## SDT:
{ 0xB6,1,{0x04}},

//#Set Gate EQ        
{ 0xB7,2,{0x74,0x74}},

//#Set Source EQ        
{ 0xB8,4,{0x01,0x07,0x07,0x07}},
                       
//#Inversion Control                
{ 0xBC,3,{0x02,0x02,0x02}},            

//#Porch Adjust                
{ 0xBD,5,{0x01,0x84,0x07,0x31,0x00 }},                                           
{ 0xBE,5,{0x01,0x84,0x07,0x31,0x00  }},                                     
{ 0xBF,5,{0x01,0x84,0x07,0x31,0x00  }},          

//{ 0x36,1,{0xc0}},   
//#TE ON                       
{ 0x35,1,{0x00}},             
                             
//#StartUp                     
{ 0x11,1,{0x00 }},         
 {REGFLAG_DELAY, 120, {}},

{ 0x3A,1,{0x77 }},  

//#ENABLE CMD2 Page 0               
{ 0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},

{ 0xC7,1,{0x02}},
{ 0xC9,1,{0x11}},


//FScan// 
{ 0xCA, 12,{0x01, 0xE4, 0xE4, 0xE4, 0xE4, 0xE4, 0xE4, 0xE4, 0x08, 0x08, 0x00, 0x00}},
//CRL=0// 
{ 0xB1, 2,{0xFC, 0x00}},
//480 x 854//
{ 0xB5,1,{0x6B}},

//BScan//
//{ 0xCA, 0x01, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x08, 0x08, 0x00, 0x01
//CRL=1// 
//{ 0xB1, 0xFC, 0x02

//CMD3 for NT35512//
{ 0xFF,4,{0xAA,0x55,0xA5,0x80}},
{ 0xF7,15,{0x63,0x40,0x00,0x00,0x00,0x01,0xC4,0xA2,0x00,0x02,0x64,0x54,0x48,0x00,0xD0}},
{ 0xFF,4,{0xAA,0x55,0xA5,0x00}},

//#Display On                
{ 0x29,1,{0x00}},

{REGFLAG_END_OF_TABLE, 0x00, {}}


};
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
            /*
                if (cmd != 0xFF && cmd != 0x2C && cmd != 0x3C) {
                    //#if defined(BUILD_UBOOT)
                    //	printf("[DISP] - uboot - REG_R(0x%x) = 0x%x. \n", cmd, table[i].para_list[0]);
                    //#endif
                    while(read_reg(cmd) != table[i].para_list[0]);		
                }
				*/
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
    
    // enable tearing-free
    params->dbi.te_mode 			= LCM_DBI_TE_MODE_DISABLED;//LCM_DBI_TE_MODE_DISABLED;
   // params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;
    
    params->dsi.mode   = BURST_VDO_MODE;//CMD_MODE;
    
    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_TWO_LANE;

    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
    
    params->dsi.intermediat_buffer_num = 2;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
    
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    
    params->dsi.word_count=480*3;	//DSI CMD mode need set these two bellow params, different to 6577
    params->dsi.vertical_active_line=854;
  
     params->dsi.vertical_sync_active				= 2;//6;
    params->dsi.vertical_backporch					= 20;// 70;
    params->dsi.vertical_frontporch					= 20;//70;	
    params->dsi.vertical_active_line				= FRAME_HEIGHT;
    
    params->dsi.horizontal_sync_active				= 2;//6;
    params->dsi.horizontal_backporch				= 80;//22;
    params->dsi.horizontal_frontporch				= 80;//120;
    params->dsi.horizontal_blanking_pixel				= 60;
    //params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
    params->dsi.compatibility_for_nvk = 0;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's
/*
	params->dsi.HS_TRAIL = 7;
        params->dsi.HS_ZERO = 6;
        params->dsi.HS_PRPR = 4;
        params->dsi.LPX = 13;
        params->dsi.TA_SACK = 1;
        params->dsi.TA_GET = 15;
        params->dsi.TA_SURE = 4;
        params->dsi.TA_GO = 12;
        params->dsi.CLK_TRAIL = 13;
        params->dsi.CLK_ZERO = 12;
        params->dsi.LPX_WAIT = 10;
        params->dsi.CONT_DET = 0;
        params->dsi.CLK_HS_PRPR = 4;
*/
    // Bit rate calculation

    params->dsi.pll_div1=1;		// div1=0,1,2,3;div1_real=1,2,4,4
    params->dsi.pll_div2=0;		// div2=0,1,2,3;div2_real=1,2,4,4
    params->dsi.fbk_div =14;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		

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

#ifdef BUILD_LK
	printf("MYCAT nt35512 lk lcm_init\n");
#else
      printk("MYCAT nt35512  kernel lcm_init\n");
#endif
 lcm_compare_id();
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(120);
push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
 //   init_lcm_registers();
// lcm_compare_id();
}


static void lcm_suspend(void)
{
    unsigned int data_array[16];
#if defined(BUILD_LK)

#else
      data_array[0] = 0x00002200;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(20);
#endif

    data_array[0] = 0x00280500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(10);

    data_array[0] = 0x00100500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(120);

  

   // data_array[0] = 0x014F1500;
   // dsi_set_cmdq(data_array, 1, 1);
   // MDELAY(40);
}


static void lcm_resume(void)
{
	
		//add by xia lei jie for LCD INIT
		lcm_init();
	
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







#define GPIO_LCD_PIN_ID GPIO97
static unsigned int lcm_compare_id(void)
{
	int i;
#if defined(BUILD_LK)
	upmu_set_rg_vgp2_vosel(5);
	upmu_set_rg_vgp2_en(1);

	upmu_set_rg_vgp3_vosel(3);
	upmu_set_rg_vgp3_en(1);	
#else
	hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_2800, "Lance_LCM");
       hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_1800, "Lance_LCM");
#endif
  MDELAY(100);
  	mt_set_gpio_mode(GPIO_LCD_PIN_ID, GPIO_MODE_00);
	 mt_set_gpio_dir(GPIO_LCD_PIN_ID, GPIO_DIR_IN);
	//mt_set_gpio_pull_enable(GPIO_LCD_PIN_ID, 1);
	//mt_set_gpio_pull_select(GPIO_LCD_PIN_ID, GPIO_PULL_UP);	
  	
 	i=mt_get_gpio_in(GPIO_LCD_PIN_ID);  
#ifdef BUILD_LK
	printf("MYCAT NT35512 lk  lcm_compare_id=%d\n",i);
#else
     printk("MYCAT NT35512 kernel  lcm_compare_id=%d\n",i);
#endif
	if(i == 1)
		return 1;
	else
    		return 1;
}


// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER nt35512_hansheng_hs5053fwnc25_vdo_fwvga_lcm_drv = 
{
    .name			= "nt35512_hansheng",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,

    .compare_id    = lcm_compare_id,
   // .update         = lcm_update
};
