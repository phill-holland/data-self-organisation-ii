#include "genetic/templates/genetic.h"
#include "genetic/templates/serialiser.h"
#include "point.h"
#include "parameters.h"
#include <vector>
#include <unordered_map>
#include <random>

#ifndef _ORGANISATION_GENETIC_CACHE
#define _ORGANISATION_GENETIC_CACHE

namespace organisation
{
    namespace genetic
    {
        class cache : public templates::genetic, public templates::serialiser
        {
            static std::mt19937_64 generator;

            int _width, _height, _depth;

        public:
            std::vector<std::tuple<int,point>> values;
            std::unordered_map<int,point> points;

        public:
            cache(parameters &settings) 
            { 
                //_width = w; _height = h; _depth = d; 
                _width = settings.width;
                _height = settings.height;
                _depth = settings.depth;
            }

        public:
            size_t size() { return values.size(); }
            void clear() 
            { 
                values.clear(); 
                points.clear();
            }

            bool empty()
            {
                return values.empty() || points.empty();
            }

            bool set(int value, point position);

            std::string serialise();
            void deserialise(std::string source);

            bool validate(data &source);

        public:
            void generate(data &source);
            bool mutate(data &source);
            void append(genetic *source, int src_start, int src_end);

        public:
            void copy(const cache &source);
            bool equals(const cache &source);
        };
    };
};

#endif