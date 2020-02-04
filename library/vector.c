#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vector.h"


const Vector VEC_ZERO = {
    .x = 0,
    .y = 0
};

Vector vec_add(Vector v1, Vector v2) {
    Vector sum = {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y
    };
    return sum;
}

Vector vec_subtract(Vector v1, Vector v2) {
    Vector diff = {
        .x = v1.x - v2.x,
        .y = v1.y - v2.y
    };
    return diff;
}

Vector vec_multiply(double scalar, Vector v) {
    Vector scaled = {
        .x = scalar * v.x,
        .y = scalar * v.y
    };
    return scaled;
}

Vector vec_negate(Vector v) {
    return vec_multiply(-1, v);
}

double vec_dot(Vector v1, Vector v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

double vec_len(Vector v){
  return pow(vec_dot(v, v), 0.5);
}

double vec_cross(Vector v1, Vector v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

Vector vec_rotate(Vector v, double angle) {
    Vector rotated = {
        .x = cos(angle) * v.x - sin(angle) * v.y,
        .y = sin(angle) * v.x + cos(angle) * v.y
    };
    return rotated;
}

Vector vec_unit(Vector v){
  if(vec_len(v) == 0){
    return VEC_ZERO;
  }
  return vec_multiply(1.0/vec_len(v), v);
}

double vec_angle(Vector v){
  if (vec_len(v) == 0){
    return 0;
  }
  double angle = acos(v.x / vec_len(v));
  if (v.y < 0){
    angle = 2 * M_PI - angle;
  }
  return angle;
}

double vec_distance(Vector v1, Vector v2){
  return vec_len(vec_subtract(v1, v2));
}

double vec_get_x(Vector v) {
    return v.x;
}

double vec_get_y(Vector v) {
    return v.y;
}

Vector vec_set_x(Vector v, double x) {
    v.x = x;
    return v;
}

Vector vec_set_y(Vector v, double y) {
    v.y = y;
    return v;
}

Vector *vmalloc(Vector v) {
    Vector *temp = malloc(sizeof(Vector));
    *temp = v;
    return temp;
}
