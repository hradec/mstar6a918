//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <exports.h>
#include <environment.h>
#include <errno.h>
#include <MsTypes.h>
#include <MsDebug.h>
#include <MsRawIO.h>
#include <MsEnvironment.h>
#include <MsMmap.h>
#include <MsUtility.h>

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define MMAP_ID_LXMEM "E_LX_MEM"
#define MMAP_ID_EMAC "E_EMAC_MEM"
#define MMAP_ID_LXMEM2 "E_LX_MEM2"
#define MMAP_ID_GE "E_MST_GEVQ"
#define MMAP_ID_GOP "E_MST_GOP_REGDMA"
#define MMAP_ID_PM51_USAGE "E_MMAP_ID_PM51_USAGE_MEM"
#define MMAP_ID_FLUSH_BUFFER "E_MMAP_ID_FLUSH_BUFFER"
#define MMAP_ID_PM51_CODE "E_MMAP_ID_PM51_CODE_MEM"
#define MMAP_ID_XC_MAIN_FB "E_MMAP_ID_XC_MAIN_FB"
#define MMAP_ID_FRC_4K2K "E_MMAP_ID_FRC_4K2K"


#define ENV_DRAM_LEN "DRAM_LEN"
#define ENV_LX_MEM_LEN "LX_MEM"
#define ENV_EMAC_MEM_LEN "EMAC_MEM"
#define ENV_LX_MEM2 "LX_MEM2"
#define ENV_FLUSH_BUF_ADDR "BB_ADDR"
#define ENV_PM51_ADDR "PM51_ADDR"
#define ENV_PM51_LEN "PM51_LEN"



#define ENV_GE_MEM "GE_ADDR"
#define ENV_GE_LEN "GE_LEN"
#define ENV_GOP_MEM "GOP_ADDR"
#define ENV_GOP_LEN "GOP_LEN"

#define ENV_SYNC_MMAP "sync_mmap"

#if defined(__ARM__)
#define MIU1_LOGIC_ADR_OFFSET 0xA0000000
#else
#define MIU1_LOGIC_ADR_OFFSET 0x60000000
#endif

#if (CONFIG_OAD_IN_MBOOT)
#define OAD_IN_MBOOT         "OAD_IN_MBOOT"
#endif

#define ENV_51ONRAM         "51OnRam"
#if ((ENABLE_BOOTING_FROM_EXT_SPI_WITH_PM51==1)||(ENABLE_BOOTING_FROM_OTP_WITH_PM51==1))
#define ENV_51ONRAM_CFG         "1"
#else
#define ENV_51ONRAM_CFG         "0"
#endif


char mmapID[][MAX_MMAP_ID_LEN]={
        "E_LX_MEM",
        "E_LX_MEM2",
        "E_LX_MEM3",               
        "E_MMAP_ID_NUTTX_MEM",
        "E_MMAP_ID_HW_AES_BUF",
        "E_MMAP_ID_PM51_USAGE_MEM",
        "E_MMAP_ID_MAD_R2",
        "E_MMAP_ID_MAD_SE",
        "E_MMAP_ID_MAD_DEC",
        "E_MMAP_ID_JPD_READ",
        "E_MMAP_ID_JPD_WRITE",
        "E_MMAP_ID_PHOTO_INTER",
        "E_DFB_FRAMEBUFFER",
        "E_MMAP_ID_PM51_CODE_MEM",
        "E_MMAP_ID_XC_MAIN_FB",
        "E_MMAP_ID_FRC_4K2K",
        "E_MMAP_ID_VE",
		"E_MMAP_ID_VDEC_CPU",
#if (ENABLE_MSTAR_PUMABOOT)        
        "E_MMAP_ID_VDEC_CPU", // VDEC_AEON        
        "E_MMAP_ID_VDEC_FRAMEBUFFER", // VDEC_FRAMEBUFFER        
        "E_MMAP_ID_VDEC_BITSTREAM", // VDEC_BITSTREAM        
        "E_MMAP_ID_SCALER_DNR_SUB_BUF", // SCALER_DNR        
        "E_MMAP_ID_XC_MLOAD", // MENULOAD_BUFFER 
#endif
        ""};
#if CONFIG_RESCUE_ENV
    #if (ENABLE_MODULE_ENV_IN_SERIAL)
    #define SECTOR_SIZE   0x10000
    #define ENV_SECTOR_SIZE   SECTOR_SIZE    
    #elif (ENABLE_MODULE_ENV_IN_NAND)
    extern  int ubi_get_volume_size(char *, size_t* );
    extern int ubi_get_leb_size(void);
    #define SECTOR_SIZE   ubi_get_leb_size()
    #define ENV_SECTOR_SIZE   SECTOR_SIZE      
    #elif (ENABLE_MODULE_ENV_IN_MMC)
    #define SECTOR_SIZE   0x200
    #define ENV_SECTOR_SIZE   0x10000    
    #endif
#endif  

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
extern unsigned int cfg_env_offset;
//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Extern Functions
//-------------------------------------------------------------------------------------------------
extern int snprintf(char *str, size_t size, const char *fmt, ...);
//-------------------------------------------------------------------------------------------------
//  Private Functions
//-------------------------------------------------------------------------------------------------
static int del_cfg(char *source,char *delCfg);
static int set_info_exchange_cfg(void);
#if CONFIG_MINIUBOOT
#else
static int set_security_cfg(void);
static int set_panelinit_cfg(void);
static void if_51OnRam_set(void);
#endif
static int set_env_cfg(void);
int syncMmap2bootargs(void);
static int _syncLen2bootargs(char *id, char *envName);
static int _syncAddr2bootargs(char *id, char *envName);
int delMmapInfoFromEnv(void);
int saveMmapInfoToEnv(void);
int syncMmapToEnv(void);
int processMmapInfoOnEnv(BOOLEAN save2env);
void Confirm_LX_Infomation(void);

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

static int del_cfg(char *source,char *delCfg)
{
    char *substr = NULL;
    char *pPreEnvEnd = NULL;
   
    UBOOT_TRACE("IN\n");
    if(source==NULL)
    {
        UBOOT_ERROR("The input parameter 'source' is a null pointer\n");
        return -1;
    }

    if(delCfg==NULL)
    {
        UBOOT_ERROR("The input parameter 'delCfg' is a null pointer\n");
        return -1;
    }

    if(strlen(source)==0)
    {
        UBOOT_ERROR("The length of source is zero\n");
        return -1;
    }

       
    substr=strstr(source,delCfg);
    if(substr == NULL)
    {
        return 1;
    }
    else
    {
        pPreEnvEnd = strchr(substr,' ');
        if(((unsigned int)pPreEnvEnd-(unsigned int)source+1)<strlen(source))
        {
            strcpy(substr,pPreEnvEnd+1); //the +1 is for skip space
        }
        else
        {
            UBOOT_DEBUG("This member is the last one in the bootargs\n");
            //clear the rst of size. 
            *(substr-1)='\0';
            memset(substr,0,strlen(substr));
        }
    }        
    UBOOT_TRACE("OK\n");
    return 0;
}

int do_del_boogargs_cfg (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   int ret;
   UBOOT_TRACE("IN\n");
   if(argc!=2)
   {
        return cmd_usage(cmdtp);
   }

   ret = del_bootargs_cfg(argv[1]);
   if(ret==0)
   {
        UBOOT_TRACE("OK\n");
   }
   else
   {
        UBOOT_ERROR("delete %s fail\n",argv[1]);
   }
   return ret;
}

int del_bootargs_cfg(char *delCfg)
{
    int ret=0;
    char *bootarg=NULL;
    char *OriArg = NULL;
    UBOOT_TRACE("IN\n");
    if(delCfg==NULL)
    {
        UBOOT_ERROR("The input parameter 'delCfg' is a null pointer\n");
        return -1;
    }

    bootarg = getenv("bootargs");
    if(bootarg==NULL)
    {
        UBOOT_ERROR("No env 'bootargs'\n");
        return -1;
    }

    OriArg=malloc(strlen(bootarg)+1);
    if(OriArg==NULL)
    {
        UBOOT_ERROR("malloc for tempBuf fail\n");
        return -1;
    }
    memset(OriArg,0,strlen(bootarg)+1);
    strcpy(OriArg,bootarg);
        
    ret=del_cfg(OriArg,delCfg);
    if(ret==0)
    {
        UBOOT_DEBUG("OriArg=%s\n",OriArg);
        setenv("bootargs", OriArg);
        saveenv();
        UBOOT_TRACE("OK\n");
    }
    else if(ret==1)
    {
        UBOOT_DEBUG("No[%s] in bootargs\n",delCfg);
    }
    else
    {
        UBOOT_ERROR("delete %s fail\n",delCfg);
        return -1;
    }
    free(OriArg);    
    return 0;
}

int do_set_bootargs_cfg (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   int ret;
   unsigned int prefixLen=0;
   unsigned int contentLen=0;
   char *buffer=NULL;
   UBOOT_TRACE("IN\n");
   if(argc!=3)
   {
        return cmd_usage(cmdtp);
   }
   prefixLen=strlen(argv[1]);
   contentLen=strlen(argv[2]);
   //2? for '=' and the end symbol
   #define BUFFER_SIZE (prefixLen+contentLen+2)
   buffer=malloc(BUFFER_SIZE);
   if(buffer==NULL)
   {
       UBOOT_ERROR("malloc fail\n");
       return -1;
   }
   snprintf(buffer,BUFFER_SIZE,"%s=%s",argv[1],argv[2]);
   UBOOT_DEBUG("buffer: %s\n",buffer);
   ret = set_bootargs_cfg(argv[1],buffer,0);
   if(ret==0)
   {
        UBOOT_TRACE("OK\n");
   }
   else
   {
        UBOOT_ERROR("delete %s fail\n",argv[1]);
   }
   free(buffer);
   #undef BUFFER_SIZE
   return ret;
}

int set_bootargs_cfg(char * prefix_cfg,char *setCfg,MS_BOOL bDontSaveEnv)
{
    char *preCheck=NULL;
    char *bootargs = NULL;
    char *OriArg = NULL;
    char *NewArg = NULL;
    int NewArgLen = 0;
    UBOOT_TRACE("IN\n");

    if(prefix_cfg==NULL)
    {
        UBOOT_ERROR("The input parameter 'prefix_cfg' is a null pointer\n");
        return -1;
    }

    if(setCfg==NULL)
    {
        UBOOT_ERROR("The input parameter 'setCfg' is a null pointer\n");
        return -1;
    }

    //get origin bootargs
    bootargs = getenv("bootargs");
    if (bootargs == NULL)
    {
        UBOOT_ERROR("bootargs doesn't exist\n");
        return -1;
    }
    preCheck=strstr(bootargs,setCfg);
    if(preCheck!=0)
    {
        UBOOT_DEBUG("%s has already existed\n",setCfg);
        UBOOT_TRACE("OK\n");
        return 0;
    }

    OriArg = (char*)malloc(strlen(bootargs) + 1);
    if(OriArg == NULL)
    {
        UBOOT_ERROR("malloc for OriArg fail\n");
        return -1;
    }
    strncpy(OriArg, bootargs,strlen(bootargs)+1);
    UBOOT_DEBUG("OriArg=%s\n",OriArg);
    //if setCfg exist, delete it.
    del_cfg(OriArg,prefix_cfg);
    UBOOT_DEBUG("OriArg=%s\n",OriArg);
    //add the NewCfg to Arg's tail.
    NewArgLen = strlen(OriArg) + strlen(setCfg) + 2 ;
    NewArg = malloc(NewArgLen);

    if(NewArg==NULL)
    {
        free(OriArg);
        UBOOT_ERROR("malloc for NewArg fail\n");
        return -1;
    }
    UBOOT_DEBUG("OriArg=%s\n",OriArg);

    snprintf(NewArg, NewArgLen, "%s %s", OriArg, setCfg);
    setenv("bootargs", NewArg);
    if(0 == bDontSaveEnv)
    {
        saveenv();
    }
    free(OriArg);
    free(NewArg);

    UBOOT_TRACE("OK\n");
    return 0;
}

#if defined (CONFIG_POINTS_HANDLER_ENABLE)
#define INFO_EN_CHECKPOINTS_CFG     "en_chk_p"
int do_add_bootcheckpoints (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]){
    int ret = 0;
    char str[2];
    UBOOT_TRACE("IN\n");

    if(argc > 1)   // any value to disable "test customer performance index"
    {
        str[0] = '0';
    }
    else
    {
        str[0] = '1';
    }
    str[1] = '\0';

    ret = setenv("en_chk_p", str);
    if(ret==0)
    {
        UBOOT_TRACE("OK\n");
        saveenv();
    }
    else
    {
        UBOOT_ERROR("set en_chk_p=1 to bootargs fail\n");
    }

    return ret;
}
#endif

int do_add_moduletest (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]){
    int ret = 0;
    UBOOT_TRACE("IN\n");
    ret=set_bootargs_cfg("moduletest","moduletest=true",0);
    if(ret==0)
    {
        UBOOT_TRACE("OK\n");
    }
    else
    {
        UBOOT_ERROR("set moduletest=ture to bootargs fail\n");
    }
    return ret;
}

int do_add_autotest (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]){
    int ret = 0;
    UBOOT_TRACE("IN\n");
    ret=set_bootargs_cfg("autotest","autotest=true",0);
    if(ret==0)
    {
        UBOOT_TRACE("OK\n");
    }
    else
    {
        UBOOT_ERROR("set autotest=ture to bootargs fail\n");
    }
    return ret;
}


int do_add_hsl (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]){
    int ret = 0;
    UBOOT_TRACE("IN\n");
    ret=set_bootargs_cfg("hsl","hsl=true",0);
    if(ret==0)
    {
        UBOOT_TRACE("OK\n");
    }
    else
    {
        UBOOT_ERROR("set hsl=true to bootargs fail\n");
    }
    return ret;
}

#if CONFIG_MINIUBOOT
#else
static int set_security_cfg(void)
{
    int ret = 0;
    UBOOT_TRACE("IN\n");
    ret=set_bootargs_cfg(SECURITY_ENV_CFG_PREFIX,SECURITY_ENV_CFG,0);
    if(ret==0)
    {
        UBOOT_TRACE("OK\n");
    }
    else
    {
        UBOOT_ERROR("set hsl=true to bootargs fail\n");
    }
    return ret; 
}
#endif

static int set_info_exchange_cfg(void)
{
    char* pInfo = getenv(INFO_EXCHANGE_CFG);
    UBOOT_TRACE("IN\n");
    if (!pInfo || strcmp(pInfo, INFO_EXCHANGE_STORAGE) != 0)
    {
        UBOOT_DEBUG("write %s=%s\n",INFO_EXCHANGE_CFG,INFO_EXCHANGE_STORAGE);
        setenv(INFO_EXCHANGE_CFG, INFO_EXCHANGE_STORAGE);
#if CONFIG_MINIUBOOT
#else
        saveenv();
#endif
    }
    UBOOT_TRACE("OK\n");
    return 0;
}

static int set_env_cfg(void)
{
   int ret=0;
   char *buf=NULL;
   UBOOT_TRACE("IN\n");
   
   buf=malloc(CMD_BUF);
   if(buf==NULL)
   {
       UBOOT_ERROR("malloc fail\n");
       return -1;
   }
   memset(buf,0,CMD_BUF);
   snprintf(buf,CMD_BUF,"ENV_VAR_OFFSET=0x%x", cfg_env_offset);
   UBOOT_DEBUG("cmd=%s\n",buf);

   ret=set_bootargs_cfg("ENV_VAR_OFFSET",buf,0);
   if(ret==-1)
   {
        UBOOT_ERROR("set %s to bootargs fail\n",buf);
        free(buf);
        return -1;
   }
   memset(buf,0,CMD_BUF);
   snprintf(buf,CMD_BUF,"ENV_VAR_SIZE=0x%x", CONFIG_ENV_VAR_SIZE);
   UBOOT_DEBUG("cmd=%s\n",buf);   

   ret=set_bootargs_cfg("ENV_VAR_SIZE",buf,0);
   if(ret==-1)
   {
        UBOOT_ERROR("set %s to bootargs fail\n",buf);
        free(buf);        
        return -1;
   } 

   ret=set_bootargs_cfg(ENV_CFG_PREFIX,ENV_CFG,0);
   if(ret==-1)
   {
        free(buf);
        UBOOT_ERROR("set %s to bootargs fail.\n",ENV_CFG);   
        return -1;
   }
   free(buf);
   UBOOT_TRACE("OK\n");
   return ret;
}

#define LOGO_IN_MBOOT "BOOTLOGO_IN_MBOOT"
#if CONFIG_MINIUBOOT
#else
static int set_panelinit_cfg(void)
{
    int ret =0;
    UBOOT_TRACE("IN\n");

#ifdef CONFIG_PANEL_INIT
    UBOOT_DEBUG("do config panel init\n");
    ret = set_bootargs_cfg(LOGO_IN_MBOOT,LOGO_IN_MBOOT,0);
#else
    UBOOT_DEBUG("no config panel init\n");
    ret = del_bootargs_cfg(LOGO_IN_MBOOT);
#endif
    if(ret==0)
    {
        UBOOT_TRACE("OK\n");
    }
    else
    {
        UBOOT_ERROR("set %s to bootargs fail\n",LOGO_IN_MBOOT);
    }
    return ret;
}

static void if_51OnRam_set(void)
{
    char* pInfo= NULL;
    UBOOT_TRACE("IN\n");    
    pInfo = getenv(ENV_51ONRAM);
    if (pInfo == NULL)
    {
        UBOOT_DEBUG("setenv %s %s\n",ENV_51ONRAM,ENV_51ONRAM_CFG);
        setenv(ENV_51ONRAM,ENV_51ONRAM_CFG);
        saveenv();
    }
    UBOOT_TRACE("OK\n");
}
#endif

int do_config2env( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");
    char* pInfo;
    pInfo = NULL;
#if (CONFIG_OAD_IN_MBOOT) 
    pInfo = getenv(OAD_IN_MBOOT);
    if (pInfo == NULL)
    {
        UBOOT_DEBUG("setenv %s 1\n",OAD_IN_MBOOT);
        setenv(OAD_IN_MBOOT, "1");
        saveenv();
    }
#endif
#if CONFIG_MINIUBOOT
#else
    if_51OnRam_set();
#endif

    UBOOT_TRACE("OK\n");
    return 0;   
}
    

int do_set_bootargs( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");
    UBOOT_INFO("============= set bootargs ===============\n");
    set_info_exchange_cfg();
#if CONFIG_MINIUBOOT
    set_bootargs_cfg(LOGO_IN_MBOOT,LOGO_IN_MBOOT,0);
#else
    set_panelinit_cfg();
#endif
    set_env_cfg();
#if CONFIG_MINIUBOOT
#else
    set_security_cfg();
#endif
    UBOOT_TRACE("OK\n");
    return 0;
}

int syncMmapToEnv(void)
{
        UBOOT_TRACE("IN\n");    
        delMmapInfoFromEnv();
        saveMmapInfoToEnv();
        saveenv();
        UBOOT_TRACE("OK\n");
        return 0;
}

int delMmapInfoFromEnv(void)
{
    return processMmapInfoOnEnv(FALSE);
}

int saveMmapInfoToEnv(void)
{
    return processMmapInfoOnEnv(TRUE);
}

int processMmapInfoOnEnv(BOOLEAN save2env)
{
    U32 i=0;
    U32 addr=0xFFFF;
    U32 len=0xFFFF;
    char *ptr=NULL;
    UBOOT_TRACE("IN\n");
    while(1)
    {
        ptr=mmapID[i];
        UBOOT_DEBUG("mmapID[%d]=%s\n",i,ptr);
        if(strcmp(ptr,"")==0)
        {
                UBOOT_TRACE("OK\n");
                return 0;
        }
        if(save2env==TRUE)
        {
            if(0==get_addr_from_mmap(ptr, &addr))
            {
            save_addr_to_env(ptr,addr);
            }
            if(0==get_length_from_mmap(ptr,&len))
            {         
            save_len_to_env(ptr,len);
        }
        }
        else{
            del_addr_from_env(ptr);
            del_len_from_env(ptr);
        }
        i++;
    }
    UBOOT_TRACE("OK\n");
    return 0;
}

int syncMmap2bootargs(void)
{
    int ret=0;
    unsigned int addr=0;
    unsigned int len=0;
    char buf[CMD_BUF];
    unsigned int min_interval=0;
    
    UBOOT_TRACE("IN\n");
    
    //Sync LX_MEM
    ret=_syncLen2bootargs(MMAP_ID_LXMEM,ENV_LX_MEM_LEN);
    if(ret!=0)
    {
        UBOOT_ERROR("sync LX_MEM fail\n");
        return -1;
    }

    //Sync EMAC_MEM
    ret=_syncLen2bootargs(MMAP_ID_EMAC,ENV_EMAC_MEM_LEN);
    if(ret!=0)
    {
        UBOOT_ERROR("sync EMAC_MEM fail\n");
        return -1;
    }

    //Sync BB_ADDR
    ret=_syncAddr2bootargs(MMAP_ID_FLUSH_BUFFER,ENV_FLUSH_BUF_ADDR);
    if(ret!=0)
    {
        UBOOT_ERROR("sync BB_ADDR fail\n");
        return -1;
    }

    //Sync PM51_ADDR
    ret=_syncAddr2bootargs(MMAP_ID_PM51_USAGE,ENV_PM51_ADDR);
    if(ret!=0)
    {
        UBOOT_ERROR("sync PM51_ADDR fail\n");
        return -1;
    }

    //Sync PM51_LEN
    ret=_syncLen2bootargs(MMAP_ID_PM51_USAGE,ENV_PM51_LEN);
    if(ret!=0)
    {
        UBOOT_ERROR("sync PM51_LEN fail\n");
        return -1;
    }    

    //Sync LX_MEM2
    ret=get_miu_interval(&min_interval);
    if(ret!=0)
    {
        UBOOT_ERROR("get min interval fail\n");
        return -1;
    }
    
    ret=get_addr_from_mmap(MMAP_ID_LXMEM2, &addr); 
    if(ret!=0)
    {
        UBOOT_ERROR("get %s fail\n",MMAP_ID_LXMEM2);
        return -1;
    }
    
    ret=get_length_from_mmap(MMAP_ID_LXMEM2, &len); 
    if(ret!=0)
    {
        UBOOT_ERROR("get %s fail\n",MMAP_ID_LXMEM2);
        return -1;
    }
    memset(buf,0,sizeof(buf));
    snprintf(buf,CMD_BUF,"%s=0x%x,0x%x",ENV_LX_MEM2,(addr&(~min_interval))+MIU1_LOGIC_ADR_OFFSET,len);
    UBOOT_DEBUG("buf=%s\n",buf);
    ret=set_bootargs_cfg(ENV_LX_MEM2,buf,0);
    if(ret!=0)
    {
        UBOOT_ERROR("sync LX_MEM2 fail\n");
        return -1;
    }

    //Sync DRAM Length
    ret=get_dram_length(&len);
    if(ret!=0)
    {
        UBOOT_ERROR("get dram length fail\n");
        return -1;
    }

    memset(buf,0,sizeof(buf));
    snprintf(buf,CMD_BUF,"%s=0x%x",ENV_DRAM_LEN,len);
    UBOOT_DEBUG("buf=%s\n",buf);
    ret=set_bootargs_cfg(ENV_DRAM_LEN,buf,0);
    if(ret!=0)
    {
        UBOOT_ERROR("sync dram length fail\n");
        return -1;
    }
  
    UBOOT_TRACE("OK\n");    
    return 0;
}

static int _syncLen2bootargs(char *id, char *envName)
{
    int ret=0;
    unsigned int len=0;
    char buf[CMD_BUF];
    UBOOT_TRACE("IN\n");
    if(id==NULL)
    {
        UBOOT_ERROR("id is a null pointer\n");
        return -1;   
    }
    if(envName==NULL)
    {
        UBOOT_ERROR("envName is a null pointer\n");
        return -1;   
    }    
    
    ret=get_length_from_mmap(id, &len); 
    if(ret!=0)
    {
        UBOOT_ERROR("get %s fail\n",envName);
        return -1;
    }
    memset(buf,0,sizeof(buf));
    snprintf(buf,CMD_BUF,"%s=0x%x",envName,len);
    UBOOT_DEBUG("buf=%s\n",buf);
    ret=set_bootargs_cfg(envName,buf,0);
    if(ret==0)
    {
        UBOOT_TRACE("OK\n");
    }
    else
    {
        UBOOT_ERROR("sync %s to bootargs fail\n",id);
    }
    return 0;
}

static int _syncAddr2bootargs(char *id, char *envName)
{
    int ret=0;
    unsigned int addr=0;
    char buf[CMD_BUF];
    unsigned int min_interval=0;
    UBOOT_TRACE("IN\n");
    if(id==NULL)
    {
        UBOOT_ERROR("id is a null pointer\n");
        return -1;   
    }
    if(envName==NULL)
    {
        UBOOT_ERROR("envName is a null pointer\n");
        return -1;   
    }    
    
    ret=get_addr_from_mmap(id, &addr); 
    if(ret!=0)
    {
        UBOOT_ERROR("get %s fail\n",envName);
        return -1;
    }
    ret=get_miu_interval(&min_interval);
    if(ret!=0)
    {
        UBOOT_ERROR("get min interval fail\n");
        return -1;
    }
    memset(buf,0,sizeof(buf));
    snprintf(buf,CMD_BUF,"%s=0x%x",envName,addr&(~min_interval));
    UBOOT_DEBUG("buf=%s\n",buf);
    ret=set_bootargs_cfg(envName,buf,0);
    if(ret==0)
    {
        UBOOT_TRACE("OK\n");
    }
    else
    {
        UBOOT_ERROR("sync %s to bootargs fail\n",id);
    }
    return 0;
}

int setMmapInfo2Env(char *id_mmap, char *env_addr, char *env_len)
{
    char *buf=NULL;
    char *pEnv_addr=NULL;
    char *pEnv_len=NULL;    
    int ret=0;
    unsigned int addr=0;
    unsigned int len=0;
    UBOOT_TRACE("IN\n"); 

    if((id_mmap==NULL)||(env_addr==NULL)||(env_len==NULL))
    {
        UBOOT_ERROR("One of the parameters is null pointer\n");
        return -1;
    }
    UBOOT_DEBUG("id_mmap=%s\n",id_mmap);
    UBOOT_DEBUG("env_addr=%s\n",env_addr);    
    UBOOT_DEBUG("env_len=%s\n",env_len);        
    
    pEnv_addr=getenv(env_addr);
    pEnv_len=getenv(env_len);    
    if((pEnv_addr==NULL)||(pEnv_len==NULL))
    {
        buf=malloc(CMD_BUF);
        if(buf==NULL)
        {
            UBOOT_ERROR("malloc for buf fail\n");
            return -1;
        }
        memset(buf,0,CMD_BUF);
        
        ret=get_addr_from_mmap(id_mmap, &addr);
        if(ret!=0)
        {
            free(buf);
            UBOOT_ERROR("get %s addr fail\n",id_mmap);
            return -1;
        }

        ret=get_length_from_mmap(id_mmap, &len);
        if(ret!=0)
        {
            free(buf);
            UBOOT_ERROR("get %s len fail\n",id_mmap);
            return -1;
        }
        snprintf(buf,CMD_BUF,"setenv %s 0x%x;setenv %s 0x%x;saveenv",env_addr,addr,env_len,len);
        UBOOT_DEBUG("cmd=%s\n",buf);
        ret=run_command(buf,0);
        if(ret!=0)
        {
            free(buf);
            UBOOT_ERROR("set %s=%x to env fail\n",env_addr,addr);
            return -1;
        }
        free(buf);
    }
    else
    {
        UBOOT_DEBUG("%s has already existed\n",id_mmap);
    }

    UBOOT_TRACE("OK\n");        
    return 0;
}

int do_mmap_to_env( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret=0;
    UBOOT_TRACE("IN\n");
    if(argc!=4)
    {
        cmd_usage(cmdtp);
        return -1;
    }
    if((argv[1]==NULL)||(argv[2]==NULL)||(argv[3]==NULL))
    {
        UBOOT_ERROR("One of the parameters is null pointer\n");
        return -1;
    }
    ret=setMmapInfo2Env(argv[1],argv[2],argv[3]);
    if(ret!=0)
    {
        UBOOT_ERROR("handle %s fail\n",argv[1]);
        return -1;
    }
    UBOOT_TRACE("OK\n");        
    return 0; 
}

void Confirm_LX_Infomation(void)
{
    MS_BOOL ret = TRUE;
    unsigned int u32MapLX = 0;
    unsigned int u32BootargLX = 0;    
    char bootarg_lx[16]={0};
    char * ptoken    = NULL;
    char *map_lx_addr  = NULL;        
    char *map_lx_lenth  = NULL;
    char *bootarg = NULL;    
    UBOOT_TRACE("IN\n"); 
    // get lx memory information from bootarg
    bootarg = getenv("bootargs");
    if (bootarg != NULL)
    {
        // confirm E_LX_MEM_LEN
        if( (ptoken = strstr(bootarg,"LX_MEM")) != NULL)
        {
            memset(bootarg_lx,0,sizeof(bootarg_lx));        
            strncpy(bootarg_lx, ptoken+7, 10);
            map_lx_lenth = getenv("E_LX_MEM_LEN");
            if(map_lx_lenth != NULL)
            {
                u32MapLX=simple_strtoul(map_lx_lenth,NULL,16);
                u32BootargLX=simple_strtoul(bootarg_lx,NULL,16); 
                UBOOT_DEBUG("MapLX1=0x%x  ; BootargLX1=0x%x\n",u32MapLX,u32BootargLX);
                if(u32MapLX != u32BootargLX)
                {
                    UBOOT_ERROR("map_lx_lenth = 0x%x  ; BootargLX = 0x%x\n",u32MapLX,u32BootargLX);
                    ret = FALSE;
                }
            }
        }
        if( (ptoken = strstr(bootarg,"LX_MEM2")) != NULL)
        {        
            // confirm E_LX_MEM2_ADR
            memset(bootarg_lx,0,sizeof(bootarg_lx));
            strncpy(bootarg_lx, ptoken+8, 10);
            u32BootargLX=simple_strtoul(bootarg_lx,NULL,16);   
            map_lx_addr = getenv("E_LX_MEM2_ADR");
            if(map_lx_addr != NULL && (u32BootargLX>0))
            {
                u32MapLX=simple_strtoul(map_lx_addr,NULL,16);

                UBOOT_DEBUG("MapLX2_ADR=0x%x  ; BootargLX2_ADR=0x%x\n",u32MapLX+CONFIG_SYS_MIU0_BUS,u32BootargLX);
                
                if((u32MapLX+CONFIG_SYS_MIU0_BUS) != u32BootargLX)
                {
                    UBOOT_ERROR("map_lx2_addr = 0x%x  ; BootargLX2_addr = 0x%x\n",u32MapLX,u32BootargLX);
                    ret = FALSE;
                }
            }  
            // confirm E_LX_MEM2_LEN
            memset(bootarg_lx,0,sizeof(bootarg_lx));
            strncpy(bootarg_lx, ptoken+19, 10);
            u32BootargLX=simple_strtoul(bootarg_lx,NULL,16);
            map_lx_lenth = getenv("E_LX_MEM2_LEN");
            if(map_lx_lenth != NULL && (u32BootargLX>0))
            {
                u32MapLX=simple_strtoul(map_lx_lenth,NULL,16);
                
                UBOOT_DEBUG("MapLX2_LENTH=0x%x  ; BootargLX2_LENTH=0x%x\n",u32MapLX,u32BootargLX);
                if(u32MapLX != u32BootargLX)
                {
                    UBOOT_ERROR("map_lx2_lenth = 0x%x  ; BootargLX2_lenth = 0x%x\n",u32MapLX,u32BootargLX);
                    ret = FALSE;
                }
            }           
        }
        if( (ptoken = strstr(bootarg,"LX_MEM3")) != NULL)
        {
            // E_LX_MEM3_ADR
            memset(bootarg_lx,0,sizeof(bootarg_lx));
            strncpy(bootarg_lx, ptoken+8, 10);
            u32BootargLX=simple_strtoul(bootarg_lx,NULL,16);            
            map_lx_addr = getenv("E_LX_MEM3_ADR");
            if(map_lx_addr != NULL && (u32BootargLX>0))
            {
                u32MapLX=simple_strtoul(map_lx_addr,NULL,16);

                UBOOT_DEBUG("MapLX3_ADDR=0x%x  ; BootargLX3_ADDR=0x%x\n",u32MapLX+CONFIG_SYS_MIU0_BUS,u32BootargLX);
                if((u32MapLX+CONFIG_SYS_MIU0_BUS) != u32BootargLX)
                {
                    UBOOT_ERROR("map_lx3_addr = 0x%x  ; BootargLX3_addr = 0x%x\n",u32MapLX,u32BootargLX);
                    ret = FALSE;
                }
            }
            // E_LX_MEM3_LEN
            memset(bootarg_lx,0,sizeof(bootarg_lx));        
            strncpy(bootarg_lx, ptoken+19, 10);
            u32BootargLX=simple_strtoul(bootarg_lx,NULL,16);
            map_lx_lenth = getenv("E_LX_MEM3_LEN");
            if(map_lx_lenth != NULL && (u32BootargLX>0))
            {
                u32MapLX=simple_strtoul(map_lx_lenth,NULL,16);
                
                UBOOT_DEBUG("MapLX3_LENTH=0x%x  ; BootargLX3_LENTH=0x%x\n",u32MapLX,u32BootargLX);
                if(u32MapLX != u32BootargLX)
                {
                    UBOOT_ERROR("map_lx3_lenth = 0x%x  ; BootargLX3_lenth = 0x%x\n",u32MapLX,u32BootargLX);
                    ret = FALSE;
                }
            }             
        }
    }
    if(ret == FALSE)
    {
        UBOOT_ERROR("========================================================\n");    
        UBOOT_ERROR("===  MBOOT's LX_INFO is different from SN's LX_INFO. ===\n");
        UBOOT_ERROR("===  Please Update Bootarg's Lx_MEM Information!     ===\n");
        UBOOT_ERROR("========================================================\n");    
        jump_to_console();
    }

    UBOOT_TRACE("OK\n"); 
}


int do_sync_mmap_to_env( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char cmdBuf[CMD_BUF];
    char *pEnv=NULL;
    unsigned char sync_mmap=0;
    UBOOT_TRACE("IN\n");
    if(argc!=1)
    {
        cmd_usage(cmdtp);
        return -1;
    }
    pEnv=getenv(ENV_SYNC_MMAP);
    if(pEnv!=NULL)
    {
        sync_mmap=simple_strtol(pEnv, NULL, 16);
    }
    else
    {
        UBOOT_DEBUG("no env 'sync_mmap'\n");
        sync_mmap=1;
    }

   if(sync_mmap==1)
   {
        syncMmapToEnv();
        Confirm_LX_Infomation();
        #if 0
        //Sync bootargs
        ret=syncMmap2bootargs();
        if(ret==-1)
        {
            UBOOT_ERROR("sync the part of mmap info to bootargs fail\n");
            return -1;
        }
        #endif        
        memset(cmdBuf,0,CMD_BUF);
        snprintf(cmdBuf,CMD_BUF,"setenv %s 0;saveenv",ENV_SYNC_MMAP);
        UBOOT_DEBUG("cmdBuf=%s\n",cmdBuf);
        if(0!=run_command(cmdBuf,0))
        {
            UBOOT_ERROR("fail\n");
            return -1;
        }        
    }
    else{
        UBOOT_DEBUG("The mmap info in env is the newest\n");
    }

    UBOOT_TRACE("OK\n");        
    return 0; 
}

#define BOOTTIME_SBOOT_STR "BOOTTIME_SBOOT"
#define BOOTTIME_UBOOT_STR "BOOTTIME_UBOOT"
#include <stdlib.h>
#include <malloc.h>
#include "MsSystem.h"

unsigned long G_MS_BOOTTIME_SBOOT=1; // global variable for storing the boot time used in sboot (ms)
unsigned long G_MS_BOOTTIME_UBOOT=1; // global variable for storing the boot time used in sboot (ms)

void _boottime_set_to_env(void)
{
    extern int snprintf(char *str, size_t size, const char *fmt, ...);
    char *strEnv = NULL;
    const int u32EnvSize = CMD_BUF;

    G_MS_BOOTTIME_UBOOT = MsSystemGetBootTime();

    strEnv = malloc(u32EnvSize);
    if(strEnv  != NULL)
    {
        memset(strEnv , 0, u32EnvSize);
        snprintf(strEnv , u32EnvSize-1, "%s=%lu", BOOTTIME_SBOOT_STR, G_MS_BOOTTIME_SBOOT);
        if(0 != set_bootargs_cfg((char*)BOOTTIME_SBOOT_STR, strEnv, 1))
        {
            printf("%s: Error: set_bootargs_cfg failed at %d\n", __func__, __LINE__);
        }

        memset(strEnv , 0,u32EnvSize);
        snprintf(strEnv , u32EnvSize-1, "%s=%lu", BOOTTIME_UBOOT_STR, G_MS_BOOTTIME_UBOOT);
        if(0 != set_bootargs_cfg((char*)BOOTTIME_UBOOT_STR, strEnv, 1))
        {
            printf("%s: Error: set_bootargs_cfg failed at %d\n", __func__, __LINE__);
        }
        free(strEnv );
    }
}

#if CONFIG_RESCUE_ENV
unsigned int GetRealOffset(unsigned int u32EnvOffset)
{
    unsigned int u32Size=0;
    unsigned int u32RealOffset=0;
    int ret = 0;
#if (ENABLE_MODULE_ENV_IN_SERIAL)
    ret = getSpiSize(&u32Size);
    u32RealOffset = u32Size - (u32EnvOffset*SECTOR_SIZE);
#elif (ENABLE_MODULE_ENV_IN_NAND)
    char cmd_buf[CMD_BUF]="\0";
    sprintf(cmd_buf, "ubi part %s", NAND_DEFAULT_PARTITION);
    run_command(cmd_buf, 0);
    ubi_get_volume_size(NAND_DEFAULT_VOLUME,&u32Size);
    UBOOT_DEBUG("u32Size : 0x%x\n",u32Size);
    UBOOT_DEBUG("ubi_get_leb_size : 0x%x\n",SECTOR_SIZE);
    u32RealOffset = u32Size - (u32EnvOffset*SECTOR_SIZE);
#elif (ENABLE_MODULE_ENV_IN_MMC)   
    #include <MsMmc.h>
    ret = get_mmc_partsize(MMC_DEFAULT_VOLUME,&u32Size);
    u32RealOffset = u32Size - (u32EnvOffset*SECTOR_SIZE);
#else
    #error "please set the correct uboot environment storage!!\n"
#endif
    if(0 != ret)
    {
        UBOOT_ERROR("Error, at %d", __LINE__);
        return 0;
    }
    return u32RealOffset;
}

extern uchar default_environment_rescue[];
env_t *env_ptr_rescue = NULL;
extern int default_environment_rescue_size;

void env_relocate_spec_rescue (void);
int restoreenv_from_rescue(void)
{
    //NOTE: 1. write current env to flash
//    saveenv_rescue();
    //NOTE: 2. do recovery
    ssize_t    len=0;
    char *res=NULL;
    unsigned int u32EnvOffset=0;
    UBOOT_TRACE("IN\n");

    memset (env_ptr_rescue, 0, CONFIG_ENV_SIZE);
    env_relocate_spec_rescue();

    res = (char *)&(env_ptr_rescue->data);
    len = hexport_r(&env_htab_rescue, '\0', &res, ENV_SIZE);
    if (len < 0) 
    {
        UBOOT_ERROR("Cannot export environment: errno = %d\n", errno);
        return -1;
    }
    MsApiChunkHeader_GetValue(CH_UBOOT_ENVIRONMENT_ROM_OFFSET,&u32EnvOffset);
    u32EnvOffset = GetRealOffset(u32EnvOffset);
    env_ptr_rescue->crc   = crc32(0, env_ptr_rescue->data, ENV_SIZE);    
    int ret = raw_write((unsigned int)env_ptr_rescue, u32EnvOffset, CONFIG_ENV_SIZE);
    int retBak = raw_write((unsigned int)env_ptr_rescue, u32EnvOffset + ENV_SECTOR_SIZE, CONFIG_ENV_SIZE);
    if( (ret != 0) || (retBak != 0) )
    {
        return -1;
    }
    UBOOT_TRACE("OK\n");    
    return 0;    
}

int saveenv_rescue(void)
{
    ssize_t    len=0;
    char *res=NULL;
    unsigned int u32EnvRescueOffset = 0;
    UBOOT_TRACE("IN\n");    
    res = (char *)&(env_ptr_rescue->data);
    len = hexport_r(&env_htab_rescue, '\0', &res, ENV_SIZE);
    if (len < 0) {
        error("Cannot export environment: errno = %d\n", errno);
        return -1;
    }

    env_ptr_rescue->crc   = crc32(0, env_ptr_rescue->data, ENV_SIZE);
    MsApiChunkHeader_GetValue(CH_RESCURE_ENVIRONMENT_ROM_OFFSET,&u32EnvRescueOffset);        
    u32EnvRescueOffset = GetRealOffset(u32EnvRescueOffset);
    int ret = raw_write((unsigned int)env_ptr_rescue,u32EnvRescueOffset , CONFIG_ENV_SIZE);
    int retBak = raw_write((unsigned int)env_ptr_rescue,u32EnvRescueOffset + ENV_SECTOR_SIZE, CONFIG_ENV_SIZE);    
    if( (ret != 0) || (retBak != 0) )
    {
        return -1;
    }
    UBOOT_TRACE("OK\n");
    return 0;
}

void env_relocate_spec_rescue (void)
{
    extern void set_default_env_rescue(const char *s);
    unsigned int u32EnvRescueOffset = 0;
    void* pEnvBuf = NULL;
    int ret=0;    
    UBOOT_TRACE("IN\n");

    pEnvBuf = malloc(CONFIG_ENV_SIZE);
    if(pEnvBuf==NULL) 
    {
        UBOOT_ERROR("malloc() failed\n");
        return;
    }
    memset(pEnvBuf, 0, CONFIG_ENV_SIZE);
    MsApiChunkHeader_GetValue(CH_RESCURE_ENVIRONMENT_ROM_OFFSET,&u32EnvRescueOffset);            
    u32EnvRescueOffset = GetRealOffset(u32EnvRescueOffset);
    printf("u32EnvRescueOffset = 0x%x\n",u32EnvRescueOffset);
#if 0 // use BDMA instead of memcpy to speed up boot time
    flush_cache((MS_PHYADDR)pEnvBuf,CONFIG_ENV_SIZE);
    MDrv_BDMA_FlashCopy2Dram(  CONFIG_ENV_RESCUE_ADDR , (MS_PHYADDR)pEnvBuf, CONFIG_ENV_SIZE);
#else
    ret = raw_read((unsigned int)pEnvBuf, u32EnvRescueOffset, CONFIG_ENV_SIZE);
    if(ret != 0)
    {    
        UBOOT_ERROR("Write CONFIG_ENV_RESCUE_ADDR Fail\n");
        return;
    }

    //memcpy(pEnvBuf, (void*)(SPIFLASH_KSEG0_ADDR + CONFIG_ENV_RESCUE_ADDR), CONFIG_ENV_SIZE);
#endif

    void* pEnvBufBak = malloc(CONFIG_ENV_SIZE);
    if(pEnvBufBak==NULL){
        free(pEnvBuf);
        UBOOT_ERROR("malloc() failed\n");
        return;
    }
    memset(pEnvBufBak, 0, CONFIG_ENV_SIZE);
#if 0 // use BDMA instead of memcpy to speed up boot time
    flush_cache( (MS_PHYADDR)  pEnvBufBak,CONFIG_ENV_SIZE);
    MDrv_BDMA_FlashCopy2Dram(  CONFIG_ENV_RESCUE_ADDR_BAK , (MS_PHYADDR)pEnvBufBak, CONFIG_ENV_SIZE);
#else
    ret = raw_read((unsigned int)pEnvBufBak, u32EnvRescueOffset+ENV_SECTOR_SIZE, CONFIG_ENV_SIZE);
    if(ret != 0)
    {    
        UBOOT_ERROR("Write CONFIG_ENV_RESCUE_ADDR_BAK Fail\n");
        return;
    }

    //memcpy(pEnvBufBak, (void*)(SPIFLASH_KSEG0_ADDR + CONFIG_ENV_RESCUE_ADDR_BAK), CONFIG_ENV_SIZE);
#endif

    size_t offsetCrc = offsetof(env_t, crc);
    size_t offsetData = offsetof(env_t, data);

    ulong crc = *(ulong *)(pEnvBuf + offsetCrc);
    ulong crcNew = crc32(0, pEnvBuf + offsetData, ENV_SIZE);
    ulong crcBak = *(ulong *)(pEnvBufBak + offsetCrc);
    ulong crcNewBak = crc32(0, pEnvBufBak + offsetData, ENV_SIZE);

#if 0
    printf("%s: crc: 0x%04lx, at %d\n", __func__, crc, __LINE__);//DBG
    printf("%s: crcNew: 0x%04lx, at %d\n", __func__, crcNew, __LINE__);//DBG
    printf("%s: crcBak: 0x%04lx, at %d\n", __func__, crcBak, __LINE__);//DBG
    printf("%s: crcNewBak: 0x%04lx, at %d\n", __func__, crcNewBak, __LINE__);//DBG
#endif

    if ( (crc != crcNew) && (crcBak != crcNewBak) )
    {
        set_default_env_rescue("!bad CRC");
        memcpy (env_ptr_rescue->data, default_environment_rescue, default_environment_rescue_size);
        env_ptr_rescue->crc = crc32(0, env_ptr_rescue->data, ENV_SIZE);
//        gd->env_valid = 1;
        goto Exit;
    }

    if (crc != crcNew)
    {
        raw_write((unsigned int)pEnvBufBak,u32EnvRescueOffset, CONFIG_ENV_SIZE);
    }
    else if (crcBak != crcNewBak)
    {
        raw_write((unsigned int)pEnvBuf,u32EnvRescueOffset+ENV_SECTOR_SIZE, CONFIG_ENV_SIZE);
    }

    memcpy(env_ptr_rescue, (crc != crcNew) ? pEnvBufBak : pEnvBuf, CONFIG_ENV_SIZE);

    ret = env_import_rescue((const char *)env_ptr_rescue, 1);
    if (ret)
    {
//        gd->env_valid = 1;
    }
    else
    {
        UBOOT_ERROR("env_import_rescue() failed\n");        
    }
    UBOOT_TRACE("OK\n");
Exit:
    free(pEnvBufBak);
    free(pEnvBuf);
}
#endif

