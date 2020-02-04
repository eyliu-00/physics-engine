#include "forces.h"
#include "aux.h"
#include "collision.h"
#include "scene.h"
#include <math.h>
#include <stdio.h>

#define MIN_DISTANCE 20.0

void gravity_2_body(void *aux){
    Body *body1 = aux_get_body((Aux *)aux, 0);
    Body *body2 = aux_get_body((Aux *)aux, 1);
    double G = aux_get_constant((Aux *)aux, 0);
    double distance = body_distance(body1, body2);
    if (distance < MIN_DISTANCE) { /* So the force cannot approach infinity. */
        distance = MIN_DISTANCE;
    }
    double magnitude = body_get_mass(body1) * body_get_mass(body2) * G / pow(distance, 2);
    Vector direction = vec_unit(vec_subtract(body_get_centroid(body1), body_get_centroid(body2)));
    //second force is negated bc its in the opposite direction
    body_add_force(body1, vec_negate(vec_multiply(magnitude, direction)));
    body_add_force(body2, vec_multiply(magnitude, direction));
}

void create_newtonian_gravity(Scene *scene, double G, Body *body1, Body *body2){
    Aux *grav_aux = aux_init(2, 1);
    aux_body_add(grav_aux, body1);
    aux_body_add(grav_aux, body2);
    aux_constant_add(grav_aux, G);
    List *bodies = list_init(2, NULL);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, gravity_2_body, grav_aux, bodies, (FreeFunc) aux_free);
}

void spring_2_body(void *aux){
  Body *body1 = aux_get_body((Aux *)aux, 0);
  Body *body2 = aux_get_body((Aux *)aux, 1);
  double k = aux_get_constant((Aux *)aux, 0);
  double displacement = body_distance(body1, body2);
  Vector direction = vec_unit(vec_subtract(body_get_centroid(body1), body_get_centroid(body2)));
  body_add_force(body1, vec_negate(vec_multiply(k * displacement, direction)));
  body_add_force(body2, vec_multiply(k * displacement, direction));
}

void create_spring(Scene *scene, double k, Body *body1, Body *body2){
  Aux *spring_aux = aux_init(2, 1);
  aux_body_add(spring_aux, body1);
  aux_body_add(spring_aux, body2);
  aux_constant_add(spring_aux, k);
  List *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, spring_2_body, spring_aux, bodies, (FreeFunc) aux_free);
}

void drag_2_body(void *aux){
  Body *body = aux_get_body((Aux *)aux, 0);
  double gamma = aux_get_constant((Aux *)aux, 0);
  body_add_force(body, vec_multiply(-gamma, body_get_velocity(body)));
}


void create_drag(Scene *scene, double gamma, Body *body){
  Aux *drag_aux = aux_init(1, 1);
  aux_body_add(drag_aux, body);
  aux_constant_add(drag_aux, gamma);
  List *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, drag_2_body, drag_aux, bodies, (FreeFunc) aux_free);
}

typedef struct collision_aux {
  Body *body1;
  Body *body2;
  CollisionHandler handler;
  void *aux;
  FreeFunc freer;
} ColAux;

void col_aux_free(ColAux *colaux){
  if(colaux->freer != NULL){
    colaux->freer(colaux->aux);
  }
  free(colaux);
}

void generic_collision(void *aux){
  ColAux temp = *(ColAux *)aux;
  CollisionInfo info = find_collision(body_get_shape(temp.body1), body_get_shape(temp.body2));
  if(info.collided){
    temp.handler(temp.body1, temp.body2, info.axis, temp.aux);
  }
}

void create_collision(Scene *scene, Body *body1, Body *body2, CollisionHandler handler, void *aux, FreeFunc freer){
  ColAux *collisiondata = malloc(sizeof(ColAux));
  *collisiondata = (ColAux){body1, body2, handler, aux, freer};
  List *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, generic_collision, (void *)collisiondata, bodies, (FreeFunc) col_aux_free);
}

void destructive_collision_handler(Body *body1, Body *body2, Vector axis, void *aux){
  body_remove(body1);
  body_remove(body2);
}

void create_destructive_collision(Scene *scene, Body *body1, Body *body2) {
  create_collision(scene, body1, body2, destructive_collision_handler, NULL, NULL);
}


void physics_collision_handler(Body *body1, Body *body2, Vector axis, void *aux){
  double elasticity = *(double *)aux;
  double reduced_mass = body_get_mass(body1) * body_get_mass(body2) / (body_get_mass(body1) + body_get_mass(body2));
  if(body_get_mass(body1) == INFINITY){
    reduced_mass = body_get_mass(body2);
  } else if(body_get_mass(body2) == INFINITY){
    reduced_mass = body_get_mass(body1);
  }
  double vel_diff = fabs(vec_dot(axis, body_get_velocity(body1)) - vec_dot(axis, body_get_velocity(body2)));
  Vector centroid_diff = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
  double pm_one = vec_dot(axis, centroid_diff) / fabs(vec_dot(axis, centroid_diff));
  body_add_impulse(body1, vec_multiply(reduced_mass * (1 + elasticity) * vel_diff * pm_one * -1, axis));
  body_add_impulse(body2, vec_multiply(reduced_mass * (1 + elasticity) * vel_diff * pm_one, axis));


  while(find_collision(body_get_shape(body1), body_get_shape(body2)).collided){
    body_tick(body1, .001);
    body_tick(body2, .001);
  }
}


void create_physics_collision(Scene *scene, double elasticity, Body *body1, Body *body2){
  double *elast = malloc(sizeof(double));
  *elast = elasticity;
  create_collision(scene, body1, body2, physics_collision_handler, (void *)elast, free);
}
