#ifndef CAVEGEN_CONFIGREADER_H
#define CAVEGEN_CONFIGREADER_H

#include <unordered_map>

/*
 * Quick and dirty line-by-line config reader
 * Files are:
 * # comment
 * config = integer
 */
class ConfigReader
{
private:
	std::unordered_map<std::string, int> configs;

public:
	ConfigReader(std::string filename);

	void read(std::string filename);

	int get_config(std::string config, int default_value);
};


#endif //CAVEGEN_CONFIGREADER_H