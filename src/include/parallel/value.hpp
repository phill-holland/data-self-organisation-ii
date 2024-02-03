#include "point.h"

#ifndef _PARALLEL_VALUE
#define _PARALLEL_VALUE

namespace organisation
{    
    namespace parallel
    {        
        class value
        {
        public:
            point position;
            int value;
            int client;

        public:
            bool operator==(const ::organisation::parallel::value &src) const
            {
                return src.position == position &&
                        src.value == value &&
                        src.client == client;
            }
        };
    };
};

#endif