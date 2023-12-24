#include "data.h"
#include "vector.h"
#include <string>
#include <vector>
#include <tuple>

#ifndef _ORGANISATION_HISTORY
#define _ORGANISATION_HISTORY

namespace organisation
{    
    class history
    { 
        std::vector<std::tuple<vector, int>> values;

    public:
        void push(vector source, int value);
        std::string get(data &source);

        void append(std::string filename, data &source);
    };
};

#endif