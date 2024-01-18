#include <CL/sycl.hpp>
#include "parallel/device.hpp"
#include "parallel/queue.hpp"

#ifndef _PARALLEL_INSERTS
#define _PARALLEL_INSERTS

namespace organisation
{    
    namespace parallel
    {        
        class inserts
        {
            ::parallel::device *dev;
            ::parallel::queue *queue;

            sycl::float4 *deviceNewPositions;
            int *deviceNewValues;
            int *deviceNewClient;

            int *deviceInputData;
            int *deviceInserts;
            int *deviceInsertsClone;
            int *deviceInsertsIdx;

            int *deviceNewInserts;
            int *deviceTotalNewInserts;
            int *hostTotalNewInserts;

            int *deviceNewInserts;
            int *deviceTotalNewInserts;
            int *hostTotalNewInserts;

            bool init;

        public:
            inserts(::parallel::device &dev, ::parallel::queue *q) { makeNull(); reset(dev, q); }
            ~inserts() { cleanup(); }

            bool initalised() { return init; }
            void reset(::parallel::device &dev, ::parallel::queue *q);

            void clear();
            void insert();

        public:
            void copy(::organisation::schema **source, int source_size);

        protected:
            void makeNull();
            void cleanup();
        }
    };
};