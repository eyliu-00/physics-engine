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

#define SHIP_MAX_SPEED (1/DT)
#define SHIP_ACCELERATION (0.1/DT)
#define SHIP_MASS 1
#define IMPULSE_MAGNITUDE (0.1/DT)
#define SHIP_VEL (1/DT)
#define SHIP_COLOR ((RGBColor) {.r = 15.0/255, .g = 188.0/255, .b = 0})
#define MAX_LIVES 5

#define INITIAL_POSITION ((Vector) {-CANVAS_WIDTH/4, -CANVAS_HEIGHT/4})

#define NUM_PLANETS 10
#define PLANET_MASS 100000.0

#define NUM_ASTEROIDS 20
#define ASTEROID_MASS 10.0
#define ASTEROID_INIT_VEL polar_to_cartesian(10, rand() % 6)

#define NUM_ALIENS 10

#define CANVAS_WIDTH 1000
#define CANVAS_HEIGHT 1000

#define BACKGROUND_INDEX 0
#define SHIP_INDEX 1
#define HABITABLE_PLANET_INDEX 2
#define FIRST_PLANET_INDEX 3
#define FIRST_ASTEROID_INDEX (FIRST_PLANET_INDEX + NUM_PLANETS)
#define FIRST_ALIEN_INDEX (FIRST_ASTEROID_INDEX + NUM_ASTEROIDS)

#define G 1

/* To do next:
 * make ship wrap around edge
 * make planets non-random (do later)
 */

typedef struct body_info {
    int health;
} BodyInfo;

BodyInfo *body_info_init(int health) {
    BodyInfo *body_info = malloc(sizeof(BodyInfo));
    assert(body_info);
    body_info->health = health;
    return body_info;
}

RGBColor health_to_color(int health) {
    double d = (double) health / MAX_LIVES * 3;
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
    List *ship_shape = make_rectangle(10, 10);
    Body *ship = body_init_with_info(ship_shape, SHIP_MASS, SHIP_COLOR, info, free);
    body_set_centroid(ship, INITIAL_POSITION);
    scene_add_body(scene, ship);
    return ship;
}

Body *create_background(Scene *scene, RGBColor color) {
    List *background_shape = make_rectangle(CANVAS_HEIGHT, CANVAS_WIDTH);
    Body *background = body_init_with_info(background_shape, LARGE_NUMBER, color, NULL, NULL);
    body_set_velocity(background, VEC_ZERO);
    body_set_centroid(background, VEC_ZERO);
    scene_add_body(scene, background);
    return background;
}

Vector polar_to_cartesian(double radius, double angle) {
    return (Vector) {radius * cos(angle), radius * sin(angle)};
}

double vec_to_angle(Vector v) {
    if (v.x == 0) {
        return M_PI/2; // could also be M_PI * 3/2
    }
    return atan(v.y / v.x) + M_PI * (v.x < 0);
}

void move_ship(char key, KeyEventType type, double held_time, Scene *scene) {

    Body *ship = scene_get_body(scene, SHIP_INDEX);
    Vector velocity = body_get_velocity(ship);
    double speed = vec_len(velocity);
    double angle = vec_to_angle(velocity);

    if (type == KEY_PRESSED) {
            switch (key) {
                case RIGHT_ARROW:
                    body_add_impulse(ship, polar_to_cartesian(IMPULSE_MAGNITUDE, angle - M_PI/2));
                    break;
                case LEFT_ARROW:
                    body_add_impulse(ship, polar_to_cartesian(IMPULSE_MAGNITUDE, angle + M_PI/2));
                    break;
                case UP_ARROW:
                    if (vec_len(body_get_velocity(ship)) < SHIP_MAX_SPEED ) {
                        body_add_impulse(ship, polar_to_cartesian(IMPULSE_MAGNITUDE, angle));
                    }
                    break;
                case DOWN_ARROW:
                    body_add_impulse(ship, polar_to_cartesian(-IMPULSE_MAGNITUDE, angle));
                    break;
            }
    }
    if (type == KEY_RELEASED) {
        /* Set velocity of ship to 0 only if left or right arrow keys
           are released. */
        switch (key) {
            case RIGHT_ARROW:
            case UP_ARROW:
            case DOWN_ARROW:
            case LEFT_ARROW:
                // body_set_velocity(scene_get_body(scene, SHIP_INDEX), VEC_ZERO);
                break;
        }
    }

    printf("speed: %f\t\tangle: %f\n", speed, angle);
}

void create_aliens(Scene *scene) {
    for (int i = 0; i < NUM_ALIENS; i++) {
        double y_pos = rand() % ((int)(CANVAS_HEIGHT)) - CANVAS_HEIGHT/2;
        double x_pos = rand() % ((int)(CANVAS_WIDTH)) - CANVAS_WIDTH/2;
        BodyInfo *info = body_info_init(2);
        List *alien_shape = make_rectangle(10, 10);
        Body *alien = body_init_with_info(alien_shape, SMALL_MASS, COLOR_RED, info, free);
        body_set_centroid(alien, (Vector) {x_pos, y_pos});
        body_set_velocity(alien, ASTEROID_INIT_VEL);
        scene_add_body(scene, alien);
    }
}

void create_asteroids(Scene *scene) {
    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        double y_pos = rand() % ((int)(CANVAS_HEIGHT)) - CANVAS_HEIGHT/2;
        double x_pos = rand() % ((int)(CANVAS_WIDTH)) - CANVAS_WIDTH/2;
        BodyInfo *info = body_info_init(LARGE_NUMBER);
        List *asteroid_shape = make_ngon(20, 5);
        Body *asteroid = body_init_with_info(asteroid_shape, ASTEROID_MASS, COLOR_YELLOW, info, free);
        body_set_centroid(asteroid, (Vector) {x_pos, y_pos});
        body_set_velocity(asteroid, VEC_ZERO);
        scene_add_body(scene, asteroid);
    }
}

void create_planets(Scene *scene) {
    for (int i = 0; i < NUM_PLANETS; i++) {
        double y_pos = rand() % ((int)(CANVAS_HEIGHT)) - CANVAS_HEIGHT/2;
        double x_pos = rand() % ((int)(CANVAS_WIDTH)) - CANVAS_WIDTH/2;
        BodyInfo *info = body_info_init(LARGE_NUMBER);
        List *planet_shape = make_ngon(20, 10);
        Body *planet = body_init_with_info(planet_shape, PLANET_MASS, COLOR_BLUE, info, free);
        body_set_centroid(planet, (Vector) {x_pos, y_pos});
        body_set_velocity(planet, VEC_ZERO);
        scene_add_body(scene, planet);
    }
}

Body *create_habitable_planet(Scene *scene) {
    double y_pos = CANVAS_HEIGHT / 4;
    double x_pos = CANVAS_WIDTH / 4;
    BodyInfo *info = body_info_init(LARGE_NUMBER);
    List *planet_shape = make_ngon(20, 10);
    Body *planet = body_init_with_info(planet_shape, PLANET_MASS, COLOR_GREEN, info, free);
    body_set_centroid(planet, (Vector) {x_pos, y_pos});
    body_set_velocity(planet, VEC_ZERO);
    scene_add_body(scene, planet);
    return planet;
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
    Body *habitable_planet = create_habitable_planet(scene);
    create_planets(scene);
    create_asteroids(scene);
    create_aliens(scene);

    Vector bottom_left = (Vector) {-CANVAS_WIDTH/2, -CANVAS_HEIGHT/2};
    Vector top_right = (Vector) {CANVAS_WIDTH/2, CANVAS_HEIGHT/2};
    sdl_init(bottom_left, top_right);
    sdl_render_scene(scene);
    sdl_on_key((KeyHandler) move_ship);

    double elapsed = 0;
    bool isOver = false;

    // add forces here
    for (int i = HABITABLE_PLANET_INDEX; i < FIRST_ASTEROID_INDEX; i ++) {
        Body *planet = scene_get_body(scene, i);
        create_newtonian_gravity(scene, G, planet, ship);
    }
    for (int i = HABITABLE_PLANET_INDEX; i < FIRST_ASTEROID_INDEX; i ++) {
        Body *planet = scene_get_body(scene, i);
        for (int j = FIRST_ASTEROID_INDEX; j < FIRST_ALIEN_INDEX; j ++) {
            Body *asteroid = scene_get_body(scene, j);
            create_newtonian_gravity(scene, G, planet, asteroid);
        }
    }

    while (!sdl_is_done()) {
        double dt = time_since_last_tick();
        elapsed += dt;

        /* Lose condition. */

        /* Win condition. */

        if (elapsed > DT && !isOver) {

            // /* Regulate ship speed. */
            // Vector ship_vel = body_get_velocity(ship);
            // double ship_speed = vec_len(ship_vel);
            // if (ship_speed > SHIP_MAX_SPEED) {
            //     body_set_velocity(ship, vec_multiply(SHIP_MAX_SPEED / ship_speed, body_get_velocity(ship)));
            // }

            scene_tick(scene, elapsed);
            sdl_render_scene(scene);
            elapsed = 0;
        }
    }
    scene_free(scene);
}
