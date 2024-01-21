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
        const static int MAX_COLLISIONS = 26;
        const static int MAX_INSERTS = 10;
        const static int MAX_INPUT_DATA =  15;
    
        const static int HOST_BUFFER = 20;

        const static int ITERATIONS = 20;

    public:
        int max_values;
        int max_movements;
        int max_collisions;
        int max_inserts;
        int max_input_data;

        int host_buffer;

        int iterations;
    
    public:
        int width, height, depth;


        organisation::data mappings;
        inputs::input input;

        int clients;
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
    };
};

#endif