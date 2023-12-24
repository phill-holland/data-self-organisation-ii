#include "data.h"
#include "parallel/program.hpp"

#ifndef _ORGANISATION_POPULATION_PARAMETERS
#define _ORGANISATION_POPULATION_PARAMETERS

namespace organisation
{
    namespace populations
    {        
        class parameters
        {
        public:
            ::parallel::device *dev;
            ::parallel::queue *q;
            
            parallel::parameters params;
        
            organisation::data mappings;
            std::vector<std::string> expected;

            int clients;
            int size;
        };
    };
};

#endif