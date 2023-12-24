#include "math.h"

#ifndef _ORGANISATION_VECTOR
#define _ORGANISATION_VECTOR

namespace organisation
{    
    class vector
    {  
    public:
        int x,y,z,w;

    public:
        vector() { x = y = z = 0; w = 1; }
        vector(int a, int b, int c) { x = a; y = b; z = c; w = 1; }
        vector(int a, int b, int c, int d) { x = a; y = b; z = c; w = d; }

        vector inverse()
        {
            return { x * -1, y * -1, z * -1, w };
        }

        vector normalise()
        {
            return { x / w, y / w, z / w, w };
        }

        bool decode(int index)
        {
            if ((index >= 0) && (index <= 27))
            {
                div_t r = div(index, 9);
                z = (float)r.quot - 1;

                div_t j = div(r.rem, 3);
                y = (float)j.quot - 1;
                x = (float)j.rem - 1;
                w = 0.0f;

                return true;
            }

            return false;
        }

        int encode()
        {
            int tx = (int)roundf(x) + 1;
            int ty = (int)roundf(y) + 1;
            int tz = (int)roundf(z) + 1;

            if ((tx < 0) || (tx > 2)) return 0;
            if ((ty < 0) || (ty > 2)) return 0;
            if ((tz < 0) || (tz > 2)) return 0;

            return (abs(tz) * (3 * 3)) + (abs(ty) * 3) + abs(tx);
        }
    };
};

#endif


/*
auto is_dominant = [](int *a, int *b) 
	{ 
		const int dimensions = 2;
		bool any = false;
		for (int i = 0; i < dimensions; ++i)
		{
			//Log << a[i];
			// modify kdtree for this condition, between left and right insert
			if(a[i] > b[i]) return false;
			any |= (a[i] < b[i]);
			//if((*epochs)[i]->results.score > (*source.epochs)[i]->results.score) return false;
			//any |= ((*epochs)[i]->results.score < (*source.epochs)[i]->results.score);
		}

		return any;
	};

*/