#include "statistics.h"
#include "point.h"
#include <string>
#include <vector>
#include <tuple>

#ifndef _ORGANISATION_COMPUTE
#define _ORGANISATION_COMPUTE

namespace organisation
{
    /*
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
    */

    class compute
    {
    public:
        std::vector<std::tuple<std::string, point>> value;
        std::vector<std::string> expected;
        statistics::data stats;
    };
};

#endif