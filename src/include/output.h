#include <vector>
#include <string>
#include "point.h"

#ifndef _ORGANISATION_OUTPUT
#define _ORGANISATION_OUTPUT

namespace organisation
{
    namespace outputs
    {
        class data
        {
        public:
            std::string value;
            int client;
            int index;
            point position;

        public:
            data(std::string _value = "", int _client = 0, int _index = 0, point _position = point(0,0,0))
            {
                value = _value;
                client = _client;
                index = _index;
                position = _position;
            }

        public:
            bool operator==(const data &src) const
            {
                return value == src.value && client == src.client && index == src.index && position == src.position;
            }
        };

        class output
        {
        public:
            std::vector<data> values;

        public:
            void clear()
            {
                values.clear();
            }

        public:
            bool operator==(const output &src) const
            {
                return values == src.values;
            }
        };
    };
};


#endif