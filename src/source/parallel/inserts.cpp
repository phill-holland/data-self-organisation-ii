#include "parallel/inserts.hpp"

void organisation::parallel::inserts::reset(::parallel::device &dev, 
                                            ::parallel::queue *q, 
                                            parameters &settings,
                                            int length)
{
    init = false; cleanup();

    this->dev = &dev;
    this->queue = q;
    this->settings = settings;
    this->length = length;

    sycl::queue &qt = ::parallel::queue(dev).get();

    deviceNewPositions = sycl::malloc_device<sycl::float4>(length, qt);
    if(deviceNewPositions == NULL) return;

    deviceNewValues = sycl::malloc_device<int>(length, qt);
    if(deviceNewValues == NULL) return;

    deviceNewClient = sycl::malloc_device<int>(length, qt);
    if(deviceNewClient == NULL) return;

    deviceInputData = sycl::malloc_device<int>(settings.max_input_data * settings.epochs() , qt);
    if(deviceInputData == NULL) return;

    deviceInserts = sycl::malloc_device<int>(settings.max_inserts * settings.clients, qt);
    if(deviceInserts == NULL) return;

    deviceInsertsClone = sycl::malloc_device<int>(settings.max_inserts * settings.clients, qt);
    if(deviceInsertsClone == NULL) return;

    deviceInsertsIdx = sycl::malloc_device<int>(settings.clients, qt);
    if(deviceInsertsIdx == NULL) return;

    deviceInputIdx = sycl::malloc_device<int>(settings.clients, qt);
    if(deviceInputIdx == NULL) return;

    deviceNewInserts = sycl::malloc_device<int>(length, qt);
    if(deviceNewInserts == NULL) return;

    deviceTotalNewInserts = sycl::malloc_device<int>(1, qt);
    if(deviceTotalNewInserts == NULL) return;

    hostTotalNewInserts = sycl::malloc_host<int>(1, qt);
    if(hostTotalNewInserts == NULL) return;

    hostInputData = sycl::malloc_host<int>(settings.max_input_data * settings.input.size(), qt);
    if(hostInputData == NULL) return;

    init = true;
}

void organisation::parallel::inserts::clear()
{
}

void organisation::parallel::inserts::insert(int epoch)
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)settings.clients};

#warning do this somewhere, I don't know where!?!?!
//qt.memcpy(deviceInsertsClone, deviceInserts, sizeof(int) * MAX_INPUT_DATA * params.input.size()).wait();

    qt.memset(deviceTotalNewInserts, 0, sizeof(int)).wait();

    auto epoch_offset = epoch * settings.epochs();

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

        auto _max_inserts = settings.max_inserts;
        auto _max_values = settings.max_values;
        
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

void organisation::parallel::inserts::set(organisation::data &mappings, inputs::input &source)
{
    memset(hostInputData, -1, settings.max_input_data * settings.epochs());

    for(int i = 0; i < source.size(); ++i)
    {
        inputs::epoch e;
        if(source.get(e, i))
        {
            std::vector<int> temp = mappings.get(e.input);
            int len = temp.size();
            if(len > settings.max_input_data) len = settings.max_input_data;

            for(int j = 0; j < len; ++j)
            {
                hostInputData[j + (i * settings.max_input_data)] = temp[j];
            }
        }
    }
    
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    qt.memcpy(deviceInputData, hostInputData, sizeof(int) * settings.max_input_data * settings.epochs()).wait();
}

void organisation::parallel::inserts::copy(::organisation::schema **source, int source_size)
{
    /*
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
    */ 
}

void organisation::parallel::inserts::makeNull()
{
    dev = NULL;

    deviceNewPositions = NULL;
    deviceNewValues = NULL;
    deviceNewClient = NULL;
    
    deviceInputData = NULL;
    deviceInserts = NULL;
    deviceInsertsClone = NULL;
    deviceInsertsIdx = NULL;
    
    deviceInputIdx = NULL;

    deviceNewInserts = NULL;
    deviceTotalNewInserts = NULL;

    hostTotalNewInserts = NULL;    
    hostInputData = NULL;
}

void organisation::parallel::inserts::cleanup()
{
    if(dev != NULL) 
    {   
        sycl::queue q = ::parallel::queue(*dev).get();

        if(hostInputData != NULL) sycl::free(hostInputData, q);
        if(hostTotalNewInserts != NULL) sycl::free(hostTotalNewInserts, q);        
        if(deviceTotalNewInserts != NULL) sycl::free(deviceTotalNewInserts, q);
        if(deviceNewInserts != NULL) sycl::free(deviceNewInserts, q);      
        if(deviceInputIdx != NULL) sycl::free(deviceInputIdx, q);
        if(deviceInsertsIdx != NULL) sycl::free(deviceInsertsIdx, q); 
        if(deviceInsertsClone != NULL) sycl::free(deviceInsertsClone, q);
        if(deviceInserts != NULL) sycl::free(deviceInserts, q);
        if(deviceInputData != NULL) sycl::free(deviceInputData, q);
        if(deviceNewClient != NULL) sycl::free(deviceNewClient, q);
        if(deviceNewValues != NULL) sycl::free(deviceNewValues, q);
        if(deviceNewPositions != NULL) sycl::free(deviceNewPositions, q);          
    }
}
