// sys.c
#include "sys.h"  
//	 

//********************************************************************************
//????
//?
//  


//THUMB?????????
//??????????????WFI  
__asm void WFI_SET(void)
{
	WFI;		  
}
//??????(?????fault?NMI??)
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
//??????
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
//??????
//addr:????
__asm void MSR_MSP(uint32_t	addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}
