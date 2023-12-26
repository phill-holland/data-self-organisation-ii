#include "genetic/templates/generic.h"
#include "point.h"
#include <vector>
#include <unordered_map>
#include <random>

#ifndef _ORGANISATION_GENETIC_CACHE
#define _ORGANISATION_GENETIC_CACHE

namespace organisation
{
    namespace genetic
    {
        class cache : public templates::genetic
        {
            static std::mt19937_64 generator;

            int _width, _height, _depth;

        public:
            std::vector<std::tuple<int,point>> values;
            std::unordered_map<int,point> points;

        public:
            cache(int w, int h, int d) 
            { 
                _width = w; _height = h; _depth = d; 
            }

        public:
            size_t size() { return values.size(); }
            void clear() { values.clear(); }
            void generate(data &source);
            void mutate(data &source, int offset);
        };
    };
};

#endif