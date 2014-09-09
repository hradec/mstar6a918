/*

typedef enum {
    TZ_LLV_R0,   // = 0, Arm GP R0
    TZ_LLV_R1,
    TZ_LLV_R2,
    TZ_LLV_R3,
    TZ_LLV_R4,
    TZ_LLV_R5,
    TZ_LLV_R6,
    TZ_LLV_R7,
    TZ_LLV_R8,
    TZ_LLV_R9,
    TZ_LLV_R10,
    TZ_LLV_R11,
    TZ_LLV_R12,
    TZ_LLV_R13,  // = 13, Arm Stack Pointer
    TZ_LLV_R14,  // = 14, Arm Link Register
    TZ_LLV_CPSR, // = 15, Arm Current Processor Status Register
    TZ_LLV_SPSR,    
    TZ_LLV_REGS_MAX
}TZ_LLV_REGS;
*/
/*
typedef enum {

    TZ_LLV_MODE_SVC,
    TZ_LLV_MODE_USR,
    TZ_LLV_MODE_IRQ,
    TZ_LLV_MODE_FIQ,
    TZ_LLV_MODE_MON,
    TZ_LLV_MODE_MAX

}TZ_LLV_CPU_MODE;
*/
/*
typedef enum {
      TZ_LLV_RES_OK                      //= 0
     ,TZ_LLV_RES_ERR                     //= 1
     ,TZ_LLV_RES_UND                     //= 2
     ,TZ_LLV_RES_MAX
}TZ_LLV_Result;
*/

//#include <linux/spinlock.h>
//#include <linux/mutex.h>
#include <linux/rwsem.h>
struct smc_struct{

    volatile unsigned int cmd1;
    volatile unsigned int cmd2;

    unsigned int smc_flag;
    volatile unsigned int* private_data;
    struct rw_semaphore smc_lock;
};

void smc_call(struct smc_struct *tz);

//TZ_LLV_Result Tz_LLV_Ctrl_Get_Reg(TZ_LLV_REGS reg, int *preg);
//TZ_LLV_Result Tz_LLV_Ctrl_SMC(unsigned int *args_1, unsigned int *args_2, unsigned int *args_3, unsigned int *args_4);

