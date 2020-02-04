#include <stdlib.h>
#include "aux.h"
#include "list.h"

typedef struct aux {
  List *bodies;
  List *constants;
} Aux;

Aux *aux_init(size_t num_bodies, size_t num_constants){
  Aux *new_aux = malloc(sizeof(Aux));
  assert(new_aux != NULL);
  new_aux->bodies = list_init(num_bodies, NULL);
  new_aux->constants = list_init(num_constants, free);
  return new_aux;
}

void aux_free(Aux *aux){
  list_free(aux->bodies);
  list_free(aux->constants);
}

size_t aux_num_bodies(Aux *aux) {
  return list_size(aux->bodies);
}

size_t aux_num_constants(Aux *aux) {
  return list_size(aux->constants);
}

void aux_body_add(Aux *aux, Body *body) {
  list_add(aux->bodies, body);
}

void aux_constant_add(Aux *aux, double constant) {
  double *temp = malloc(sizeof(double));
  *temp = constant;
  list_add(aux->constants, temp);
}

Body *aux_get_body(Aux *aux, size_t index) {
  return (Body *)list_get(aux->bodies, index);
}

double aux_get_constant(Aux *aux, size_t index) {
  return *(double *)list_get(aux->constants, index);
}
