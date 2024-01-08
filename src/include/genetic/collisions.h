#include "genetic/templates/genetic.h"
#include "genetic/templates/serialiser.h"
#include "data.h"
#include <vector>
#include <random>
#include <string>

#ifndef _ORGANISATION_GENETIC_COLLISIONS
#define _ORGANISATION_GENETIC_COLLISIONS

namespace organisation
{
    namespace genetic
    {
        class collisions : public templates::genetic, public templates::serialiser
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

            std::string serialise();
            void deserialise(std::string source);

        public:
            void copy(const collisions &source);
            bool equals(const collisions &source);
        };
    };
};

#endif