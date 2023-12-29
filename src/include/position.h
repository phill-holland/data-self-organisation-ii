#include "point.h"

#ifndef _ORGANISATION_POSITION
#define _ORGANISATION_POSITION

namespace organisation
{
    class position
    {
    public:
        point pos;
        int index;

    public:
        position(point p = {0,0,0}, int idx = 0)
        {
            pos = p;
            index = idx;
        }
    };
};

#endif