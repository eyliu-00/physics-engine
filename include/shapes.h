#ifndef __SHAPES_H__
#define __SHAPES_H__

#include <stdlib.h>
#include <math.h>
#include <stddef.h>
#include "list.h"
#include "polygon.h"

/**
 *  Makes a pacman-esque shape which is basically a 360-gon with a chunk cut out
 *  @param radius the radius of the pacman in pixels
 *  @return a List representing the pacman
 */
List *make_pacman(double radius);

/**
 *  Makes a square of side length side centered at 0, 0
 *  @param side the side length
 *  @return a List representing the square
 */
List *make_square(double side);

/**
 *  Makes a rectangle of height (y) height and length (x) length centered at 0, 0
 *  @param height the y-dimension of the rectangle
 *  @param length the x-dimension of the rectangle
 *  @return a List representing the rectangle
 */
List *make_rectangle(double height, double length);

/**
 *  Makes a regular n-gon of a given radius and number of sides
 *  @param sides the number of sides
 *  @param radius the radius
 *  @return a List representing the n-gon
 */
 List *make_ngon(int sides, double radius);

/**
 *  Makes an n-pointed star (convex polygon with 2n sides).
 *  @param points the number of points for the star to have
 *  @param radius the radius of the star in pixels
 *  @return a List representing the star
 */
List *make_n_star(int points, double radius);

/* Makes a section of a circle. */
List *make_rounded_paddle(int sides, double radius, double angle);

#endif
