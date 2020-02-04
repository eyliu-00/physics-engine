#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "color.h"
#include "sdl_wrapper.h"
#include "body.h"
#include "scene.h"
#include "shapes.h"
#include "collision.h"

#define DT 0.01
#define SHOOT_TIME 1.5
#define SHOOT_DELAY 0.25

#define SHIP_HEIGHT 10
#define SHIP_LENGTH 30
#define SHIP_MASS 100
#define SHIP_VEL (1/DT)
#define SHIP_COLOR ((RGBColor) {.r = 15.0/255, .g = 188.0/255, .b = 0})

#define ENEMY_RADIUS 20
#define ENEMY_MASS 300
#define ENEMY_VEL (0.2/DT)
#define ENEMY_COLOR ((RGBColor) {150.0/255, 150.0/255, 150.0/255})
#define SPACING 10
#define NUM_ROWS 3
#define NUM_COLS 8
#define NUM_ENEMIES (NUM_ROWS * NUM_COLS)

#define BULLET_HEIGHT 5
#define BULLET_LENGTH 2
#define BULLET_MASS 5
#define BULLET_VEL (5/DT)

#define CANVAS_WIDTH ((ENEMY_RADIUS * 2) * NUM_COLS + (SPACING * (NUM_COLS + 1)))
#define CANVAS_HEIGHT CANVAS_WIDTH

// A global variable to delay shooting by SHOOT_DELAY if not in death laser mode.
double interval = 0;
bool deathlasermode = false;

/*  TODO:
    X Move the enemies
    X Make the bullets
    X Handle ship shooting (need an extra handler...? char *key? char in key?)
      - Solution: just don't. Use the up arrow.
    X Randomized enemy shooting
    X Collisions!!!
      - Check collisions only between ship bullet and invader; between invader
        bullet and ship; and between ship and invader
    X Win/loss conditions
*/

/*  General notes:
    Space invaders are 2 radii wide and 1 radius tall (functionally) because
    their mouths are big and go past the equator.
    The void *info is a boolean here and is used to determine if bodies can
    collide with each other ("friendly fire"). true = allies, false = enemies.
*/

// checks if the body is on the edge of the screen
bool on_edge(Body *body) {
  double centroidx = body_get_centroid(body).x;
  if (fabs(centroidx) > (CANVAS_WIDTH / 2 - ENEMY_RADIUS)) {
    return true;
  }
  return false;
}

// spawns a bullet. dir true = up, false = down
void shoot(Body *origin, bool dir, Scene *scene) {
  List *bullet_shape = make_rectangle(BULLET_HEIGHT, BULLET_LENGTH);
  Vector centroid = VEC_ZERO;
  if (origin != NULL){
  centroid = body_get_centroid(origin);
  }
  bool ally = true;
  // Body *bullet = NULL;
  if (dir) {
    Body *bullet = body_init_with_info(bullet_shape, BULLET_MASS, SHIP_COLOR, (void *)ally, NULL);
    body_set_velocity(bullet, (Vector) {0, BULLET_VEL});
    body_set_centroid(bullet, centroid);
    scene_add_body(scene, bullet);
  }
  else {
    ally = false;
    Body *bullet = body_init_with_info(bullet_shape, BULLET_MASS, ENEMY_COLOR, (void *)ally, NULL);
    body_set_velocity(bullet, (Vector) {0, -BULLET_VEL});
    body_set_centroid(bullet, centroid);
    scene_add_body(scene, bullet);
  }
}


// Moves the ship if left and right arrow keys are pressed. Stops the ship
// if they are released. I don't know if this actually works.
void move_ship(char key, KeyEventType type, double held_time, Scene *scene) {
  if (type == KEY_PRESSED) {
    switch (key) {
      case UP_ARROW:
        if (deathlasermode == false) {
          if (interval > SHOOT_DELAY) {
            shoot(scene_get_body(scene, 0), true, scene);
            interval = 0;
          }
        }
        else {
          shoot(scene_get_body(scene, 0), true, scene);
        }
        break;
      case RIGHT_ARROW:
        body_set_velocity(scene_get_body(scene, 0), (Vector){SHIP_VEL, 0});
        break;
      case LEFT_ARROW:
        body_set_velocity(scene_get_body(scene, 0), (Vector){-SHIP_VEL, 0});
        break;
      case 'd':
        deathlasermode = !deathlasermode;
        if (deathlasermode) {
          body_set_color(scene_get_body(scene, 0), (RGBColor) {1.0, 0, 0});
        }
        else {
          body_set_color(scene_get_body(scene, 0), SHIP_COLOR);
        }
        break;
    }
  }
  if (type == KEY_RELEASED) {
    switch (key) {
      case RIGHT_ARROW:
      case LEFT_ARROW:
        body_set_velocity(scene_get_body(scene, 0), VEC_ZERO);
        break;
    }

  }
}

// Makes the space invaders and adds them to the scene.
void make_enemies(int num_rows, int num_cols, Scene *scene) {
  double y_pos = (CANVAS_HEIGHT / 2) - ENEMY_RADIUS - SPACING;
  double x_pos = (-CANVAS_WIDTH / 2) + ENEMY_RADIUS + SPACING;
  double yshift = ENEMY_RADIUS + SPACING;
  bool ally = false;
  for (int i = 0; i < num_rows; i++) {
    for (int j = 0; j < num_cols; j++) {
      List *enemy_shape = make_pacman(ENEMY_RADIUS); // a space invader is just a really big mouth pacman right
      polygon_rotate(enemy_shape, 3 * M_PI/2, VEC_ZERO); // spins it around so mouth points down
      Body *enemy = body_init_with_info(enemy_shape, ENEMY_MASS, ENEMY_COLOR, (void *)ally, NULL);
      // makes num_cols space invaders w centroids separated by shift, i.e. by
      // 2 invader radii and a spacing.
      body_set_centroid(enemy, (Vector) {x_pos + j * (2 * ENEMY_RADIUS), y_pos});
      body_set_velocity(enemy, (Vector) {ENEMY_VEL, 0});
      scene_add_body(scene, enemy);
    }
    // after finished with one row, move the next row down by shift
    y_pos -= yshift;
  }
}


// moves enemies down + reverses their direction if enemies touch the sides
void move_enemy(Scene *scene, int num_enemies) {
  double shift = (ENEMY_RADIUS + SPACING) * 3;
  for (int i = 1; i <= num_enemies; i++) {
    Vector centroid = body_get_centroid(scene_get_body(scene, i));
    if (on_edge(scene_get_body(scene, i))) {
      // moves centroid down by shift
      //Vector newcentroid = vec_add(centroid, (Vector){0, -shift});
      Vector newcentroid = vec_add(vec_multiply(0.97, centroid), (Vector){0, -shift});
      body_set_centroid(scene_get_body(scene, i), newcentroid);
      // makes it go in the other direction
      Vector newvel = vec_negate(body_get_velocity(scene_get_body(scene, i)));
      body_set_velocity(scene_get_body(scene, i), newvel);
    }
  }
}

int main(int argc, char *argv[]) {
  srand(time(0));

  List *ship_shape = make_rectangle(SHIP_HEIGHT, SHIP_LENGTH);
  bool ally = true;
  Body *ship = body_init_with_info(ship_shape, SHIP_MASS, SHIP_COLOR, (void *)ally, NULL);
  body_set_centroid(ship, (Vector) {0, -CANVAS_HEIGHT/2 + ENEMY_RADIUS});
  Scene *scene = scene_init();
  scene_add_body(scene, ship);

  make_enemies(NUM_ROWS, NUM_COLS, scene);
  int num_enemies_left = NUM_ENEMIES;

  Vector bottom_left = (Vector) {-CANVAS_WIDTH/2, -CANVAS_HEIGHT/2};
  Vector top_right = (Vector) {CANVAS_WIDTH/2, CANVAS_HEIGHT/2};
  sdl_init(bottom_left, top_right);
  sdl_render_scene(scene);
  sdl_on_key((KeyHandler) move_ship);
  double elapsed = 0;
  bool isover = false;

  while (!sdl_is_done()) {
    double time = time_since_last_tick();
    elapsed += time;
    interval += time;

    // Win conditions.
    // if the first body in the scene is an enemy, i.e. if the ship is destroyed
    if (!((bool)body_get_info(scene_get_body(scene, 0))) && !isover) {
      isover = true;
      // lose condition
      sdl_on_key((KeyHandler) NULL);
      for (int i = 0; i < scene_bodies(scene); i++) {
        body_set_velocity(scene_get_body(scene, i), VEC_ZERO);
      }
      List *octagon = make_ngon(8, 3 * ENEMY_RADIUS);
      Body *ohno = body_init(octagon, ENEMY_MASS, (RGBColor) {1, 0, 0});
      body_set_rotation(ohno, M_PI / 8);
      scene_add_body(scene, ohno);
      scene_tick(scene, elapsed);
      sdl_render_scene(scene);
    }
    // else if no enemies left
    else if (num_enemies_left == 0 && !isover) {
      isover = true;
      // win condition
      sdl_on_key((KeyHandler) NULL);
      for (int i = 0; i < scene_bodies(scene); i++) {
        scene_remove_body(scene, i);
      }
      body_set_velocity(scene_get_body(scene, 0), VEC_ZERO);
      List *star = make_n_star(5, 3 * ENEMY_RADIUS);
      Body *goodjob = body_init(star, SHIP_MASS, SHIP_COLOR);
      scene_add_body(scene, goodjob);
      scene_tick(scene, DT);
      sdl_render_scene(scene);
    }

    // printf("%f\n", interval);
    if (elapsed > DT && !isover) {
      if (on_edge(ship)){
        body_set_velocity(ship, VEC_ZERO);
        body_set_centroid(ship, (Vector) {body_get_centroid(ship).x * .99, -CANVAS_HEIGHT/2 + ENEMY_RADIUS});
      }
      move_enemy(scene, num_enemies_left);
      scene_tick(scene, elapsed);
//      size_t num_bodies = scene_bodies(scene);

      // collisions between invaders, invader bullets and ship, ship bullets
      for (int i = 0; i < scene_bodies(scene); i++) { // the bodies we are testing
        for (int j = num_enemies_left + 1; j < scene_bodies(scene); j++) { // the bodies they are colliding with
          //if (i != j) { // if they are different bodies
            if (find_collision(body_get_shape(scene_get_body(scene, i)), body_get_shape(scene_get_body(scene, j))).collided) {
              // the info of the tested body
              bool thisbody = (bool)body_get_info(scene_get_body(scene, i));
              // the info of the body we are colliding with
              bool otherbody = (bool)body_get_info(scene_get_body(scene, j));
              if (thisbody != otherbody) { // if they are on opposite teams
                // remove them both
                scene_remove_body(scene, j);
                scene_remove_body(scene, i);
                if ((i > 0) && (i <= num_enemies_left)) { // if the body was an invader
                  num_enemies_left--;
                }
                else if ((j > 0) && (j <= num_enemies_left)) {
                  num_enemies_left--;
                }
              }
            }
          //}
        }
      }

      // if interval is greater than shooting time interval, chooses a random
      // enemy (+1 because the first body is the ship) and makes it shoot.
      if (interval > SHOOT_TIME) {
        int rand_enemy = (rand() % num_enemies_left) + 1;
        shoot(scene_get_body(scene, rand_enemy), false, scene);
        interval = 0;
      }

      // remove bullets that are offscreen
      if (num_enemies_left + 1 <= scene_bodies(scene)) { // if there are bullets
        for (int i = (num_enemies_left + 1); i < scene_bodies(scene); i++) {
          double centroidy = body_get_centroid(scene_get_body(scene, i)).y;
          // if the bullet is too far offscreen
          if (fabs(centroidy) > (CANVAS_HEIGHT / 2 + 2 * BULLET_HEIGHT)) {
            scene_remove_body(scene, i);
          }
        }
      }
      sdl_render_scene(scene);
      elapsed = 0;
    }



  }
  scene_free(scene);
}
