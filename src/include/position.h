#include "point.h"
#include "vector.h"

#ifndef _ORGANISATION_POSITION
#define _ORGANISATION_POSITION

namespace organisation
{
    class position
    {
    public:
        point current, next;
        vector direction;
        int value;
        int index;

    public:
        position(int _value = -1, int _index = 0)
        {
            value = _value;
            index = _index;
        }

    public:
        bool isempty() { return value == -1; }
    };
};

#endif