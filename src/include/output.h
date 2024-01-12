#include <vector>
#include <string>

#ifndef _ORGANISATION_OUTPUT
#define _ORGANISATION_OUTPUT

namespace organisation
{
    class output
    {
    public:
        std::vector<std::string> values;

    public:
        void clear()
        {
            values.clear();
        }
    };
};


#endif