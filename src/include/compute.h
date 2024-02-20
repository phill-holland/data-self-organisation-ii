#include "statistics.h"
#include <string>

#ifndef _ORGANISATION_COMPUTE
#define _ORGANISATION_COMPUTE

namespace organisation
{
    class compute
    {
    public:
        std::string expected, value;
        statistics::data stats;

    public:
        compute(std::string _expected = "", std::string _value = "", statistics::data _statistics = { }) 
        { 
            expected = _expected;
            value = _value;
            stats = _statistics;
        }
    };
};

#endif