/** @file intrinsics.c
*
* @brief intrinsic functions for compatibility with some code
*
* @author Alvaro Prieto
*         derived from: http://old.nabble.com/Intrinsic-functions-used-in-the-TI-Sports-Watch-source-code-td28745754.html
*/
#include "intrinsics.h"

/*******************************************************************************
 * @fn     void _delay_cycles( void )
 * @brief  delay for 'cycles' number of cycles
 * used by: rf1a.c, bsp_board.c 
 * ****************************************************************************/
void _delay_cycles(unsigned long cycles)
{
   
   for( ; cycles > 0; cycles-- ) {
      nop();
      nop();
   }

}

/*******************************************************************************
 * @fn     void __set_interrupt_state( void )
 * @brief  set interrupt to state
 * used by: bsp_msp430_defs.h, rf1a.c 
 * ****************************************************************************/
void  __set_interrupt_state(unsigned short state)
{
   __asm__("bis %0,r2" : : "ir" ((uint16_t) state));
}

/*******************************************************************************
 * @fn     unsigned short __even_in_range(unsigned short value, 
                                                unsigned short bound)
 * @brief  used in some switch statements
 * used by: Port_1( ), RTC_ISR( ), ADC12ISR( ) 
 * ****************************************************************************/
unsigned short __even_in_range(unsigned short value, unsigned short bound)
{
	return ( (value <= bound) && (value >= 0) && !(value % 2) ) ? value : 0;
}

//__get_interrupt_state  
/*******************************************************************************
 * @fn     void __get_interrupt_state( void )
 * @brief  get interrupt to state
 * used by: bsp_msp430_defs.h, rf1a.c
 * ****************************************************************************/
unsigned short __get_interrupt_state(void)
{
   return(READ_SR & 0x0008);
}

