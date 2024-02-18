#include <vector>
#include <string>

#ifndef _ORGANISATION_STATISTICS
#define _ORGANISATION_STATISTICS

namespace organisation
{
    namespace statistics
    {
        class statistic
        {
        public:
            int collisions;
            int epoch;
            int client;

        public:
            statistic(int _collisions = 0, int _epoch = 0, int _client = 0)
            {
                collisions = _collisions;
                epoch = _epoch;
                client = _client;
            }
        };
    }; 
};


#endif