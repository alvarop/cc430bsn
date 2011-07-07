/** @file leds.c
*
* @brief LED functions
*
* @author Alvaro Prieto
*/
#include "leds.h"

// Need to change which port the LEDs are in depending on device
// There must be a better way of doing this...
#if defined(__CC430F6137__)

#define LEDOUT PJOUT
#define LEDDIR PJDIR

#elif defined(__MSP430F2274__)

#define LEDOUT P1OUT
#define LEDDIR P1DIR

#endif

/*******************************************************************************
 * @fn     void setup_leds( void )
 * @brief  configure led ports and initialize OFF
 * ****************************************************************************/
void setup_leds( void )
{
 
#if defined(__CC430F6137__)

  // Initialize outputs OFF
  LEDOUT  &= ~0x07;

  // Set pins J.0, J.1, and J.2 to outputs
  LEDDIR  |= 0x07;
  
#elif defined(__MSP430F2274__)

  // Initialize outputs OFF
  LEDOUT  &= ~0x03;

  // Set pins J.0, J.1, and J.2 to outputs
  LEDDIR  |= 0x03;

#endif
}

/*******************************************************************************
 * @fn     void led1_on( void )
 * @brief  turn on LED1
 * ****************************************************************************/
void __inline__ led1_on( )
{
  LEDOUT |= 0x01;
}

/*******************************************************************************
 * @fn     void led1_off( void )
 * @brief  turn off LED1
 * ****************************************************************************/
void __inline__ led1_off( )
{
  LEDOUT &= ~0x01;
}

/*******************************************************************************
 * @fn     void led1_toggle( void )
 * @brief  toggle LED1
 * ****************************************************************************/
void __inline__ led1_toggle( )
{
  LEDOUT ^= 0x01;
}

/*******************************************************************************
 * @fn     void led2_on( void )
 * @brief  turn on LED2
 * ****************************************************************************/
void __inline__ led2_on( )
{
  LEDOUT |= 0x02;
}

/*******************************************************************************
 * @fn     void led2_off( void )
 * @brief  turn off LED2
 * ****************************************************************************/
void __inline__ led2_off( )
{
  LEDOUT &= ~0x02;
}

/*******************************************************************************
 * @fn     void led2_toggle( void )
 * @brief  toggle LED2
 * ****************************************************************************/
void __inline__ led2_toggle( )
{
  LEDOUT ^= 0x02;
}
#if defined(__CC430F6137__)
/*******************************************************************************
 * @fn     void led3_on( void )
 * @brief  turn on LED3
 * ****************************************************************************/
void __inline__ led3_on( )
{
  LEDOUT |= 0x04;
}

/*******************************************************************************
 * @fn     void led3_off( void )
 * @brief  turn off LED23
 * ****************************************************************************/
void __inline__ led3_off( )
{
  LEDOUT &= ~0x04;
}

/*******************************************************************************
 * @fn     void led3_toggle( void )
 * @brief  toggle LED3
 * ****************************************************************************/
void __inline__ led3_toggle( )
{
  LEDOUT ^= 0x04;
}
#endif
/*******************************************************************************
 * @fn     void leds_write( uint8_t value )
 * @brief  change all LEDs at once
 * ****************************************************************************/
void __inline__ leds_write( uint8_t value )
{
  LEDOUT &= ~0x07; // Clear LEDS
  LEDOUT ^= ( value ) & 0x07; // Write value
}

