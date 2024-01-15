#include <CL/sycl.hpp>
#include "parallel/device.hpp"
#include "parallel/queue.hpp"
#include "templates/programs.h"
#include "parameters.h"
#include "program.h"
#include "schema.h"
#include "data.h"
#include "output.h"
#include "input.h"

#ifndef _PARALLEL_PROGRAM
#define _PARALLEL_PROGRAM

namespace organisation
{    
    namespace parallel
    {        
        class program : public templates::programs
        {
            ::parallel::device *dev;
            ::parallel::queue *queue;

            // ***
            sycl::float4 *devicePositions;
            int *deviceValues;
            int *deviceMovement;
            int *deviceClient;

            sycl::float4 *deviceMovements;
            sycl::float4 *deviceCollisions;
            // ***
            

            sycl::float4 *hostPositions;
            int *hostValues;
            int *hostMovement;
            int *hostClient;

            /*
            int *deviceOutput;
            int *deviceOutputIteration;
            //int *deviceOutputEpoch;
            int *deviceOutputEndPtr;

            int *hostOutput;
            int *hostOutputIteration;
            //int *hostOutputEpoch;
            int *hostOutputEndPtr;

            sycl::float4 *deviceReadPositionsA;
            sycl::float4 *deviceReadPositionsB;
            //int *deviceReadPositionsEpochA;
            //int *deviceReadPositionsEpochB;
            int *deviceReadPositionsEndPtr;

            sycl::float4 *hostSourceReadPositions;
            sycl::float4 *deviceSourceReadPositions;
            */
            parameters params;

            int clients;
            int length;

            bool init;

        private:
            const static int ITERATIONS = 20;
            const static int HOST_BUFFER = 20;
            
        public:
            program(::parallel::device &dev, ::parallel::queue *q, parameters settings, int clients) 
            { 
                makeNull(); 
                reset(dev, q, settings, clients); 
            }
            ~program() { cleanup(); }

            bool initalised() { return init; }
            void reset(::parallel::device &dev, ::parallel::queue *q, parameters settings, int clients);

            void clear();

            void run(organisation::data &mappings);        
            void set(inputs::input &source);
            std::vector<output> get(organisation::data &mappings);

        public:
            void copy(::organisation::schema **source, int source_size);
            
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