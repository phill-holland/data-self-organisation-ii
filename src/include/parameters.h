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

        //const static int EPOCHS = 1;

    public:
        //::parallel::device *dev;
        //::parallel::queue *q;
        
        //parallel::parameters params;
        int width, height, depth;
        //int length;

        //int epochs;

        organisation::data mappings;
        inputs::input input;

        int clients;
        int size;

    public:
        parameters(int _width = WIDTH, int _height = HEIGHT, int _depth = DEPTH) 
        {
            width = _width;
            height = _height;
            depth = _depth;
            
            //length = _width * _height * _depth;

            //epochs = EPOCHS;
        }            

        int length()
        { 
            return (width * height * depth);
        }
    };
};

#endif