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
            static const int LENGTH = 26;

            static std::mt19937_64 generator;

        public:
            std::vector<int> values;

        public:
            collisions() { values.resize(LENGTH); }

            size_t size() { return values.size(); }

            void clear() 
            {
                for(auto &it: values)
                {
                    it = 0;
                }
            }

            bool empty() { return false; }

            void generate(data &source);
            bool mutate(data &source);
            void append(genetic *source, int src_start, int src_end);
            
            std::string serialise();
            void deserialise(std::string source);

            bool validate(data &source);

        public:
            void copy(const collisions &source);
            bool equals(const collisions &source);
        };
    };
};

#endif