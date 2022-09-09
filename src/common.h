#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "raylib.h"

#define SCALED_W (s->image.width*s->scale)
#define SCALED_H (s->image.height*s->scale)

// Get one color value from a pixel in the image data. GP_ defines should be used for 'c'.
#define GETPIXEL(x, y, c) \
	((unsigned char *)s->image.data)[(y*s->image.width + x)*4 + c]

#define GP_RED 0
#define GP_GREEN 1
#define GP_BLUE 2
#define GP_ALPHA 3

#define DRAWTEXTF(text, x, y, color) \
	DrawTextEx(s->font, text, (Vector2) {x, y}, 20, 2, color)

#define MESSAGE(msg) {s->messageTimer = 360; strcpy(s->message, msg);}


typedef struct State {
	int frameCount;
	bool shouldClose;

	int scale;
	bool grid;
	Vector2 offset;
	Color colorA, colorB;

	enum {
		T_INVALID = -1,
		T_PENCIL,
		T_ERASER,
		T_BUCKET,
		T_LINE,
		T_RECTANGLE,
		T_CIRCLE,
		T_COUNT
	} tool;

	Font font;
	Image image;
	Texture imagetx;
	RenderTexture alphaPattern;

	int messageTimer;
	char message[128];

	char renderedCommand[128];
	char commandBuf[256];
	int commandCursor;

	char bottomText[256];

	int prevMouseX, prevMouseY;
	Vector2 mouseDragStart;

	Color fillOld, fillNew;
} State;

#endif