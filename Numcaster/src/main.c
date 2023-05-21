#include <eadk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Benco";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

eadk_rect_t test = {50,50,5,30};

#define M_PI 3.1415
#define M_PI_2 M_PI / 2

#define MAP_HEIGHT 4
#define MAP_WIDTH 6
#define STEP_SIZE 0.045
#define FOV M_PI / 2.7
#define HALF_FOV FOV * 0.5
#define ANGLE_STEP FOV / 160.0
#define WALL_HEIGHT 100.0

#define MAP_SIZE MAP_HEIGHT*MAP_WIDTH

const int map[MAP_SIZE] = {
  1,1,1,1,1,1,
  1,0,0,1,0,1,
  1,0,0,0,0,1,
  1,1,1,1,1,1
};

/*

eadk_color_t random_color() {
  return (eadk_color_t)eadk_random();
}

eadk_rect_t random_screen_rect() {
  uint16_t x = eadk_random() % (EADK_SCREEN_WIDTH - 1);
  uint16_t y = eadk_random() % (EADK_SCREEN_HEIGHT - 1);
  uint16_t width = eadk_random() % (EADK_SCREEN_WIDTH - x);
  uint16_t height = eadk_random() % (EADK_SCREEN_HEIGHT - y);
  return (eadk_rect_t){x, y, width, height};
}

void draw_random_colorful_rectangles() {
  eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_black);
  for (int i=0; i<100; i++) {
    eadk_display_push_rect_uniform(random_screen_rect(), random_color());
  }
}

void draw_random_buffer() {
  eadk_rect_t rect = {0, 0, 30, 30};
  size_t bufferSize = rect.width*rect.height*sizeof(eadk_color_t);
  eadk_color_t * pixels = (eadk_color_t *)malloc(bufferSize);
  if (pixels == NULL) {
    return;
  }
  memset(pixels, 0, bufferSize);
  for (int i=0; i<rect.width*rect.height; i++) {
    pixels[i] = random_color();
  }
  eadk_display_push_rect(rect, pixels);
  free(pixels);
}

*/

void draw_wall(eadk_rect_t wall) {
  eadk_display_push_rect_uniform(wall, eadk_color_green);
}

void distance(float a, float b) {
  return sqrtf((a * a) + (b * b));
}

void point_in_wall(float x, float y) {
    // Vérifier si les coordonnées x et y sont valides pour la carte
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return false;
    }

    // Convertir les coordonnées flottantes en entiers
    int ix = (int)x;
    int iy = (int)y;

    // Calculer l'index correspondant aux coordonnées x et y dans la liste
    int index = iy * MAP_WIDTH + ix;

    // Vérifier si le point est dans un mur (valeur 1) ou non (valeur 0)
    if (map[index] == 1) {
        return true;
    } else {
        return false;
    }
}

void horizontal_intersection(float x, float y, float angle) {
  bool up = fabsf(floorf(fmod(angle / M_PI, 2.00))) != 0.0;

  float first_y = up ? ceilf(y) - y : floorf(y) - y ; 
  float first_x = (-1.0 * first_y) / tanf(angle);

  float dy = up ? 1.0 : -1.0 ;
  float dx = (-1 * dy) / tanf(angle);

  float next_y = first_y;
  float next_x = first_x;

  for (int i = 0; i < 256; i++)
  {
    float current_x = next_x + x;
    float current_y = up ? next_y + y : next_y + y - 1.0;

    if (point_in_wall(current_x, current_y)) {
      return;
    }

    next_x += dx;
    next_y += dy;
  }
  
  return distance(next_x, next_y);
}

void vertical_intersection(float x, float y, float angle) {
  bool right = fabsf(floorf(fmod((angle - M_PI_2) / M_PI, 2.00))) != 0.0;

  float first_x = right ? ceilf(x) - x : floorf(x) - x;
  float first_y = -1.0 * tanf(angle) * first_x;

  float dx = right ? 1.0 : -1.0 ;
  float dy = dx * (-1.0 * (tanf(angle)));

  float next_x = first_x;
  float next_y = first_y;

  for (int i = 0; i < 256; i++)
  {
    float current_x = right ? next_x + x : next_x + x - 1.0 ;
    float current_y = next_y + y;

    if (point_in_wall(current_x, current_y)) {
      return;
    }

    next_x += dx;
    next_y += dy;
  }
  
  distance(next_x, next_y);
}

void get_view(float x, float y, float angle) {
  float starting_angle = angle + HALF_FOV;

  float walls[EADK_SCREEN_WIDTH];

  for (int i = 0; i < EADK_SCREEN_WIDTH; i++)
  {
    float angle2 = starting_angle - i * ANGLE_STEP;

    float h_dist = horizontal_intersection(x, y, angle2);
    float v_dist = vertical_intersection(x, y, angle2);

    walls[i] = h_dist > v_dist ? WALL_HEIGHT / v_dist : WALL_HEIGHT / h_dist ;
  }
  
  return walls;
}

void input() {
  float player_x = 1.0;
  float player_y = 1.0;
  float player_angle = 0.0;

  while (true) {
    eadk_keyboard_state_t keyboard = eadk_keyboard_scan();

    //QUIT
    if (eadk_keyboard_key_down(keyboard, eadk_key_back)) {
      return;
    }

    float previous_x = player_x;
    float previous_y = player_y;

    //LEFT
    if (eadk_keyboard_key_down(keyboard, eadk_key_left)) {
      player_angle += STEP_SIZE;
    }
    //RIGHT
    if (eadk_keyboard_key_down(keyboard, eadk_key_right)) {
      player_angle -= STEP_SIZE;
    }
    //UP
    if (eadk_keyboard_key_down(keyboard, eadk_key_up)) {
      player_x += cos(player_angle) * STEP_SIZE;
      player_y += -1.0 * sin(player_angle) * STEP_SIZE;
    }
    //DOWN
    if (eadk_keyboard_key_down(keyboard, eadk_key_down)) {
      player_x -= cos(player_angle) * STEP_SIZE;
      player_y -= -1.0 * sin(player_angle) * STEP_SIZE;
    }

    if (point_in_wall(player_x, player_y)) {
      player_x = previous_x;
      player_y = previous_y;
    }

    float walls[EADK_SCREEN_WIDTH] = get_view(player_x, player_y, player_angle);

    for (int i = 0; i < EADK_SCREEN_WIDTH; i++)
    {
      eadk_rect_t wall = {i, 80 - EADK_SCREEN_HEIGHT, 1, walls[i]};
      eadk_display_push_rect_uniform(wall, eadk_color_green);
    }

    draw_wall(test);
    eadk_timing_msleep(20);
  }
}

int main(int argc, char * argv[]) {
  eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_white);
  //eadk_display_draw_string("Asterixcel VS Chadbelix", (eadk_point_t){0, 0}, true, eadk_color_black, eadk_color_white);
  //move_pointer();
  input();
}
