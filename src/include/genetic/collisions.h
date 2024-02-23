#include "genetic/templates/genetic.h"
#include "genetic/templates/serialiser.h"
#include "data.h"
#include "parameters.h"
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

            std::vector<int> values;

            int length;

        public:
            collisions(parameters &settings) 
            {  
                length = settings.max_collisions * settings.mappings.maximum();
                values.resize(length);
            }

            size_t size() 
            { 
                return values.size(); 
            }

            void clear() 
            {
                for(auto &it:values)
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
            bool get(int &result, int idx);
            bool set(int source, int idx);

        public:
            void copy(const collisions &source);
            bool equals(const collisions &source);
        };
    };
};

#endif