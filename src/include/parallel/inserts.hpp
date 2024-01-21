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

        class value
        {
        public:
            point position;
            int value;
            int client;
        };

        class inserts
        {            
            friend class program;

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

            int *deviceTotalNewInserts;
            int *hostTotalNewInserts;

            int *hostInputData;
            int *hostInserts;

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
            int insert(int epoch);

            void set(organisation::data &mappings, inputs::input &source);
            std::vector<value> get();

        public:
            void copy(::organisation::schema **source, int source_size);

        protected:
            std::vector<int> get(int *source, int length);
            std::vector<sycl::float4> get(sycl::float4 *source, int length);

        protected:
            void makeNull();
            void cleanup();
        };
    };
};

#endif