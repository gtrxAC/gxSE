#include "raylib.h"
#include "tinyfiledialogs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define SCALED_W (image.width*scale)
#define SCALED_H (image.height*scale)

// Get one color value from a pixel in the image data. GP_ defines should be used for 'c'.
#define GETPIXEL(x, y, c) \
	((unsigned char *)image.data)[(y*image.width + x)*4 + c]

#define GP_RED 0
#define GP_GREEN 1
#define GP_BLUE 2
#define GP_ALPHA 3

#define DRAWTEXTF(text, x, y, color) \
	DrawTextEx(font, text, (Vector2) {x, y}, 20, 2, color)

int scale = 4;
int frameCount = 0;
bool linePaint;
bool grid;
char bottomText[256];
Font font;
Image image;
Texture imagetx;
RenderTexture alphaPattern;
Color colorA = WHITE;
Color colorB = BLACK;
Vector2 offset;
Vector2 mouseDragStart;

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

int getMouseX(void) {
	int result = (GetMouseX() - offset.x)/scale;

	if (result < 0) result = 0;
	else if (result >= image.width) result = image.width - 1;
	return result;
}

int getMouseY(void) {
	int result = (GetMouseY() - offset.y)/scale;
	
	if (result < 0) result = 0;
	else if (result >= image.height) result = image.height - 1;
	return result;
}

void redraw(void) {
	UnloadTexture(imagetx);
	imagetx = LoadTextureFromImage(image);
}

void reloadAlphaPattern(void) {
	UnloadRenderTexture(alphaPattern);
	alphaPattern = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	int patternScale = scale/2;
	if (patternScale < 3) patternScale = 3;

	BeginTextureMode(alphaPattern);
	for (int x = 0; x < GetScreenWidth(); x += patternScale) {
		for (int y = 0; y < GetScreenHeight(); y += patternScale) {
			char shade = x%(patternScale*2) ^ y%(patternScale*2) ? 85 : 170;
			DrawRectangle(x, y, patternScale, patternScale, (Color) {shade, shade, shade, 255});
		}
	}
	EndTextureMode();
}

Color fillOld, fillNew;

// https://en.wikipedia.org/wiki/Flood_fill
// Note: variables fillOld, fillNew should be set before calling
void floodFill(int x, int y) {
	if (x < 0 || x >= image.width) return;
	if (y < 0 || y >= image.height) return;

	if (
		(
			GETPIXEL(x, y, GP_RED) != fillOld.r ||
			GETPIXEL(x, y, GP_GREEN) != fillOld.g ||
			GETPIXEL(x, y, GP_BLUE) != fillOld.b ||
			GETPIXEL(x, y, GP_ALPHA) != fillOld.a
		) && (
			GETPIXEL(x, y, GP_ALPHA) || fillOld.a
		)
	) return;

	GETPIXEL(x, y, GP_RED) = fillNew.r;
	GETPIXEL(x, y, GP_GREEN) = fillNew.g;
	GETPIXEL(x, y, GP_BLUE) = fillNew.b;
	GETPIXEL(x, y, GP_ALPHA) = fillNew.a;
	floodFill(x, y + 1);
	floodFill(x, y - 1);
	floodFill(x - 1, y);
	floodFill(x + 1, y);
}

void input(void) {
	if (IsKeyPressed(KEY_L)) linePaint = !linePaint;

	if (IsKeyPressed(KEY_N)) {
		char *size = tinyfd_inputBox("New image", "Enter size as WxH, or one number used as both width and height:", " ");
		if (!size) return;

		int count, width, height;
		const char **sizes = TextSplit(size, 'x', &count);

		switch (count) {
			case 1: {
				width = atoi(sizes[0]);
				height = width;
				break;
			}

			case 2: {
				width = atoi(sizes[0]);
				height = atoi(sizes[1]);
				break;
			}

			default: {
				tinyfd_messageBox("Error", "Invalid size specified", "ok", "error", 1);
				return;
			}
		}

		UnloadImage(image);
		image = GenImageColor(width, height, BLANK);
		redraw();
	}

	if (IsKeyPressed(KEY_S)) {
		char path[512];
		strcpy(path, GetWorkingDirectory());
		strcat(path, "/*");
		char *filename = tinyfd_saveFileDialog("Save as", path, 0, NULL, NULL);
		if (filename) ExportImage(image, filename);
	}

	if (IsKeyPressed(KEY_LEFT)) {
		tool--;
		if (tool < 0) tool = 0;
	}
	if (IsKeyPressed(KEY_RIGHT)) {
		tool++;
		if (tool >= T_COUNT) tool = T_COUNT - 1;
	}

	if (IsKeyPressed('G')) grid = !grid;

	scale += GetMouseWheelMove();
	if (scale > 10) scale += GetMouseWheelMove();
	if (scale < 1) scale = 1;
	if (scale > 50) scale = 50;
	if (GetMouseWheelMove() != 0.0f) reloadAlphaPattern();
}

void update(void) {
	if (IsWindowResized()) reloadAlphaPattern();

	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		Color color = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? colorA : colorB;

		switch (tool) {
			case T_PENCIL: {
				if (linePaint) {
					ImageDrawLine(
						&image,
						getMouseX() + GetMouseDelta().x/scale,
						getMouseY() + GetMouseDelta().y/scale,
						getMouseX(),
						getMouseY(),
						color
					);
				} else {
					ImageDrawPixel(&image, getMouseX(), getMouseY(), color);
				}
				break;
			}

			case T_ERASER: {
				GETPIXEL(getMouseX(), getMouseY(), GP_RED) = 0;
				GETPIXEL(getMouseX(), getMouseY(), GP_GREEN) = 0;
				GETPIXEL(getMouseX(), getMouseY(), GP_BLUE) = 0;
				GETPIXEL(getMouseX(), getMouseY(), GP_ALPHA) = 0;
				break;
			}

			case T_BUCKET: {
				fillOld = GetPixelColor(
					&GETPIXEL(getMouseX(), getMouseY(), GP_RED),
					PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
				);
				if (ColorToInt(fillOld) != ColorToInt(color)) {
					fillNew = color;
					floodFill(getMouseX(), getMouseY());
				}
				break;
			}
		}
		redraw();
	}

	if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
		offset.x += GetMouseDelta().x;
		offset.y += GetMouseDelta().y;
	}
}

void draw(void) {
	Rectangle imageRec = (Rectangle) {offset.x, offset.y, SCALED_W, SCALED_H};

	DrawTextureRec(alphaPattern.texture, imageRec, offset, WHITE);
	DrawRectangleLines(offset.x - 1, offset.y - 1, SCALED_W + 2, SCALED_H + 2, WHITE);
	DrawTextureEx(imagetx, offset, 0.0f, scale, WHITE);

	DrawRectangleLines(
		getMouseX()*scale + offset.x,
		getMouseY()*scale + offset.y,
		scale, scale, RED
	);

	if (grid && scale > 1) {
		for (int x = 0; x < image.width; x++) {
			for (int y = 0; y < image.height; y++) {
				DrawLine(x*scale + offset.x, offset.y, x*scale + offset.x, SCALED_H + offset.y, WHITE);
				DrawLine(offset.x, y*scale + offset.y, SCALED_W + offset.x, y*scale + offset.y, WHITE);
			}	
		}
	}

	bottomText[0] = 0;
	strcat(bottomText, TextFormat("%d x %d x %d   ", image.width, image.height, scale));
	strcat(bottomText, TextFormat("Mouse %d, %d   ", getMouseX(), getMouseY()));
	strcat(bottomText, TextFormat("ColA %d %d %d %d   ", colorA.r, colorA.g, colorA.b, colorA.a));
	strcat(bottomText, TextFormat("ColB %d %d %d %d   ", colorB.r, colorB.g, colorB.b, colorB.a));
	strcat(bottomText, TextFormat("Tool %d   ", tool));
	strcat(bottomText, "Press H for help");

	for (int x = -1; x < 2; x++) {
		for (int y = -1; y < 2; y++) {
			DRAWTEXTF(bottomText, x, GetScreenHeight() - 20 + y, BLACK);
		}
	}

	DRAWTEXTF(bottomText, 0, GetScreenHeight() - 20, WHITE);
}

int main(int argc, char **argv) {
	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(1024, 768, "gxSE");
	SetTargetFPS(120);

	font = LoadFontEx("assets/font.ttf", 20, NULL, 0);

	image = GenImageColor(32, 32, BLANK);
	reloadAlphaPattern();

	while (!WindowShouldClose()) {
		input();
		update();

		BeginDrawing();
		ClearBackground(BLACK);
		draw();
		EndDrawing();
		frameCount++;
	}

	UnloadImage(image);
	UnloadTexture(imagetx);
	CloseWindow();
	return 0;
}