#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "polygon.h"


double polygon_area(List *polygon){
    size_t num_vert = list_size(polygon);

    // Uses the Shoelace Theorem to calculate the area of the polygon.
    double area = (*(Vector *)list_get(polygon, num_vert-1)).x * ((*(Vector *)list_get(polygon, 0)).y - (*(Vector *)list_get(polygon, num_vert-2)).y);
    area += (*(Vector *)list_get(polygon, 0)).x * ((*(Vector *)list_get(polygon, 1)).y - (*(Vector *)list_get(polygon, num_vert-1)).y);
    for(size_t i = 1; i < num_vert - 1; ++i) {
        area += (*(Vector *)list_get(polygon, i)).x * ((*(Vector *)list_get(polygon, i+1)).y - (*(Vector *)list_get(polygon, i-1)).y);
    }
    return fabs(0.5 * area);
}

Vector polygon_centroid(List *polygon){
    size_t num_vert = list_size(polygon);
    // Calculates the centroid of the polygon from its vertices.
    double cx = ((*(Vector *)list_get(polygon, num_vert-1)).x + (*(Vector *)list_get(polygon, 0)).x) * ((*(Vector *)list_get(polygon, num_vert-1)).x * (*(Vector *)list_get(polygon, 0)).y - (*(Vector *)list_get(polygon, 0)).x * (*(Vector *)list_get(polygon, num_vert-1)).y);
    double cy = ((*(Vector *)list_get(polygon, num_vert-1)).y + (*(Vector *)list_get(polygon, 0)).y) * ((*(Vector *)list_get(polygon, num_vert-1)).x * (*(Vector *)list_get(polygon, 0)).y - (*(Vector *)list_get(polygon, 0)).x * (*(Vector *)list_get(polygon, num_vert-1)).y);
    double area = polygon_area(polygon);
    for(size_t i = 0; i < num_vert - 1; ++i){
        cx += ((*(Vector *)list_get(polygon, i)).x + (*(Vector *)list_get(polygon, i+1)).x) * ((*(Vector *)list_get(polygon, i)).x * (*(Vector *)list_get(polygon, i+1)).y - (*(Vector *)list_get(polygon, i+1)).x * (*(Vector *)list_get(polygon, i)).y);
        cy += ((*(Vector *)list_get(polygon, i)).y + (*(Vector *)list_get(polygon, i+1)).y) * ((*(Vector *)list_get(polygon, i)).x * (*(Vector *)list_get(polygon, i+1)).y - (*(Vector *)list_get(polygon, i+1)).x * (*(Vector *)list_get(polygon, i)).y);
    }
    Vector centroid = {
        .x = cx / (6 * area),
        .y = cy / (6 * area)
    };
    return centroid;
}

void polygon_translate(List *polygon, Vector translation){
    for(size_t i = 0; i < list_size(polygon); ++i){
        Vector *translate = malloc(sizeof(Vector));
        *translate = vec_add((*(Vector *)list_get(polygon, i)), translation);
        list_set(polygon, i, translate);
    }
}

void polygon_rotate(List *polygon, double angle, Vector point){
    polygon_translate(polygon, vec_negate(point));
    for(size_t i = 0; i < list_size(polygon); ++i){
        Vector *rotate = malloc(sizeof(Vector));
        *rotate = vec_rotate(*(Vector *)list_get(polygon, i), angle);
        list_set(polygon, i, rotate);
    }
    polygon_translate(polygon, point);
}
