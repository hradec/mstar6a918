#define MSTAR_MIU0_BUS_BASE                      0x20000000
#define MSTAR_MIU1_BUS_BASE                      0xA0000000
#define MSTAR_MIU0_NONCACHED_BASE                0x40000000
#define MSTAR_MIU1_NONCACHED_BASE                0xC0000000

#define FREQ_PARAMETER                           0x2978D5
#define REG_CLK_GEN_BASE                         0xFD201600
#define REG_CKG_MIPS                             REG_CLK_GEN_BASE + 0x0011*4
#define REG_FREQ_BASE                            0xFD221800
#define REG_FREQ_LOW                             REG_FREQ_BASE + 0x0060*4
#define REG_FREQ_HIGH                            REG_FREQ_BASE + 0x0061*4  
#define REG_CKG_EMAC_RX                          REG_FREQ_BASE + 0x0062*4  
#define REG_freq_low_bound_l                     REG_FREQ_BASE + 0x00A0*2
#define REG_freq_low_bound_h                     REG_FREQ_BASE + 0x00A2*2
#define REG_freq_high_bound_l                    REG_FREQ_BASE + 0x00A4*2
#define REG_freq_high_bound_h                    REG_FREQ_BASE + 0x00A6*2
#define REG_0x110C_B0                            REG_FREQ_BASE + 0x00B0*2
#define REG_0x110C_AA                            REG_FREQ_BASE + 0x00AA*2
#define REG_0x110C_AE                            REG_FREQ_BASE + 0x00AE*2
#define REG_0x110C_B2                            REG_FREQ_BASE + 0x00B2*2
#define REG_0x110C_A8                            REG_FREQ_BASE + 0x00A8*2
#define REG(addr)                                *(volatile u32 *)(addr)