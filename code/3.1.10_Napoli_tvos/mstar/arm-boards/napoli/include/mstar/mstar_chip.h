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
#define REG(addr)                                *(volatile u32 *)(addr)