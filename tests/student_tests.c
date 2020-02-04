#include "forces.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

List *make_shape() {
    List *shape = list_init(4, free);
    Vector *v = malloc(sizeof(*v));
    *v = (Vector) {-1, -1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {+1, -1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {+1, +1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {-1, +1};
    list_add(shape, v);
    return shape;
}

void test_drag() {
  const double M = 10;
  const double gamma = .9;
  const double Vi = 3;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  double elapsed = 0;
  Scene *scene = scene_init();
  Body *mass = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
  body_set_velocity(mass, (Vector) {Vi, 0});
  scene_add_body(scene, mass);
  create_drag(scene, gamma, mass);
  for (int i = 0; i < STEPS; i++) {
      elapsed = i * DT;
      // printf("\nVelocity Real: {%f, %f}\n", body_get_velocity(mass).x, body_get_velocity(mass).y);
      // printf("\nVelocity Expected: {%f, %f}\n", ((Vector) {Vi * pow(M_E, -gamma * elapsed / M), 0}).x, ((Vector) {Vi * pow(M_E, -gamma * elapsed / M), 0}).y);

      assert(vec_isclose(
          body_get_velocity(mass),
          (Vector) {Vi * pow(M_E, -gamma * elapsed / M), 0}
      ));
      scene_tick(scene, DT);
  }
  scene_free(scene);
}

void test_orbit() {
  const double M1 = 1.0, M2 = 1.0 * 1e5;
  const double G = 10;
  const double DT = 1e-5;
  const int STEPS = 500000;
  Scene *scene = scene_init();
  Body *earth = body_init(make_shape(), M1, (RGBColor) {0, 0, 0});
  scene_add_body(scene, earth);
  body_set_centroid(earth, (Vector) {100, 0});
  body_set_velocity(earth, (Vector) {0, 100});
  Body *sun = body_init(make_shape(), M2, (RGBColor) {0, 0, 0});
  scene_add_body(scene, sun);
  create_newtonian_gravity(scene, G, earth, sun);
  for (int i = 0; i < STEPS; i++) {
      // printf("\nDistance Vec: %f\n", vec_distance(body_get_centroid(earth), body_get_centroid(sun)));
      // printf("Distance body: %f\n", body_distance(earth, sun));

      assert(within(1e-2, vec_distance(body_get_centroid(earth), body_get_centroid(sun)), 100));
      scene_tick(scene, DT);
  }
  scene_free(scene);
}

double spring_energy(Body *b1, Body *b2, double k){
  return .5 * k * pow(vec_distance(body_get_centroid(b1), body_get_centroid(b2)), 2);
}
double kinetic_energy(Body *body) {
    Vector v = body_get_velocity(body);
    return body_get_mass(body) * vec_dot(v, v) / 2;
}

void test_spring_energy(){
  const double M = 10;
  const double K = 2;
  const double A = 3;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  Scene *scene = scene_init();
  Body *mass1 = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
  body_set_centroid(mass1, (Vector) {A, 0});
  scene_add_body(scene, mass1);
  Body *mass2 = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
  body_set_centroid(mass2, (Vector) {-A, 0});
  scene_add_body(scene, mass2);
  create_spring(scene, K, mass1, mass2);
  double initial_energy = spring_energy(mass1, mass2, K);

  for (int i = 0; i < STEPS; i++) {
    double energy = spring_energy(mass1, mass2, K) +
        kinetic_energy(mass1) + kinetic_energy(mass2);
      assert(within(1e-2, energy, initial_energy));
      scene_tick(scene, DT);
  }
  scene_free(scene);
}


int main(int argc, char *argv[]) {
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
      read_testname(argv[1], testname, sizeof(testname));
  }
    DO_TEST(test_orbit)
    DO_TEST(test_drag)
    DO_TEST(test_spring_energy)

    return 0;
}
