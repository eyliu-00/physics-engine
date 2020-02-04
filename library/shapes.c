#include "shapes.h"
#include <stdio.h>

#define PAC_ANG (4 * M_PI/3)
#define PAC_DELT 300
#define PAC_DTHETA ((2 * M_PI - PAC_ANG) / PAC_DELT)


List *make_square(double side) {
    /* List *sq = list_init(4, free);
    list_add(sq, vmalloc((Vector){side/2,  side/2}));
    list_add(sq, vmalloc((Vector){-side/2, side/2}));
    list_add(sq, vmalloc((Vector){-side/2, -side/2}));
    list_add(sq, vmalloc((Vector){side/2,  -side/2}));
    return sq; */
    return make_rectangle(side, side);
}

List *make_rectangle(double height, double length) {
  List *rec = list_init(4, free);
  list_add(rec, vmalloc((Vector){length/2, height/2}));
  list_add(rec, vmalloc((Vector){-length/2, height/2}));
  list_add(rec, vmalloc((Vector){-length/2, -height/2}));
  list_add(rec, vmalloc((Vector){length/2, -height/2}));
  return rec;
}

List *make_pacman(double radius){
    List *pac = list_init(PAC_DELT+2, free);
    list_add(pac, vmalloc(VEC_ZERO));
    Vector pac_point = (Vector){radius, 0};
    pac_point = vec_rotate(pac_point, PAC_ANG/2);
    for(int i = 0; i <= PAC_DELT; i++){
      list_add(pac, vmalloc(pac_point));
      pac_point = vec_rotate(pac_point, PAC_DTHETA);
    }
    return pac;
}

List *make_ngon(int sides, double radius){
  List *gon = list_init(sides, free);
  Vector point = (Vector){0, radius};
  for(int i = 0; i < sides; i++){
    list_add(gon, vmalloc(point));
    point = vec_rotate(point, 2 * M_PI / sides);
  }
  return gon;
}

List *make_rounded_paddle(int sides, double radius, double angle){
    List *gon = list_init(sides, free);
    Vector point = (Vector){0, radius};
    point = vec_rotate(point, -1 * angle / 2);
    for (int i = 0; i < sides; i++){
        list_add(gon, vmalloc(point));
        point = vec_rotate(point, angle / sides);
    }
    return gon;
}


List *make_n_star(int points, double radius){
    // smol and lomg are vectors to the dips and points of the star
    double theta = 2 * M_PI / (points * 2);
    Vector lomg = {
        .x = 0,
        .y = radius
    };
    Vector smol = vec_multiply(.35, lomg);
    smol = vec_rotate(smol, theta);
    List *star = list_init(points * 2, free);
    for(int i = 0; i < points; i++){
        list_add(star, vmalloc(lomg));
        list_add(star, vmalloc(smol));
        lomg = vec_rotate(lomg, 2 * theta);
        smol = vec_rotate(smol, 2 * theta);
    }
    return star;
}
