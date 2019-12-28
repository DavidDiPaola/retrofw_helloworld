/* 2019 David DiPaola, licensed CC0 (public domain worldwide) */

#include <stdio.h>

#include <stdint.h>

#include <stdlib.h>  /* for exit() */

#include <SDL.h>

#if defined(PLATFORM_LDK)
#include "ldk.h"
#endif

#define _CLAMP(var, min, max) var = (var < min) ? min : ((var > max) ? max : var)

static const ssize_t screen_width  = 320;
static const ssize_t screen_height = 240;

#if defined(PLATFORM_LINUX)

static const int screen_full = 0;
static const int screen_vsync = 1;

static SDLKey sdlk_up    = SDLK_UP;
static SDLKey sdlk_down  = SDLK_DOWN;
static SDLKey sdlk_left  = SDLK_LEFT;
static SDLKey sdlk_right = SDLK_RIGHT;
static SDLKey sdlk_esc   = SDLK_ESCAPE;

#elif defined(PLATFORM_LDK)

static const int screen_full = 1;
static const int screen_vsync = 1;

static SDLKey sdlk_up    = ldk_sdlk_UP;
static SDLKey sdlk_down  = ldk_sdlk_DOWN;
static SDLKey sdlk_left  = ldk_sdlk_LEFT;
static SDLKey sdlk_right = ldk_sdlk_RIGHT;
static SDLKey sdlk_esc   = ldk_sdlk_POWER;

#else

#error No platform defined

#endif

struct {
	struct {
		SDL_Surface * screen;
	} sdl;
} state;

static void
_close(unsigned int status) {
	SDL_Quit();

	exit(status);
}

static void
_init(void) {
	int status;
	
	status = SDL_Init(SDL_INIT_VIDEO);
	if (status < 0) {
		fprintf(stderr, "ERROR: SDL_Init(): %s" "\n", SDL_GetError());
		_close(1);
	}

	state.sdl.screen = SDL_SetVideoMode(
		screen_width, screen_height,
		/*bpp=*/16,
		SDL_SWSURFACE | (screen_full ? SDL_FULLSCREEN : 0)
	);
	if (!state.sdl.screen) {
		fprintf(stderr, "ERROR: SDL_SetVideoMode(): %s" "\n", SDL_GetError());
		_close(1);
	}

	#if defined(PLATFORM_LDK)
	SDL_ShowCursor(0);
	#endif
}

static void
_surface_from(
	uint16_t * image, size_t image_width, size_t image_height,
	SDL_Surface * * out_surface
) {
	SDL_Surface * surface = SDL_CreateRGBSurfaceFrom(
		image, image_width, image_height,
		/*depth=*/sizeof(*image)*8,
		/*pitch=*/sizeof(*image)*image_width,
		/*Rmask=*/0b11111<<11, /*Gmask=*/0b111111<<5, /*Bmask=*/0b11111<<0, /*Amask=*/0
	);
	if (!surface) {
		fprintf(stderr, "ERROR: SDL_CreateRGBSurfaceFrom(): %s" "\n", SDL_GetError());
		_close(1);
	}

	(*out_surface) = surface;
}

static void
_surface_blit(SDL_Surface * surface, ssize_t x, ssize_t y) {
	SDL_Rect dstrect = { .x=x, .y=y };
	int status = SDL_BlitSurface(
		surface, /*srcrect=*/NULL,
		state.sdl.screen, &dstrect
	);  /* TODO may fail with status=-2. See: https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlblitsurface.html */
	if (status < 0) {
		fprintf(stderr, "ERROR: SDL_BlitSurface(): %s" "\n", SDL_GetError());
		_close(1);
	}
}

static void
_screen_clear(void) {
	int status = SDL_FillRect(state.sdl.screen, /*dstrect=*/NULL, 0x0000);
	if (status < 0) {
		fprintf(stderr, "ERROR: SDL_FillRect(): %s" "\n", SDL_GetError());
		_close(1);
	}
}

static void
_screen_flip(void) {
	int status = SDL_Flip(state.sdl.screen);
	if (status < 0) {
		fprintf(stderr, "ERROR: SDL_Flip(): %s" "\n", SDL_GetError());
		_close(1);
	}
}

#define _w 0xFFFF
#define _b 0x0000
uint16_t ball_image[] = {
	_b, _b, _w, _w, _w, _w, _b, _b,
	_b, _w, _w, _w, _w, _w, _w, _b,
	_w, _w, _w, _w, _w, _w, _w, _w,
	_w, _w, _w, _w, _w, _w, _w, _w,
	_w, _w, _w, _w, _w, _w, _w, _w,
	_w, _w, _w, _w, _w, _w, _w, _w,
	_b, _w, _w, _w, _w, _w, _w, _b,
	_b, _b, _w, _w, _w, _w, _b, _b,
};
#undef _w
#undef _b
const size_t ball_image_width  = 8;
const size_t ball_image_height = 8;

int
main() {
	_init();

	SDL_Surface * ball;
	_surface_from(ball_image, ball_image_width, ball_image_height, &ball);
	ssize_t ball_x = 0;
	ssize_t ball_y = 0;
	ssize_t ball_dx = 0;
	ssize_t ball_dy = 0;

	int quit = 0;
	while (!quit) {
		ball_x += ball_dx;
		_CLAMP(ball_x, 0, screen_width);
		ball_y += ball_dy;
		_CLAMP(ball_y, 0, screen_height);

		_screen_clear();
		_surface_blit(ball, ball_x, ball_y);
		_screen_flip();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				SDLKey key = event.key.keysym.sym;
				if (key == sdlk_esc) {
					quit = 1;
					break;
				}
				else if (key == sdlk_up) {
					ball_dy = -1;
				}
				else if (key == sdlk_down) {
					ball_dy = 1;
				}
				else if (key == sdlk_left) {
					ball_dx = -1;
				}
				else if (key == sdlk_right) {
					ball_dx = 1;
				}
			}
			else if (event.type == SDL_KEYUP) {
				SDLKey key = event.key.keysym.sym;
				if ((key==sdlk_up) || (key==sdlk_down)) {
					ball_dy = 0;
				}
				else if ((key==sdlk_left) || (key==sdlk_right)) {
					ball_dx = 0;
				}
			}
		}
	}

	return 0;
}

