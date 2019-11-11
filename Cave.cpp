#include "Cave.h"

#include <time.h>
#include <random>
#include <iostream>
#include <vector>

const short border_size = 10;

// Directions in which we consider for cleanup and movement
// Allowing 8-direction movement makes the caves boring
const SDL_Point dirs[4] = {
		{0,  -1},
		{-1, 0},
		{1,  0},
		{0,  1}};

Cave::Cave(unsigned int h, unsigned int w, unsigned int new_seed)
{
	width = w;
	height = h;
	map = NULL;

	seed = new_seed;
	if (seed == 0)
	{
		srand(time(NULL));
		seed = rand();
	}

	tiles = new short *[height];
	for (unsigned int y = 0; y < height; ++y)
	{
		tiles[y] = new short[width];
		for (unsigned int x = 0; x < width; ++x) tiles[y][x] = TILE_STONE;
	}

	std::cout << "Cave ready to be generated!" << std::endl;
};

Cave::~Cave()
{
	destroy();
}

void Cave::generate(short amount, short miner_chance, short cleanups,
					unsigned char air_neighbours, unsigned char stone_neighbours)
{
	std::cout << "Generating..." << std::endl;

	// Miners vector
	std::vector<SDL_Point> miners;
	miners.reserve(amount);

	// Mersenne twister is used here as a random number generator
	std::mt19937 randEngine(seed);
	// Different ranges created
	// Uniform distributions seem to do the work well enough
	std::uniform_int_distribution<short> move_dir(0, 3);
	std::uniform_int_distribution<short> spawn_chance(1, 100);
	std::uniform_int_distribution<short> team_size(1, 10);
	std::uniform_int_distribution<short> spawn_loc(-5, 5);
	// Want to have some variations on the edges for massive caves
	std::uniform_int_distribution<short> edge_offset(1, 5);

	std::cout << "Generation seed: " << seed << std::endl;

	// Spawn initial miners
	miners.resize(team_size(randEngine));
	for (unsigned long c = 0; c < miners.size(); ++c)
	{
		miners[c].x = ceil(width / 2) + spawn_loc(randEngine);
		miners[c].y = ceil(height / 2) + spawn_loc(randEngine);
	}

	// Start working with the miners
	unsigned int miners_generated = miners.size();
	unsigned int c = 0;
	// Generate until we have generated too much miners
	while ((miners_generated <= amount))
	{
		// Reset miner's activity if needed
		bool active = false;
		for (unsigned int c2 = 0; c2 < 4 && !active; ++c2)
		{
			if (tiles[miners[c].y + dirs[c2].y][miners[c].x + dirs[c2].x] == TILE_STONE)
			{
				// Whether the miner is active or not
				// Miners become inactive when they have nothing to dig around them
				active = true;
			}
		}

		// The only miner can't stop moving
		if (miners.size() == 1 || active)
		{
			// Choose a movement direction
			short move = move_dir(randEngine);
			unsigned int moveX = miners[c].x + dirs[move].x;
			unsigned int moveY = miners[c].y + dirs[move].y;

			if ((moveX < width - border_size - edge_offset(randEngine)) &&
				(moveX > border_size + edge_offset(randEngine)))
			{
				if ((moveY < height - border_size - edge_offset(randEngine)) &&
					(moveY > border_size + edge_offset(randEngine)))
				{
					// Last miner will always move somewhere and try to spawn another miner
					if (miners.size() == 1 || tiles[moveY][moveX] == TILE_STONE)
					{
						tiles[moveY][moveX] = TILE_AIR;

						// Try to spawn a new miner
						if (spawn_chance(randEngine) <= miner_chance)
						{
							SDL_Point new_pos = dirs[move_dir(randEngine)];
							SDL_Point old_pos = miners[c];

							// We want to spawn a miner on an empty tile
							while (tiles[old_pos.y + new_pos.y][old_pos.x + new_pos.x] == TILE_STONE)
							{
								new_pos = dirs[move_dir(randEngine)];
							}
							miners.push_back({old_pos.x + new_pos.x, old_pos.y + new_pos.y});
							++miners_generated;
						}
						// Move the old miner
						miners[c].x = moveX;
						miners[c].y = moveY;
					}
				}
			}
		}
		else
			// Remove inactive miners
			miners.erase(miners.begin() + c);
		// Go to the next miner
		++c;
		if (c >= miners.size())
			c = 0;
	}

	std::cout << "Cleaning up..." << std::endl;
	// Cleanup
	for (int i = 0; i < cleanups; ++i)
		clean_up(air_neighbours, stone_neighbours);
	std::cout << "Done!" << std::endl;
};

void Cave::generate_decorations(unsigned char waterfalls, unsigned char pool, unsigned char grass_intensity)
{
	// Step 1: get all edges and the lowest point for the pooling
	// NOTE: y in screen space, so lowest_y is actually highest_y
	unsigned int lowest_y = 0;
	std::vector<SDL_Point> down_edge;
	SDL_Point *cur_falls;
	cur_falls = new SDL_Point[waterfalls];
	for (unsigned int y = 1; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			if (tiles[y][x] == TILE_AIR)
			{
				if (y > lowest_y)
					lowest_y = y;
				if (tiles[y - 1][x] == TILE_STONE)
					down_edge.push_back({(int) x, (int) y});
			}
		}
	}

	// Step 2: generate the pool
	for (unsigned int y = lowest_y; y >= lowest_y - pool; --y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			if (tiles[y][x] == TILE_AIR)
			{
				tiles[y][x] = TILE_WATER;
			}
		}
	}

	// Step 3: generate waterfalls
	std::mt19937 randEngine(seed);
	// Could do something more fancy, but this should be enough
	// Ideally prioritize higher tiles more

	int placed = 0;
	while (placed < waterfalls)
	{
		std::uniform_int_distribution<int> fall_pos(0, down_edge.size() - 1);
		int new_fall_index = fall_pos(randEngine);
		SDL_Point new_fall = down_edge[new_fall_index];
		if (new_fall.y < lowest_y - pool)
		{
			cur_falls[placed] = new_fall;
			++placed;
		}
		down_edge.erase(down_edge.begin() + new_fall_index);
	}

	for (int c = 0; c < waterfalls; ++c)
	{
		spread_waterfall(cur_falls[c], lowest_y - pool);
	}

	delete[] cur_falls;

	// Step 4 - grass, to avoid growing grass in the water
	for (unsigned int y = 1; y < lowest_y - pool; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			if (tiles[y][x] == TILE_AIR && tiles[y + 1][x] == TILE_STONE)
				spread_grass({(int) x, (int) y + 1}, grass_intensity);
		}
	}

}

void Cave::spread_waterfall(SDL_Point start, unsigned int pool)
{
	if (start.y == pool)
		return;
	while (tiles[start.y][start.x] == TILE_AIR)
	{
		tiles[start.y][start.x] = TILE_WATER;
		start.y += 1;
	}
	// We hit a rock, step back
	start.y -= 1;
	// Fun side-effect of comparing against air instead of not rock
	// is that we can see streams getting bigger after merge!
	// Fun Fact: that was an accident
	if (tiles[start.y][start.x - 1] == TILE_AIR)
		// Move left
		spread_waterfall({start.x - 1, start.y}, pool);
	if (tiles[start.y][start.x + 1] == TILE_AIR)
		// Move right
		spread_waterfall({start.x + 1, start.y}, pool);
}

void Cave::spread_grass(SDL_Point start, short intensity)
{
	if (intensity == 0)
		return;
	if (tiles[start.y][start.x] == TILE_STONE
		&& tiles[start.y + 1][start.x] != TILE_WATER && tiles[start.y - 1][start.x] != TILE_WATER
		&& tiles[start.y][start.x + 1] != TILE_WATER && tiles[start.y][start.x - 1] != TILE_WATER)
		tiles[start.y][start.x] = TILE_GRASS;
	// Side effect of not checking for air - greenery can spread to the ceiling. Looks kinda cool!
	spread_grass({start.x + 1, start.y}, intensity - 1);
	spread_grass({start.x - 1, start.y}, intensity - 1);
	spread_grass({start.x, start.y + 1}, intensity - 1);
	spread_grass({start.x, start.y - 1}, intensity - 1);
}

void Cave::clean_up(unsigned char air_neighbours, unsigned char stone_neighbours)
{
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			// How many stone tiles surrounds the current one
			short sur = 0;
			for (short dir = 0; dir < 4; ++dir)
			{
				if ((x + dirs[dir].x > 1) && (x + dirs[dir].x < width - 1))
				{
					if ((y + dirs[dir].y > 1) && (y + dirs[dir].y < height - 1))
					{
						if (tiles[y + dirs[dir].y][x + dirs[dir].x] == TILE_STONE)
						{
							++sur;
						}
					}
					else ++sur;
				}
				else ++sur;
			}
			if ((sur < air_neighbours) && (tiles[y][x] == TILE_STONE))
				tiles[y][x] = TILE_AIR;
			else if ((sur == stone_neighbours) && (tiles[y][x] == TILE_AIR))
				tiles[y][x] = TILE_STONE;
		}
	}
};

void Cave::gen_image()
{
	if (map != NULL) SDL_FreeSurface(map);
	map = SDL_CreateRGBSurface(0, width * 4, height * 4, 32, 0, 0, 0, 0);
	SDL_Rect tile;
	tile.h = 4;
	tile.w = 4;
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			tile.x = x * 4;
			tile.y = y * 4;

			switch (tiles[y][x])
			{
				case TILE_STONE:
					SDL_FillRect(map, &tile, SDL_MapRGB(map->format, 72, 72, 0));
					break;
				case TILE_AIR:
					SDL_FillRect(map, &tile, SDL_MapRGB(map->format, 250, 250, 160));
					break;
				case TILE_WATER:
					SDL_FillRect(map, &tile, SDL_MapRGB(map->format, 0, 0, 255));
					break;
				case TILE_GRASS:
					SDL_FillRect(map, &tile, SDL_MapRGB(map->format, 0, 128, 0));
					break;
				default:
					SDL_FillRect(map, &tile, SDL_MapRGB(map->format, 255, 0, 0));
					break;
			}
			if ((x == 0) || (x == width - 1)) SDL_FillRect(map, &tile, SDL_MapRGB(map->format, 200, 200, 200));
			if ((y == 0) || (y == height - 1)) SDL_FillRect(map, &tile, SDL_MapRGB(map->format, 200, 200, 200));
		}
	}
};

SDL_Surface *Cave::get_map()
{
	return map;
};

void Cave::destroy()
{
	if (map != NULL)
	{
		SDL_FreeSurface(map);
		map = NULL;
	}
	if (tiles != NULL)
	{
		for (unsigned int y = 0; y < height; ++y)
		{
			delete[] tiles[y];
		}
		delete[] tiles;
		tiles = NULL;
	}
};