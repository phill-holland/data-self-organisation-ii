#include <vector>
#include <string>

#ifndef _ORGANISATION_STATISTICS
#define _ORGANISATION_STATISTICS

namespace organisation
{
    namespace statistics
    {
        class data
        {
        public:
            int collisions;
            int client;

        public:
            data(int _collisions = 0, int _client = 0)
            {
                collisions = _collisions;
                client = _client;
            }
        };

        class statistics
        {
        public:
            std::vector<data> values;

        public:
            void clear()
            {
                values.clear();
            }
        };   
    }; 
};


#endif