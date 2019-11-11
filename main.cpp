/*
-----Includes-----
*/
#include <iostream>
#include <chrono>
#include "Cave.h"
#include "ConfigReader.h"

#define WIDTH 640
#define HEIGHT 480

#define MAX_ZOOM 8

int main(int argc, char **argv)
{
	/*
	-----Initilization-----
	*/
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	// Configuration reading
	ConfigReader reader("config.txt");
	short miner_amount = (short) reader.get_config("max_miners", DEFAULT_MAX_MINERS);
	short miner_chance = (short) reader.get_config("miner_chance", DEFAULT_NEW_MINER_CHANCE);
	short cleanups = (short) reader.get_config("cleanups", DEFAULT_CLEANUPS);
	int seed = reader.get_config("seed", 0);
	short air_neighbours = (short) reader.get_config("cleanup_air_max", DEFAULT_AIR_MAX);
	short stone_neighbours = (short) reader.get_config("cleanup_stone_required", DEFAULT_STONE_REQ);
	unsigned char waterfalls = reader.get_config("waterfalls", DEFAULT_WATERFALLS);
	unsigned char pool = reader.get_config("pool_height", DEFAULT_POOL);
	unsigned char grass_intensity = reader.get_config("grass_intensity", DEFAULT_GRASS);

	auto t1 = std::chrono::high_resolution_clock::now();
	// Generation
	Cave cave(HEIGHT / 2, WIDTH / 2, seed);
	cave.generate(miner_amount, miner_chance, cleanups, air_neighbours, stone_neighbours);
	cave.generate_decorations(waterfalls, pool, grass_intensity);
	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	std::cout << "Duration (in milliseconds): " << duration << std::endl;
	cave.gen_image();

	// SDL stuff
	SDL_Window *win = SDL_CreateWindow("Cave", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
									   SDL_WINDOW_OPENGL);
	SDL_Renderer *rend = SDL_CreateRenderer(win, -1, NULL);
	bool quit = false;
	bool mouseHold = false;

	// Blitting rectangle
	SDL_Rect blit;
	blit.h = HEIGHT * 2;
	blit.w = WIDTH * 2;
	blit.x = 0;
	blit.y = 0;

	// Drawing
	SDL_Texture *blitTex = SDL_CreateTextureFromSurface(rend, cave.get_map());
	int map_width;
	int map_height;
	SDL_QueryTexture(blitTex, NULL, NULL, &map_width, &map_height);
	std::cout << "mousewheel up to zoom in, mousewheel down to zoom out, click and drag to move" << std::endl;

	// Main event loop
	while (!quit)
	{
		SDL_Event evt;
		while (SDL_PollEvent(&evt))
		{
			if (evt.type == SDL_QUIT)
			{
				quit = true;
			}

			if (evt.type == SDL_MOUSEBUTTONDOWN)
			{
				mouseHold = true;
			}
			else if (evt.type == SDL_MOUSEBUTTONUP)
			{
				mouseHold = false;
			}

			if (evt.type == SDL_MOUSEWHEEL)
			{
				// 8x zoom at max
				if (evt.wheel.y > 0 && blit.h != map_height / MAX_ZOOM && blit.w != map_width / MAX_ZOOM)
				{
					blit.h /= 2;
					blit.w /= 2;
				}
				if (evt.wheel.y < 0 && blit.h != map_height && blit.w != map_width)
				{
					blit.h *= 2;
					blit.w *= 2;
				}
				// Snap the map at maximum zoom
				if (blit.h == map_height && blit.w == map_width)
				{
					blit.x = 0;
					blit.y = 0;
				}
			}

			if (evt.type == SDL_MOUSEMOTION && mouseHold)
			{
				if ((blit.x - evt.motion.xrel > 0) && (blit.x + blit.w - evt.motion.xrel < map_width))
					blit.x -= evt.motion.xrel;
				if ((blit.y - evt.motion.yrel > 0) && (blit.y + blit.h - evt.motion.yrel < map_height))
					blit.y -= evt.motion.yrel;
			}

			if (evt.type == SDL_KEYUP && evt.key.keysym.sym == SDLK_SPACE)
			{
				continue;
			}
		}
		SDL_RenderClear(rend);
		SDL_RenderCopy(rend, blitTex, &blit, NULL);
		SDL_RenderPresent(rend);
	}
	SDL_DestroyTexture(blitTex);
	//Quit
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
