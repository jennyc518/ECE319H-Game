

/*
 * Switch.cpp
 *
 *      Author: Amy and Jenny
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){

  IOMUX->SECCFG.PINCM[PA12INDEX] |= 0x00040081; // input, no pull
  //IOMUX->SECCFG.PINCM[PA28INDEX] |= 0x00040081; // input, no pull
  IOMUX->SECCFG.PINCM[PA15INDEX] |= 0x00040081; // input, no pull
  //IOMUX->SECCFG.PINCM[PA16INDEX] |= 0x00040081; // input, no pull
  IOMUX->SECCFG.PINCM[PA17INDEX] |= 0x00040081; // input, no pull
  IOMUX->SECCFG.PINCM[PA18INDEX] |= 0x00040081; // input, no pull
  IOMUX->SECCFG.PINCM[PA31INDEX] |= 0x00040081; // input, no pull
  IOMUX->SECCFG.PINCM[PA8INDEX] |= 0x00040081; // input, no pull

  
  //   // Player 1
  // IOMUX->SECCFG.PINCM[PA15INDEX] |= 0x00040081; // input, no pull
  // IOMUX->SECCFG.PINCM[PA16INDEX] |= 0x00040081; // input, no pull
  // IOMUX->SECCFG.PINCM[PA17INDEX] |= 0x00040081; // input, no pull
  // IOMUX->SECCFG.PINCM[PA18INDEX] |= 0x00040081; // input, no pull

  // // Player 2
  // IOMUX->SECCFG.PINCM[PA10INDEX] |= 0x00040081; // input, no pull
  // IOMUX->SECCFG.PINCM[PA11INDEX] |= 0x00040081; // input, no pull
  // IOMUX->SECCFG.PINCM[PA25INDEX] |= 0x00040081; // input, no pull
  // IOMUX->SECCFG.PINCM[PA24INDEX] |= 0x00040081; // input, no pull
}

// return current state of switches
uint32_t Switch_In(void){
  //   // write this
  // uint32_t data = GPIOA->DIN31_0;
  // // player 2 in bits 3-0, in order above
  // // player 1 in bits 7-4
  // data = ((data>>9)&0x4) || ((data>>24)&0x3) || ((data>>7)&0x8) || ((data>>8)&0x80) || ((data>>10)&0x40) || ((data>>12)&0x20) || ((data>>14)&0x10);
  // return data; // return 0; //replace this your code

  uint32_t gpio = GPIOA->DIN31_0;
  uint32_t sw = 0;

    // ----------- PLAYER 2 (bits 3-0) -----------
  if(gpio & (1 << 12)) sw |= (1 << 3); // Left  -> bit 3
  //if(gpio & (1 << 28)) sw |= (1 << 2); // Down  -> bit 2

  // ----------- PLAYER 1 (bits 7-4) -----------
  if(gpio & (1 << 15)) sw |= (1 << 7); // Left  -> bit 7
  //if(gpio & (1 << 16)) sw |= (1 << 6); // Down  -> bit 6
  if(gpio & (1 << 17)) sw |= (1 << 5); // Jump  -> bit 5
  if(gpio & (1 << 18)) sw |= (1 << 4); // Right -> bit 4

  if(gpio & (1 << 31)) sw |= (1 << 0); // Right -> bit 0
  if(gpio & (1 << 8)) sw |= (1 << 1); // Jump  -> bit 1

  // // ----------- PLAYER 2 (bits 3-0) -----------
  // if(gpio & (1 << 10)) sw |= (1 << 3); // Left  -> bit 3
  // if(gpio & (1 << 11)) sw |= (1 << 2); // Down  -> bit 2
  // if(gpio & (1 << 25)) sw |= (1 << 1); // Jump  -> bit 1
  // if(gpio & (1 << 24)) sw |= (1 << 0); // Right -> bit 0

  // // ----------- PLAYER 1 (bits 7-4) -----------
  // if(gpio & (1 << 15)) sw |= (1 << 7); // Left  -> bit 7
  // if(gpio & (1 << 16)) sw |= (1 << 6); // Down  -> bit 6
  // if(gpio & (1 << 17)) sw |= (1 << 5); // Jump  -> bit 5
  // if(gpio & (1 << 18)) sw |= (1 << 4); // Right -> bit 4

  return sw;

}
