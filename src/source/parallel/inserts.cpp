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

    deviceInputData = sycl::malloc_device<int>(settings.max_input_data * settings.epochs(), qt);
    if(deviceInputData == NULL) return;

    deviceInserts = sycl::malloc_device<int>(settings.max_inserts * settings.clients, qt);
    if(deviceInserts == NULL) return;

    deviceInsertsClone = sycl::malloc_device<int>(settings.max_inserts * settings.clients, qt);
    if(deviceInsertsClone == NULL) return;

    deviceInsertsIdx = sycl::malloc_device<int>(settings.clients, qt);
    if(deviceInsertsIdx == NULL) return;

    deviceInputIdx = sycl::malloc_device<int>(settings.clients, qt);
    if(deviceInputIdx == NULL) return;

    deviceTotalNewInserts = sycl::malloc_device<int>(1, qt);
    if(deviceTotalNewInserts == NULL) return;

    hostTotalNewInserts = sycl::malloc_host<int>(1, qt);
    if(hostTotalNewInserts == NULL) return;

    hostInputData = sycl::malloc_host<int>(settings.max_input_data * settings.input.size(), qt);
    if(hostInputData == NULL) return;

    hostInserts = sycl::malloc_host<int>(settings.max_inserts * settings.host_buffer, qt);
    if(hostInserts == NULL) return;

    clear();

    init = true;
}

void organisation::parallel::inserts::clear()
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);

    std::vector<sycl::event> events;

    events.push_back(qt.memset(deviceNewPositions, 0, sizeof(sycl::float4) * length));
    events.push_back(qt.memset(deviceNewValues, -1, sizeof(int) * length));
    events.push_back(qt.memset(deviceNewClient, -1, sizeof(int) * length));

    events.push_back(qt.memset(deviceInputData, -1, settings.max_input_data * settings.epochs()));
    events.push_back(qt.memset(deviceInserts, -1, sizeof(int) * settings.max_inserts * settings.clients));
    events.push_back(qt.memset(deviceInsertsClone, -1, sizeof(int) * settings.max_inserts * settings.clients));

    sycl::event::wait(events);
}

int organisation::parallel::inserts::insert(int epoch)
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)settings.clients};

// don't clear this, until copied into main program!!!
// no no, this has to get cleared each loop!! (otherwise get two client inserts
// at once!!

    qt.memset(deviceTotalNewInserts, 0, sizeof(int)).wait();

    auto epoch_offset = epoch * settings.epochs();
    sycl::float4 starting = { (float)settings.starting.x, (float)settings.starting.y, (float)settings.starting.z, 0.0f };

    qt.submit([&](auto &h) 
    {        
        auto _inserts = deviceInserts;
        auto _insertsIdx = deviceInsertsIdx;
        auto _insertsClone = deviceInsertsClone;

        auto _inputData = deviceInputData;
        auto _inputIdx = deviceInputIdx;

        auto _totalNewInserts = deviceTotalNewInserts;
        
        auto _values = deviceNewValues;
        auto _positions = deviceNewPositions;
        auto _clients = deviceNewClient;

        auto _starting = starting;

        auto _epoch_offset = epoch_offset;

        auto _max_inserts = settings.max_inserts;
        auto _max_values = settings.max_values;
        
        h.parallel_for(num_items, [=](auto client) 
        {
            int offset = (client * _max_inserts);
            int a = _insertsIdx[client];
            _inserts[a + offset]--;

            if(_inserts[a + offset] < 0)
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
                    _positions[dest] = starting;
                    _clients[dest] = client;
                }
            }
        });
    }).wait();

    qt.memcpy(hostTotalNewInserts, deviceTotalNewInserts, sizeof(int)).wait();

    return hostTotalNewInserts[0];
}

void organisation::parallel::inserts::set(organisation::data &mappings, inputs::input &source)
{
    memset(hostInputData, -1, sizeof(int) * settings.max_input_data * settings.epochs());

    int offset = 0;
    for(int i = 0; i < source.size(); ++i)
    {        
        inputs::epoch epoch;
        if(source.get(epoch, i))
        {
            std::vector<int> temp = mappings.get(epoch.input);
            int len = temp.size();
            if(len > settings.max_input_data) len = settings.max_input_data;

            for(int j = 0; j < len; ++j)
            {
                hostInputData[j + offset] = temp[j];
            }

            offset += settings.max_input_data;
        }
    }
    
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    qt.memcpy(deviceInputData, hostInputData, sizeof(int) * settings.max_input_data * settings.epochs()).wait();
}

std::vector<organisation::parallel::value> organisation::parallel::inserts::get()
{
    std::vector<value> result;

    std::vector<int> values = get(deviceNewValues, length);
    std::vector<int> clients = get(deviceNewClient, length);
    std::vector<sycl::float4> positions = get(deviceNewPositions, length);

    if(values.size() == clients.size() == positions.size() == hostTotalNewInserts[0])
    {
        int len = values.size();
        for(int i = 0; i < len; ++i)
        {
            value temp;

            temp.value = values[i];
            temp.client = clients[i];
            temp.position = point(positions[i].x(), positions[i].y(), positions[i].z());

            result.push_back(temp);
        }
    }

    return result;
}

void organisation::parallel::inserts::copy(::organisation::schema **source, int source_size)
{
    memset(hostInserts, -1, sizeof(int) * settings.max_inserts * settings.host_buffer);

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)settings.clients};

    int dest_index = 0;
    int index = 0;

    for(int source_index = 0; source_index < source_size; ++source_index)
    {
        organisation::program *prog = &source[source_index]->prog;

        int i_count = 0;
        for(auto &it: prog->insert.values)
        {
            hostInserts[i_count + (index * settings.max_inserts)] = it;

            ++i_count;
            if(i_count > settings.max_inserts) break;
        }

        ++index;
        if(index >= settings.host_buffer)
        {
            std::vector<sycl::event> events;

            events.push_back(qt.memcpy(&deviceInserts[dest_index * settings.max_inserts], hostInserts, sizeof(int) * settings.max_inserts * index));

            memset(hostInserts, -1, sizeof(int) * settings.max_inserts * settings.host_buffer);

            sycl::event::wait(events);

            dest_index += settings.host_buffer;
            index = 0;            
        }
    }

    if(index > 0)
    {
        std::vector<sycl::event> events;

        events.push_back(qt.memcpy(&deviceInserts[dest_index * settings.max_inserts], hostInserts, sizeof(int) * settings.max_inserts * index));

        sycl::event::wait(events);
    }   

    qt.memcpy(deviceInsertsClone, deviceInserts, sizeof(int) * settings.max_inserts * settings.clients).wait();
}

std::vector<int> organisation::parallel::inserts::get(int *source, int length)
{
	int *temp = new int[length];
	if (temp == NULL) return { };

    sycl::queue q = ::parallel::queue(*dev).get();

    q.memcpy(temp, source, sizeof(int) * length).wait();

    std::vector<int> result;

	for (int i = 0; i < length; ++i)
	{
		if (temp[i] != -1)
            result.push_back(temp[i]);
	}

	delete[] temp;

    return result;
}

std::vector<sycl::float4> organisation::parallel::inserts::get(sycl::float4 *source, int length)
{
    sycl::float4 *temp = new sycl::float4[length];
    if (temp == NULL) return { };

    sycl::queue q = ::parallel::queue(*dev).get();

    q.memcpy(temp, source, sizeof(sycl::float4) * length).wait();

    std::vector<sycl::float4> result;
    
	for (int i = 0; i < length; ++i)
	{
        int ix = (int)(temp[i].x() * 100.0f);
        int iy = (int)(temp[i].y() * 100.0f);
        int iz = (int)(temp[i].z() * 100.0f);

        if ((ix != 0) || (iy != 0) || (iz != 0))
        {
            result.push_back(temp[i]);			
		}
	}
	
	delete[] temp;

    return result;
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

    deviceTotalNewInserts = NULL;

    hostTotalNewInserts = NULL;    
    hostInputData = NULL;
    hostInserts = NULL;
}

void organisation::parallel::inserts::cleanup()
{
    if(dev != NULL) 
    {   
        sycl::queue q = ::parallel::queue(*dev).get();

        if(hostInserts != NULL) sycl::free(hostInserts, q);
        if(hostInputData != NULL) sycl::free(hostInputData, q);
        if(hostTotalNewInserts != NULL) sycl::free(hostTotalNewInserts, q);        
        if(deviceTotalNewInserts != NULL) sycl::free(deviceTotalNewInserts, q);
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
