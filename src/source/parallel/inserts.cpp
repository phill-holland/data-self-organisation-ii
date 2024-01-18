#include "parallel/inserts.hpp"

void organisation::parallel::inserts::reset(::parallel::device &dev, ::parallel::queue *q)
{
    init = false; cleanup();

    this->dev = dev;
    this->queue = q;

    deviceNewPositions = sycl::malloc_device<sycl::float4>(MAX_VALUES * clients, qt);
    if(deviceNewPositions == NULL) return;

    deviceNewValues = sycl::malloc_device<int>(MAX_VALUES * clients, qt);
    if(deviceNewValues == NULL) return;

    deviceNewClient = sycl::malloc_device<int>(MAX_VALUES * clients, qt);
    if(deviceNewClient == NULL) return;

    deviceInserts = sycl::malloc_device<int>(MAX_INSERTS * clients, qt);
    if(deviceInserts == NULL) return;

    deviceInsertsClone = sycl::malloc_device<int>(MAX_INSERTS * clients , qt);
    if(deviceInsertsClone == NULL) return;

    deviceInsertsIdx = sycl::malloc_device<int>(clients, qt);
    if(deviceInsertsIdx == NULL) return;

    deviceInputData = sycl::malloc_device<int>(MAX_INPUT_DATA * settings.input.size() , qt);
    if(deviceInputData == NULL) return;

    deviceInputIdx = sycl::malloc_device<int>(clients, qt);
    if(deviceInputIdx == NULL) return;

    init = true;
}

void organisation::parallel::inserts::clear()
{
}

void organisation::parallel::inserts::insert()
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)clients};

    qt.memset(deviceTotalNewInserts, 0, sizeof(int)).wait();

    auto epoch_offset = epoch * params.input.size();

    qt.submit([&](auto &h) 
    {        
        auto _inserts = deviceInserts;
        auto _insertsIdx = deviceInsertsIdx;
        auto _insertsClone = deviceInsertsClone;

        auto _inputData = deviceInputData;
        auto _inputIdx = deviceInputIdx;

        auto _totalNewInserts = deviceNewInserts;
        
        auto _values = deviceNewValues;
        auto _positions = deviceNewPositions;
        auto _client = deviceNewClient;

        auto _epoch_offset = epoch_offset;

        auto _max_inserts = MAX_INSERTS;
        auto _max_values = MAX_VALUES;
        
        h.parallel_for(num_items, [=](auto client) 
        {
            int offset = (client * _max_inserts);
            int a = _insertsIdx[client];
            _inserts[a + offset]--;

            if(_inserts[a + offset] <= 0)
            {
                _insertsIdx[client]++;
                int b = _inputIdx[client];
                int newValueToInsert = _inputData[b + epoch_offset];
                _inputIdx[client]++;

                if(_inputData[_inputIdx[client]] == -1)
                    _inputIdx[client] = 0;

                if(_inserts[_insertsIdx[client]] == -1)
                    _insertsIdx[client] = 0;

                _inserts[a + offset] = _insertsClone[a + offset];

                cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                                            sycl::memory_scope::device, 
                                            sycl::access::address_space::ext_intel_global_device_space> ar(_totalNewInserts[0]);

                int dest = ar.fetch_add(1);
                if(dest < _max_values)                
                {
                    _values[dest] = newValueToInsert;
                    _client[dest] = client;
                    _positions[dest] = { 10.0f, 10.0f, 10.0f, 10.0f };     
                }
            }
        });
    }).wait();

    qt.memcpy(hostTotalNewInserts, deviceTotalNewInserts, sizeof(int)).wait();
}

void organisation::parallel::inserts::copy(::organisation::schema **source, int source_size)
{
        //memset(hostPositions, -1, sizeof(sycl::float4) * MAX_VALUES * HOST_BUFFER);
    //memset(hostValues, -1, sizeof(int) * MAX_VALUES * HOST_BUFFER);
    memset(hostInserts, -1, sizeof(int) * MAX_INSERTS * HOST_BUFFER);
    //memset(hostMovements, -1, sizeof(sycl::float4) * MAX_MOVEMENTS * HOST_BUFFER);
    //memset(hostCollisions, -1, sizeof(sycl::float4) * MAX_COLLISIONS * HOST_BUFFER);
    memset(hostClient, -1, sizeof(int) * MAX_VALUES * HOST_BUFFER);

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)clients};

    int dest_index = 0;
    int index = 0;

    for(int source_index = 0; source_index < source_size; ++source_index)
    {
        organisation::program *prog = &source[source_index]->prog;

        int i_count = 0;
        for(auto &it: prog->insert.values)
        {
            hostInserts[d_count + (index * MAX_INSERTS)] = it;

            ++i_count;
            if(i_count > MAX_INSERTS) break;
        }

        hostClient[index] = dest_index;

        ++index;
        if(index >= HOST_BUFFER)
        {
            std::vector<sycl::event> events;

            events.push_back(qt.memcpy(&deviceInserts[dest_index * MAX_INSERTS], hostInserts, sizeof(int) * MAX_INSERTS * index));
            //events.push_back(qt.memcpy(&deviceClient[dest_index], hostClient, sizeof(int) * index));

            memset(hostInserts, -1, sizeof(int) * MAX_INSERTS * HOST_BUFFER);
            //memset(hostClient, -1, sizeof(int) * MAX_VALUES * HOST_BUFFER);

            sycl::event::wait(events);

            dest_index += HOST_BUFFER;
            index = 0;            
        }
    }

    if(index > 0)
    {
        std::vector<sycl::event> events;

        events.push_back(qt.memcpy(&deviceInserts[dest_index * MAX_INSERTS], hostInserts, sizeof(int) * MAX_INSERTS * index));
        //events.push_back(qt.memcpy(&deviceClient[dest_index], hostClient, sizeof(int) * index));

        sycl::event::wait(events);
    }    
}

void organisation::parallel::inserts::makeNull()
{
}

void organisation::parallel::inserts::cleanup()
{
}
