#ifndef __COLOR_H__
#define __COLOR_H__

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white).
 */
typedef struct {
    float r;
    float g;
    float b;
} RGBColor;

typedef struct {
    float h;
    float s;
    float v;
} HSVColor;

#define COLOR_BLACK  ((RGBColor) {0, 0, 0})
#define COLOR_WHITE  ((RGBColor) {1, 1, 1})
#define COLOR_RED    ((RGBColor) {1, 0, 0})
#define COLOR_GREEN  ((RGBColor) {0, 1, 0})
#define COLOR_BLUE   ((RGBColor) {0, 0, 1})
#define COLOR_YELLOW ((RGBColor) {1, 1, 0})
#define COLOR_MAGENTA ((RGBColor) {1, 0, 1})

#endif // #ifndef __COLOR_H__
