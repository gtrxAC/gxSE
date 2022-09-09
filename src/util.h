#ifndef UTIL_H
#define UTIL_H

#include "common.h"

int getMouseX(State *s);
int getMouseY(State *s);
void redraw(State *s);
void reloadAlphaPattern(State *s);
void floodFill(State *s, int x, int y);
void drawLine(State *s, int x1, int y1, int x2, int y2, Color color);
void updateCursor(State *s);

#endif