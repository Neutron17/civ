#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <execinfo.h>
#include "array.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void sigHandler(int sig);
int isDir(const char *name);
bool isSupportedFile(const char *name);
void addDirChildrenToArr(const char *dname, Array *arr, int fnameMaxLen);
void printTrace(void);

void __assert_m(bool expr, const char *restrict msg, const char *restrict file, const char *restrict func, int line);
#define ASSERT_M(EXPR, MSG) __assert_m(EXPR, MSG, __FILE__, __func__, __LINE__)
#define ASSERT(EXPR) ASSERT_M(EXPR, NULL)

const char *supportedExts[4] = {
	".png", ".jpg", ".jpeg", ".webp"
};
bool quit = false;
bool isDebug = false;
bool showWarnings = false;

/* TODO
 * parseArgs
 *
 */

int main(int argc, char *argv[]) {
	signal(SIGINT, sigHandler);
	if(argc < 2) {
		fprintf(stderr, "ERROR: Not enough arguments\n");
		return -1;
	}
	Array imgs = array_create(argc);
	for(int i = 1; i < argc; i++)
		addDirChildrenToArr(argv[i], &imgs, 128);
	if(imgs.used < 1)
		goto cleanUp;
	if(isDebug) {
		for (int i = 0; i < imgs.used; i++)
			printf("%s\n", (char *)array_get(imgs, i));
	}

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		fprintf(stderr, "ERROR: Couldn't init sdl, '%s'\n", SDL_GetError());
		return -1;
	}
	if(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_WEBP) == 0) {
		fprintf(stderr, "ERROR: Couldn't init SDL_image, '%s'\n", IMG_GetError());
		return -1;
	}
	int width = 1024, height = 720;
	SDL_Window *window = SDL_CreateWindow("C Image Viewer", 0, 0, width, height, SDL_WINDOW_RESIZABLE);
	if(!window) {
		fprintf(stderr, "ERROR: Couldn't create window, '%s'\n", SDL_GetError());
		return -2;
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(!renderer) {
		fprintf(stderr, "ERROR: Couldn't create renderer, '%s'\n", SDL_GetError());
		return -3;
	}
	int pointer = 0;
	SDL_Texture *text = NULL;
	{
		char *str = (char *)array_get(imgs, pointer);
		ASSERT_M(str, "First image was NULL");
		SDL_SetWindowTitle(window, str);
		text = IMG_LoadTexture(renderer, str);
	}
	SDL_Event e;
	while(!quit) {
		SDL_WaitEvent(&e);
		switch (e.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_WINDOWEVENT:
				switch (e.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						SDL_GetWindowSize(window, &width, &height);
						break;
					default:
						break;
				}
				break;
			case SDL_KEYDOWN:
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_L:
					case SDL_SCANCODE_N:
					case SDL_SCANCODE_J:
					case SDL_SCANCODE_S:
					case SDL_SCANCODE_RIGHT: {
						if(pointer >= imgs.used-1)
							pointer = 0;
						else
							pointer++;
						if(isDebug)
							printf("ptr: %d %s %d\n", pointer, (char *)array_get(imgs, pointer), imgs.used);
						char *str = (char *) array_get(imgs, pointer);
						ASSERT_M(str, "filename of image was NULL");
						SDL_SetWindowTitle(window, str);
						SDL_DestroyTexture(text);
						text = IMG_LoadTexture(renderer, str);
						break;
					}
					case SDL_SCANCODE_H:
					case SDL_SCANCODE_K:
					case SDL_SCANCODE_A:
					case SDL_SCANCODE_W:
					case SDL_SCANCODE_LEFT: {
						if(pointer <= 0)
							pointer = imgs.used-1;
						else
							pointer--;
						if(isDebug)
							printf("ptr: %d %s %d\n", pointer, (char *)array_get(imgs, pointer), imgs.used);
						SDL_DestroyTexture(text);
						char *str = (char *) array_get(imgs, pointer);
						ASSERT_M(str != NULL, "filename of image was NULL");
						SDL_SetWindowTitle(window, str);
						text = IMG_LoadTexture(renderer, str);
						break;
					}
					case SDL_SCANCODE_Q:
						quit = true;
						break;
					case SDL_SCANCODE_D:
						puts("Debug Info:");
						puts(" - Stack Trace");
						printTrace();
						printf(" - SDL\n"
							"\tWindow: %p\n"
							"\tWindow_w: %d\n"
							"\tWindow_h: %d\n"
							"\tRenderer: %p\n"
							"\tTexture: %p\n"
							"\tSDL_GetError: '%s'\n"
							"\tIMG_GetError: '%s'\n"
							" - Other stuff\n"
							"\terrno: %d\n"
							"\tstrerror: '%s'\n"
							"\tprogram_name: '%s'\n"
							"\targc: %d\n"
							" - Images Array\n"
							"\tsize: %d\n"
							"\tused: %d\n"
							"\tarray: %p\n"

							,(void *)window, width, height, (void *)renderer, (void *)text, SDL_GetError(), IMG_GetError(),
							errno, strerror(errno), argv[0], argc,
							imgs.size, imgs.used, (void *)imgs.array
							);
						for(int i = 0; i < imgs.used; i++) {
							char *curr = array_get(imgs, i);
							if(!curr) {
								printf("\t\t%d: NULL\n", i);
								break;
							}
							printf("\t\t%d: %p", i, (void *)curr);
							fflush(stdout);
							printf(" '%s'\n", curr);
						}
						break;
					default: {}
				}
				break;
			default: {}
		}
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, text, NULL, NULL);
		SDL_RenderPresent(renderer);
	}
cleanUp:
	for(int i = 0; i < imgs.used; i++)
		free(array_get(imgs, i));
	array_destroy(&imgs);
	SDL_DestroyTexture(text);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}

void addDirChildrenToArr(const char *dname, Array *arr, int fnameMaxLen) {
	static int depth = 0;
	depth++;
	if(depth > 5)
		return;
	unsigned nameMax = fnameMaxLen;
	if(fnameMaxLen <= 0)
		nameMax = 64;
	if(strnlen(dname, nameMax+1) > nameMax) {
		fprintf(stderr, "ERROR: '%s' is longer than max length(%d)\n", dname, nameMax);
	}
	if(isDir(dname)) {
		DIR *d = opendir(dname);
		struct dirent *dir;
		if (d) {
			while ((dir = readdir(d)) != NULL) {
				if(dir->d_name[0] == '.')
					continue;
				int sprntfo = -1;
				char *name = malloc(nameMax);
				if(dname[strnlen(dname, nameMax)-1] == '/')
					sprntfo = snprintf(name, nameMax, "%s%s", dname, dir->d_name);
				else
					sprntfo = snprintf(name, nameMax, "%s/%s", dname, dir->d_name);

				if(sprntfo > nameMax) {
					fprintf(stderr, "ERROR: '%s...' Too long filename\n", name);
					return;
				}
				if(!isDir(name)) {
					if(isSupportedFile(name))
						array_add(arr, (void *)name);
				} else {
					char tmp[nameMax];
					if(dname[strnlen(dname, nameMax)-1] == '/')
						sprntfo = snprintf(tmp, nameMax, "%s%s", dname, dir->d_name);
					else
						sprntfo = snprintf(tmp, nameMax, "%s/%s", dname, dir->d_name);

					if(sprntfo > nameMax) {
						fprintf(stderr, "ERROR: '%s...' Too long filename\n", tmp);
						return;
					}
					addDirChildrenToArr(tmp, arr, nameMax);
				}
			}
			closedir(d);
		}
	} else {
		if(isSupportedFile(dname))
			array_add(arr, (void *)dname);
	}
}

bool isSupportedFile(const char *name) {
	char *substr = NULL;
	bool found = false;
	for(int i = 0; i < 4; i++) {
		substr = NULL;
		substr = strstr(name, supportedExts[i]);
		if(substr) {
			found = true;
			break;
		}
	}
	if(!found && showWarnings)
		printf("WARNING: '%s' has unsupported extension\n", name);
	return found;
}

int isDir(const char *name) {
	struct stat statbuf;
	if (stat(name, &statbuf) != 0)
		return 0;
	return S_ISDIR(statbuf.st_mode);
}

void __assert_m(bool expr, const char *restrict msg, const char *restrict file, const char *restrict func, int line) {
	if(!expr) {
		char mesg[256];
		snprintf(mesg, 256, "ERROR: Assertion failed\n"
				"\tin file: '%s'\n"
				"\tin function: '%s'\n"
				"\tat line: %d\n"
				"\terrno: %d\n"
				"\tstrerror: '%s'\n"
				"\tSDL_GetError: '%s'\n"
				"\tIMG_GetError: '%s'\n",

				file, func, line, errno, strerror(errno), SDL_GetError(), IMG_GetError()
		);
		if(msg) {
			strncat(mesg, "\tMessage: '", 12);
			strncat(mesg, msg, 32);
			strncat(mesg, "'", 2);

			//snprintf(mesg, 256, "%s\tMessage: '%s'\n", mesg, msg);
		}
		fprintf(stderr, "%s", mesg);
		FILE *file = fopen("niv_err.log", "a");
		if(!file) {
			fprintf(stderr, "ERROR: Couldn't open 'niv_err.log' file\n");
			abort();
		}
		fprintf(file, "%s", mesg);
		fclose(file);
		abort();
	}
}

void printTrace(void) {
	void *array[10];
	char **strings;
	int size, i;
	size = backtrace (array, 10);
	strings = backtrace_symbols (array, size);
	if (strings != NULL) {
	printf("\tObtained %d stack frames.\n", size);
	for (i = 0; i < size; i++)
		printf ("\t%s\n", strings[i]);
	}
	free (strings);
}

void sigHandler(int sig) {
	quit = false;
}

