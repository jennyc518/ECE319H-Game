#include "Player.h"
#include "Switch.h"
#include <cstdint>
#include "images/images.h"
#include "../inc/ST7735.h"

#include "Platform.h"


#define SPRITE_W 13
#define SPRITE_H 13
#define GROUND_Y 147 // Ground level - Needs change based on where in map
#define GRAVITY 1

extern int map;

Player::Player(int32_t x, int32_t y,const unsigned short *img, status_t status){
    this->x = x;
    this->y = y;
    this->image = img;
    this->onGround = true;
}

void Player::set_vx(int32_t vx){
    this->vx = vx;
}

void Player::set_vy(int32_t vy){
    this->vy = vy;
}

void Player::jump() {
  
    if(onGround) vy = -8;         // initial jump velocity
    onGround = false;

}

void Player::applyGravity() {
  if(!onGround) {
    vy += GRAVITY;
  }
}

void Player::move(){
    x += vx;
    y += vy;

    // if(y > GROUND_Y){
    //     y = GROUND_Y;
    //     vy = 0;
    //     onGround = true;
    // }

    if(x < 0) x = 0;
    if(x >= 115) x = 115;

    if(y > 160) y = 160;

    checkPlatformCollisions();
}

bool Player::moved(){
    return !(x == prevx && y == prevy);
}

void Player::draw() {

        ST7735_DrawBitmap(x, y, image, SPRITE_W, SPRITE_H);
        // ST7735_FillRect(x, y - SPRITE_W, SPRITE_W, SPRITE_H, ST7735_ORANGE);

    prevx = x;
    prevy = y;

}

bool Player::checkCollision(Player &other) {
  return !(x + SPRITE_W <= other.x ||       // this is left of other
           x >= other.x + SPRITE_W ||       // this is right of other
           y + SPRITE_H <= other.y ||       // this is below other
           y >= other.y + SPRITE_H);        // this is above other
}

void Player::set_role(status_t r) { 
    role = r; 
}

status_t Player::get_role() { 
    return role; 
}

int32_t Player::getX() { 
    return x; 
}

int32_t Player::getY() { 
    return y; 
}

void Player::reset(int32_t x, int32_t y){
    this->x = x;
    this->y = y;
    // ST7735_FillScreen(ST7735_LIGHTGREY);
    ST7735_FillScreen(ST7735_WHITE);
}


void Player::collide(int32_t x, int32_t y){
    ST7735_DrawBitmap(x, y, image, 20, 20);
}


void Player::cover(){

    ST7735_FillRect(prevx, prevy - SPRITE_H - 1, SPRITE_W, SPRITE_H + 2, ST7735_WHITE);
    // ST7735_FillRect(prevx, prevy - SPRITE_H - 1, SPRITE_W, SPRITE_H + 2, ST7735_LIGHTGREY);

    // if(onGround) ST7735_FillRect(x - 2, y - SPRITE_H - 2, SPRITE_W+4, SPRITE_H+4, ST7735_WHITE);
    // else {
    //     // if(vy <= 0) ST7735_FillRect(prevx, prevy, SPRITE_W, SPRITE_H, ST7735_WHITE); 
    //     // else ST7735_FillRect(prevx, prevy - SPRITE_H, SPRITE_W, SPRITE_H, ST7735_WHITE); 
    //     // if(vx <= 0) ST7735_FillRect(prevx + SPRITE_W, prevy, SPRITE_W, SPRITE_H, ST7735_WHITE); 
    //     // else if (vx >= 0) ST7735_FillRect(prevx - SPRITE_W, prevy, SPRITE_W, SPRITE_H, ST7735_WHITE); 

    //     ST7735_FillRect(prevx, prevy - SPRITE_H, SPRITE_W, SPRITE_H, ST7735_WHITE);

    //     // if(vy < 0) ST7735_FillRect(prevx, prevy, SPRITE_W, SPRITE_H, ST7735_WHITE); 
    //     // else ST7735_FillRect(prevx, prevy - SPRITE_H, SPRITE_W, SPRITE_H, ST7735_WHITE); 

    // }

    // if(onGround) ST7735_FillRect(x - 2, y - SPRITE_H - 2, SPRITE_W+4, SPRITE_H+4, ST7735_WHITE);
    // else ST7735_FillRect(x - 2, y - 1.5*SPRITE_H - 2, 2*SPRITE_W+4, 2*SPRITE_H+4, ST7735_WHITE); 
}

void Player::checkPlatformCollisions() {
  onGround = false;

  int playerBottomY = y;
  int prevBottomY = prevy;
  int playerTopY = y - SPRITE_H;
  int prevTopY = prevy - SPRITE_H;
  int playerLeftX = x;
  int playerRightX = x + SPRITE_W;

  Platform* bestLanding = nullptr;
  int closestY = 1000; // very far initially
  Platform p;

  for(int i = 0; i < 7; i++) {
    if (map == 1) {
      p = platforms1[i];
    }
    else {
      p = platforms2[i];
    }

    bool withinX = playerRightX > p.x && playerLeftX < p.x + p.w;

    // --- Land on platform ---
    bool falling = vy >= 0;
    bool wasAbove = prevBottomY <= p.y;
    bool nowOnTop = playerBottomY >= p.y && playerBottomY <= p.y + p.h;

    if(falling && wasAbove && nowOnTop && withinX) {
      if(p.y < closestY) {
        if (map == 1) {
          bestLanding = &platforms1[i];
        }
        else {
          bestLanding = &platforms2[i];
        }
        closestY = p.y;
      }
    }

    // --- Bump bottom of platform ---
    bool jumpingUp = vy < 0;
    bool wasBelow = prevTopY >= p.y + p.h;
    bool nowHitsBottom = playerTopY <= p.y + p.h && playerTopY >= p.y;

    if(jumpingUp && wasBelow && nowHitsBottom && withinX) {
      vy = 0;
      y = p.y + p.h + SPRITE_H;
    }
  }

  if(bestLanding) {
    y = bestLanding->y;
    vy = 0;
    onGround = true;
  }
}

// void Player::checkPlatformCollisions() {
//   onGround = false;

//   int playerBottomY = y;
//   int prevBottomY = prevy; // <- previous bottom Y
//   int playerTopY = y - SPRITE_H;
//   int prevTopY = prevy - SPRITE_H;
//   int playerLeftX = x;
//   int playerRightX = x + SPRITE_W;

//   for(int i = 0; i < numPlatforms; i++) {
//     Platform p = platforms[i];

//     // --- Land on platform ---
//     bool falling = vy >= 0;
//     bool wasAbove = prevBottomY <= p.y;
//     bool nowOnTop = playerBottomY >= p.y && playerBottomY <= p.y + p.h;
//     bool withinX = playerRightX > p.x && playerLeftX < p.x + p.w;

//     if(falling && wasAbove && nowOnTop && withinX) {
//       y = p.y;        // snap to platform top
//       vy = 0;
//       onGround = true;
//       return;
//     }

//     // --- Bump bottom of platform ---
//     bool jumpingUp = vy < 0;
//     bool wasBelow = prevTopY >= p.y + p.h;
//     bool nowHitsBottom = playerTopY <= p.y + p.h && playerTopY >= p.y;
//     if(jumpingUp && wasBelow && nowHitsBottom && withinX) {
//       vy = 0;
//       y = p.y + p.h + SPRITE_H;
//     }
//   }

//   // if(y >= GROUND_Y){
//   //   y = GROUND_Y;
//   //   vy = 0;
//   //   onGround = true;
//   // }

// }



void Player::setPos(int32_t x, int32_t y){
  this->x = x;
  this->y = y;
}


void Player::drawArrow(){
  ST7735_DrawBitmap(x, y, image, 13, 4);
  prevx = x;
    prevy = y;
}