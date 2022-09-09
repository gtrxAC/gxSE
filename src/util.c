#include "common.h"

int getMouseX(State *s) {
	int result = (GetMouseX() - s->offset.x)/s->scale;

	if (result < 0) result = 0;
	else if (result >= s->image.width) result = s->image.width - 1;
	return result;
}

int getMouseY(State *s) {
	int result = (GetMouseY() - s->offset.y)/s->scale;
	
	if (result < 0) result = 0;
	else if (result >= s->image.height) result = s->image.height - 1;
	return result;
}

void redraw(State *s) {
	UnloadTexture(s->imagetx);
	s->imagetx = LoadTextureFromImage(s->image);
}

void reloadAlphaPattern(State *s) {
	UnloadRenderTexture(s->alphaPattern);
	s->alphaPattern = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	int patternScale = s->scale/2;
	if (patternScale < 3) patternScale = 3;

	BeginTextureMode(s->alphaPattern);
	for (int x = 0; x < GetScreenWidth(); x += patternScale) {
		for (int y = 0; y < GetScreenHeight(); y += patternScale) {
			char shade = x%(patternScale*2) ^ y%(patternScale*2) ? 85 : 170;
			DrawRectangle(x, y, patternScale, patternScale, (Color) {shade, shade, shade, 255});
		}
	}
	EndTextureMode();
}

// https://en.wikipedia.org/wiki/Flood_fill
// Note: variables fillOld, fillNew should be set before calling
void floodFill(State *s, int x, int y) {
	if (x < 0 || x >= s->image.width) return;
	if (y < 0 || y >= s->image.height) return;

	if (
		(
			GETPIXEL(x, y, GP_RED) != s->fillOld.r ||
			GETPIXEL(x, y, GP_GREEN) != s->fillOld.g ||
			GETPIXEL(x, y, GP_BLUE) != s->fillOld.b ||
			GETPIXEL(x, y, GP_ALPHA) != s->fillOld.a
		) && (
			GETPIXEL(x, y, GP_ALPHA) || s->fillOld.a
		)
	) return;

	GETPIXEL(x, y, GP_RED) = s->fillNew.r;
	GETPIXEL(x, y, GP_GREEN) = s->fillNew.g;
	GETPIXEL(x, y, GP_BLUE) = s->fillNew.b;
	GETPIXEL(x, y, GP_ALPHA) = s->fillNew.a;
	floodFill(s, x, y + 1);
	floodFill(s, x, y - 1);
	floodFill(s, x - 1, y);
	floodFill(s, x + 1, y);
}

void drawLine(State *s, int x1, int y1, int x2, int y2, Color color) {
	float step, x, y, i;
	float dx = (x2 - x1);
	float dy = (y2 - y1);

	if (abs(dx) >= abs(dy)) step = abs(dx);
	else step = abs(dy);

	if (!step) {
		ImageDrawPixel(&s->image, x1, y1, color);
		return;
	}

	dx = dx / step;
	dy = dy / step;
	x = x1;
	y = y1;

	for (float i = 1; i <= step; i++) {
		ImageDrawPixel(&s->image, x, y, color);
		x += dx;
		y += dy;
	}
}

void updateCursor(State *s) {
	memset(s->renderedCommand, 0, 128);
	strncpy(s->renderedCommand, s->commandBuf, s->commandCursor);
	strcat(s->renderedCommand, "|");
	strcat(s->renderedCommand, &s->commandBuf[s->commandCursor]);
}