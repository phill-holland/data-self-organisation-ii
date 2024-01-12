#include <vector>
#include <string>

#ifndef _ORGANISATION_INPUT
#define _ORGANISATION_INPUT

namespace organisation
{
    namespace inputs
    {
        class epoch
        {
        public:
            std::string input;
            //std::string expected;
        };

        class input
        {
        public:
            std::vector<epoch> values;

        public:
            void clear()
            {
                values.clear();
            }
        };
    };
};


#endif