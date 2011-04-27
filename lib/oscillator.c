/** @file oscillator.c
*
* @brief Oscillator configuration functions
*
* @author Alvaro Prieto
*         derived from work by Corey Provencher
*/
#include "oscillator.h"
#include "intrinsics.h"
/*******************************************************************************
 * @fn     void setup_oscillator( void )
 * @brief  configures oscillator for 12MHz operation derived from 32.768kHz
 *       low-frequency crystal driving ACLK.
 * ****************************************************************************/
void setup_oscillator( void )
{
  // ---------------------------------------------------------------------
  // Enable 32kHz ACLK
  P5SEL |= 0x03;                            // Select XIN, XOUT on P5.0 and P5.1
  UCSCTL6 &= ~(XT1OFF + XT1DRIVE_3);        // XT1 On, Lowest drive strength
  UCSCTL6 |= XCAP_3;                        // Internal load cap

  UCSCTL3 = SELA__XT1CLK;                   // Select XT1 as FLL reference
  UCSCTL4 = SELA__XT1CLK | SELS__DCOCLKDIV | SELM__DCOCLKDIV; // SMCLK == MCLK

  // ---------------------------------------------------------------------
  // Configure CPU clock for 12MHz
  _BIS_SR(SCG0);                  // Disable the FLL control loop
  UCSCTL0 = 0x0000;          // Set lowest possible DCOx, MODx
  UCSCTL1 = DCORSEL_5;       // Select suitable range
  UCSCTL2 = FLLD_1 + 0x16E;  // Set DCO Multiplier
  _BIC_SR(SCG0);                  // Enable the FLL control loop

  // Worst-case settling time for the DCO when the DCO range bits have been
  // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
  // UG for optimization.
  // 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
  _delay_cycles(250000); // CHANGE

  // Loop until XT1 & DCO stabilizes, use do-while to insure that
  // body is executed at least once
  do
  {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
  } while ((SFRIFG1 & OFIFG));
}
