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
#include <stdio.h>
#include <string.h>
#include <memory.h>

typedef unsigned char U8;
typedef unsigned short int U16;
typedef unsigned long int U32;


#define SIGNATURE_LEN               256
#define RSA_PUBLIC_KEY_LEN          (256+4)

#define FRAGMENT_NUM 8
typedef struct
{
    U32 u32Addr;
    U32 u32Size;
}IMAGE_INFOR;


typedef struct
{
  U8 u8SecIdentify[8]; 	
  IMAGE_INFOR info;
  U8 u8Signature[SIGNATURE_LEN];
}SUB_SECURE_INFO;

unsigned int  _atoi(char *str);
unsigned int  _atoi(char *str)
{
	
	unsigned int  value=0;

       if(*str=='\0') return  value;

	if((str[0]=='0')&&((str[1]=='x')||(str[1]=='X'))){   
	// 16Hex
		str+=2;
		while(1){

		   if(*str>=0x61)
		   	*str-=0x27;
		   else if(*str>=0x41)
		   	*str-=0x07;
		   
		   value|=(*str-'0');
		   str++;
		   //i++;
	          if(*str=='\0') break;
		   value=value<<4;	  
	      }
	}
	else{
	// 10 Dec

	       unsigned int  len,tmp=1;;	
		len=strlen(str);
		while(len){
			if(*str>'9') return 0;
			
			value+=((str[len-1]-'0')*tmp);

			len--;
			tmp=tmp*10;
	       }
	}
	return value;
	
}
int _GenSubSecureInfo(U8 *utilityPath, U8 *outFileName, U8 *intFileName, U8 *outAddr, U8 *private_key, U8 *public_key);
int _GenSubSecureInfo(U8 *utilityPath, U8 *outFileName, U8 *intFileName, U8 *outAddr, U8 *private_key, U8 *public_key)
{
    FILE *fr=NULL;
	FILE *fw=NULL;
	U32 fileSize=0;
	SUB_SECURE_INFO subSecureInfo;
	U8 cmdBuf[1000];
	U8 tempBuf1[1000];
	U8 tempBuf2[1000];
	
	fr=fopen(intFileName,"r");
	if(fr==NULL){
		printf("[ERROR] open %s fail\n",intFileName);
	}
	
	fseek(fr, 0, SEEK_END);
	fileSize=ftell(fr);
	fseek(fr, 0, SEEK_SET);
	fclose(fr);
	
	subSecureInfo.u8SecIdentify[0]=0x53;
	subSecureInfo.u8SecIdentify[1]=0x45;
	subSecureInfo.u8SecIdentify[2]=0x43;
	subSecureInfo.u8SecIdentify[3]=0x55;
	subSecureInfo.u8SecIdentify[4]=0x52;
	subSecureInfo.u8SecIdentify[5]=0x49;
	subSecureInfo.u8SecIdentify[6]=0x54;
	subSecureInfo.u8SecIdentify[7]=0x59;
	subSecureInfo.info.u32Addr=_atoi(outAddr);
	subSecureInfo.info.u32Size=fileSize;
	
	#if 1
	sprintf(cmdBuf,"%s/rsa_sign %s %s ",utilityPath, intFileName,private_key);
    system(cmdBuf);
	memset(tempBuf2,0,sizeof(tempBuf1));
	strcat(strcpy(tempBuf1, intFileName),".sig");
	memset(tempBuf2,0,sizeof(tempBuf2));
	strcat(strcpy(tempBuf2, intFileName),".sig.bin");
	#else
	memset(tempBuf1,0,sizeof(tempBuf1));
	strcat(strcpy(tempBuf1, intFileName),".sign.str");
	memset(cmdBuf,0,sizeof(cmdBuf));
	sprintf(cmdBuf,  "%s/cryptest.exe na_sign %s %s %s %s",utilityPath, private_key, public_key, intFileName,tempBuf1);
    system(cmdBuf);
	
	memset(tempBuf2,0,sizeof(tempBuf2));
	strcat(strcpy(tempBuf2, intFileName),".sign.bin");
	memset(cmdBuf,0,sizeof(cmdBuf));
	sprintf(cmdBuf,  "%s/str2hex.exe  %s %s ",utilityPath, tempBuf1, tempBuf2);
    system(cmdBuf);
	#endif
	
	fr=fopen(tempBuf2,"r");
	if(fr==NULL){
		printf("[ERROR] open %s fail\n",tempBuf2);
		return -1;
	}
	
	fseek(fr, 0, SEEK_END);
	fileSize=ftell(fr);
	fseek(fr, 0, SEEK_SET);
	fread(subSecureInfo.u8Signature, SIGNATURE_LEN,sizeof(U8),fr);
	fclose(fr);
	
	#if 1
	if(remove(tempBuf1)!=0){
		printf("[ERROR] remove %s fail\n",tempBuf1);
		return -1;
    }
	#endif
	
	if(remove(tempBuf2)!=0){
		printf("[ERROR] remove %s fail\n",tempBuf2);
		return -1;
    }
	
	fw=fopen(outFileName,"w");
	if(fw==NULL){
		printf("[ERROR] open %s fail\n",outFileName);
		return -1;
	}
	
	fwrite(&subSecureInfo, sizeof(SUB_SECURE_INFO),sizeof(U8),fw);
	fclose(fw);
	return 0;
}

int main(char argc, char *argv[])
{
	#define OUTPUT_FILE_NAME argv[1]
	#define INPUT_FILE_NAME  argv[2]
	#define OUTPUT_FILE_ADDRESS argv[3]
	#define PRIVATE_KEY argv[4]
	#define PUBLIC_KEY argv[5]
	#define FRAGMENT_ENABLE argv[6]
	#define UTILITY_PATH argv[7]
	//#define FRAGMENT_NUMBER argv[7]
	
	FILE *fr=NULL;
	U32 fragmentEnable=0;
	U32 fragmentNumber=0;
	U32 fragmentSize=0;
	int ret=0;
	U32 i=0;
	U32 fileSize=0;
	U8 cmdBuf[1000];
	U8 tempBuf1[1000];
	U8 tempBuf2[1000];
	
	if(argc<8){
		printf("[HELP] argv[1] OUTPUT_FILE_NAME \n");
		printf("[HELP] argv[2] INPUT_FILE_NAME \n");
		printf("[HELP] argv[3] OUTPUT_FILE_ADDRESS \n");
		printf("[HELP] argv[4] PRIVATE_KEY \n");
		printf("[HELP] argv[5] PUBLIC_KEY \n");
		printf("[HELP] argv[6] FRAGMENT_ENABLE \n");
		//printf("[HELP] argv[7] FRAGMENT_NUMBER \n");
		return -1;
	}
	
	fragmentEnable=_atoi(FRAGMENT_ENABLE);
	//fragmentNumber=_atoi(FRAGMENT_NUMBER);
	fragmentNumber=FRAGMENT_NUM;
	
	if(fragmentEnable==0){
		ret=_GenSubSecureInfo(UTILITY_PATH,OUTPUT_FILE_NAME,INPUT_FILE_NAME,OUTPUT_FILE_ADDRESS,PRIVATE_KEY,PUBLIC_KEY);
	}
	else{
		fr=fopen(INPUT_FILE_NAME,"r");
		if(fr==NULL){
			printf("[ERROR] open %s fail\n",INPUT_FILE_NAME);
		}
	
		fseek(fr, 0, SEEK_END);
		fileSize=ftell(fr);
		fseek(fr, 0, SEEK_SET);
		fclose(fr);
		fragmentSize=(fileSize/fragmentNumber);
		fragmentSize+=(fileSize%fragmentNumber);
		memset(cmdBuf,0,sizeof(cmdBuf));
		sprintf(cmdBuf,  "split -d -a 1 -b %d %s %s.", fragmentSize,INPUT_FILE_NAME,INPUT_FILE_NAME );
		system(cmdBuf);
		
		for(i=0;i<fragmentNumber;i++){
		
			memset(tempBuf1,0,sizeof(tempBuf1));
			sprintf(tempBuf1,  "%s.%d", INPUT_FILE_NAME,i);
			memset(tempBuf2,0,sizeof(tempBuf2));
			sprintf(tempBuf2,  "%s.%d", OUTPUT_FILE_NAME,i);
			
			ret=_GenSubSecureInfo(UTILITY_PATH,tempBuf2,tempBuf1,OUTPUT_FILE_ADDRESS,PRIVATE_KEY,PUBLIC_KEY);
			if(ret<0) return -1;
			memset(cmdBuf,0,sizeof(cmdBuf));
			sprintf(cmdBuf,"cat %s >> %s", tempBuf2,OUTPUT_FILE_NAME );
			system(cmdBuf);
			
			memset(cmdBuf,0,sizeof(cmdBuf));
			sprintf(cmdBuf,"rm %s;rm %s", tempBuf2,tempBuf1 );
			system(cmdBuf);
		}
		
		
	}
	return ret;
}
