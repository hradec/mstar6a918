////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   Mhal_emac.c
/// @brief  EMAC Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/mii.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/pci.h>
#include "mhal_emac.h"


#define MHal_MAX_INT_COUNTER    100
//-------------------------------------------------------------------------------------------------
//  EMAC hardware for Titania
//-------------------------------------------------------------------------------------------------

u32 MHal_EMAC_ReadReg32(struct net_device *ndev, u32 xoffset )
{
    u32 address = ndev->base_addr + xoffset*2;

    u32 xoffsetValueL = *( ( volatile u32* ) address ) & 0x0000FFFF;
    u32 xoffsetValueH = *( ( volatile u32* ) ( address + 4) ) << 0x10;
    return( xoffsetValueH | xoffsetValueL );
}

void MHal_EMAC_WritReg32(struct net_device *ndev ,u32 xoffset, u32 xval )
{
    u32 address = ndev->base_addr + xoffset*2;

    *( ( volatile u32 * ) address ) = ( u32 ) ( xval & 0x0000FFFF );
    *( ( volatile u32 * ) ( address + 4 ) ) = ( u32 ) ( xval >> 0x10 );
}

u32 MHal_EMAC_ReadRam32( u32 uRamAddr, u32 xoffset)
{
    return (*( u32 * ) ( ( char * ) uRamAddr + xoffset ) );
}

void MHal_EMAC_WritRam32( u32 uRamAddr, u32 xoffset, u32 xval )
{
    *( ( u32 * ) ( ( char * ) uRamAddr + xoffset ) ) = xval;
}

void MHal_EMAC_Write_SA1_MAC_Address(struct net_device *ndev, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32(ndev, REG_ETH_SA1L, w0 );
    MHal_EMAC_WritReg32(ndev, REG_ETH_SA1H, w1 );
}

void MHal_EMAC_Write_SA2_MAC_Address(struct net_device *ndev, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32(ndev, REG_ETH_SA2L, w0 );
    MHal_EMAC_WritReg32(ndev, REG_ETH_SA2H, w1 );
}

void MHal_EMAC_Write_SA3_MAC_Address(struct net_device *ndev, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32(ndev, REG_ETH_SA3L, w0 );
    MHal_EMAC_WritReg32(ndev, REG_ETH_SA3H, w1 );
}

void MHal_EMAC_Write_SA4_MAC_Address(struct net_device *ndev, u8 m0, u8 m1, u8 m2, u8 m3, u8 m4, u8 m5 )
{
    u32 w0 = ( u32 ) m3 << 24 | m2 << 16 | m1 << 8 | m0;
    u32 w1 = ( u32 ) m5 << 8 | m4;
    MHal_EMAC_WritReg32(ndev, REG_ETH_SA4L, w0 );
    MHal_EMAC_WritReg32(ndev, REG_ETH_SA4H, w1 );
}

//-------------------------------------------------------------------------------------------------
//  R/W EMAC register for Titania
//-------------------------------------------------------------------------------------------------

void MHal_EMAC_update_HSH(struct net_device *ndev, u32 mc0, u32 mc1)
{
    MHal_EMAC_WritReg32(ndev, REG_ETH_HSL, mc0 );
    MHal_EMAC_WritReg32(ndev, REG_ETH_HSH, mc1 );
}

//-------------------------------------------------------------------------------------------------
//  Read control register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CTL( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32(ndev, REG_ETH_CTL );
}

//-------------------------------------------------------------------------------------------------
//  Write Network control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_CTL(struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32(ndev, REG_ETH_CTL, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Network configuration register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CFG( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_CFG );
}

//-------------------------------------------------------------------------------------------------
//  Write Network configuration register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_CFG( struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32(ndev, REG_ETH_CFG, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read RBQP
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RBQP( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_RBQP );
}
//-------------------------------------------------------------------------------------------------
//  Write RBQP
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_RBQP(struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32(ndev, REG_ETH_RBQP, xval );
}

//-------------------------------------------------------------------------------------------------
//  Write Transmit Address register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_TAR(struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32(ndev, REG_ETH_TAR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read RBQP
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_TCR( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32(ndev, REG_ETH_TCR);
}

//-------------------------------------------------------------------------------------------------
//  Write Transmit Control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_TCR(struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_ETH_TCR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Transmit Status Register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_TSR(struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32(ndev, REG_ETH_TSR, xval );
}

u32 MHal_EMAC_Read_TSR( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32(ndev, REG_ETH_TSR );
}

u32 MHal_EMAC_Read_RSR( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32(ndev, REG_ETH_RSR );
}

void MHal_EMAC_Write_RSR(struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32(ndev, REG_ETH_RSR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt status register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_ISR(struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev,  REG_ETH_ISR, xval );
}

u32 MHal_EMAC_Read_ISR( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32(ndev, REG_ETH_ISR );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt enable register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IER( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_IER );
}

//-------------------------------------------------------------------------------------------------
//  Write Interrupt enable register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_IER( struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_ETH_IER, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt disable register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IDR( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32(ndev, REG_ETH_IDR );
}

//-------------------------------------------------------------------------------------------------
//  Write Interrupt disable register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_IDR(struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_ETH_IDR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Interrupt mask register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_IMR( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_IMR );
}

//-------------------------------------------------------------------------------------------------
//  Read PHY maintenance register
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_MAN(struct net_device *ndev)
{
    return MHal_EMAC_ReadReg32(ndev, REG_ETH_MAN );
}

//-------------------------------------------------------------------------------------------------
//  Write PHY maintenance register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_MAN(struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_ETH_MAN, xval );
}

//-------------------------------------------------------------------------------------------------
//  Write Receive Buffer Configuration
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_BUFF( struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_ETH_BUFF, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Receive Buffer Configuration
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_BUFF( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_BUFF );
}

//-------------------------------------------------------------------------------------------------
//  Read Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RDPTR( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_BUFFRDPTR );
}

//-------------------------------------------------------------------------------------------------
//  Write Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_RDPTR( struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_ETH_BUFFRDPTR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Write Receive First Full Pointer
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_WRPTR( struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_ETH_BUFFWRPTR, xval );
}

//-------------------------------------------------------------------------------------------------
//  Frames transmitted OK
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_FRA( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_FRA );
}

//-------------------------------------------------------------------------------------------------
//  Single collision frames
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SCOL( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_SCOL );
}

//-------------------------------------------------------------------------------------------------
//  Multiple collision frames
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_MCOL( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_MCOL );
}

//-------------------------------------------------------------------------------------------------
//  Frames received OK
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_OK( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_OK );
}
//-------------------------------------------------------------------------------------------------
//  Frame check sequence errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SEQE( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32(ndev, REG_ETH_SEQE );
}
//-------------------------------------------------------------------------------------------------
//  Alignment errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ALE( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_ALE );
}
//-------------------------------------------------------------------------------------------------
//  Late collisions
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_LCOL( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_LCOL );
}

//-------------------------------------------------------------------------------------------------
//  Excessive collisions
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ECOL( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_ECOL );
}

//-------------------------------------------------------------------------------------------------
//  Transmit under-run errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_TUE( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_TUE );
}

//-------------------------------------------------------------------------------------------------
//  Carrier sense errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_CSE( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_CSE );
}

//-------------------------------------------------------------------------------------------------
//  Receive resource error
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RE( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_RE );
}

//-------------------------------------------------------------------------------------------------
//  Received overrun
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ROVR( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_ROVR );
}

//-------------------------------------------------------------------------------------------------
//  Received symbols error
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SE( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_SE );
}

//-------------------------------------------------------------------------------------------------
//  Excessive length errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_ELR( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32(ndev, REG_ETH_ELR );
}

//-------------------------------------------------------------------------------------------------
//  Receive jabbers
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_RJB( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_RJB );
}

//-------------------------------------------------------------------------------------------------
//  Undersize frames
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_USF( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_USF );
}

//-------------------------------------------------------------------------------------------------
//  SQE test errors
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_SQEE( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_SQEE );
}

//-------------------------------------------------------------------------------------------------
//  Read Julian 100
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_JULIAN_0100( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32(ndev, REG_EMAC_JULIAN_0100 );
}


//-------------------------------------------------------------------------------------------------
//  Write Julian 100
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0100(struct net_device *ndev,  u32 xval )
{
    MHal_EMAC_WritReg32(ndev, REG_EMAC_JULIAN_0100, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Julian 104
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_JULIAN_0104( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_EMAC_JULIAN_0104 );
}

//-------------------------------------------------------------------------------------------------
//  Write Julian 104
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0104( struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_EMAC_JULIAN_0104, xval );
}

//-------------------------------------------------------------------------------------------------
//  Read Julian 108
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_Read_JULIAN_0108( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_EMAC_JULIAN_0108 );
}

//-------------------------------------------------------------------------------------------------
//  Write Julian 108
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Write_JULIAN_0108( struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_EMAC_JULIAN_0108, xval );
}

void MHal_EMAC_Set_Tx_JULIAN_T(struct net_device *ndev, u32 xval)
{
	u32 value;
	value = MHal_EMAC_ReadReg32(ndev, 0x134);
	value &= 0xff0fffff;
	value |= xval << 20;

	MHal_EMAC_WritReg32(ndev, 0x134, value);
}


u32 MHal_EMAC_Get_Tx_FIFO_Threshold(struct net_device *ndev)
{
	return (MHal_EMAC_ReadReg32(ndev, 0x134) & 0x00f00000) >> 20;
}

void MHal_EMAC_Set_Rx_FIFO_Enlarge(struct net_device *ndev, u32 xval)
{
	u32 value;
	value = MHal_EMAC_ReadReg32(ndev, 0x134);
	value &= 0xfcffffff;
	value |= xval << 24;

	MHal_EMAC_WritReg32(ndev, 0x134, value);
}

u32 MHal_EMAC_Get_Rx_FIFO_Enlarge(struct net_device *ndev)
{
	return (MHal_EMAC_ReadReg32(ndev, 0x134) & 0x03000000) >> 24;
}

void MHal_EMAC_Set_Miu_Priority(struct net_device *ndev, u32 xval)
{
	u32 value;

	value = MHal_EMAC_ReadReg32(ndev, 0x100);
	value &= 0xfff7ffff;
	value |= xval << 19;

	MHal_EMAC_WritReg32(ndev, 0x100, value);
}

u32 MHal_EMAC_Get_Miu_Priority(struct net_device *ndev)
{
	return (MHal_EMAC_ReadReg32(ndev, 0x100) & 0x00080000) >> 19;
}

void MHal_EMAC_Set_Tx_Hang_Fix_ECO(struct net_device *ndev, u32 xval)
{
	u32 value;
	value = MHal_EMAC_ReadReg32(ndev, 0x134);
	value &= 0xfffbffff;
	value |= xval << 18;

	MHal_EMAC_WritReg32(ndev, 0x134, value);
}

void MHal_EMAC_Set_MIU_Out_Of_Range_Fix(struct net_device *ndev, u32 xval)
{
	u32 value;
	value = MHal_EMAC_ReadReg32(ndev, 0x134);
	value &= 0xefffffff;
	value |= xval << 28;

	MHal_EMAC_WritReg32(ndev, 0x134, value);
}

void MHal_EMAC_Set_Rx_Tx_Burst16_Mode(struct net_device *ndev, u32 xval)
{
	u32 value;
	value = MHal_EMAC_ReadReg32(ndev, 0x134);
	value &= 0xdfffffff;
	value |= xval << 29;

	MHal_EMAC_WritReg32(ndev, 0x134, value);
}

void MHal_EMAC_Set_Tx_Rx_Req_Priority_Switch(struct net_device *ndev, u32 xval)
{
	u32 value;
	value = MHal_EMAC_ReadReg32(ndev, 0x134);
	value &= 0xfff7ffff;
	value |= xval << 19;

	MHal_EMAC_WritReg32(ndev, 0x134, value);
}

void MHal_EMAC_Set_Rx_Byte_Align_Offset(struct net_device *ndev, u32 xval)
{
	u32 value;
	value = MHal_EMAC_ReadReg32(ndev, 0x134);
	value &= 0xf3ffffff;
	value |= xval << 26;

	MHal_EMAC_WritReg32(ndev, 0x134, value);
}

void MHal_EMAC_Write_Protect(struct net_device *ndev, u32 start_addr, u32 length)
{
    u32 value;

    value = MHal_EMAC_ReadReg32(ndev, 0x11c);
	value &= 0x0000ffff;
	value |= ((start_addr+length) >> 3) << 16;
    MHal_EMAC_WritReg32(ndev, 0x11c, value);

    value = MHal_EMAC_ReadReg32(ndev, 0x120);
	value &= 0x00000000;
	value |= ((start_addr+length) >> 3) >> 16;
    value |= (start_addr >> 3) << 16;
    MHal_EMAC_WritReg32(ndev, 0x120, value);

    value = MHal_EMAC_ReadReg32(ndev, 0x124);
	value &= 0xffff0000;
    value |= (start_addr >> 3) >> 16;
    MHal_EMAC_WritReg32(ndev, 0x124, value);
}

void MHal_EMAC_HW_init(struct net_device *ndev)
{
    MHal_EMAC_Set_Tx_JULIAN_T(ndev, 0x4);
    MHal_EMAC_Set_Rx_FIFO_Enlarge(ndev, 0x2);

    MHal_EMAC_Set_Miu_Priority(ndev, 0x1);

    MHal_EMAC_Set_Tx_Hang_Fix_ECO(ndev, 1);

    MHal_EMAC_Set_MIU_Out_Of_Range_Fix(ndev, 1);

    MHal_EMAC_Set_Rx_Tx_Burst16_Mode(ndev, 1);
}

//-------------------------------------------------------------------------------------------------
//  PHY INTERFACE
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enable the MDIO bit in MAC control register
// When not called from an interrupt-handler, access to the PHY must be
// protected by a spinlock.
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_enable_mdi( struct net_device *ndev )
{
    u32 xval;
    xval = MHal_EMAC_Read_CTL(ndev);
    xval |= EMAC_MPE;
    MHal_EMAC_Write_CTL( ndev, xval );
}

//-------------------------------------------------------------------------------------------------
//  Disable the MDIO bit in the MAC control register
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_disable_mdi( struct net_device *ndev )
{
    u32 xval;
    xval = MHal_EMAC_Read_CTL(ndev);
    xval &= ~EMAC_MPE;
    MHal_EMAC_Write_CTL( ndev, xval );
}

//-------------------------------------------------------------------------------------------------
// Write value to the a PHY register
// Note: MDI interface is assumed to already have been enabled.
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_write_phy(struct net_device *ndev, unsigned char phy_addr, unsigned char address, u32 value )
{
    struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(ndev);
	u32 uRegVal;
    u32 uCTL;
	u32 uRegBase;
	
    if (LocPtr->phytype == PHY_INTERNAL)
    {
	    uRegBase = LocPtr->reg_phy_address; 

        *(volatile unsigned int *)(uRegBase + address*4) = value;
        udelay( 1 );
    }
    else
    {
        uRegVal = 0; 
	    uCTL = 0;
        uRegVal =  ( EMAC_HIGH | EMAC_CODE_802_3 | EMAC_RW_W) | (( phy_addr & 0x1F ) << PHY_ADDR_OFFSET )
                    | ( address << PHY_REGADDR_OFFSET ) | (value & 0xFFFF);

        uCTL = MHal_EMAC_Read_CTL(ndev);
        MHal_EMAC_enable_mdi(ndev);

        MHal_EMAC_Write_MAN( ndev, uRegVal );
        // Wait until IDLE bit in Network Status register is cleared //
        uRegVal = MHal_EMAC_ReadReg32( ndev, REG_ETH_SR );  //Must read Low 16 bit.
        while ( !( uRegVal & EMAC_IDLE ) )
        {
            uRegVal = MHal_EMAC_ReadReg32( ndev, REG_ETH_SR );
            barrier();
        }
        MHal_EMAC_Write_CTL(ndev,uCTL);
     }
}

//-------------------------------------------------------------------------------------------------
// Read value stored in a PHY register.
// Note: MDI interface is assumed to already have been enabled.
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_read_phy(struct net_device *ndev, unsigned char phy_addr, unsigned char address, u32* value )
{
    struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(ndev);
    u32 uRegBase;  
    u32 tempvalue;
    u32 uRegVal;
    u32 uCTL;

    if (LocPtr->phytype == PHY_INTERNAL)
    {
	    uRegBase = LocPtr->reg_phy_address;
        phy_addr =0;

        tempvalue = *(volatile unsigned int *)(uRegBase + 0x04);
        tempvalue |= 0x0004;
        *(volatile unsigned int *)(uRegBase + 0x04) = tempvalue;
        udelay( 1 );
        *value = *(volatile unsigned int *)(uRegBase + address*4);
    }
    else
    {  
        uRegVal = 0;
        uCTL = 0;

        uRegVal = (EMAC_HIGH | EMAC_CODE_802_3 | EMAC_RW_R)
                | ((phy_addr & 0x1f) << PHY_ADDR_OFFSET) | (address << PHY_REGADDR_OFFSET) | (0) ;

        uCTL = MHal_EMAC_Read_CTL(ndev);
        MHal_EMAC_enable_mdi(ndev);
        MHal_EMAC_Write_MAN(ndev, uRegVal);

        //Wait until IDLE bit in Network Status register is cleared //
        uRegVal = MHal_EMAC_ReadReg32( ndev, REG_ETH_SR );  //Must read Low 16 bit.
        while ( !( uRegVal & EMAC_IDLE ) )
        {
            uRegVal = MHal_EMAC_ReadReg32( ndev, REG_ETH_SR );
            barrier();
        }
        *value = ( MHal_EMAC_Read_MAN( ndev ) & 0x0000ffff );
        MHal_EMAC_Write_CTL( ndev, uCTL );
    }
}


//-------------------------------------------------------------------------------------------------
// Update MAC speed and H/F duplex
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_update_speed_duplex(struct net_device *ndev, u32 uspeed, u32 uduplex )
{
    u32 xval;

    xval = MHal_EMAC_ReadReg32( ndev, REG_ETH_CFG ) & ~( EMAC_SPD | EMAC_FD );

    if ( uspeed == SPEED_100 )
    {
        if ( uduplex == DUPLEX_FULL )    // 100 Full Duplex //
        {
            xval = xval | EMAC_SPD | EMAC_FD;
        }
        else                           // 100 Half Duplex ///
        {
            xval = xval | EMAC_SPD;
        }
    }
    else
    {
        if ( uduplex == DUPLEX_FULL )    //10 Full Duplex //
        {
            xval = xval | EMAC_FD;
        }
        else                           // 10 Half Duplex //
        {
        }
    }
    MHal_EMAC_WritReg32( ndev, REG_ETH_CFG, xval );
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_get_SA1H_addr( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_SA1H );
}

u32 MHal_EMAC_get_SA1L_addr( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_SA1L );
}

u32 MHal_EMAC_get_SA2H_addr( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev, REG_ETH_SA2H );
}

u32 MHal_EMAC_get_SA2L_addr( struct net_device *ndev )
{
    return MHal_EMAC_ReadReg32( ndev,  REG_ETH_SA2L );
}

void MHal_EMAC_Write_SA1H( struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_ETH_SA1H, xval );
}

void MHal_EMAC_Write_SA1L( struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_ETH_SA1L, xval );
}

void MHal_EMAC_Write_SA2H( struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_ETH_SA2H, xval );
}

void MHal_EMAC_Write_SA2L( struct net_device *ndev, u32 xval )
{
    MHal_EMAC_WritReg32( ndev, REG_ETH_SA2L, xval );
}

void* MDev_memset( void* s, u32 c, unsigned long count )
{
    char* xs = ( char* ) s;

    while ( count-- )
        *xs++ = c;

    return s;
}

//-------------------------------------------------------------------------------------------------
// Check INT Done
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_CheckINTDone( struct net_device *ndev )
{
    u32 retIntStatus;
    struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(ndev);

    retIntStatus = MHal_EMAC_Read_ISR(ndev);
    LocPtr->CurUVE.cntChkINTCounter = ( LocPtr->CurUVE.cntChkINTCounter % MHal_MAX_INT_COUNTER );
    LocPtr->CurUVE.cntChkINTCounter ++;

    if ( ( retIntStatus & EMAC_INT_DONE ) ||
         ( LocPtr->CurUVE.cntChkINTCounter == ( MHal_MAX_INT_COUNTER - 1 ) ) )
    {
        LocPtr->CurUVE.flagISR_INT_DONE = 0x01;
        return TRUE;
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
// EMAC Negotiation PHY
//-------------------------------------------------------------------------------------------------
u32 MHal_EMAC_NegotiationPHY( struct net_device *ndev, int phyaddr)
{
    struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(ndev);
    // Set PHY --------------------------------------------------------------
    u32 retValue = 0;
    u32 bmsr;

    // IMPORTANT: Get real duplex by negotiation with peer.
    u32 word_ETH_CTL = MHal_EMAC_Read_CTL(ndev);
    MHal_EMAC_Write_CTL(ndev, 0x0000001C | word_ETH_CTL );

    LocPtr->CurBCE.duplex = 1;   // Set default as Half-duplex if negotiation fails.
    retValue = 1;

    LocPtr->CurUVE.flagISR_INT_DONE = 0x00;
    LocPtr->CurUVE.cntChkINTCounter = 0;
    LocPtr->CurUVE.cntChkCableConnect = 0;


    MHal_EMAC_read_phy(ndev, phyaddr, MII_BMSR, &bmsr);
    if ( (bmsr & BMSR_100FULL) || (bmsr & BMSR_10FULL) )
    {
        LocPtr->CurBCE.duplex = 2;
        retValue = 2;
    }
    else
    {
        LocPtr->CurBCE.duplex = 1;
        retValue = 1;
    }

    // NOTE: REG_ETH_CFG must be set according to new ThisBCE.duplex.

    MHal_EMAC_Write_CTL(ndev, word_ETH_CTL );
    // Set PHY --------------------------------------------------------------
    return(retValue);

}

//-------------------------------------------------------------------------------------------------
// EMAC Hardware register set
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// EMAC Timer set for Receive function
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_timer_callback( struct net_device *ndev, unsigned long value )
{
    u32 uRegVal;
    uRegVal = MHal_EMAC_Read_IER(ndev);
    uRegVal |= ( EMAC_INT_RCOM );
    MHal_EMAC_Write_IER( ndev, uRegVal );
}

//-------------------------------------------------------------------------------------------------
// EMAC clock on/off
//-------------------------------------------------------------------------------------------------
void MHal_EMAC_Power_On_Clk( struct net_device *dev )
{
    struct EMAC_private *LocPtr = (struct EMAC_private *) netdev_priv(dev);

	u32 reg_phy_bank0 = LocPtr->reg_phy_address;
	u32 reg_phy_bank1 = reg_phy_bank0 + 0x200;
	u32 reg_phy_bank2 = reg_phy_bank0 + 0x400;
	
    //emac_clk gen
#ifdef CONFIG_ETHERNET_ALBANY
    *( ( u32 * ) ( ( char * ) 0xBF203C00 + 0x1C*4 ) ) = 0x0;
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x5A*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x5A*4) ) & 0xFF00) | 0x0056 ); //gain shift
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x27*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x27*4) ) & 0x00FF) | 0x0200 ); //det max
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x28*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x28*4) ) & 0x00FF) | 0x0100 ); //det min
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x3B*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x3B*4) ) & 0x00FF) | 0x1800 ); //snr len (emc noise)
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x21*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x21*4) ) & 0x00FF) | 0x1500 ); //snr k value
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x6F*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x6F*4) ) & 0xFF00) | 0x0059 ); // PLL DIV
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x7A*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x7A*4) ) & 0xFF00) | 0x0021 ); // function PLL MOD
    *( ( u32 * ) ( ( char * ) reg_phy_bank0 +  0x39*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank0 +  0x39*4) ) & 0xFF00) | 0x0080); // lpbk_enable set to 0
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x7E*4) ) = 0; // Power-on LDO
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x5B*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x5B*4) ) & 0x00FF) | 0x0700 ); // Power-on ADC
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x65*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x65*4) ) & 0x00FF) | 0x1100 ); // Power-on BGAP
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x66*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x66*4) ) & 0xFF00) | 0x0080 ); // Power-on ADCPL
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x66*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x66*4) ) & 0x00FF) | 0xd100 ); // Power-on ADCPL
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x6A*4) ) = (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x6A*4) ) & 0xFF00); // Power-on LPF_OP
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x5C*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x5C*4) ) & 0x00FF) | 0x4000 ); // Power-on LP
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x5D*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x5D*4) ) & 0x00FF) | 0x0500 ); // Power-on REF
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x75*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x75*4) ) & 0xFF00) | 0x0046 ); // PORST
    *( ( u32 * ) ( ( char * ) reg_phy_bank2 +  0x50*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank2 +  0x50*4) ) & 0x00FF) ); // VBUF
    *( ( u32 * ) ( ( char * ) reg_phy_bank2 +  0x1D*4) ) = 0x0003; // PD_TX_IDAC, PD_TX_LD = 0
    *( ( u32 * ) ( ( char * ) reg_phy_bank2 +  0x62*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank2 +  0x62*4) ) & 0x00FF) ); // 100gat
    *( ( u32 * ) ( ( char * ) reg_phy_bank2 +  0x18*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank2 +  0x18*4) ) & 0xFF00) | 0x0043 ); // 200gat
    *( ( u32 * ) ( ( char * ) reg_phy_bank2 +  0x1c*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank2 +  0x1c*4) ) & 0x00FF) | 0x4100 ); // en_100t_phase
    *( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x6C*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank1 +  0x6C*4) ) & 0xFF00) | 0x40 ); // 100T TX performance (TEST_TX)    
    
    *( ( u32 * ) ( ( char * ) reg_phy_bank2 +  0x74*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank2 +  0x74*4) ) & 0xFF00) | 0x0006 );
    *( ( u32 * ) ( ( char * ) reg_phy_bank0 +  0x15*4) ) = ( (*( ( u32 * ) ( ( char * ) reg_phy_bank0 +  0x15*4) ) & 0x00FF) );
    
    //disable EEE
    *( ( u32 * ) ( ( char * ) reg_phy_bank0 +  0x16*4) ) = ( (*( ( u32 * ) ( ( char * ) 0xBF243000 +  0x16*4) ) & 0xFFFF) | 0x4000 );

#else
    *( ( u32 * ) ( ( char * ) 0xBF201600 +  0x60*4) ) = 0x0400;
    *( ( u32 * ) ( ( char * ) 0xBF201600 +  0x61*4) ) = 0x0004;
    *( ( u32 * ) ( ( char * ) 0xBF201600 +  0x62*4) ) = 0x0000;
#endif

    //chiptop [15] allpad_in
    *( ( u32 * ) ( ( char * ) 0xBF203C00 + 0x50*4 ) ) &= 0x7FFF;
#ifdef CONFIG_ETHERNET_ALBANY
    //chiptop pad_top [9:8]
    *( ( u32 * ) ( ( char * ) 0xBF203C00 + 0x5A*4) ) = (*( ( u32 * ) ( ( char * ) 0xBF203C00 +  0x5A*4) ) | 0x0010); //LED patch
    *( ( u32 * ) ( ( char * ) 0xBF203C00 + 0x6F*4) ) = (*( ( u32 * ) ( ( char * ) 0xBF203C00 +  0x6F*4) ) & 0xFEFF); //LED patch
#else
    *( ( u32 * ) ( ( char * ) 0xBF203C00 + 0x6F*4 ) ) |= 0x0100;  //et_mode = 1
#endif

}

void MHal_EMAC_Power_Off_Clk( struct net_device *ndev )
{
    //emac_clk gen
}
