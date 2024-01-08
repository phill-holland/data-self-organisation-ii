#include "genetic/templates/genetic.h"
#include "genetic/templates/serialiser.h"
#include "data.h"
#include <vector>
#include <random>
#include <string>

#ifndef _ORGANISATION_GENETIC_INSERT
#define _ORGANISATION_GENETIC_INSERT

namespace organisation
{
    namespace genetic
    {
        class insert : public templates::genetic, public templates::serialiser
        {
            static std::mt19937_64 generator;

            const static int LENGTH = 15;
            const static int MIN = 1;
            const static int MAX = 5;
            
            int current;
            int counter;

        public:
            std::vector<int> values;

        public:
            insert() { clear(); }

        public:
            size_t size() { return values.size(); }

            void clear() 
            {
                values.clear();
                
                current = 0;
                counter = 0;
            }

            void start();
            bool get();

            void generate(data &source);
            void mutate(data &source);
            void copy(genetic *source, int src_start, int src_end, int dest_start);

            std::string serialise();
            void deserialise(std::string source);

        public:
            void copy(const insert &source);
            bool equals(const insert &source);
        };
    };
};

#endif