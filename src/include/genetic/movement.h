#include "genetic/templates/generic.h"
#include "point.h"
#include "vector.h"
#include "data.h"
#include <vector>
#include <random>

#ifndef _ORGANISATION_GENETIC_MOVEMENT
#define _ORGANISATION_GENETIC_MOVEMENT

namespace organisation
{
    namespace genetic
    {
        class movement : public templates::genetic
        {
            static std::mt19937_64 generator;

            int _width, _height, _depth;

        public:
            point starting;
            std::vector<organisation::vector> directions;

        public:
            movement(int width, int height, int depth)
            {
                _width = width;
                _height = height;
                _depth = depth;
            }

            size_t size() { return directions.size(); }

            void clear() 
            {
                directions.clear();
            }

        public:
            void generate(data &source);
            void mutate(data &source);
            void copy(genetic *source, int src_start, int src_end, int dest_start);
        };
    };
};

#endif