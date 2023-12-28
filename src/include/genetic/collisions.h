#include "genetic/templates/genetic.h"
#include "data.h"
#include <vector>
#include <random>

#ifndef _ORGANISATION_GENETIC_COLLISIONS
#define _ORGANISATION_GENETIC_COLLISIONS

namespace organisation
{
    namespace genetic
    {
        class collisions : public templates::genetic
        {
            static std::mt19937_64 generator;

        public:
            std::vector<int> values;

        public:
            size_t size() { return values.size(); }

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