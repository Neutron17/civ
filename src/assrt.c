#include "assrt.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_image.h>

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

