#include "schema.h"
#include "data.h"
#include "fifo.h"
#include "schemas.h"
#include "parallel/program.hpp"
#include "parameters.h"
#include "region.h"
#include "results.h"
#include <random>
#include <atomic>

#ifndef _ORGANISATION_POPULATION
#define _ORGANISATION_POPULATION

namespace organisation
{
    namespace populations
    {        
        class population
        {
            static const int minimum = 100, maximum = 10000;

            static std::mt19937_64 generator;

            organisation::schemas *schemas;            
            organisation::schema **intermediateA, **intermediateB, **intermediateC;

            parallel::program *programs;

            int dimensions;

            parameters settings;

            bool init;

        public:
            population(parameters &params) { makeNull(); reset(params); }
            ~population() { cleanup(); }

            bool initalised() { return init; }
            void reset(parameters &params);

            void clear();
                    
            organisation::schema go(std::vector<std::string> expected, int &count, int iterations = 0);

        void generate();

        protected:
            bool get(schema &destination, region r);
            bool set(schema &source, region r);
            
        protected:
            schema *best(region r);
            schema *worst(region r);

        protected:
            void pull(organisation::schema **buffer, region r);
            void push(organisation::schema **buffer, region r);

        protected:
            results execute(organisation::schema **buffer, std::vector<std::string> expected);
            
        protected:
            void makeNull();
            void cleanup();
        };
    };
};

#endif