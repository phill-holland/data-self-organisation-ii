#include "genetic/templates/generic.h"
#include "genetic/movement.h"
#include "data.h"
#include <vector>
#include <random>

#ifndef _ORGANISATION_GENETIC_MOVEMENTS
#define _ORGANISATION_GENETIC_GMOVEMENTS

namespace organisation
{
    namespace genetic
    {
        class movements : public templates::genetic
        {
            static std::mt19937_64 generator;

            int _width, _height, _depth;

        public:
            std::vector<movement> values;

        public:
            movements(int w, int h, int d) 
            { 
                _width = w; _height = h; _depth = d; 
            }

        public:
            size_t size();

            void clear()
            {
                values.clear();
            }

            void generate(data &source);
            void mutate(data &source);
            void copy(genetic *source, int src_start, int src_end, int dest_start);
        };
    };
};

#endif