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
            size_t size() 
            { 
                int total = 0;
                for(auto &it: values)
                {
                    total += it.directions.size() + 1;
                }

                return total;
            }

            void clear()
            {
                values.clear();
            }

            void generate(data &source);
            void mutate(data &source, int offset);
        };
    };
};

#endif