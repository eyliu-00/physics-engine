#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"

#define CIRCLE_POINTS 40

#define MAX ((Vector) {.x = 80.0, .y = 80.0})

#define N_ROWS 11
#define ROW_SPACING 3.6
#define COL_SPACING 3.5
#define WALL_ANGLE atan2(ROW_SPACING, COL_SPACING / 2)
#define WALL_LENGTH hypot(MAX.x / 2, MAX.y)

#define PEG_RADIUS 0.5
#define BALL_RADIUS 1.0
#define DROP_INTERVAL 1.0 // s
#define ELASTICITY 0.3
#define WALL_WIDTH 1.0
#define DELTA_X 1.0
#define DROP_Y (MAX.y - 3.0)
#define START_VELOCITY ((Vector) {.x = 0.0, .y = -8.0})

#define BALL_MASS 2.0

#define BALL_COLOR ((RGBColor) {1, 0, 0})
#define PEG_COLOR ((RGBColor) {0, 1, 0})
#define WALL_COLOR ((RGBColor) {0, 0, 1})

#define G 6.67E-11 // N m^2 / kg^2
#define M 6E24 // kg
#define g 9.8 // m / s^2
#define R (sqrt(G * M / g)) // m

typedef enum {
    BALL,
    FROZEN,
    WALL, // or peg
    GRAVITY
} BodyType;

BodyType get_type(Body *body) {
    return *(BodyType *) body_get_info(body);
}

/** Generates a random number between 0 and 1 */
double rand_double(void) {
    return (double) rand() / RAND_MAX;
}

/** Constructs a rectangle with the given dimensions centered at (0, 0) */
List *rect_init(double width, double height) {
    Vector half_width  = {.x = width / 2, .y = 0.0},
           half_height = {.x = 0.0, .y = height / 2};
    List *rect = list_init(4, free);
    Vector *v = malloc(sizeof(*v));
    *v = vec_add(half_width, half_height);
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_subtract(half_height, half_width);
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_negate(*(Vector *) list_get(rect, 0));
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_subtract(half_width, half_height);
    list_add(rect, v);
    return rect;
}

/** Constructs a circles with the given radius centered at (0, 0) */
List *circle_init(double radius) {
    List *circle = list_init(CIRCLE_POINTS, free);
    double arc_angle = 2 * M_PI / CIRCLE_POINTS;
    Vector point = {.x = radius, .y = 0.0};
    for (int i = 0; i < CIRCLE_POINTS; i++) {
        Vector *v = malloc(sizeof(*v));
        *v = point;
        list_add(circle, v);
        point = vec_rotate(point, arc_angle);
    }
    return circle;
}

/** Computes the center of the peg in the given row and column */
Vector get_peg_center(int row, int col) {
    Vector center = {
        .x = MAX.x / 2 + (col - row * 0.5) * COL_SPACING,
        .y = MAX.y - (row + 1) * ROW_SPACING
    };
    return center;
}

/** Creates an Earth-like mass to accelerate the balls */
Body *get_gravity_body() {
    // Will be offscreen, so shape is irrelevant
    List *gravity_ball = rect_init(1, 1);
    BodyType *type = malloc(sizeof(*type));
    *type = GRAVITY;
    Body *body = body_init_with_info(gravity_ball, M, WALL_COLOR, type, free);

    // Move a distnace R below the scene
    Vector gravity_center = {.x = MAX.x / 2, .y = -R};
    body_set_centroid(body, gravity_center);

    return body;
}

/** Creates a ball with the given starting position and velocity */
Body *get_ball(Vector center, Vector velocity) {
    List *shape = circle_init(BALL_RADIUS);
    BodyType *info = malloc(sizeof(*info));
    *info = BALL;
    Body *ball = body_init_with_info(shape, BALL_MASS, BALL_COLOR, info, free);

    body_set_centroid(ball, center);
    body_set_velocity(ball, velocity);

    return ball;
}

/** Collision handler to freeze a ball when it collides with a frozen body */
void freeze(Body *ball, Body *target, Vector axis, void *aux) {
    // Skip body if it was already frozen
    if (body_is_removed(ball)) return;

    // Replace the ball with a frozen version
    Scene *scene = (Scene *) aux;
    body_remove(ball);
    Body *frozen = get_ball(body_get_centroid(ball), VEC_ZERO);
    *((BodyType *) body_get_info(frozen)) = FROZEN;
    scene_add_body(scene, frozen);

    // Make other falling bodies freeze when they collide with this body
    size_t body_count = scene_bodies(scene);
    for (size_t i = 0; i < body_count; i++) {
        Body *body = scene_get_body(scene, i);
        if (get_type(body) == BALL) {
            create_collision(scene, body, frozen, freeze, scene, NULL);
        }
    }
}

/** Adds a ball to the scene */
void add_ball(Scene *scene, Body *gravity_body, List *obstacles) {
    // Add the ball to the scene.
    Vector ball_center = {
        .x = MAX.x / 2 + (rand_double() - 0.5) * DELTA_X,
        .y = DROP_Y
    };
    Body *ball = get_ball(ball_center, START_VELOCITY);
    scene_add_body(scene, ball);

    // Simulate earth's gravity acting on the ball.
    create_newtonian_gravity(scene, G, gravity_body, ball);

    // Add collisions between all bodies
    size_t obstacle_count = list_size(obstacles);
    for (size_t i = 0; i < obstacle_count; i++) {
        Body *obstacle = list_get(obstacles, i);
        create_physics_collision(scene, ELASTICITY, ball, obstacle);
    }

    // Make ball freeze when it hits the ground or other balls
    size_t body_count = scene_bodies(scene);
    for (size_t i = 0; i < body_count; i++) {
        Body *body = scene_get_body(scene, i);
        if (get_type(body) == FROZEN) {
            create_collision(scene, ball, body, freeze, scene, NULL);
        }
    }
}

/** Builds the scene to render */
List *add_obstacles(Scene *scene){
    List *obstacles = list_init(N_ROWS * N_ROWS, NULL);

    // Add N_ROWS and N_COLS of pegs.
    for (int i = 1; i <= N_ROWS; i++) {
        for (int j = 0; j <= i; j++) {
            List *polygon = circle_init(PEG_RADIUS);
            BodyType *type = malloc(sizeof(*type));
            *type = WALL;
            Body *body =
                body_init_with_info(polygon, INFINITY, PEG_COLOR, type, free);
            body_set_centroid(body, get_peg_center(i, j));
            scene_add_body(scene, body);
            list_add(obstacles, body);
        }
    }

    // Add walls
    List *rect = rect_init(WALL_LENGTH, WALL_WIDTH);
    polygon_translate(rect, (Vector) {.x = WALL_LENGTH / 2, .y = 0.0});
    polygon_rotate(rect, WALL_ANGLE, VEC_ZERO);
    BodyType *type = malloc(sizeof(*type));
    *type = WALL;
    Body *body = body_init_with_info(rect, INFINITY, WALL_COLOR, type, free);
    scene_add_body(scene, body);
    list_add(obstacles, body);

    rect = rect_init(WALL_LENGTH, WALL_WIDTH);
    polygon_translate(rect, (Vector) {.x = MAX.x - WALL_LENGTH / 2, .y = 0.0});
    polygon_rotate(rect, -WALL_ANGLE, (Vector) {.x = MAX.x, .y = 0.0});
    type = malloc(sizeof(*type));
    *type = WALL;
    body = body_init_with_info(rect, INFINITY, WALL_COLOR, type, free);
    scene_add_body(scene, body);
    list_add(obstacles, body);

    // Ground is special; it freezes balls when they touch it
    rect = rect_init(MAX.x, WALL_WIDTH);
    type = malloc(sizeof(*type));
    *type = FROZEN;
    body = body_init_with_info(rect, INFINITY, WALL_COLOR, type, free);
    body_set_centroid(body, (Vector) {.x = MAX.x / 2, .y = WALL_WIDTH / 2});
    scene_add_body(scene, body);

    return obstacles;
}

int main(int argc, char **argv){
    // Initialize the random number generator
    srand(time(NULL));

    // Initialize scene
    sdl_init(VEC_ZERO, MAX);
    Scene *scene = scene_init();

    // Add the gravity body to the scene
    Body *gravity_body = get_gravity_body();
    scene_add_body(scene, gravity_body);

    // Add pegs and walls
    List *obstacles = add_obstacles(scene);

    // Repeatedly render scene
    double time_since_drop = INFINITY;
    while (!sdl_is_done()){
        double dt = time_since_last_tick();

        // Add a new ball every DROP_INTERVAL seconds
        time_since_drop += dt;
        if (time_since_drop > DROP_INTERVAL) {
            add_ball(scene, gravity_body, obstacles);
            time_since_drop = 0.0;
        }

        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }

    // Clean up scene
    list_free(obstacles);
    scene_free(scene);
    return 0;
}
