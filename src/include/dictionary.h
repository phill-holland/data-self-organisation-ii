#include <string>
#include <vector>
#include <random>

#ifndef _ORGANISATION_DICTIONARY
#define _ORGANISATION_DICTIONARY

namespace organisation
{    
    class dictionary
    {
        static std::mt19937_64 generator;

        std::vector<std::string> words;

    public:
        dictionary();

        std::vector<std::string> get() const { return words; }
        std::string random(int length = 0, std::vector<std::string> excluded= {}) const;
    };
};

#endif