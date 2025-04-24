#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>

typedef enum {runner, it} status_t;

class Player {
private:
  int32_t x, y;
  int32_t prevx, prevy;
  int32_t vx, vy;
  const unsigned short *image;
  status_t role;

public:

  bool onGround;

  Player(int32_t x, int32_t y, const unsigned short *img, status_t role);
  
  void set_vx(int32_t vx);
  void set_vy(int32_t vy);
  void move();
  void draw();
  bool checkCollision(Player &other);
  void set_role(status_t r);
  status_t get_role();
  int32_t getX();
  int32_t getY();
  bool moved();
  void cover();

  void setPos(int32_t x, int32_t y);

  // TESTING STILL
  void jump(); // start a jump
  void applyGravity(); // simulate gravity

  void reset(int32_t x, int32_t y);

  void collide(int32_t x, int32_t y);

  void checkLevel();

  void checkPlatformCollisions();

  void drawArrow();


};

#endif