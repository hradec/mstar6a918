
typedef enum
{
      TZ_RESULT_OK			//= 0
     ,TZ_RESULT_ERR			//= 1
     ,TZ_RESULT_UND			//= 2
     ,TZ_RESULT_MAX
} TZResult; 

//This filed predefine the opcode of SMC protocol, 
typedef enum
{
     TZ_SMC_CLASS_SMC_NORMAL_CALL		//= 0
    ,TZ_SMC_CLASS_INTERRUPT			//= 1  
    ,TZ_SMC_CLASS_WSM                        //= 2
    ,TZ_SMC_CLASS_MAX                

} TZ_SMC_CLASS; 

