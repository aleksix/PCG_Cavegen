#include "ConfigReader.h"

#include <fstream>
#include <algorithm>

ConfigReader::ConfigReader(std::string filename)
{
	read(filename);
}

void ConfigReader::read(std::string filename)
{
	// Taken from https://www.walletfox.com/course/parseconfigfile.php
	std::ifstream config(filename);

	if (config.is_open())
	{
		std::string line;
		while (getline(config, line))
		{
			line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
			if (line[0] == '#' || line.empty())
				continue;
			int delimiterPos = line.find("=");
			std::string name = line.substr(0, delimiterPos);
			int value = std::stoi(line.substr(delimiterPos + 1));
			configs[name] = value;
		}
		config.close();
	}
}

int ConfigReader::get_config(std::string config, int default_value)
{
	if (configs.find(config) == configs.end())
		return default_value;
	return configs.at(config);
}