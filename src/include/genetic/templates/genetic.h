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
            virtual ~genetic() { }

        public:
            virtual size_t size() = 0;     
            virtual void clear() = 0;
            virtual void generate(data &source) = 0;
            virtual void mutate(data &source) = 0;
            virtual void copy(genetic *source, int src_start, int src_end, int dest_start) = 0;
            virtual bool validate(data &source) = 0;
        };
    };
};

#endif