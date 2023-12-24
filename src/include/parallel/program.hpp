#include <CL/sycl.hpp>
#include "parallel/device.hpp"
#include "parallel/queue.hpp"
#include "program.h"
#include "schema.h"
#include "data.h"

#ifndef _PARALLEL_PROGRAM
#define _PARALLEL_PROGRAM

namespace organisation
{    
    namespace parallel
    {
        class parameters
        {
            const static int WIDTH = 5;
            const static int HEIGHT = 5;
            const static int DEPTH = 5;

            const static int IN = 3;
            const static int OUT = 3;

            const static int EPOCHS = 1;

            int length;

        public:
            int width, height, depth;
            int in, out;
            int epochs;

        public:
            parameters(int _width = WIDTH, int _height = HEIGHT, int _depth = DEPTH, 
                       int _in = IN, int _out = OUT, int _epochs = EPOCHS) 
            {
                width = _width;
                height = _height;
                depth = _depth;
                
                in = _in;
                out = _out;

                length = _width * _height * _depth;

                epochs = EPOCHS;
            }            

            int size() { return length; }
        };

        class output
        {
        public:
            std::vector<std::string> values;
        };
        
        class program
        {
            ::parallel::device *dev;

            int *deviceValues;
            //int *deviceInGates;
            //int *deviceOutGates;
            //int *deviceMagnitudes;

            int *hostValues;
            //int *hostInGates;
            //int *hostOutGates;
            //int *hostMagnitudes;

            int *deviceOutput;
            int *deviceOutputIteration;
            int *deviceOutputEpoch;
            int *deviceOutputEndPtr;

            int *hostOutput;
            int *hostOutputIteration;
            int *hostOutputEpoch;
            int *hostOutputEndPtr;

            sycl::float4 *deviceReadPositionsA;
            sycl::float4 *deviceReadPositionsB;
            int *deviceReadPositionsEpochA;
            int *deviceReadPositionsEpochB;
            int *deviceReadPositionsEndPtr;

            sycl::float4 *hostSourceReadPositions;
            sycl::float4 *deviceSourceReadPositions;

            parameters params;

            int clients;
            int length;

            bool init;

        private:
            const static int ITERATIONS = 20;
            const static int HOST_BUFFER = 20;
            
        public:
            program(::parallel::device &dev, parameters settings, int clients) { makeNull(); reset(dev, settings, clients); }
            ~program() { cleanup(); }

            bool initalised() { return init; }
            void reset(::parallel::device &dev, parameters settings, int clients);

            void clear(::parallel::queue *q = NULL);

            void run(::parallel::queue *q = NULL);        
            void set(std::vector<sycl::float4> positions, ::parallel::queue *q = NULL);
            std::vector<output> get(organisation::data &mappings, ::parallel::queue *q = NULL);

        public:
            void copy(::organisation::schema **source, int source_size, ::parallel::queue *q = NULL);
            
        public:
            void outputarb(int *source, int length);
            void outputarb(sycl::float4 *source, int length);

        protected:
            void makeNull();
            void cleanup();
        };
    };
};

#endif