#include "data.h"
#include <stddef.h>

#ifndef _ORGANISATION_GENETIC_TEMPLATES_GENETIC
#define _ORGANISATION_GENETIC_TEMPLATES_GENETIC

namespace organisation
{
    namespace templates
    {
        class genetic
        {
        public:
            virtual size_t size() = 0;     
            virtual void clear() = 0;
            virtual void generate(data &source) = 0;
            virtual void mutate(data &source, int offset) = 0;
            //virtual void copy(genetic *dest, genetic *source, int start, int end) = 0;       
        };
    };
};

#endif