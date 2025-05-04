#include "configLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Function to load configuration settings from a file
std::map<std::string, std::string> loadConfig(const std::string &filename)
{
    std::map<std::string, std::string> config;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open config file: " + filename);
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value))
        {
            config[key] = value;
        }
        else
        {
            throw std::invalid_argument("Invalid format in config file: " + filename);
        }
    }
    return config;
}
