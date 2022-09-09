#include "common.h"
#include "command.h"
#include "util.h"

const char *tools[] = {
	"PENCIL",
	"ERASER",
	"BUCKET",
	"LINE",
	"RECT",
	"CIRCLE"
};

void input(State *s) {
	int chr = GetCharPressed();
	switch (chr) {
		case 0: {
			int key = GetKeyPressed();
			switch (key) {
				// backspace
				case KEY_BACKSPACE: {
					int len = strlen(s->commandBuf);
					if (!len) break;
					s->commandBuf[len - 1] = 0;
					s->commandCursor--;
					updateCursor(s);
					break;
				}

				case KEY_ENTER: {
					command(s, s->commandBuf);
					// fall through
				}

				case KEY_ESCAPE: {
					s->commandBuf[0] = 0;
					s->commandCursor = 0;
					updateCursor(s);
					break;
				}
			}
			break;
		}

		default: {
			int len = strlen(s->commandBuf);
			s->commandBuf[len] = chr;
			s->commandBuf[len + 1] = 0;
			s->commandCursor++;
			updateCursor(s);
			break;
		}
	}

	if (IsKeyPressed(KEY_LEFT)) {
		s->tool--;
		if (s->tool < 0) s->tool = 0;
	}
	if (IsKeyPressed(KEY_RIGHT)) {
		s->tool++;
		if (s->tool >= T_COUNT) s->tool = T_COUNT - 1;
	}

	if (IsKeyPressed('G')) s->grid = !s->grid;

	s->scale += GetMouseWheelMove();
	if (s->scale > 10) s->scale += GetMouseWheelMove();
	if (s->scale < 1) s->scale = 1;
	if (s->scale > 50) s->scale = 50;
	if (GetMouseWheelMove() != 0.0f) reloadAlphaPattern(s);
}

void update(State *s) {
	if (IsWindowResized()) reloadAlphaPattern(s);

	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		Color color = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? s->colorA : s->colorB;

		switch (s->tool) {
			case T_PENCIL: {
				drawLine(s, getMouseX(s), getMouseY(s), s->prevMouseX, s->prevMouseY, color);
				break;
			}

			case T_ERASER: {
				GETPIXEL(getMouseX(s), getMouseY(s), GP_RED) = 0;
				GETPIXEL(getMouseX(s), getMouseY(s), GP_GREEN) = 0;
				GETPIXEL(getMouseX(s), getMouseY(s), GP_BLUE) = 0;
				GETPIXEL(getMouseX(s), getMouseY(s), GP_ALPHA) = 0;
				break;
			}

			case T_BUCKET: {
				s->fillOld = GetPixelColor(
					&GETPIXEL(getMouseX(s), getMouseY(s), GP_RED),
					PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
				);
				if (ColorToInt(s->fillOld) != ColorToInt(color)) {
					s->fillNew = color;
					floodFill(s, getMouseX(s), getMouseY(s));
				}
				break;
			}
		}
		redraw(s);
	}

	if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
		s->offset.x += GetMouseDelta().x;
		s->offset.y += GetMouseDelta().y;
	}
}

void draw(State *s) {
	Rectangle imageRec = (Rectangle) {s->offset.x, s->offset.y, SCALED_W, SCALED_H};

	DrawTextureRec(s->alphaPattern.texture, imageRec, s->offset, WHITE);
	DrawRectangleLines(s->offset.x - 1, s->offset.y - 1, SCALED_W + 2, SCALED_H + 2, WHITE);
	DrawTextureEx(s->imagetx, s->offset, 0.0f, s->scale, WHITE);

	DrawRectangleLines(
		getMouseX(s)*s->scale + s->offset.x,
		getMouseY(s)*s->scale + s->offset.y,
		s->scale, s->scale, RED
	);

	if (s->grid && s->scale > 1) {
		for (int x = 0; x < s->image.width; x++) {
			for (int y = 0; y < s->image.height; y++) {
				DrawLine(x*s->scale + s->offset.x, s->offset.y, x*s->scale + s->offset.x, SCALED_H + s->offset.y, WHITE);
				DrawLine(s->offset.x, y*s->scale + s->offset.y, SCALED_W + s->offset.x, y*s->scale + s->offset.y, WHITE);
			}	
		}
	}

	s->bottomText[0] = 0;
	strcat(s->bottomText, TextFormat("%d x %d x %d   ", s->image.width, s->image.height, s->scale));
	strcat(s->bottomText, TextFormat("Mouse %d, %d   ", getMouseX(s), getMouseY(s)));
	strcat(s->bottomText, TextFormat("ColA %d %d %d %d   ", s->colorA.r, s->colorA.g, s->colorA.b, s->colorA.a));
	strcat(s->bottomText, TextFormat("ColB %d %d %d %d   ", s->colorB.r, s->colorB.g, s->colorB.b, s->colorB.a));
	strcat(s->bottomText, TextFormat("Tool %d   ", s->tool));
	strcat(s->bottomText, TextFormat("FPS %d   ", GetFPS()));
	strcat(s->bottomText, "Press H for help");

	if (strlen(s->message) && s->messageTimer) {
		for (int x = -1; x < 2; x++) {
			for (int y = -1; y < 2; y++) {
				DRAWTEXTF(s->message, x, 20 + y, BLACK);
			}
		}

		DRAWTEXTF(s->message, 0, 20, YELLOW);
		s->messageTimer--;
	}

	for (int x = -1; x < 2; x++) {
		for (int y = -1; y < 2; y++) {
			DRAWTEXTF(s->bottomText, x, GetScreenHeight() - 20 + y, BLACK);
		}
	}

	DRAWTEXTF(s->bottomText, 0, GetScreenHeight() - 20, WHITE);

	for (int x = -1; x < 2; x++) {
		for (int y = -1; y < 2; y++) {
			DRAWTEXTF(s->renderedCommand, x, y, BLACK);
		}
	}

	DRAWTEXTF(s->renderedCommand, 0, 0, WHITE);
}

int main(int argc, char **argv) {
	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(1024, 768, "gxSE");
	SetTargetFPS(120);
	SetExitKey(0);

	State *s = calloc(1, sizeof(State));
	s->scale = 4;
	s->colorA = WHITE;
	s->colorB = BLACK;

	s->font = LoadFontEx("assets/font.ttf", 20, NULL, 0);

	s->image = GenImageColor(256, 256, BLANK);
	reloadAlphaPattern(s);

	while (!WindowShouldClose() && !s->shouldClose) {
		input(s);
		update(s);

		BeginDrawing();
		ClearBackground(BLACK);
		draw(s);

		s->prevMouseX = getMouseX(s);
		s->prevMouseY = getMouseY(s);

		EndDrawing();
		s->frameCount++;
	}

	UnloadImage(s->image);
	UnloadTexture(s->imagetx);
	CloseWindow();
	return 0;
}