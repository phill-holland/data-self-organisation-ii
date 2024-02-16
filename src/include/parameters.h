#include "data.h"
#include "input.h"
#include "point.h"

#ifndef _ORGANISATION_PARAMETERS
#define _ORGANISATION_PARAMETERS

namespace organisation
{
    class parameters
    {
        const static int WIDTH = 5;
        const static int HEIGHT = 5;
        const static int DEPTH = 5;

        const static int MAX_VALUES = 20;
        const static int MAX_MOVEMENTS = 30;
        const static int MAX_COLLISIONS = 27;
        const static int MAX_INSERTS = 10;
        const static int MAX_INPUT_DATA =  15;
    
        const static int HOST_BUFFER = 20;

        const static int ITERATIONS = 20;

        const static bool BEST = true;
        const static bool WORST = true;

        const static bool OUTPUT_STATIONARY_ONLY = false;

    public:
        int max_values;
        int max_movements;
        int max_collisions;
        int max_inserts;
        int max_input_data;

        int host_buffer;

        int iterations;
    
        bool best;
        bool worst;

        bool output_stationary_only;

    public:
        int width, height, depth;

        organisation::data mappings;
        inputs::input input;

        point dim_clients;
        int population;

        point starting;

    public:
        parameters(int _width = WIDTH, int _height = HEIGHT, int _depth = DEPTH) 
        {
            width = _width;
            height = _height;
            depth = _depth;

            max_values = MAX_VALUES;
            max_movements = MAX_MOVEMENTS;
            max_collisions = MAX_COLLISIONS;
            max_inserts = MAX_INSERTS;
            max_input_data = MAX_INPUT_DATA;
        
            host_buffer = HOST_BUFFER;

            iterations = ITERATIONS;

            best = BEST;
            worst = WORST;

            output_stationary_only = OUTPUT_STATIONARY_ONLY;
                
            starting.x = width / 2;
            starting.y = height / 2;
            starting.z = depth / 2;
        }            

        int length()
        { 
            return (width * height * depth);
        }

        int epochs() 
        {
            return input.size();
        }

        int clients()
        {
            return dim_clients.x * dim_clients.y * dim_clients.z;
        }
    };
};

#endif