////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drvMVOP.h
/// @brief  MVOP Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_MVOP_H_
#define _DRV_MVOP_H_

#ifdef __cplusplus
extern "C"
{
#endif

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
/// Version string.
#define MSIF_MVOP_LIB_CODE              {'M','V','O','P'}    //Lib code
#define MSIF_MVOP_LIBVER                {'0','8'}            //LIB version
#define MSIF_MVOP_BUILDNUM              {'2','9'}            //Build Number
#define MSIF_MVOP_CHANGELIST            {'0','0','5','4','2','7','6','5'} //P4 ChangeList Number

#define MVOP_DRV_VERSION                /* Character String for DRV/API version             */  \
    MSIF_TAG,                           /* 'MSIF'                                           */  \
    MSIF_CLASS,                         /* '00'                                             */  \
    MSIF_CUS,                           /* 0x0000                                           */  \
    MSIF_MOD,                           /* 0x0000                                           */  \
    MSIF_CHIP,                                                                                  \
    MSIF_CPU,                                                                                   \
    MSIF_MVOP_LIB_CODE,                 /* IP__                                             */  \
    MSIF_MVOP_LIBVER,                   /* 0.0 ~ Z.Z                                        */  \
    MSIF_MVOP_BUILDNUM,                 /* 00 ~ 99                                          */  \
    MSIF_MVOP_CHANGELIST,               /* CL#                                              */  \
    MSIF_OS


#define _SUPPORT_IMG_OFFSET_

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_MVOP_OK            = 0,
    E_MVOP_FAIL          = 1,
    E_MVOP_INVALID_PARAM = 2,
    E_MVOP_NOT_INIT      = 3,
    E_MVOP_UNSUPPORTED   = 4
} MVOP_Result;

/// MVOP tile format
typedef enum
{
    E_MVOP_TILE_8x32  = 0,
    E_MVOP_TILE_16x32 = 1,
    E_MVOP_TILE_NONE  = 2
} MVOP_TileFormat;

/// MVOP RGB format
typedef enum
{
    E_MVOP_RGB_565  = 0,
    E_MVOP_RGB_888  = 1,
    E_MVOP_RGB_NONE = 0xff
} MVOP_RgbFormat;

/// MVOP input mode
typedef enum
{
    VOPINPUT_HARDWIRE = 0,        //!< hardwire mode (MVD)
    VOPINPUT_HARDWIRECLIP = 1,    //!< hardware clip mode (MVD)
    VOPINPUT_MCUCTRL = 2,         //!< MCU control mode (M4VD, JPG)
    VOPINPUT_IFRAME = 3,          //!< MCU control mode (MVD)
} VOPINPUTMODE;

/// MVOP input mode
typedef enum
{
    E_VOPMIRROR_VERTICAL      = 1,
    E_VOPMIRROR_HORIZONTALL   =  2,
    E_VOPMIRROR_HVBOTH       =  3,
    E_VOPMIRROR_NONE         =  4,
} MVOP_DrvMirror;

/// MVOP repeat field mode
typedef enum
{
    E_MVOP_RPTFLD_NONE = 0,     //!< normal display top and bottom fields
    E_MVOP_RPTFLD_TOP  = 1,     //!< top field
    E_MVOP_RPTFLD_BOT  = 2,     //!< bottom field
} MVOP_RptFldMode;

typedef enum
{
    MVOP_MPEG4 = 0,
    MVOP_MJPEG,
    MVOP_H264,
    // <ATV MM>
    MVOP_RM, //jyliu.rm
    MVOP_TS,
    MVOP_MPG,
    MVOP_UNKNOW,
    // </ATV MM>
}MVOP_VIDEO_TYPE;


/// MVOP input parameter
typedef struct
{
    MS_U32 u32YOffset;
    MS_U32 u32UVOffset;
    MS_U16 u16HSize;
    MS_U16 u16VSize;
    MS_U16 u16StripSize;

    MS_BOOL bYUV422;    //!< YUV422 or YUV420
    MS_BOOL bSD;        //!< SD or HD
    MS_BOOL bProgressive;   //!< Progressive or Interlace

    // in func MDrv_MVOP_Input_Mode(), bSD is used to set dc_strip[7:0].
    // in func MDrv_MVOP_Input_Mode_Ext(), bSD is don't care and
    //    dc_strip[7:0] is set according to Hsize
    MS_BOOL bUV7bit;        // +S3, UV 7 bit or not
    MS_BOOL bDramRdContd;   // +S3, continue read out or jump 32
    MS_BOOL bField;         // +S3, Field 0 or 1 //(stbdc)for u4 bottom field enabled when bfield is true
    MS_BOOL b422pack;       // +S3, YUV422 pack mode
    MS_U16 u16CropX;
    MS_U16 u16CropY;
    MS_U16 u16CropWidth;
    MS_U16 u16CropHeight;
    MVOP_VIDEO_TYPE enVideoType;
} MVOP_InputCfg;


typedef struct
{
    MS_U16 u16V_TotalCount;        ///< Vertical total count
    MS_U16 u16H_TotalCount;        ///< Horizontal total count
    MS_U16 u16VBlank0_Start;       ///< Vertical blank 0 start
    MS_U16 u16VBlank0_End;         ///< Vertical blank 0 End
    MS_U16 u16VBlank1_Start;       ///< Vertical blank 1 start
    MS_U16 u16VBlank1_End;         ///< Vertical blank 1 End
    MS_U16 u16TopField_Start;      ///< Top field start
    MS_U16 u16BottomField_Start;   ///< bottom field start
    MS_U16 u16TopField_VS;         ///< top field vsync
    MS_U16 u16BottomField_VS;      ///< bottom field vsync
    MS_U16 u16HActive_Start;       ///< Horizontal disaply start

    MS_BOOL bInterlace;            ///< interlace or not
    MS_U8 u8Framerate;             ///< frame rate
    MS_U16 u16H_Freq ;             ///< horizontal frequency
    MS_U16 u16Num;                 ///< MVOP SYNTHESIZER numerator
    MS_U16 u16Den;                 ///< MVOP SYNTHESIZER denumerator
    MS_U8 u8MvdFRCType;            ///< flag for frame rate convert

    MS_U16 u16ExpFrameRate;        ///< Frame Rate
    MS_U16 u16Width;               ///< Width
    MS_U16 u16Height;              ///< Height
    MS_U16 u16HImg_Start;          ///< Horizontal disaply start
    MS_U16 u16VImg_Start0;         ///< Vertical disaply start
    MS_U16 u16VImg_Start1;         ///< Vertical disaply start
    MS_BOOL bHDuplicate;           ///< flag for vop horizontal duplicate
} MVOP_Timing;

//Display information For GOP
typedef struct
{
    MS_U32 VDTOT; //Output vertical total
    MS_U32 DEVST; //Output DE vertical start
    MS_U32 DEVEND;//Output DE Vertical end
    MS_U32 HDTOT;// Output horizontal total
    MS_U32 DEHST; //Output DE horizontal start
    MS_U32 DEHEND;// Output DE horizontal end
    MS_BOOL bInterlaceMode;
}MVOP_DST_DispInfo;

//MVOP timing information from mvop registers
typedef struct
{
    MS_U16 u16V_TotalCount;        ///< Vertical total count
    MS_U16 u16H_TotalCount;        ///< Horizontal total count
    MS_U16 u16VBlank0_Start;       ///< Vertical blank 0 start
    MS_U16 u16VBlank0_End;         ///< Vertical blank 0 End
    MS_U16 u16VBlank1_Start;       ///< Vertical blank 1 start
    MS_U16 u16VBlank1_End;         ///< Vertical blank 1 End
    MS_U16 u16TopField_Start;      ///< Top field start
    MS_U16 u16BottomField_Start;   ///< bottom field start
    MS_U16 u16TopField_VS;         ///< top field vsync
    MS_U16 u16BottomField_VS;      ///< bottom field vsync
    MS_U16 u16HActive_Start;       ///< Horizontal disaply start
    MS_BOOL bInterlace;            ///< interlace or not
    MS_BOOL bEnabled;              ///< MVOP is enabled or not
}MVOP_TimingInfo_FromRegisters;

typedef enum
{
    MVOP_PATTERN_NORMAL     = 0,
    MVOP_PATTERN_COLORBAR   = 1,
    MVOP_PATTERN_FRAMECOLOR = 2,
    MVOP_PATTERN_YCBCR      = 3,
    MVOP_PATTERN_Y          = 4,
    MVOP_PATTERN_CB         = 5,
    MVOP_PATTERN_CR         = 6,
    MVOP_PATTERN_DEFAULT    = 7   //new pattern = (last parrtern + 1) % MVOP_PATTERN_DEFAULT
} MVOP_Pattern;

typedef enum
{
    MVOP_INPUT_DRAM = 0,
    MVOP_INPUT_H264 = 1,    //SVD for T2
    MVOP_INPUT_MVD  = 2,
    MVOP_INPUT_RVD  = 3,
    MVOP_INPUT_CLIP = 4,
    MVOP_INPUT_JPD  = 5,
    MVOP_INPUT_HVD_3DLR = 6,    ///< HVD 3D L/R mode
    MVOP_INPUT_MVD_3DLR = 7,    ///< MVD 3D L/R mode
    MVOP_INPUT_DRAM_3DLR= 8,
    MVOP_INPUT_UNKNOWN = -1
} MVOP_InputSel;

/// MVOP VideoStat data structure
typedef struct
{
    MS_U16 u16HorSize;
    MS_U16 u16VerSize;
    MS_U16 u16FrameRate;
    MS_U8 u8AspectRate;
    MS_U8 u8Interlace;
    MS_U16 u16HorOffset;
    MS_U16 u16VerOffset;
} MVOP_VidStat;

/// MVOP driver info.
typedef struct _MVOP_DrvInfo
{
    MS_U32 u32MaxSynClk;
    MS_U32 u32MinSynClk;
    MS_U32 u32MaxFreerunClk;
} MVOP_DrvInfo;

///MVOP driver status
typedef struct _MVOP_DrvStatus
{
    MS_BOOL bIsInit;
    MS_BOOL bIsEnable;
    MS_BOOL bIsUVShift;
    MS_BOOL bIsBlackBG;
    MS_BOOL bIsMonoMode;
    MS_BOOL bIsSetTiming;
} MVOP_DrvStatus;

///MVOP module enum
typedef enum
{
    E_MVOP_MODULE_MAIN = 0,
    E_MVOP_MODULE_SUB  = 1,
    E_MVOP_MODULE_MAX
} MVOP_Module;

///MVOP device enum
typedef enum
{
    E_MVOP_DEV_0    = 0,    ///< Main mvop
    E_MVOP_DEV_1    = 1,    ///< Sub mvop
    E_MVOP_DEV_2    = 2,    ///< 3rd mvop
    E_MVOP_DEV_NONE = 0xff
} MVOP_DevID;

///MVOP handle to carry out MVOP info.
typedef struct _MVOP_Handle
{
    MVOP_Module eModuleNum;
} MVOP_Handle;

///MVOP set command used by MDrv_MVOP_SetCommand()
typedef enum
{
    E_MVOP_CMD_SET_TYPE = 0x100,
    E_MVOP_CMD_SET_VSIZE_MIN,             ///< 0x101 Enable vsize minimum checking
    E_MVOP_CMD_SET_STB_FD_MASK_CLR,       ///< 0x102 Force set fd_mask to low
    E_MVOP_CMD_SET_3DLR_INST_VBLANK,      ///< 0x103 Vertical blanking lines between L & R for 3D L/R mode.
    E_MVOP_CMD_SET_3DLR_ALT_OUT,          ///< 0x104 Alternative lines output for 3D L/R mode. If interlace output, both view output the same field.
    E_MVOP_CMD_SET_RGB_FMT,               ///< 0x105 RGB format input: MVOP_RgbFormat.
    E_MVOP_CMD_SET_SW_CTRL_FIELD_ENABLE,  ///< 0x106 Force read top or bottom field.
    E_MVOP_CMD_SET_SW_CTRL_FIELD_DSIABLE, ///< 0x107 Disable force read control
    E_MVOP_CMD_SET_3DLR_2ND_CFG,          ///< 0x108 Enable supporting 2nd pitch(h/vsize) for 3D L/R mode.
    E_MVOP_CMD_SET_VSIZE_DUPLICATE,       ///< 0x109 Enable/Disable VSize duplicate.
    E_MVOP_CMD_SET_3DLR_ALT_OUT_SBS,      ///< 0x10A Line alternative read of 3D L/R mode, side-by-side output.
    E_MVOP_CMD_SET_FIELD_DUPLICATE,       ///< 0x10B Repeat field for interlace source. (only input one field)
    E_MVOP_CMD_SET_VSYNC_MODE,            ///< 0x10C Set VSync mode (0: original; 1: new)

    E_MVOP_CMD_GET_TYPE = 0x400,
    E_MVOP_CMD_GET_3DLR_ALT_OUT,          ///< 0x401 Query if it is 3D L/R alternative lines output.
    E_MVOP_CMD_GET_MIRROR_MODE,           ///< 0x402 Get Mirror Mode
    E_MVOP_CMD_GET_3DLR_2ND_CFG,          ///< 0x403 Get if 3D LR 2nd pitch is enabled.
    E_MVOP_CMD_GET_VSIZE_DUPLICATE,       ///< 0x404 Get if VSize duplicate is enabled.
    E_MVOP_CMD_GET_BASE_ADD_BITS,         ///< 0x405 Get bits of y/uv address.
} MVOP_Command;

typedef enum
{
    E_MVOP_SYNCMODE,
    E_MVOP_FREERUNMODE,
    E_MVOP_160MHZ = 160000000ul,
    E_MVOP_172MHZ = 172000000ul,
    E_MVOP_192MHZ = 192000000ul,
}MVOP_FREQUENCY;

// Interrupt
typedef enum
{
    E_MVOP_INT_NONE    = 0,
    E_MVOP_INT_BUFF_UF = 0x01, ///< buffer underflow
    E_MVOP_INT_BUFF_OF = 0x02, ///< buffer overflow
    E_MVOP_INT_VSYNC   = 0x04, ///< Vsync interrupt
    E_MVOP_INT_HSYNC   = 0x08, ///< Hsync interrupt
    E_MVOP_INT_RDY     = 0x10, ///< DC ready interrupt
    E_MVOP_INT_FDCHNG  = 0x20, ///< field change
} MVOP_IntType;

typedef struct
{
    MS_U8  u8StrVer;
    MS_U8  u8Rsrvd;
    MS_U16 u16HSize;
    MS_U16 u16VSize;
    MS_U16 u16Fps;
} MVOP_CapInput;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MDrv_MVOP_Init (void);
void MDrv_MVOP_Exit(void);
void MDrv_MVOP_Enable ( MS_BOOL bEnable );
MVOP_Result MDrv_MVOP_SetInputCfg (MVOP_InputSel in, MVOP_InputCfg* pCfg);
MVOP_Result MDrv_MVOP_SetOutputCfg(MVOP_VidStat *pstVideoStatus, MS_BOOL bEnHDup);
void MDrv_MVOP_EnableBlackBG ( void );
void MDrv_MVOP_EnableUVShift(MS_BOOL bEnable);
void MDrv_MVOP_SetMonoMode(MS_BOOL bEnable);

MS_U16 MDrv_MVOP_GetHSize(void);
MS_U16 MDrv_MVOP_GetVSize(void);
MS_U16 MDrv_MVOP_GetHStart(void);
MS_U16 MDrv_MVOP_GetVStart(void);
MS_BOOL MDrv_MVOP_GetIsInterlace(void);
MS_BOOL MDrv_MVOP_GetIsHDuplicate(void);
MVOP_Result MDrv_MVOP_GetIsEnable ( MS_BOOL* pbEnable );
MVOP_Result MDrv_MVOP_GetOutputTiming(MVOP_Timing* pMVOPTiming);
MVOP_Result MDrv_MVOP_GetLibVer(const MSIF_Version **ppVersion);
MS_BOOL MDrv_MVOP_CheckCapability(MS_U16 u16HSize, MS_U16 u16VSize, MS_U16 u16Fps);
MS_U16 MDrv_MVOP_GetMaxHOffset(MS_U16 u16HSize, MS_U16 u16VSize, MS_U16 u16Fps);
MS_U16 MDrv_MVOP_GetMaxVOffset(MS_U16 u16HSize, MS_U16 u16VSize, MS_U16 u16Fps);

void MDrv_MVOP_SetDbgLevel(MS_U8 level);
const MVOP_DrvInfo* MDrv_MVOP_GetInfo(void);
MS_BOOL MDrv_MVOP_GetStatus(MVOP_DrvStatus *pMVOPStat);
void MDrv_MVOP_SetPattern(MVOP_Pattern enMVOPPattern);
MS_BOOL MDrv_MVOP_SetTileFormat(MVOP_TileFormat eTileFmt);
MS_BOOL MDrv_MVOP_GetDstInfo(MVOP_DST_DispInfo *pDstInfo, MS_U32 u32SizeofDstInfo);
MS_BOOL MDrv_MVOP_SetFixVtt(MS_U16 u16FixVtt);
MS_BOOL MDrv_MVOP_SetMMIOMapBase(void);
MVOP_Result MDrv_MVOP_MiuSwitch(MS_U8 u8Miu);
MS_BOOL MDrv_MVOP_SetBaseAdd(MS_U32 u32YOffset, MS_U32 u32UVOffset, MS_BOOL bProgressive, MS_BOOL b422pack);
void MDrv_MVOP_SEL_OP_FIELD(MS_BOOL bEnable);
void MDrv_MVOP_SetRegSizeFromMVD(MS_BOOL bEnable);
void MDrv_MVOP_SetStartPos(MS_U16 u16Xpos ,MS_U16 u16Ypos);
MS_BOOL MDrv_MVOP_SetImageWidthHight(MS_U16 u16Width ,MS_U16 u16Height);
void MDrv_MVOP_SetVOPMirrorMode(MS_BOOL bEnable,MVOP_DrvMirror eMirrorMode);
void MDrv_MVOP_INV_OP_VS(MS_BOOL bEnable);
void MDrv_MVOP_FORCE_TOP(MS_BOOL bEnable);
MS_BOOL MDrv_MVOP_EnableFreerunMode(MS_BOOL bEnable);
void MDrv_MVOP_GetBaseAdd(MS_U32* u32YOffset, MS_U32* u32UVOffset);
//Functions for Sub MVOP
MS_BOOL MDrv_MVOP_SubGetStatus(MVOP_DrvStatus *pMVOPStat);
MS_U16 MDrv_MVOP_SubGetHStart(void);
MS_U16 MDrv_MVOP_SubGetVStart(void);
void MDrv_MVOP_SubSetPattern(MVOP_Pattern enMVOPPattern);
MS_BOOL MDrv_MVOP_SubSetTileFormat(MVOP_TileFormat eTileFmt);
MS_BOOL MDrv_MVOP_SubSetMMIOMapBase(void);
void MDrv_MVOP_SubInit(void);
void MDrv_MVOP_SubExit(void);
void MDrv_MVOP_SubEnable(MS_BOOL bEnable);
MVOP_Result MDrv_MVOP_SubGetIsEnable (MS_BOOL* pbEnable);
void MDrv_MVOP_SubEnableUVShift(MS_BOOL bEnable);
void MDrv_MVOP_SubEnableBlackBG(void);
void MDrv_MVOP_SubSetMonoMode(MS_BOOL bEnable);
MVOP_Result MDrv_MVOP_SubSetInputCfg (MVOP_InputSel in, MVOP_InputCfg* pCfg);
MVOP_Result MDrv_MVOP_SubSetOutputCfg(MVOP_VidStat *pstVideoStatus, MS_BOOL bEnHDup);
MVOP_Result MDrv_MVOP_SubGetOutputTiming(MVOP_Timing* pMVOPTiming);
MS_U16 MDrv_MVOP_SubGetHSize(void);
MS_U16 MDrv_MVOP_SubGetVSize(void);
MS_BOOL MDrv_MVOP_SubGetIsInterlace(void);
MS_BOOL MDrv_MVOP_SubGetIsHDuplicate(void);
MS_BOOL MDrv_MVOP_SubCheckCapability(MS_U16 u16HSize, MS_U16 u16VSize, MS_U16 u16Fps);
MS_U16 MDrv_MVOP_SubGetMaxHOffset(MS_U16 u16HSize, MS_U16 u16VSize, MS_U16 u16Fps);
MS_U16 MDrv_MVOP_SubGetMaxVOffset(MS_U16 u16HSize, MS_U16 u16VSize, MS_U16 u16Fps);
MS_BOOL MDrv_MVOP_SubGetDstInfo(MVOP_DST_DispInfo *pDstInfo, MS_U32 u32SizeofDstInfo);
MS_BOOL MDrv_MVOP_SubSetFixVtt(MS_U16 u16FixVtt);
MVOP_Result MDrv_MVOP_SubMiuSwitch(MS_U8 u8Miu);
void MDrv_MVOP_SubSetVOPMirrorMode(MS_BOOL bEnable, MVOP_DrvMirror eMirrorMode);
MS_BOOL MDrv_MVOP_SubEnableFreerunMode(MS_BOOL bEnable);
MS_BOOL MDrv_MVOP_SubSetBaseAdd(MS_U32 u32YOffset, MS_U32 u32UVOffset, MS_BOOL bProgressive, MS_BOOL b422pack);

// { Functions for the 3rd MVOP
//Basic control functions
MVOP_Result MDrv_MVOP_EX_Init(MVOP_DevID eID, MS_U32 u32InitParam);
MVOP_Result MDrv_MVOP_EX_Exit(MVOP_DevID eID, MS_U32 u32ExitParam);
MVOP_Result MDrv_MVOP_EX_Enable(MVOP_DevID eID, MS_BOOL bEnable);
MVOP_Result MDrv_MVOP_EX_SetInputCfg (MVOP_DevID eID, MVOP_InputSel in, MVOP_InputCfg* pCfg);
MVOP_Result MDrv_MVOP_EX_SetOutputCfg(MVOP_DevID eID, MVOP_VidStat *pstVideoStatus, MS_BOOL bEnHDup);

//Advanced control functions
MVOP_Result MDrv_MVOP_EX_SetPattern(MVOP_DevID eID, MVOP_Pattern enMVOPPattern);
MVOP_Result MDrv_MVOP_EX_SetTileFormat(MVOP_DevID eID, MVOP_TileFormat eTileFmt);
MVOP_Result MDrv_MVOP_EX_EnableUVShift(MVOP_DevID eID, MS_BOOL bEnable);
MVOP_Result MDrv_MVOP_EX_EnableBlackBG(MVOP_DevID eID);
MVOP_Result MDrv_MVOP_EX_SetMonoMode(MVOP_DevID eID, MS_BOOL bEnable);
MVOP_Result MDrv_MVOP_EX_SetFixVtt(MVOP_DevID eID, MS_U16 u16FixVtt);
MVOP_Result MDrv_MVOP_EX_MiuSwitch(MVOP_DevID eID, MS_U8 u8Miu);
MVOP_Result MDrv_MVOP_EX_SetVOPMirrorMode(MVOP_DevID eID, MS_BOOL bEnable, MVOP_DrvMirror eMirrorMode);
MVOP_Result MDrv_MVOP_EX_EnableFreerunMode(MVOP_DevID eID, MS_BOOL bEnable);

// Query functions
MVOP_Result MDrv_MVOP_EX_GetOutputTiming(MVOP_DevID eID, MVOP_Timing* pMVOPTiming);
MVOP_Result MDrv_MVOP_EX_GetIsEnable (MVOP_DevID eID, MS_BOOL* pbEnable);
MS_U16 MDrv_MVOP_EX_GetHStart(MVOP_DevID eID);
MS_U16 MDrv_MVOP_EX_GetVStart(MVOP_DevID eID);
MS_U16 MDrv_MVOP_EX_GetHSize(MVOP_DevID eID);
MS_U16 MDrv_MVOP_EX_GetVSize(MVOP_DevID eID);
MS_BOOL MDrv_MVOP_EX_GetIsInterlace(MVOP_DevID eID);
MS_BOOL MDrv_MVOP_EX_GetIsHDuplicate(MVOP_DevID eID);
MS_BOOL MDrv_MVOP_EX_GetStatus(MVOP_DevID eID, MVOP_DrvStatus *pMVOPStat);
MS_BOOL MDrv_MVOP_EX_CheckCapability(MVOP_DevID eID, MS_U32 u32InParam);
MS_BOOL MDrv_MVOP_EX_GetDstInfo(MVOP_DevID eID, MVOP_DST_DispInfo *pDstInfo, MS_U32 u32SizeofDstInfo);
//   Functions for the 3rd MVOP }

MVOP_Result MDrv_MVOP_SendBlueScreen(MS_U16 u16Width, MS_U16 u16Height);

MVOP_Result MDrv_MVOP_SetCommand(MVOP_Handle* stHd, MVOP_Command eCmd, void* pPara);
MVOP_Result MDrv_MVOP_GetCommand(MVOP_Handle* stHd, MVOP_Command eCmd, void* pPara, MS_U32 u32ParaSize);
void MDrv_MVOP_SetFrequency(MVOP_FREQUENCY eFreq);
#ifdef __cplusplus
}
#endif

#endif // _DRV_MVOP_H_
