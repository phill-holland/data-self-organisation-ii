#include <CL/sycl.hpp>
#include "parallel/device.hpp"
#include "parallel/queue.hpp"
#include "templates/programs.h"
#include "parallel/inserts.hpp"
#include "parallel/map.hpp"
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

            sycl::float4 *devicePositions;
            sycl::float4 *deviceNextPositions;
            sycl::float4 *deviceNextHalfPositions;
            int *deviceValues;

            int *deviceMovementIdx;            
            int *deviceClient;

            sycl::float4 *deviceMovements;
            sycl::float4 *deviceCollisions;

            sycl::float4 *hostPositions;
            int *hostValues;
            sycl::float4 *hostMovements;
            sycl::float4 *hostCollisions;
            int *hostClient;
            
            map *impacter;
            inserts *inserter;

            parameters settings;

            int totalValues;

            bool init;
            
        public:
            program(::parallel::device &dev, ::parallel::queue *q, parameters settings) 
            { 
                makeNull(); 
                reset(dev, q, settings); 
            }
            ~program() { cleanup(); }

            bool initalised() { return init; }
            void reset(::parallel::device &dev, ::parallel::queue *q, parameters settings);

            void clear();

            void run(organisation::data &mappings);        
            void set(organisation::data &mappings, inputs::input &source);
            std::vector<output> get(organisation::data &mappings);

        protected:
            void update();

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