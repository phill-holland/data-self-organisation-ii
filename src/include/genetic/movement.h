#include "point.h"
#include "vector.h"
#include <vector>
#include <random>

#ifndef _ORGANISATION_GENETIC_MOVEMENT
#define _ORGANISATION_GENETIC_MOVEMENT

namespace organisation
{
    namespace genetic
    {
        class movement
        {
            static std::mt19937_64 generator;

        public:
            point starting;
            std::vector<organisation::vector> directions;

        public:
            void generate(int width, int height, int depth);
            void mutate(int width, int height, int depth);
        };
    };
};

#endif