#include "parallel/inserts.hpp"

sycl::int4 MapClientIdx(const int index, const sycl::int4 dimensions)
{
    int n = dimensions.x() * dimensions.y();
    int r = index % n;
    float z = (float)((index / n));

    int j = r % dimensions.x();
    float y = (float)((r / dimensions.x()));
    float x = (float)(j);

    return { x, y, z, index };
}

void organisation::parallel::inserts::reset(::parallel::device &dev, 
                                            ::parallel::queue *q, 
                                            parameters &settings)
{
    init = false; cleanup();

    this->dev = &dev;
    this->queue = q;
    this->settings = settings;
    this->length = settings.clients();

    sycl::queue &qt = ::parallel::queue(dev).get();

    deviceNewPositions = sycl::malloc_device<sycl::float4>(length, qt);
    if(deviceNewPositions == NULL) return;

    deviceNewValues = sycl::malloc_device<int>(length, qt);
    if(deviceNewValues == NULL) return;

    deviceNewClient = sycl::malloc_device<sycl::int4>(length, qt);
    if(deviceNewClient == NULL) return;

    deviceInputData = sycl::malloc_device<int>(settings.max_input_data * settings.epochs(), qt);
    if(deviceInputData == NULL) return;

    deviceInserts = sycl::malloc_device<int>(settings.max_inserts * settings.clients(), qt);
    if(deviceInserts == NULL) return;

    deviceInsertsClone = sycl::malloc_device<int>(settings.max_inserts * settings.clients(), qt);
    if(deviceInsertsClone == NULL) return;

    deviceInsertsIdx = sycl::malloc_device<int>(settings.clients(), qt);
    if(deviceInsertsIdx == NULL) return;

    deviceInputIdx = sycl::malloc_device<int>(settings.clients(), qt);
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
    events.push_back(qt.memset(deviceNewClient, 0, sizeof(sycl::int4) * length));
    events.push_back(qt.memset(deviceInsertsIdx, 0, sizeof(int) * settings.clients()));
    events.push_back(qt.memset(deviceInputIdx, 0, sizeof(int) * settings.clients()));

    sycl::event::wait(events);
}

int organisation::parallel::inserts::insert(int epoch)
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)length};

    qt.memset(deviceTotalNewInserts, 0, sizeof(int)).wait();

    auto epoch_offset = (epoch > settings.epochs() ? settings.epochs() : epoch) * settings.max_input_data;

    sycl::float4 starting = { (float)settings.starting.x, (float)settings.starting.y, (float)settings.starting.z, 0.0f };
    sycl::int4 dim_clients = { settings.dim_clients.x, settings.dim_clients.y, settings.dim_clients.z, 0.0f };

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
        auto _dim_clients = dim_clients;

        auto _length = length;

        h.parallel_for(num_items, [=](auto client) 
        {
            if(_inputData[_inputIdx[client] + epoch_offset] == -1) return;
            
            int offset = (client * _max_inserts);            
            int a = _insertsIdx[client];            
            _inserts[a + offset]--;

            if(_inserts[a + offset] < 0)
            {                
                _insertsIdx[client]++;
                int b = _inputIdx[client];
                int newValueToInsert = _inputData[b + epoch_offset];
                _inputIdx[client]++;

                if(_inserts[_insertsIdx[client]] == -1)
                    _insertsIdx[client] = 0;

                _inserts[a + offset] = _insertsClone[a + offset];

                cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                                            sycl::memory_scope::device, 
                                            sycl::access::address_space::ext_intel_global_device_space> ar(_totalNewInserts[0]);

                int dest = ar.fetch_add(1);
                if(dest < _length)                
                {
                    _values[dest] = newValueToInsert;
                    _positions[dest] = starting;
                    _clients[dest] = MapClientIdx(client, _dim_clients);
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

    int totals = hostTotalNewInserts[0];
    if(totals > 0)
    {
        std::vector<int> values = dev->get(deviceNewValues, totals);
        std::vector<sycl::int4> clients = dev->get(deviceNewClient, totals);
        std::vector<sycl::float4> positions = dev->get(deviceNewPositions, totals);

        if((values.size() == totals)&&(clients.size() == totals)&&(positions.size() == totals))
        {
            int len = values.size();
            for(int i = 0; i < len; ++i)
            {
                value temp;

                temp.value = values[i];
                temp.client = clients[i].w();
                temp.position = point(positions[i].x(), positions[i].y(), positions[i].z());

                result.push_back(temp);
            }
        }
    }

    return result;
}

void organisation::parallel::inserts::copy(::organisation::schema **source, int source_size)
{
    memset(hostInserts, -1, sizeof(int) * settings.max_inserts * settings.host_buffer);

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)settings.clients()};

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

    qt.memcpy(deviceInsertsClone, deviceInserts, sizeof(int) * settings.max_inserts * settings.clients()).wait();
}

void organisation::parallel::inserts::outputarb(int *source, int length)
{
	int *temp = new int[length];
	if (temp == NULL) return;

    sycl::queue q = ::parallel::queue(*dev).get();

    q.memcpy(temp, source, sizeof(int) * length).wait();

    std::string result("");
	for (int i = 0; i < length; ++i)
	{
		if ((temp[i] != -1)&&(temp[i]!=0))
		{
			result += std::string("[");
			result += std::to_string(i);
			result += std::string("]");
			result += std::to_string(temp[i]);
			result += std::string(",");
		}
	}
	result += std::string("\r\n");
	
    std::cout << result;

	delete[] temp;
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
