#include "common.h"
#include "util.h"

void cmdNew(State *s, const char **words, int wordCount) {
	int width, height;

	switch (wordCount) {
		case 1: {
			width = atoi(words[0]);
			height = width;
			break;
		}

		case 2: {
			width = atoi(words[0]);
			height = atoi(words[1]);
			break;
		}

		default: {
			MESSAGE("Invalid size specified");
			return;
		}
	}

	UnloadImage(s->image);
	s->image = GenImageColor(width, height, BLANK);
	redraw(s);
}

void cmdColorCommon(State *s, const char **words, int wordCount, Color *color) {
	switch (wordCount) {
		case 1: {
			if (words[0][0] == '0' && words[0][1] == 'x') {
				*color = GetColor(strtol(words[0], NULL, 0));
			} else {
				color->r = atoi(words[0]);
				color->g = color->r;
				color->b = color->r;
				color->a = 255;
			}
			break;
		}

		case 3: {
			color->r = atoi(words[0]);
			color->g = atoi(words[1]);
			color->b = atoi(words[2]);
			color->a = 255;
			break;
		}

		case 4: {
			color->r = atoi(words[0]);
			color->g = atoi(words[1]);
			color->b = atoi(words[2]);
			color->a = atoi(words[3]);
			break;
		}

		default: {
			MESSAGE("Invalid argument count (expected 1, 3, or 4)");
			break;
		}
	}
}

void cmdColorA(State *s, const char **words, int wordCount) {
	cmdColorCommon(s, words, wordCount, &s->colorA);
}

void cmdColorB(State *s, const char **words, int wordCount) {
	cmdColorCommon(s, words, wordCount, &s->colorB);
}

char saveName[128];

void cmdSave(State *s, const char **words, int wordCount) {
	// temp buffer is needed to store the save file name, ExportImage calls
	// TextSplit which will overwrite words (arguments) which were also created
	// with TextSplit (static buffer)
	char *saveName = calloc(strlen(words[0]) + 1, 1);
	strcpy(saveName, words[0]);

	if (!ExportImage(s->image, saveName)) {
		MESSAGE("Save failed, missing permissions or unsupported format?");
	}

	free(saveName);
}

void cmdSaveQuit(State *s, const char **words, int wordCount) {
	char *saveName = calloc(strlen(words[0]) + 1, 1);
	strcpy(saveName, words[0]);

	if (ExportImage(s->image, saveName)) {
		s->shouldClose = true;
	} else {
		MESSAGE("Save failed, missing permissions or unsupported format?");
	}

	free(saveName);
}

void cmdQuit(State *s, const char **words, int wordCount) {
	s->shouldClose = true;
}

// The command system is very simple, each registered command has a name that
// it is called by, a function, and a minimum argument count.
// The function itself handles other things like parsing numbers from the args.
void command(State *s, const char *cmd) {
	int wordCount;
	const char **words = TextSplit(cmd, ' ', &wordCount);

	// Note: first word (the command name) is skipped when calling the function
	#define COMMAND(str, func, minArgs) \
		if (!strcmp(words[0], str)) { \
			if (wordCount - 1 < minArgs) { \
				MESSAGE(TextFormat("Too few arguments (expected %d)", minArgs)); \
			} else { \
				func(s, &words[1], wordCount - 1); \
			} \
		}

	COMMAND("n", cmdNew, 1);
	COMMAND("a", cmdColorA, 1);
	COMMAND("b", cmdColorB, 1);
	COMMAND("s", cmdSave, 1);
	COMMAND("qs", cmdSaveQuit, 1);
	COMMAND("qn", cmdQuit, 0);
}