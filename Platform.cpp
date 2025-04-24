#include "Platform.h"
#include "../inc/ST7735.h"


//MAP 2
Platform platforms2[] = {
  {10, 40, 45, 5},    // Upper-left
  {80, 50, 30, 5},    // Upper-right
  {50, 75, 25, 5},    // Middle
  {85, 110, 15, 5},   // Lower-right
  {5, 100, 40, 5},    // Lower-left
  {35, 125, 25, 5},   // Near bottom-center
  {0, 148, 128, 12}   // Ground
};

// MAP 1
Platform platforms1[] = {
  {50, 35, 45, 5},
  {30, 60, 30, 5},
  {115, 60, 20, 5},
  {75, 85, 20, 5},
  {20, 105, 35, 5},
  {50, 125, 20, 5},
  {0, 148, 128, 12}  // <- Ground platform across full screen width
};

//const int numPlatforms = sizeof(platforms)/sizeof(platforms[0]);

void drawPlatforms(int map){
  if (map == 1) {
    for(int i = 0; i < 7; i++){
      ST7735_FillRect(platforms1[i].x, platforms1[i].y, platforms1[i].w, platforms1[i].h, ST7735_BLACK);
    }
  }
  else {
    for(int i = 0; i < 7; i++){
      ST7735_FillRect(platforms2[i].x, platforms2[i].y, platforms2[i].w, platforms2[i].h, ST7735_BLACK);
    }
  }
}