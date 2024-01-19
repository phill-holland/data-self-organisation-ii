#include <CL/sycl.hpp>
#include "parallel/device.hpp"
#include "parallel/queue.hpp"
#include "parameters.h"
#include "schema.h"

#ifndef _PARALLEL_INSERTS
#define _PARALLEL_INSERTS

namespace organisation
{    
    namespace parallel
    {        
        class program;

        class inserts
        {            
            friend class program;

            const static int HOST_BUFFER = 20;

            ::parallel::device *dev;
            ::parallel::queue *queue;

            sycl::float4 *deviceNewPositions;
            int *deviceNewValues;
            int *deviceNewClient;

            int *deviceInputData;
            int *deviceInserts;
            int *deviceInsertsClone;
            int *deviceInsertsIdx;

            int *deviceInputIdx;

            int *deviceNewInserts;
            int *deviceTotalNewInserts;
            int *hostTotalNewInserts;

            int *hostInputData;

            parameters settings;

            int length;

            bool init;

        public:
            inserts(::parallel::device &dev, 
                    ::parallel::queue *q,
                    parameters &settings,
                    int length) 
            { 
                makeNull(); 
                reset(dev, q, settings, length); 
            }
            ~inserts() { cleanup(); }

            bool initalised() { return init; }
            void reset(::parallel::device &dev, 
                       ::parallel::queue *q,
                       parameters &settings,
                       int length);

            void clear();
            void insert(int epoch);

            void set(organisation::data &mappings, inputs::input &source);

        public:
            void copy(::organisation::schema **source, int source_size);

        protected:
            void makeNull();
            void cleanup();
        };
    };
};

#endif