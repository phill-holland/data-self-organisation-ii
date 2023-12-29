#include "genetic/templates/genetic.h"
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

        public:
            std::vector<organisation::vector> directions;

    public:
            size_t size() { return directions.size(); }

            void clear() 
            {
                directions.clear();
            }

            int get()
            {
                #wanring manage end pointer to next direction
            }

        public:
            void generate(data &source);
            void mutate(data &source);
            void copy(genetic *source, int src_start, int src_end, int dest_start);
        };
    };
};

#endif