#include "data.h"
#include "input.h"

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
    
    public:
        int max_values;
        int max_movements;
        int max_collisions;
        int max_inserts;
        int max_input_data;

    public:
        int width, height, depth;


        organisation::data mappings;
        inputs::input input;

        int clients;
        int population;

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