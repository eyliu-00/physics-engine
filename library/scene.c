/* scene.c */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "scene.h"
#include "forces.h"
#include "aux.h"
#define DEFAULT_NUM_BODIES 20
#define DEFAULT_NUM_FORCES 10


typedef struct scene {
    List *bodies;
    List *forces;
    List *auxes;
    List *auxfreers;
} Scene;

typedef struct force_object {
  ForceCreator forcer;
  Body *assoc_body1;
  Body *assoc_body2;
} ForceObj;

ForceObj *forceobj_init(ForceCreator f, Body *b1, Body *b2) {
  ForceObj *force = (ForceObj *) malloc(sizeof(ForceObj));
  force->forcer = f;
  force->assoc_body1 = b1;
  if (!(b2 == NULL)) {
    force->assoc_body2 = b2;
  }
  else {
    force->assoc_body2 = NULL;
  }
  return force;
}

void forceobj_setbody(ForceObj *f, Body *b, size_t bodynum) {
  assert(bodynum == 1 || bodynum == 2);
  if (bodynum == 1) {
    f->assoc_body1 = b;
  }
  else if (bodynum == 2) {
    f->assoc_body2 = b;
  }
}

Scene *scene_init(void) {
    Scene *scene = (Scene *) malloc(sizeof(Scene));
    scene->bodies = list_init(DEFAULT_NUM_BODIES, (FreeFunc) body_free);
    scene->forces = list_init(DEFAULT_NUM_FORCES, NULL);
    scene->auxes = list_init(DEFAULT_NUM_FORCES, NULL);
    scene->auxfreers = list_init(DEFAULT_NUM_FORCES, NULL);
    // scene->associated_bodies = list_init(DEFAULT_NUM_FORCES, NULL);
    return scene;
}

void scene_free(Scene *scene) {
    list_free(scene->bodies);
    for (size_t i = 0; i < list_size(scene->forces); i++) {
      free(list_get(scene->forces, i));
    }
    list_free(scene->forces);
    for(size_t i = 0; i < list_size(scene->auxes); i++){
      ((FreeFunc)list_get(scene->auxfreers, i))(list_get(scene->auxes, i));
    }
    list_free(scene->auxes);
    list_free(scene->auxfreers);
    // list_free(scene->associated_bodies);
    free(scene);
}

size_t scene_bodies(Scene *scene) {
    return list_size(scene->bodies);
}

Body *scene_get_body(Scene *scene, size_t index) {
    assert(index < scene_bodies(scene));
    return (Body *) list_get(scene->bodies, index);
}

List *scene_bodies_list(Scene *scene) {
    return scene->bodies;
}

void scene_add_body(Scene *scene, Body *body) {
    assert(list_size(body_get_shape(body)) >= 3);
    list_add(scene->bodies, body);
}

void scene_remove_body(Scene *scene, size_t index) {
    assert(index < scene_bodies(scene));
    body_free(list_remove(scene->bodies, index));
}

// DEPRECATED DO NOT USE
void scene_add_force_creator(Scene *scene, ForceCreator forcer, void *aux, FreeFunc freer){
    List *bodies = list_init(0, NULL);
    scene_add_bodies_force_creator(scene, forcer, aux, bodies, freer);
    list_add(scene->forces, forcer);
    list_add(scene->auxes, aux);
    list_add(scene->auxfreers, freer);
}

void scene_add_bodies_force_creator(
    Scene *scene, ForceCreator forcer, void *aux, List *bodies, FreeFunc freer) {
    ForceObj *force = NULL;
    if (list_size(bodies) == 2) {
      force = forceobj_init(forcer, (Body *)list_get(bodies, 0), (Body *)list_get(bodies, 1));
    }
    else if (list_size(bodies) == 1){
      force = forceobj_init(forcer, (Body *)list_get(bodies, 0), NULL);
    }
    list_add(scene->forces, force);
    list_add(scene->auxes, aux);
    if (freer) {
        list_add(scene->auxfreers, freer);
    } else {
        list_add(scene->auxfreers, NULL);
    }
}

void scene_tick(Scene *scene, double dt) {
    size_t num_bodies = scene_bodies(scene);
    // applies all the forces, storing in the bodies
    size_t num_forces = list_size(scene->forces);
    if (num_forces != 0) {
      for (size_t i = 0; i < num_forces; i ++) {
          ForceObj *f = (ForceObj *)list_get(scene->forces, i);
          ForceCreator force = f->forcer;
          void *aux = list_get(scene->auxes, i);
          assert(force != NULL);
          assert(aux != NULL);
          force(aux);
      }
    }
    // removes ForceObj (ForceCreator), auxes, auxfreers
    // if associated bodies are removed
    for (size_t i = 0; i < num_bodies; i++) {
      Body *body = list_get(scene->bodies, i);
      for(size_t j = 0; j < list_size(scene->forces); ++j){
        ForceObj *f = (ForceObj *)list_get(scene->forces, j);
        if (body_is_removed(body)) {
          if (f->assoc_body1 == body || f->assoc_body2 == body) {
            free(list_remove(scene->forces, j));
            ((FreeFunc)list_get(scene->auxfreers, j))(list_remove(scene->auxes, j));
            list_remove(scene->auxfreers, j);
            --j;
          }
        }
      }
    }
    // ticks all the bodies
    for (size_t i = 0; i < num_bodies; i++) {
        Body *body = list_get(scene->bodies, i);
        if (body_is_removed(body)) {
            /* Modifying the list while iterating though it. */
            body_free(list_remove(scene->bodies, i));
            i--;
        } else {
            body_tick(body, dt);
        }
        num_bodies = scene_bodies(scene);
    }
}
