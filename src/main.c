#include <eadk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Benco";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

#define M_PI 3.1415
#define M_PI_2 1,57075

#define MAP_HEIGHT 10
#define MAP_WIDTH 10
#define MOVEMENT 1.0
#define ROTATION 10.0
#define FOV 60
#define HALF_FOV 30
#define ANGLE_STEP 0.375
#define WALL_HEIGHT 100.0
#define PRECISION 16

#define DIVISER 4

double incrementAngle = 0.1875;

const int map[MAP_HEIGHT][MAP_WIDTH] = {
  {1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,1,0,1,0,0,1},
  {1,0,0,1,0,0,1,0,0,1},
  {1,0,0,1,0,0,1,0,0,1},
  {1,0,0,1,0,1,1,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1},
};

float player_x = 2.0;
float player_y = 2.0;
float player_angle = 90.0;

void input() {

    eadk_keyboard_state_t keyboard = eadk_keyboard_scan();

    //QUIT
    if (eadk_keyboard_key_down(keyboard, eadk_key_back)) {
      return;
    }

    //float previous_x = player_x;
    //float previous_y = player_y;

    //RIGHT
    if (eadk_keyboard_key_down(keyboard, eadk_key_right)) {
      player_angle += ROTATION;
    }
    //LEFT
    if (eadk_keyboard_key_down(keyboard, eadk_key_left)) {
      player_angle -= ROTATION;
    }
    //UP
    if (eadk_keyboard_key_down(keyboard, eadk_key_up)) {
      float new_x = player_x + (float)cos(player_angle * M_PI / 180) * MOVEMENT;
      float new_y = player_y + (float)sin(player_angle * M_PI / 180) * MOVEMENT;

      if (map[(int)floor(new_y)][(int)floor(new_x)] == 0)
      {
        player_x = new_x;
        player_y = new_y;
      }
      
    }
    //DOWN
    if (eadk_keyboard_key_down(keyboard, eadk_key_down)) {
      float new_x = player_x - (float)cos(player_angle * M_PI / 180) * MOVEMENT;
      float new_y = player_y - (float)sin(player_angle * M_PI / 180) * MOVEMENT;

      if (map[(int)floor(new_y)][(int)floor(new_x)] == 0)
      {
        player_x = new_x;
        player_y = new_y;
      }
    }

    eadk_timing_msleep(20);
}

void raycasting() {

  float rayAngle = player_angle - HALF_FOV;

  for (int raycount = 0; raycount < 320; raycount = raycount + DIVISER) {
    
    eadk_keyboard_state_t keyboard = eadk_keyboard_scan();

    //QUIT
    if (eadk_keyboard_key_down(keyboard, eadk_key_back)) {
      return;
    }

    float ray_x = player_x;
    float ray_y = player_y;

    float rayCos = (float)cosf((rayAngle * M_PI / 180.0)) / PRECISION;
    float raySin = (float)sinf((rayAngle * M_PI / 180.0)) / PRECISION;

    int wall = 0;
    while (wall == 0)
    {
      ray_x += rayCos;
      ray_y += raySin;

      wall = map[(int)floor(ray_y)][(int)floor(ray_x)];
    }
  
    double distance = sqrt((player_x - ray_x) * (player_x - ray_x) + (player_y - ray_y) * (player_y - ray_y));

    distance = distance * cosf((M_PI / 180) * (rayAngle - player_angle));

    int wallHeight = (int)floor(120 / distance);

    if ( (int)(120 - wallHeight) >= 1 )
    {
      eadk_rect_t rect = {raycount, 0, DIVISER, 120 - wallHeight};
      eadk_display_push_rect_uniform( rect, eadk_color_blue);
      eadk_rect_t rect2 = {raycount, 120 - wallHeight, DIVISER, wallHeight * 2};
      eadk_display_push_rect_uniform( rect2, eadk_color_red);
      eadk_rect_t rect3 = {raycount, 120 + wallHeight, DIVISER, 240 - (120 + wallHeight)};
      eadk_display_push_rect_uniform( rect3, eadk_color_green);
    }
    else
    {
      eadk_rect_t rect2 = {raycount, 0, DIVISER, 240};
      eadk_display_push_rect_uniform( rect2, eadk_color_red);
    }
    

    //eadk_display_push_rect_uniform((eadk_rect_t) {0,0,(player_x - ray_x),10}, eadk_color_blue);
    //eadk_timing_msleep(500);

    rayAngle = rayAngle + (incrementAngle * DIVISER);
  }
}

int main(int argc, char * argv[]) {

  while (true)
  {
    raycasting();
    input();

    //eadk_timing_msleep(500);

    //eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_black);
    eadk_keyboard_state_t keyboard = eadk_keyboard_scan();

    //QUIT
    if (eadk_keyboard_key_down(keyboard, eadk_key_back)) {
      break;
    }

  }
}
