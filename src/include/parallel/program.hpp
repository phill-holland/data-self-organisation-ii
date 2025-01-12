#include <CL/sycl.hpp>
#include "parallel/device.hpp"
#include "parallel/queue.hpp"
#include "templates/programs.h"
#include "parallel/collisions.hpp"
#include "parallel/inserts.hpp"
#include "parallel/map/map.hpp"
#include "parallel/map/configuration.hpp"
#include "parallel/value.hpp"
#include "parameters.h"
#include "program.h"
#include "schema.h"
#include "data.h"
#include "output.h"
#include "input.h"
#include "statistics.h"

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
            sycl::int4 *deviceValues; // ***
            sycl::float4 *deviceNextDirections;

            int *deviceMovementIdx;   
            int *deviceLifetime;         
            sycl::int4 *deviceClient;

            sycl::float4 *deviceCachePositions;
            sycl::int4 *deviceCacheValues; // ***
            sycl::int4 *deviceCacheClients;

            sycl::float4 *deviceMovements;
            int *deviceMovementsCounts;
            int *deviceCollisionCounts;
            int *hostCollisionCounts;
            
            sycl::int2 *deviceNextCollisionKeys;
            sycl::int2 *deviceCurrentCollisionKeys;
            sycl::int2 *deviceCorrectionCollisionKeys;
            
            sycl::float4 *hostCachePositions;
            sycl::int4 *hostCacheValues; // ***
            sycl::int4 *hostCacheClients;

            sycl::float4 *hostMovements;
            int *hostMovementsCounts;
                        
            // ***

            sycl::int4 *deviceOutputValues; // ***
            int *deviceOutputIndex;
            sycl::int4 *deviceOutputClient;

            int *deviceOutputTotalValues; // single int
            
            sycl::int4 *hostOutputValues; // ***
            int *hostOutputIndex; // host iteration output
            sycl::int4 *hostOutputClient;

            int *hostOutputTotalValues;

            // ***
            int *deviceTotalValues;            
            int *hostTotalValues;
            // ***
                        
            sycl::float4 *deviceNewPositions;
            sycl::int4 *deviceNewValues; // ***
            sycl::int4 *deviceNewClient;
            sycl::float4 *deviceNewNextDirections;
            int *deviceNewMovementIdx;
            // ***

            sycl::float4 *deviceOldPositions;
            int *deviceOldUpdateCounter;
            int *hostOldUpdateCounter;
            // ***

            ::parallel::mapper::map *impacter;
            
            collisions *collision;
            inserts *inserter;

            parameters settings;

            int totalOutputValues;
            int totalValues;

            std::vector<organisation::outputs::output> outputs;

            bool init;
            
        public:
            program(::parallel::device &dev, 
                    ::parallel::queue *q, 
                    ::parallel::mapper::configuration &mapping,
                    parameters settings) 
            { 
                makeNull(); 
                reset(dev, q, mapping, settings); 
            }
            ~program() { cleanup(); }

            bool initalised() { return init; }
            void reset(::parallel::device &dev, 
                       ::parallel::queue *q,                        
                       ::parallel::mapper::configuration &mapping,
                       parameters settings);

            void clear();

            void run(organisation::data &mappings);        
            void set(organisation::data &mappings, inputs::input &source);
            std::vector<outputs::output> get(organisation::data &mappings);
            std::vector<organisation::statistics::statistic> statistics();

        protected:     
            void move(organisation::data &mappings);       
            void update();
            void positions();
            void next();
            void insert(int epoch, int iteration);
            void boundaries();
            void corrections(bool debug = false);
            void outputting(int epoch, int iteration);
            void restart();            

        public:
            std::vector<value> get();

        public:
            void copy(::organisation::schema **source, int source_size);
            
        protected:
            void debug();

        protected:
            void outputarb(int *source, int length);
            void outputarb(sycl::int2 *source, int length);
            void outputarb(sycl::int4 *source, int length);
            void outputarb(sycl::float4 *source, int length);
        
        protected:
            void makeNull();
            void cleanup();
        };
    };
};

#endif