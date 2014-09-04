////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (¡§MStar Confidential Information¡¨) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////
#include <common.h>
#include <MsTypes.h>
#include "Tuner.h"

#if (FRONTEND_TUNER_TYPE == NUTUNE_FT2125_TUNER)
#include "drvTuner_NuTune_FT2125.c"
#elif (FRONTEND_TUNER_TYPE == NUTUNE_FJ2207_TUNER)
#include "drvTuner_NuTune_FJ2207.c"
#include "FJ2207/tmbslNT220x/src/tmbslNT220x.c"
#include "FJ2207/tmbslNT220x/src/tmbslNT220xInstance.c"
#include "FJ2207/tmddNT220x/src/tmddNT220x.c"
#include "FJ2207/tmddNT220x/src/tmddNT220x_Advanced.c"
#include "FJ2207/tmddNT220x/src/tmddNT220xInstance.c"
//#elif (FRONTEND_TUNER_TYPE == NUTUNE_FK1602_TUNER)
//#include "MAXLINER_MxL_601.c"
#elif (FRONTEND_TUNER_TYPE == NXP_TD1611ALF_TUNER)
#include "drvTuner_NXP_TD1611ALF.c"
#elif (FRONTEND_TUNER_TYPE == SILAB_2158_TUNER)
#include "SILAB_2158_A20.c"
#include "SILAB_2158_A20/si2158_i2c_api.c"
#include "SILAB_2158_A20/Si2158_L1_API.c"
#include "SILAB_2158_A20/Si2158_L1_Commands.c"
#include "SILAB_2158_A20/Si2158_L1_Properties.c"
#include "SILAB_2158_A20/Si2158_L2_API.c"
#include "SILAB_2158_A20/Si2158_Properties_Strings.c"
#include "SILAB_2158_A20/silabs_L0_TV_Chassis.c"
#elif (FRONTEND_TUNER_TYPE == NXP_TDA18275_TUNER)
#include "NXP_TDA18275.c"
#endif

