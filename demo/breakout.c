#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#include "color.h"
#include "sdl_wrapper.h"
#include "body.h"
#include "scene.h"
#include "shapes.h"
#include "collision.h"
#include "forces.h"

#define DT 0.01
#define LARGE_NUMBER 1000000.0
#define LARGE_MASS LARGE_NUMBER
#define SMALL_MASS 1

#define SHIP_VEL vec_len(body_get_velocity(scene_get_body(scene, 2)))
#define SHIP_COLOR (COLOR_WHITE)
#define MAX_LIVES 5

#define BLOCK_HEIGHT 10
#define BLOCK_WIDTH 30
#define SPACING 0
#define NUM_ROWS 5
#define NUM_COLS 10

#define INIT_BALL_SPEED (1/DT)
#define BALL_ACCELERATION (0.0001/DT)
#define MAX_BALL_SPEED (3/DT)
#define BALL_RADIUS 1

#define CANVAS_WIDTH (BLOCK_WIDTH * NUM_COLS + (SPACING * (NUM_COLS + 1)))
#define CANVAS_HEIGHT CANVAS_WIDTH * 0.75

#define BACKGROUND_INDEX 0
#define SHIP_INDEX 1
#define BALL_INDEX 2
#define LEFT_WALL_INDEX 3
#define RIGHT_WALL_INDEX 4
#define CEILING_INDEX 5
#define FLOOR_INDEX 6

typedef struct body_info {
    int health;
} BodyInfo;

BodyInfo *body_info_init(int health) {
    BodyInfo *body_info = malloc(sizeof(BodyInfo));
    assert(body_info);
    body_info->health = health;
    return body_info;
}

bool on_edge(Body *body) {
    double centroidx = body_get_centroid(body).x;
    return fabs(centroidx) > (CANVAS_WIDTH / 2 - BLOCK_WIDTH / 2);
}

RGBColor health_to_color(int health, int elapsed_ticks) {
    double d = (double) health / MAX_LIVES * M_PI + elapsed_ticks/100000.0;
    RGBColor color = {
        .r = fabs(cos(d)),
        .g = fabs(cos(d + M_PI * 2 / 3)),
        .b = fabs(cos(d + M_PI * 4 / 3))
    };
    return color;

    /* Alternatively, use these God-aweful bright colors. */
    switch(health) {
        case 1: return COLOR_RED;
        case 2: return COLOR_YELLOW;
        case 3: return COLOR_GREEN;
        case 4: return COLOR_BLUE;
        case 5: return COLOR_MAGENTA;
        default: return COLOR_WHITE;
    }
}

Body *create_ship(Scene *scene) {
    BodyInfo *info = body_info_init(LARGE_NUMBER);
    List *ship_shape = make_rounded_paddle(20, 50, 1);
    Body *ship = body_init_with_info(ship_shape, pow(LARGE_MASS, 1), SHIP_COLOR, info, free);
    body_set_centroid(ship, (Vector) {0, -CANVAS_HEIGHT/2 + SPACING});
    scene_add_body(scene, ship);
    return ship;
}

Body *create_ball(Body *origin, Scene *scene) {
    List *ball_shape = make_ngon(20, BALL_RADIUS * 2);
    BodyInfo *info = body_info_init(LARGE_NUMBER);
    Body *ball = body_init_with_info(ball_shape, SMALL_MASS, COLOR_WHITE, info, free);
    body_set_velocity(ball, (Vector) {1, 2});
    body_set_centroid(ball, vec_add(body_get_centroid(origin), (Vector){0, 10}));
    scene_add_body(scene, ball);
    return ball;
}

Body *create_background(Scene *scene, RGBColor color) {
    List *background_shape = make_rectangle(CANVAS_HEIGHT, CANVAS_WIDTH);
    Body *background = body_init_with_info(background_shape, LARGE_NUMBER, color, NULL, NULL);
    body_set_velocity(background, VEC_ZERO);
    body_set_centroid(background, VEC_ZERO);
    scene_add_body(scene, background);
    return background;
}

void create_screen_edges(Scene *scene, RGBColor color) {
    double wall_width = 20.0;
    double floor_height = 20.0;
    BodyInfo *info = body_info_init(2);
    List *wall_shape = make_rectangle(CANVAS_HEIGHT, wall_width);
    List *wall_shape_2 = make_rectangle(CANVAS_HEIGHT, wall_width);
    Body *left_wall  = body_init_with_info(wall_shape, LARGE_MASS, color, (void *)false, NULL);
    Body *right_wall = body_init_with_info(wall_shape_2, LARGE_MASS, color, (void *)false, NULL);
    List *floor_shape = make_rectangle(floor_height, CANVAS_WIDTH);
    List *ceiling_shape = make_rectangle(floor_height, CANVAS_WIDTH);
    Body *floor   = body_init_with_info(floor_shape, LARGE_MASS, color, info, NULL);
    Body *ceiling = body_init_with_info(ceiling_shape, LARGE_MASS, color, NULL, NULL);

    body_set_velocity(left_wall, VEC_ZERO);
    body_set_velocity(right_wall, VEC_ZERO);
    body_set_centroid(left_wall, vec_add(VEC_ZERO, (Vector){-CANVAS_WIDTH/2 - wall_width/2, 0}));
    body_set_centroid(right_wall, vec_add(VEC_ZERO, (Vector){CANVAS_WIDTH/2 + wall_width/2, 0}));
    body_set_velocity(floor, VEC_ZERO);
    body_set_velocity(ceiling, VEC_ZERO);
    body_set_centroid(floor, (Vector){0, -CANVAS_HEIGHT/2 - floor_height/2});
    body_set_centroid(ceiling, (Vector){0, CANVAS_HEIGHT/2 + floor_height/2});

    scene_add_body(scene, left_wall);
    scene_add_body(scene, right_wall);
    scene_add_body(scene, ceiling);
    scene_add_body(scene, floor);
}

void move_ship(char key, KeyEventType type, double held_time, Scene *scene) {
    if (type == KEY_PRESSED) {
        switch (key) {
            case RIGHT_ARROW:
                body_set_velocity(scene_get_body(scene, SHIP_INDEX), (Vector){SHIP_VEL, 0});
                break;
            case LEFT_ARROW:
                body_set_velocity(scene_get_body(scene, SHIP_INDEX), (Vector){-SHIP_VEL, 0});
                break;
        }
    }
    if (type == KEY_RELEASED) {
        /* Set velocity of ship to 0 only if left or right arrow keys
           are released. */
        switch (key) {
            case RIGHT_ARROW:
            case LEFT_ARROW:
                body_set_velocity(scene_get_body(scene, SHIP_INDEX), VEC_ZERO);
                break;
        }
    }
}

void make_blocks(int num_rows, int num_cols, Scene *scene) {
    double y_pos = (CANVAS_HEIGHT / 2) - BLOCK_HEIGHT / 2 - SPACING;
    double x_pos = (-CANVAS_WIDTH / 2) + BLOCK_WIDTH / 2 + SPACING;
    double yshift = BLOCK_HEIGHT + SPACING;
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < num_cols; j++) {
            BodyInfo *info = body_info_init(1 + (i + j) % MAX_LIVES);
            List *block_shape = make_rectangle(BLOCK_HEIGHT, BLOCK_WIDTH);
            Body *block = body_init_with_info(block_shape, LARGE_MASS, COLOR_BLACK, info, free);
            body_set_centroid(block, (Vector) {x_pos + j * (BLOCK_WIDTH + SPACING), y_pos});
            body_set_velocity(block, VEC_ZERO);
            scene_add_body(scene, block);
        }
        y_pos -= yshift;
    }
}

void life_depleting_collision_handler(Body *body1, Body *body2, Vector axis, void *aux) {
    double reduced_mass = body_get_mass(body1) * body_get_mass(body2) / (body_get_mass(body1) + body_get_mass(body2));
    if (body_get_mass(body1) == INFINITY){
        reduced_mass = body_get_mass(body2);
    } else if(body_get_mass(body2) == INFINITY){
        reduced_mass = body_get_mass(body1);
    }
    double vel_diff = fabs(vec_dot(axis, body_get_velocity(body1)) - vec_dot(axis, body_get_velocity(body2)));
    Vector centroid_diff = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
    double pm_one = vec_dot(axis, centroid_diff) / fabs(vec_dot(axis, centroid_diff));
    body_add_impulse(body1, vec_multiply(reduced_mass * 2 * vel_diff * pm_one * -1, axis));
    body_add_impulse(body2, vec_multiply(reduced_mass * 2 * vel_diff * pm_one, axis));
    BodyInfo *body1_info = body_get_info(body1);
    body1_info->health --;
    if (body1_info->health <= 0) {
        body_remove(body1);
    }
    BodyInfo *body2_info = body_get_info(body2);
    body2_info->health --;
    if (body2_info->health <= 0) {
        body_remove(body2);
    }

    while(find_collision(body_get_shape(body1), body_get_shape(body2)).collided){
        body_tick(body1, .001);
        body_tick(body2, .001);
    }
}

void create_life_depleting_collision(Scene *scene, Body *body1, Body *body2){
    create_collision(scene, body1, body2, life_depleting_collision_handler, NULL, NULL);
}

int main(int argc, char *argv[]) {
    srand(time(0));

    Scene *scene = scene_init();
    create_background(scene, COLOR_BLACK);
    Body *ship = create_ship(scene);
    Body *ball = create_ball(ship, scene);
    create_screen_edges(scene, COLOR_WHITE);
    make_blocks(NUM_ROWS, NUM_COLS, scene);

    Vector bottom_left = (Vector) {-CANVAS_WIDTH/2, -CANVAS_HEIGHT/2};
    Vector top_right = (Vector) {CANVAS_WIDTH/2, CANVAS_HEIGHT/2};
    sdl_init(bottom_left, top_right);
    sdl_render_scene(scene);
    sdl_on_key((KeyHandler) move_ship);

    double current_ball_speed = INIT_BALL_SPEED;
    double elapsed = 0;
    bool isover = false;

    create_physics_collision(scene, 1, ship, ball);
    for (int i = FLOOR_INDEX + 1; i < scene_bodies(scene); i ++) {
        Body *body = scene_get_body(scene, i);
        create_life_depleting_collision(scene, ball, body);
    }
    for (int i = LEFT_WALL_INDEX; i < FLOOR_INDEX; i ++) {
        Body *body = scene_get_body(scene, i);
        create_physics_collision(scene, 1, ball, body);
    }
    create_life_depleting_collision(scene, ball, scene_get_body(scene, FLOOR_INDEX));

    int elapsed_ticks = 0;

    while (!sdl_is_done()) {
        double dt = time_since_last_tick();
        elapsed += dt;
        elapsed_ticks ++;

        /* Lose condition. */
        if (((BodyInfo *)body_get_info(scene_get_body(scene, FLOOR_INDEX)))->health < 2 && !isover) {
            body_set_centroid(ship, (Vector) {0, -CANVAS_HEIGHT/2 + SPACING});
            current_ball_speed = INIT_BALL_SPEED;
            body_set_velocity(ball, (Vector) {1, 2});
            body_set_centroid(ball, vec_add(body_get_centroid(ship), (Vector){0, 10}));
            ((BodyInfo *)body_get_info(scene_get_body(scene, FLOOR_INDEX)))->health = 2;
            for (int i = FLOOR_INDEX + 1; i < scene_bodies(scene); i++) {
                body_remove(scene_get_body(scene, i));
            }
            int old_num_bodies = scene_bodies(scene);
            make_blocks(NUM_ROWS, NUM_COLS, scene);
            for (int i = old_num_bodies; i < scene_bodies(scene); i ++) {
                Body *body = scene_get_body(scene, i);
                create_life_depleting_collision(scene, ball, body);
            }
        }
        /* Win condition. */
        else if (scene_bodies(scene) == FLOOR_INDEX + 1 && !isover) {
            isover = true;
            sdl_on_key((KeyHandler) NULL);
            for (int i = 0; i < scene_bodies(scene); i++) {
                body_set_velocity(scene_get_body(scene, i), VEC_ZERO);
            }
            body_set_velocity(scene_get_body(scene, 0), VEC_ZERO);
            List *star = make_n_star(5, 3 * BLOCK_HEIGHT);
            Body *goodjob = body_init(star, LARGE_MASS, COLOR_YELLOW);
            scene_add_body(scene, goodjob);
            scene_tick(scene, DT);
            sdl_render_scene(scene);
        }

        if (elapsed > DT && !isover) {
            /* Make sure the player stays on screen. */
            if (on_edge(ship)){
                body_set_velocity(ship, VEC_ZERO);
                body_set_centroid(ship, (Vector) {body_get_centroid(ship).x * .99, -CANVAS_HEIGHT/2 + SPACING});
            }

            /* Update colors of bricks. */
            for (int i = FLOOR_INDEX + 1; i < scene_bodies(scene); i ++) {
                Body *block = scene_get_body(scene, i);
                BodyInfo *body_info = body_get_info(block);
                body_set_color(block, health_to_color(body_info->health, elapsed_ticks));
            }

            body_set_color(ship, health_to_color(((BodyInfo *)body_get_info(ship))->health, elapsed_ticks / 10));

            /* Regulate ball speed */
            Vector ball_vel = body_get_velocity(ball);
            double scaled_ball_speed = vec_len(ball_vel);
            body_set_velocity(ball, vec_multiply(current_ball_speed / scaled_ball_speed, ball_vel));
            if (current_ball_speed < MAX_BALL_SPEED) current_ball_speed += BALL_ACCELERATION;
            scene_tick(scene, elapsed);

            sdl_render_scene(scene);
            elapsed = 0;
        }
    }
    scene_free(scene);
}
