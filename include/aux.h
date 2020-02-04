#ifndef __AUX_H__
#define __AUX_H__
#include <stdlib.h>
#include "body.h"

/**
 * A struct that contains a list of bodies and a list of constants
 */
typedef struct aux Aux;

/**
 * Allocates memory for a new aux.
 * Asserts that the required memory is successfully allocated.
 *
 * @return the new Aux
 */
Aux *aux_init(size_t num_bodies, size_t num_constants);


/**
 * Releases memory allocated for a given aux
 * and all the bodies and force creators it contains.
 *
 * @param scene a pointer to a scene returned from scene_init()
 */
void aux_free(Aux *aux);

 /**
  * Gets the number of bodies in a given aux.
  *
  * @param aux a pointer to an aux returned from aux_init()
  * @return the number of bodies in the aux
  */
size_t aux_num_bodies(Aux *aux);

 /**
  * Gets the number of constants in a given aux.
  *
  * @param aux a pointer to an aux returned from aux_init()
  * @return the number of constants in the aux
  */
size_t aux_num_constants(Aux *aux);

/**
 * Adds a body to the Aux struct
 *
 * @param body the body to be added
 */
void aux_body_add(Aux *aux, Body *body);

/**
 * Adds a constant to the Aux struct
 *
 * @param constant the constant to be added
 */
void aux_constant_add(Aux *aux, double constant);

/**
 * Gets the body at a given index in an aux.
 * Asserts that the index is valid.
 *
 * @param aux a pointer to an aux returned from aux_init()
 * @param index the index of the body in the aux (starting at 0)
 * @return a pointer to the body at the given index
 */
Body *aux_get_body(Aux *aux, size_t index);

/**
 * Gets the constant at a given index in an aux.
 * Asserts that the index is valid.
 *
 * @param aux a pointer to an aux returned from aux_init()
 * @param index the index of the constant in the aux (starting at 0)
 * @return the double at the given index
 */
double aux_get_constant(Aux *aux, size_t index);


#endif
