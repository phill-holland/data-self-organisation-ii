#include <random>

#ifndef _ORGANISATION_POINT
#define _ORGANISATION_POINT

namespace organisation
{
    class point 
    {
        static std::mt19937_64 generator;

    public:    
        int x,y,z;

    public:
        point(int _x = 0, int _y = 0, int _z = 0)        
        {
            x = _x;
            y = _y;
            z = _z;
        }    

    public:
        void generate(int max_x, int max_y, int max_z, int min_x = 0, int min_y = 0, int min_z = 0);
    };
};

#endif