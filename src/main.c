#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>
#include "array.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void sigHandler(int sig);
int isDir(const char *name);
bool isSupportedFile(const char *name);

const char *supportedExts[4] = {
	".png", ".jpg", ".jpeg", ".webp"
};
bool quit = false;
bool showWarnings = false;

int main(int argc, char *argv[]) {
	signal(SIGINT, sigHandler);
	if(argc < 2) {
		fprintf(stderr, "ERROR: Not enough arguments\n");
		return -1;
	}
	Array imgs = array_create(argc);
	{
		DIR *d;
		struct dirent *dir;
		for(int i = 1; i < argc; i++) {
			if(isDir(argv[i])) {
				d = opendir(argv[i]);
				if (d) {
					while ((dir = readdir(d)) != NULL) {
						char *name = malloc(64);
						if(argv[i][strnlen(argv[i], 16)-1] == '/')
							sprintf(name, "%s%s", argv[i], dir->d_name);
						else
							sprintf(name, "%s/%s", argv[i], dir->d_name);
						// no recursion
						if(!isDir(name)) {
							if(isSupportedFile(name)) {
								array_add(&imgs, (void *)name);
							}
						}
					}
					closedir(d);
				}
			} else {
				if(isSupportedFile(argv[i]))
					array_add(&imgs, argv[i]);
			}
		}
	}
	for (int i = 0; i < imgs.used; i++) {
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
	SDL_Texture *text = IMG_LoadTexture(renderer, array_get(imgs, pointer));

	SDL_Event e;
	while(!quit) {
		printf("%s\n", IMG_GetError());
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
					case SDL_SCANCODE_N:
						if(pointer >= imgs.used-1)
							pointer = 0;
						else
							pointer++;
						printf("ptr: %d %s %d\n", pointer, (char *)array_get(imgs, pointer), imgs.used);
						SDL_DestroyTexture(text);
						text = IMG_LoadTexture(renderer, (char *)array_get(imgs, pointer));
						break;
					case SDL_SCANCODE_Q:
						quit = true;
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
	for(int i = 0; i < 0; i++)
		free(array_get(imgs, i));
	array_destroy(&imgs);
	SDL_DestroyTexture(text);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
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

void sigHandler(int sig) {
	quit = false;
}

