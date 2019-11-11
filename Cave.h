#ifndef CAVE_CLASS
#define CAVE_CLASS

#include <SDL2/SDL.h>

#define TILE_AIR 0
#define TILE_STONE 1
#define TILE_WATER 2
#define TILE_GRASS 3

const short DEFAULT_NEW_MINER_CHANCE = 8;
const short DEFAULT_MAX_MINERS = 400;
const short DEFAULT_CLEANUPS = 3;

const unsigned char DEFAULT_AIR_MAX = 2;
const unsigned char DEFAULT_STONE_REQ = 4;

const unsigned char DEFAULT_WATERFALLS = 4;
const unsigned char DEFAULT_POOL = 20;

const unsigned char DEFAULT_GRASS = 3;

/*
 * Cave generation
 * Based off of http://noelberry.ca/#thecaves
 */
class Cave
{
private:
	// Cave image
	SDL_Surface *map;
	// Size
	unsigned int width;
	unsigned int height;
	// Tiles
	short **tiles;
	unsigned int seed;

	/**
	 * Cleans up the map. Tries to remove some small air pockets in stone and remove long strands of rock
	 * @param air_neighbours  How many air tiles should neighbour a tile at most to turn it to air.
	 *  Tries to remove long strands of stone at some squares.
	 *  Not recommended for change Default: DEFAULT_AIR_MAX
	 * @param stone_neighbours How many stone tiles should an air tileneigbour exactly to become stone
	 *  Tries to remove tiny air pockets in the stone
	 *  Not recommended for change Default: DEFAULT_STONE_REQ
	 */
	void clean_up(unsigned char air_neighbours = DEFAULT_AIR_MAX, unsigned char stone_neighbours = DEFAULT_STONE_REQ);

	void spread_waterfall(SDL_Point start, unsigned int pool);

	void spread_grass(SDL_Point start, short intensity);

public:
	//Ctor
	Cave(unsigned int h, unsigned int w, unsigned int new_seed = 0);

	//Dtor
	~Cave();

	/**
	 * Generates the cave
	 * @param amount 		  Maximum number of miners at the start. Default: DEFAULT_MAX_MINERS
	 * @param miner_chance    Chance of a new miner being created. Default: DEFAULT_NEW_MINER_CHANCE
	 * @param cleanups		  Number of cleanups to perform. Default: DEFAULT_CLEANUPS
	 * @param air_neighbours  How many air tiles should neighbour a tile at most to turn it to air.
	 *  Tries to remove long strands of stone at some squares.
	 *  Not recommended for change Default: DEFAULT_AIR_MAX
	 * @param stone_neighbours How many stone tiles should an air tileneigbour exactly to become stone
	 *  Tries to remove tiny air pockets in the stone
	 *  Not recommended for change Default: DEFAULT_STONE_REQ
	 */
	void generate(short amount = DEFAULT_MAX_MINERS, short miner_chance = DEFAULT_NEW_MINER_CHANCE,
				  short cleanups = DEFAULT_CLEANUPS,
				  unsigned char air_neighbours = DEFAULT_AIR_MAX, unsigned char stone_neighbours = DEFAULT_STONE_REQ);

	/**
	 * Generates waterfall and grass
	 * @param waterfalls 	  number of waterfalls. Default: DEFAULT_WATERFALLS
	 * @param pool			  depth of the pool. Default: DEFAULT_POOL
	 * @param grass_intensity intensity of grass spreading. Default: DEFAULT_GRASS
	 */
	void generate_decorations(unsigned char waterfalls = DEFAULT_WATERFALLS, unsigned char pool = DEFAULT_POOL,
							  unsigned char grass_intensity = DEFAULT_GRASS);

	//Getters
	//Returns the cave image
	SDL_Surface *get_map();

	//Generates the image
	void gen_image();

	//Width
	inline unsigned int get_width()
	{
		return width;
	};

	//Height
	inline unsigned int get_height()
	{
		return height;
	};

	//Destroys
	void destroy();
};

#endif