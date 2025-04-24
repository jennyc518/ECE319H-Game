// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H
// Amy and Jenny
// Last Modified: 04/23/25

#include <cstdint>
#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/SlidePot.h"
#include "../inc/DAC5.h"
#include "SmallFont.h"
#include "LED.h"
#include "Player.h"
#include "Switch.h"
#include "Sound.h"
#include "Platform.h"
#include "images/images.h"
#include "../inc/ST7735.h"

extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);
extern "C" void TIMG0_IRQHandler(void);
// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}

SlidePot Sensor(1605,211); // copy calibration from Lab 7

Player player1 = Player(10, 147, Sprite1, runner);
Player player2 = Player(100, 147, Sprite2, it);

Player it_arrow = Player(player2.getX(), player2.getY() - 13, arrow, runner);

Player explosion = Player(100, 147, Collision, it);

uint32_t jflag = 0;
bool jumpHeld = false;
bool fallHeld = false;

// default to 30 seconds
volatile uint32_t time = 135;
bool gameStart = false;
bool gameOver = false;
int map = 0;

uint32_t lang = 0; // 0 is english, 1 is spanish

uint32_t prevdata = 0;

void TIMG0_IRQHandler(void) {
  uint32_t stat = TIMG0->CPU_INT.IIDX;
  if ((stat) == 1) {
    if (gameStart == true) {
      time -= 1;
      if (time == 0) {
        gameOver = true;
      }
    }
  }
}

// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
    // 1) sample slide pot
    uint32_t num = Sensor.In();
    Sensor.Save(num);
    // 2) read input switches
    player1.set_vx(0);
    player2.set_vx(0);

    uint32_t sw = Switch_In();

      // ----------- PLAYER 2 (bits 3–0) -----------
      if(sw & 0x08) player2.set_vx(-2);  // Left (bit 3)
      if(sw & 0x04) player2.set_vy(2);   // Down (bit 2)
      if(sw & 0x02) player2.jump();      // Up/Jump (bit 1)
      if(sw & 0x01) player2.set_vx(2);   // Right (bit 0)


      // ----------- PLAYER 1 (bits 7–4) -----------
      if(sw & 0x80) player1.set_vx(-2);  // Left (bit 7)
      if(sw & 0x40) player1.set_vy(2);   // Down (bit 6)
      if(sw & 0x20) player1.jump();      // Up/Jump (bit 5)
      if(sw & 0x10) player1.set_vx(2);   // Right (bit 4)
      // 3) move sprites
     if(!player1.onGround) player1.applyGravity();
    if(!player2.onGround) player2.applyGravity();
    player1.move();
    player2.move();

    if(player1.get_role() == it){
      it_arrow.setPos(player1.getX(), player1.getY() - 13);
    } else if(player2.get_role() == it){
      it_arrow.setPos(player2.getX(), player2.getY() - 13);
    }

    if(player1.checkCollision(player2)){
      if(player1.get_role() == runner){
        player1.set_role(it);
        player2.set_role(runner);
      } else {
        player1.set_role(runner);
        player2.set_role(it);
      }
      explosion.collide((player1.getX()+player2.getX())/2, (player1.getY()+player2.getY())/2);
      Sound_Collide();
      player1.reset(10, 147);
      player2.reset(100, 147);
      if(player1.get_role() == runner) it_arrow.reset(10, 134);
      else it_arrow.reset(100, 134);
    }
    // 4) start sounds
    if (gameStart) {
      uint32_t changed = prevdata ^ sw; // 01 10 = 11
      uint32_t pressed = changed & sw; // 11 10 = 10
      uint32_t released = changed & ~sw; // 11 01 = 01

      // if (!((sw & 0x02) || (sw & 0x20))) {
      //   jumpHeld = false;
      // }
      // if (jumpHeld && !Sound_Playing()) {
      //   Sound_Jump();
      // }
      // else if (fallHeld && !Sound_Playing()) {
      //   Sound_Jump();
      // }
      if (pressed & 0x22) {
        jumpHeld = true;
        Sound_Jump();
      }
      else if (released & 0x22) {
        jumpHeld = false;
        Sound_Stop();
      }
      else if (jumpHeld & (!Sound_Playing())) {
        Sound_Jump();
      }
      if (pressed & 0x44) {
        fallHeld = true;
        Sound_Land();
      }
      else if (released & 0x44) {
        fallHeld = false;
        Sound_Stop();
      }
      else if (fallHeld & (!Sound_Playing())) {
        Sound_Land();
      }
      prevdata = sw;
    }
    else {
      if (sw != 0) {
        Sound_Menu();
      }
    }
    // 5) set semaphore
    jflag = 1;
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish} Language_t;
Language_t myLanguage;

typedef enum {
  TITLE,
  SELECT_LANGUAGE,
  LANG_ENGLISH,
  LANG_SPANISH,
  CONTINUE,
  SELECT_DURATION,
  TIME_30,
  TIME_60,
  SELECT_MAP,
  MAP_1,
  MAP_2,
  MAP_3,
  SELECT_MODE,
  MODE_TAG,
  MODE_SPRINT,
  MODE_MAYHEM,
  GAME_OVER,
  YELLOW_WIN,
  PURPLE_WIN, 
  PLAY_AGAIN
} phrase_t;

const char *Phrases[][2] = {
  {"TAG!", "TAG!"},
  {"Use slidepot to \nselect language:", "Usa el \npotenciometro para \nseleccionar \nel idioma:"},
  {"English", "Ingles"},
  {"Spanish", "Espanol"},
  {"Press any button \nto continue!", "Presione cualquier \nboton para \ncontinuar!"},
  {"How long would you \nlike to play?", "Cuanto tiempo \nte gustaria jugar?"}, // \xA8Cu\xA0nto tiempo te gustar\xA1a jugar?
  {"30 seconds", "30 segundos"},
  {"60 seconds", "60 segundos"},
  {"Which map do you \nwant to play on?", "En que mapa \nquieres jugar?"},
  {"Map One", "Mapa Uno"},
  {"Map Two", "Mapa Dos"},
  {"Map Three", "Mapa Tres"},
  {"Which game mode \nwould you like?", "Que modo de \njuego te gustaria?"},
  {"Tag Frenzy", "Tag Frenzy"},
  {"Hyper Sprint", "Hyper Sprint"},
  {"Mayhem Mode", "Mayhem Mode"},
  {"GAME OVER!", "JUEGO TERMINADO!"},
  {"Yellow wins!", "Amarillo gana!"},
  {"Purple wins!","Morado gana!"},
  {"Press any button \nto play again!", "Presione cualquier \nboton para jugar \nde nuevo!"}
};

void PrintPhrase(phrase_t phrase){
  ST7735_OutString((char *)Phrases[phrase][myLanguage]);
  ST7735_OutChar('\n');
}

// use main1 to observe special characters
int main1(void){ // main1
    char l;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();

  // ST7735_DrawBitmap(0, 160, Intro, 128, 160);

  // -------------------------------------------------
  // SCREEN 1 : INTRO - Language Selection (ENGLISH)
  // -------------------------------------------------
  ST7735_FillScreen(ST7735_BLACK);
  myLanguage = English;

  ST7735_SetCursor(0, 0);
  PrintPhrase(TITLE);
  PrintPhrase(SELECT_LANGUAGE);
  PrintPhrase(LANG_ENGLISH);
  PrintPhrase(LANG_SPANISH);
  PrintPhrase(CONTINUE);

  // -------------------------------------------------
  // SCREEN 1 : INTRO - Language Selection (SPANISH)
  // -------------------------------------------------
  ST7735_FillScreen(ST7735_BLACK);
  myLanguage = Spanish;

  ST7735_SetCursor(0, 0);
  PrintPhrase(TITLE);
  PrintPhrase(SELECT_LANGUAGE);
  PrintPhrase(LANG_ENGLISH);
  PrintPhrase(LANG_SPANISH);
  PrintPhrase(CONTINUE);

  // -------------------------------------------------
  // SCREEN 2 : DURATION SELECTION (ENGLISH)
  // -------------------------------------------------
  ST7735_FillScreen(ST7735_BLACK);
  myLanguage = English;

  ST7735_SetCursor(0, 0);
  PrintPhrase(SELECT_DURATION);
  PrintPhrase(TIME_30);
  PrintPhrase(TIME_60);

  // -------------------------------------------------
  // SCREEN 2 : DURATION SELECTION (SPANISH)
  // -------------------------------------------------
  ST7735_FillScreen(ST7735_BLACK);
  myLanguage = Spanish;

  ST7735_SetCursor(0, 0);
  PrintPhrase(SELECT_DURATION);
  PrintPhrase(TIME_30);
  PrintPhrase(TIME_60);

  // -------------------------------------------------
  // SCREEN 3 : MAP SELECTION (ENGLISH)
  // -------------------------------------------------
  ST7735_FillScreen(ST7735_BLACK);
  myLanguage = English;

  ST7735_SetCursor(0, 0);
  PrintPhrase(SELECT_MAP);
  PrintPhrase(MAP_1);
  PrintPhrase(MAP_2);
  PrintPhrase(MAP_3);

  // -------------------------------------------------
  // SCREEN 3 : MAP SELECTION (SPANISH)
  // -------------------------------------------------
  ST7735_FillScreen(ST7735_BLACK);
  myLanguage = Spanish;

  ST7735_SetCursor(0, 0);
  PrintPhrase(SELECT_MAP);
  PrintPhrase(MAP_1);
  PrintPhrase(MAP_2);
  PrintPhrase(MAP_3);

  // -------------------------------------------------
  // SCREEN 4 : MODE SELECTION (ENGLISH)
  // -------------------------------------------------
  ST7735_FillScreen(ST7735_BLACK);
  myLanguage = English;

  ST7735_SetCursor(0, 0);
  PrintPhrase(SELECT_MODE);
  PrintPhrase(MODE_TAG);
  PrintPhrase(MODE_SPRINT);
  PrintPhrase(MODE_MAYHEM);

  // -------------------------------------------------
  // SCREEN 4 : MODE SELECTION (SPANISH)
  // -------------------------------------------------
  ST7735_FillScreen(ST7735_BLACK);
  myLanguage = Spanish;

  ST7735_SetCursor(0, 0);
  PrintPhrase(SELECT_MODE);
  PrintPhrase(MODE_TAG);
  PrintPhrase(MODE_SPRINT);
  PrintPhrase(MODE_MAYHEM);


  while(1){}

  // ST7735_DrawBitmap(0, 160, Intro, 128, 160);

  // ST7735_SetCursor(30, 100);
  // ST7735_OutString((char *)Game_English);
  // ST7735_OutString((char *)English_1);
  // ST7735_OutString((char *)English_2);

  // for(int myPhrase=0; myPhrase<= 2; myPhrase++){
  //   for(int myL=0; myL<= 3; myL++){
  //        ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
  //     ST7735_OutChar(' ');
  //        ST7735_OutString((char *)Phrases[myPhrase][myL]);
  //     ST7735_OutChar(13);
  //   }
  // }
  // Clock_Delay1ms(3000);
  // ST7735_FillScreen(0x0000);       // set screen to black
  // l = 128;
  // while(1){
  //   Clock_Delay1ms(2000);
  //   for(int j=0; j < 3; j++){
  //     for(int i=0;i<16;i++){
  //       ST7735_SetCursor(7*j+0,i);
  //       ST7735_OutUDec(l);
  //       ST7735_OutChar(' ');
  //       ST7735_OutChar(' ');
  //       ST7735_SetCursor(7*j+4,i);
  //       ST7735_OutChar(l);
  //       l++;
  //     }
  //   }
  // }



}

// use main2 to observe graphics
int main2(void){ // main2
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_WHITE);
  // ST7735_DrawBitmap(0, 159, Map2, 128, 160);
  ST7735_DrawBitmap(4, 159, Sprite1, 13, 13);
  ST7735_DrawBitmap(20, 50, Sprite2, 13, 13);
  // ST7735_DrawBitmap(22, 159, Player1, 18,8); // player ship bottom
  // ST7735_DrawBitmap(53, 151, Player2, 18,5);
  // ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  // ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  // ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom
  // ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  // ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  // ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  // ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  // ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);

  while(1){
  }

  for(uint32_t t=500;t>0;t=t-5){
    SmallFont_OutVertical(t,104,6); // top left
    Clock_Delay1ms(50);              // delay 50 msec
  }
  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString((char *)"GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString((char *)"Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString((char *)"Earthling!");
  ST7735_SetCursor(2, 4);
  ST7735_OutUDec(1234);
  // while(1){
  // }
}

// use main3 to test switches and LEDs
int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  while(1){
    // write code to test switches and LEDs

  }
}
// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    now = Switch_In(); // one of your buttons
    if((now & 0x04) || (now & 0x40)){
      Sound_Land(); // call one of your sounds
    }
    if((now & 0x02) || (now & 0x20)){
      Sound_Jump(); // call one of your sounds
    }
    // modify this to test all your sounds
  }
}



// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz

  TimerG12_IntArm(2666666, 0); 
  TimerG0_IntArm(40000, 1000, 1);
  
  // initialize all data structures
  __enable_irq();
  
  // -------------------------------------------------
  // SCREEN 1 : INTRO - Language Selection (ENGLISH)
  // -------------------------------------------------
  // ST7735_FillScreen(ST7735_BLACK);
  // myLanguage = English;
  while(1) {
    uint32_t sw = 0;
    uint32_t slide = 0;
    //   ST7735_SetCursor(0, 0);
    //   PrintPhrase(TITLE);
    //   PrintPhrase(SELECT_LANGUAGE);
    //   PrintPhrase(LANG_ENGLISH);
    //   PrintPhrase(LANG_SPANISH);
    //   PrintPhrase(CONTINUE);
    ST7735_DrawBitmap(0, 160, IntroScreen, 128, 160);
    uint32_t lol = 10000;
    for (int i = 0; i < lol; i++) {
      // do nothing
    }
    do {
      // read slidepot
      Sensor.Sync();
      slide = Sensor.In();
      sw = Switch_In();
    } while (sw == 0);
    sw = 0;
    // reset sw back to zero
    if (slide < 2048) {
      lang = 0;
      // -------------------------------------------------
      // SCREEN 2 : DURATION SELECTION (ENGLISH)
      // -------------------------------------------------
      ST7735_FillScreen(ST7735_BLACK);
      myLanguage = English;

      ST7735_SetCursor(0, 0);
      PrintPhrase(SELECT_DURATION);
      PrintPhrase(TIME_30);
      PrintPhrase(TIME_60);
      do {
        // read slidepot
        Sensor.Sync();
        slide = Sensor.In();
        sw = Switch_In();
      } while (sw == 0);
      sw = 0;
      if (slide >= 2048) {
        // set time to 60 seconds
        time = 270;
      }
      // else {
      // otherwise no need to do anything
      // }
      // -------------------------------------------------
    // SCREEN 3 : MAP SELECTION (ENGLISH)
    // -------------------------------------------------
      ST7735_FillScreen(ST7735_BLACK);
      myLanguage = English;

      ST7735_SetCursor(0, 0);
      PrintPhrase(SELECT_MAP);
      PrintPhrase(MAP_1);
      PrintPhrase(MAP_2);
      do {
        // read slidepot
        Sensor.Sync();
        slide = Sensor.In();
        sw = Switch_In();
      } while (sw == 0);
      sw = 0;
      if (slide < 2048) {
        map = 1;
        drawPlatforms(map);
      }
      else {
        map = 2;
        drawPlatforms(map);
      }
    }
    else {
      lang = 1;
      // -------------------------------------------------
      // SCREEN 2 : DURATION SELECTION (SPANISH)
      // -------------------------------------------------
      ST7735_FillScreen(ST7735_BLACK);
      myLanguage = Spanish;

      ST7735_SetCursor(0, 0);
      PrintPhrase(SELECT_DURATION);
      PrintPhrase(TIME_30);
      PrintPhrase(TIME_60);
      do {
        // read slidepot
        Sensor.Sync();
        slide = Sensor.In();
        sw = Switch_In();
      } while (sw == 0);
      sw = 0;
      if (slide >= 2048) {
        time = 270;
      }
      // -------------------------------------------------
    // SCREEN 3 : MAP SELECTION (SPANISH)
    // -------------------------------------------------
      ST7735_FillScreen(ST7735_BLACK);
      myLanguage = Spanish;

      ST7735_SetCursor(0, 0);
      PrintPhrase(SELECT_MAP);
      PrintPhrase(MAP_1);
      PrintPhrase(MAP_2);
      do {
        // read slidepot
        Sensor.Sync();
        slide = Sensor.In();
        sw = Switch_In();
      } while (sw == 0);
      sw = 0;
      if (slide < 2048) {
        map = 1;
        drawPlatforms(map);
      }
      else {
        map = 2;
        drawPlatforms(map);
      }
    }

    // -------------------------------------------------
    // SCREEN 3 : MAP SELECTION (ENGLISH)
    // -------------------------------------------------
    // ST7735_FillScreen(ST7735_BLACK);
    // myLanguage = English;

    // ST7735_SetCursor(0, 0);
    // PrintPhrase(SELECT_MAP);
    // PrintPhrase(MAP_1);
    // PrintPhrase(MAP_2);
    // PrintPhrase(MAP_3);

    // -------------------------------------------------
    // SCREEN 3 : MAP SELECTION (SPANISH)
    // -------------------------------------------------
    // ST7735_FillScreen(ST7735_BLACK);
    // myLanguage = Spanish;

    // ST7735_SetCursor(0, 0);
    // PrintPhrase(SELECT_MAP);
    // PrintPhrase(MAP_1);
    // PrintPhrase(MAP_2);
    // PrintPhrase(MAP_3);

    player1.reset(10, 147);
  player2.reset(10, 147);

    ST7735_FillScreen(ST7735_WHITE);
    gameStart = true;

    while(!gameOver){
      // wait for semaphore
        // clear semaphore
        // update ST7735R

       if(jflag){
      jflag = 0;
      // if(!player1.onGround) player1.applyGravity();
      // if(!player2.onGround) player2.applyGravity();

      if(player1.moved() && player2.moved()){
        player1.cover();
        player2.cover();
        it_arrow.cover();
        drawPlatforms(map);
        player1.draw();
        player2.draw();
        it_arrow.drawArrow();
      }

      else if(player1.moved()){
        player1.cover();
        it_arrow.cover();
        drawPlatforms(map);
        player1.draw();
        it_arrow.drawArrow();
      }

      else if(player2.moved()){
        player2.cover();
        it_arrow.cover();
        drawPlatforms(map);
        player2.draw();
        it_arrow.drawArrow();
      }

       }

      // check for end game or level switch
      if (gameOver) {
        ST7735_FillScreen(ST7735_BLACK);
        if (lang == 0) {
          myLanguage = English;
          ST7735_SetCursor(0, 0);
          PrintPhrase(GAME_OVER);
          if (player1.get_role() == it) {
            PrintPhrase(PURPLE_WIN);
          }
          else {
            PrintPhrase(YELLOW_WIN);
          }
          uint32_t lol = 40000000;
          for (uint32_t i = 0; i < lol; i++) {
          // do nothing
            lol = lol;
          }
          PrintPhrase(PLAY_AGAIN);
        }
        else {
          myLanguage = Spanish;
          ST7735_SetCursor(0, 0);
          PrintPhrase(GAME_OVER);
          if (player1.get_role() == it) {
            PrintPhrase(PURPLE_WIN);
          }
          else {
            PrintPhrase(YELLOW_WIN);
          }
          uint32_t lol = 40000000;
          for (uint32_t i = 0; i < lol; i++) {
          // do nothing
            lol = lol;
          }
          PrintPhrase(PLAY_AGAIN);
        }
        do {
        sw = Switch_In();
      } while (sw == 0);
        sw = 0;
      }
    }
    gameOver = false;
    gameStart = false;
    time = 135;
    sw = 0;
  }
}