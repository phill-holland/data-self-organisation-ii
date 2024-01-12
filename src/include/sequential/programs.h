#include "templates/programs.h"
#include "program.h"
#include "schema.h"
#include "data.h"
#include "output.h"
#include "input.h"

#ifndef _ORGANISATION_SEQUENTIAL_PROGRAM
#define _ORGANISATION_SEQUENTIAL_PROGRAM

namespace organisation
{    
    namespace sequential
    {
        class parameters
        {
            const static int WIDTH = 5;
            const static int HEIGHT = 5;
            const static int DEPTH = 5;

            const static int EPOCHS = 1;

            int length;

        public:
            int width, height, depth;
            int epochs;

        public:
            parameters(int _width = WIDTH, int _height = HEIGHT, int _depth = DEPTH, int _epochs = EPOCHS) 
            {
                width = _width;
                height = _height;
                depth = _depth;
                
                length = _width * _height * _depth;

                epochs = EPOCHS;
            }            

            int size() { return length; }
        };
    
        class programs : public templates::programs
        {
            parameters params;

            ::organisation::schema **schemas;
            ::organisation::output *results;
            ::organisation::inputs::input *inputs;

            int clients;
            
            bool init;

        private:
            const static int ITERATIONS = 20;
            const static int HOST_BUFFER = 20;
            
        public:
            programs(parameters settings, int clients) { makeNull(); reset(settings, clients); }
            ~programs() { cleanup(); }

            bool initalised() { return init; }
            void reset(parameters settings, int clients);

            void clear();

            void run(organisation::data &mappings);        

            void set(std::vector<inputs::input> &source);
            std::vector<output> get(organisation::data &mappings);

        public:
            void copy(::organisation::schema **source, int source_size);
            
        protected:
            void makeNull();
            void cleanup();
        };
    };
};

#endif