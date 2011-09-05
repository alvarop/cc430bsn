/** @file timers.c
*
* @brief TIMER functions
*
* @author Alvaro Prieto
*/
#include "timers.h"
#include <signal.h>

#if defined(__CC430F6137__)
#define TIMERA_CCR0_VECTOR TIMER0_A0_VECTOR
#define TIMERA_OTHER_VECTOR TIMER0_A1_VECTOR
#elif defined(__MSP430F2274__)
#define TIMERA_CCR0_VECTOR TIMERA0_VECTOR
#define TIMERA_OTHER_VECTOR TIMERA1_VECTOR
#endif

static uint8_t dummy_callback( void );

// Holds pointers to all callback functions for CCR registers (and overflow)
static uint8_t (*ccr_callbacks[TOTAL_CCRS + 1])( void ) ;
static uint8_t timer_mode;

/*******************************************************************************
 * @fn     void setup_timer_a( uint8_t mode )
 * @brief  Initialize callback functions and start timer in up mode
 * ****************************************************************************/
void setup_timer_a( uint8_t mode )
{
  uint8_t index;
  timer_mode = mode;
  
  // Make sure all callback functions are pointing somewhere
  for( index = 0; index <= TOTAL_CCRS; index++ )
  {
    ccr_callbacks[index] = dummy_callback;
  }      		
	
#if defined(__CC430F6137__)
  // ACLK, clear TAR
	// ACLK used so that counter remains active in LPM
	TA0CTL = TASSEL__ACLK + timer_mode + TAIE + TACLR;
#elif defined(__MSP430F2274__)
  // SMCLK, clear TAR
  // SMCLK used because there is no crystal to drive ACLK in the ez430-rf2500
  TA0CTL = TASSEL__SMCLK + timer_mode + TAIE + TACLR;    
#endif
	

}

/*******************************************************************************
 * @fn     register_timer_callback( uint8_t (*callback)(void), uint8_t ccr_number )
 * @brief  add callback function for CCR[ccr_number]
 * ****************************************************************************/
void register_timer_callback( uint8_t (*callback)(void), uint8_t ccr_number )
{
  if( ccr_number <= (TOTAL_CCRS) )
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
#if defined(__CC430F6137__)
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
#endif
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
#if defined(__CC430F6137__)
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
#endif
    default:
    {
      //Shouldn't happen... 
      break;   
    }
  }
}

/*******************************************************************************
 * @fn     increment_ccr( uint8_t ccr_index, uint16_t value )
 * @brief  increment the CCR by [value]
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
#if defined(__CC430F6137__)
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
#endif
    default:
    {
      //Shouldn't happen...  
      break;  
    }
  }
}

/*******************************************************************************
 * @fn     void dummy_callback( void )
 * @brief  empty function works as default callback
 * ****************************************************************************/
inline void clear_timer()
{
#if defined(__CC430F6137__)
  // ACLK, clear TAR
	// ACLK used so that counter remains active in LPM
	TA0CTL = TASSEL__ACLK + timer_mode + TAIE + TACLR;
#elif defined(__MSP430F2274__)
  // SMCLK, clear TAR
  // SMCLK used because there is no crystal to drive ACLK in the ez430-rf2500
  TA0CTL = TASSEL__SMCLK + timer_mode + TAIE + TACLR;    
#endif
}

/*******************************************************************************
 * @fn     void dummy_callback( void )
 * @brief  empty function works as default callback
 * ****************************************************************************/
static uint8_t dummy_callback( void )
{
  __no_operation();

  return 0;
}

/*******************************************************************************
 * @fn     void timerA0Interrupt( void )
 * @brief  Timer0 A0 Interrupt vector for CCR0
 * ****************************************************************************/
interrupt (TIMERA_CCR0_VECTOR) timerA0Interrupt(void)
{  

  if ( ccr_callbacks[0]() )
  {
    __bic_SR_register_on_exit(LPM1_bits);
  }

}

/*******************************************************************************
 * @fn     void timerA1Interrupt( void )
 * @brief  Timer0 A1 Interrupt vector for CCR1-4 and overflow
 * ****************************************************************************/
interrupt (TIMERA_OTHER_VECTOR) timerA1Interrupt(void) // CHANGE
{
  uint8_t wake_up = 0;
	//
	// Figure out what caused the interrupt and respond accordingly
	//
	switch ( TA0IV )
	{

		case ( TIV_CCR1 ):
    {
      wake_up = ccr_callbacks[1]();
		  break;
    }
    
    case ( TIV_CCR2 ):
    {
      wake_up = ccr_callbacks[2]();
		  break;
    }
    
    case ( TIV_CCR3 ):
    {
      wake_up = ccr_callbacks[3]();
		  break;
    }
    
    case ( TIV_CCR4 ):
    {
      wake_up = ccr_callbacks[4]();
		  break;
    }
    
		case ( TIV_OVERFLOW ):
    { 
      wake_up = ccr_callbacks[5]();
			break;
    }
    
		default:
    {
			break;
		}
	}
	
	// Depending on the return value of the callback function, exit LPM3
	if( wake_up )
	{
	  __bic_SR_register_on_exit(LPM1_bits);
	}

}

