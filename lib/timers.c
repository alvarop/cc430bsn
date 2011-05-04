/** @file timers.c
*
* @brief TIMER functions
*
* @author Alvaro Prieto
*/
#include "timers.h"
#include <signal.h>
#include "leds.h"

static void dummy_callback( void );

static void (*ccr_callbacks[TOTAL_CCRS + 1])( void );

/*******************************************************************************
 * @fn     void setup_timer_a( void )
 * @brief  configure timer_a
 * ****************************************************************************/
void setup_timer_a( void )
{
    uint8_t index;
    
    for( index = 0; index <= TOTAL_CCRS; index++ )
    {
      ccr_callbacks[index] = dummy_callback;
    }
  	//TA0CCTL0 = CCIE;                       			// CCR0 interrupt enabled
  	//TA0CCTL1 = CCIE;                       			// CCR1 interrupt enabled
  	//TA0CCTL2 = CCIE;                       			// CCR2 interrupt enabled

  	//TA0CCR0 = TIMER_LIMIT;
  	//TA0CCR1 = ADC12_SAMPLE_RATE;
  	//TA0CCR2 = OLED_REFRESH_RATE; 					// 16-bit, 32 ~= 1 ms, 32000 ~= 1 sec

    // ACLK, continuos mode, clear TAR
		// ACLK used so that counter remains active in LPM
  	TA0CTL = TASSEL__ACLK + MC_2 + TAIE + TACLR;	

}

/*******************************************************************************
 * @fn     void setup_timer_a( void (*callback)(void), uint8_t ccr_number )
 * @brief  configure timer_a
 * ****************************************************************************/
void register_timer_callback( void (*callback)(void), uint8_t ccr_number )
{
  if( ccr_number <= (TOTAL_CCRS + 1) )
  {
    ccr_callbacks[ccr_number] = callback;
  }
  return;
}

/*******************************************************************************
 * @fn     set_ccr( uint8_t ccr_index, uint16_t value )
 * @brief  set the CCR value and enable interrupts on it
 * ****************************************************************************/
void set_ccr( uint8_t ccr_index, uint16_t value )
{
  // (TA0CCR0_ + (ccr_index << 2)) = value;
  // NOTE, this could be done in a simpler way by adding the ccr_index/2 to the
  // address of TA0CCR0. The compiler is angry and I can't figure out why, so
  // this is a temporary fix...
  switch (ccr_index)
  {
    case (0):
    {
      TA0CCR0 = value;
      TA0CCTL0 = CCIE;
      break;
    }
    case (1):
    {
      TA0CCR1 = value;
      TA0CCTL1 = CCIE;
      break;
    }
    case (2):
    {
      TA0CCR2 = value;
      TA0CCTL2 = CCIE;
      break;
    }
    case (3):
    {
      TA0CCR3 = value;
      TA0CCTL3 = CCIE;
      break;
    }
    case (4):
    {
      TA0CCR4 = value;
      TA0CCTL4 = CCIE;
      break;
    }
  }
}

/*******************************************************************************
 * @fn     clear_ccr( uint8_t ccr_index )
 * @brief  clear the CCR value and disable interrupts on it
 * ****************************************************************************/
void clear_ccr( uint8_t ccr_index )
{
  switch (ccr_index)
  {
    case (0):
    {
      TA0CCR0 = 0;
      TA0CCTL0 &= ~CCIE;
      break;
    }
    case (1):
    {
      TA0CCR1 = 0;
      TA0CCTL1 &= ~CCIE;
      break;
    }
    case (2):
    {
      TA0CCR2 = 0;
      TA0CCTL2 &= ~CCIE;
      break;
    }
    case (3):
    {
      TA0CCR3 = 0;
      TA0CCTL3 &= ~CCIE;
      break;
    }
    case (4):
    {
      TA0CCR4 = 0;
      TA0CCTL4 &= ~CCIE;
      break;
    }
  }
}

/*******************************************************************************
 * @fn     increment_ccr( uint8_t ccr_index, uint16_t value )
 * @brief  increment the CCR by value
 * ****************************************************************************/
void increment_ccr( uint8_t ccr_index, uint16_t value )
{
  switch (ccr_index)
  {
    case (0):
    {
      TA0CCR0 += value;
      break;
    }
    case (1):
    {
      TA0CCR1 += value;
      break;
    }
    case (2):
    {
      TA0CCR2 += value;
      break;
    }
    case (3):
    {
      TA0CCR3 += value;
      break;
    }
    case (4):
    {
      TA0CCR4 += value;
      break;
    }
  }
}

/*******************************************************************************
 * @fn     void dummy_callback( void )
 * @brief  empty function works as default callback
 * ****************************************************************************/
static void dummy_callback( void )
{
  
}

 /*******************************************************************************
 * @fn     void timerA0Interrupt( void )
 * @brief  Timer0 A0 Interrupt vector for CCR0
 * ****************************************************************************/
wakeup interrupt (TIMER0_A0_VECTOR) timerA0Interrupt(void)
{  

  ccr_callbacks[0]();

}

/*******************************************************************************
 * @fn     void timerA1Interrupt( void )
 * @brief  Timer0 A1 Interrupt vector for CCR1-4 and overflow
 * ****************************************************************************/
wakeup interrupt (TIMER0_A1_VECTOR) timerA1Interrupt(void) // CHANGE
{

	//
	// Figure out what caused the interrupt and respond accordingly
	//
	switch ( TA0IV )
	{

		case ( TIV_CCR1 ):
    {
      ccr_callbacks[1]();
		  break;
    }
    
    case ( TIV_CCR2 ):
    {
      ccr_callbacks[2]();
		  break;
    }
    
    case ( TIV_CCR3 ):
    {
      ccr_callbacks[3]();
		  break;
    }
    
    case ( TIV_CCR4 ):
    {
      ccr_callbacks[4]();
		  break;
    }
    
		case ( TIV_OVERFLOW ):
    { 
      ccr_callbacks[5]();
			break;
    }
    
		default:
    {
			break;
		}
	}

}

