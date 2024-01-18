#include "parallel/program.hpp"
#include <algorithm>

void organisation::parallel::program::reset(::parallel::device &dev, ::parallel::queue *q, parameters settings, int clients)
{
    init = false; cleanup();

    this->dev = &dev;
    this->queue = q;
    this->params = settings;
    
    sycl::queue &qt = ::parallel::queue(dev).get();

    this->clients = clients;
    //int length = MAX_VALUES * clients;
    //this->length = settings.length() * clients;

    devicePositions = sycl::malloc_device<sycl::float4>(MAX_VALUES * clients, qt);
    if(devicePositions == NULL) return;

    deviceValues = sycl::malloc_device<int>(MAX_VALUES * clients, qt);
    if(deviceValues == NULL) return;
    
    deviceInputData = sycl::malloc_device<int>(MAX_INPUT_DATA * settings.input.size() , qt);
    if(deviceInputData == NULL) return;

    deviceInserts = sycl::malloc_device<int>(MAX_INSERTS * clients, qt);
    if(deviceInserts == NULL) return;

    deviceInsertsClone = sycl::malloc_device<int>(MAX_INSERTS * clients , qt);
    if(deviceInsertsClone == NULL) return;

    deviceMovementIdx = sycl::malloc_device<int>(clients, qt);
    if(deviceMovementIdx == NULL) return;

    deviceInsertsIdx = sycl::malloc_device<int>(clients, qt);
    if(deviceInsertsIdx == NULL) return;

    deviceInputIdx = sycl::malloc_device<int>(clients, qt);
    if(deviceInputIdx == NULL) return;

    deviceClient = sycl::malloc_device<int>(MAX_VALUES * clients, qt);
    if(deviceClient == NULL) return;

    // ***

    deviceMovements = sycl::malloc_device<sycl::float4>(MAX_MOVEMENTS * clients, qt);
    if(deviceMovements == NULL) return;

    deviceCollisions = sycl::malloc_device<sycl::float4>(MAX_COLLISIONS * clients, qt);
    if(deviceCollisions == NULL) return;

    deviceTotalValues = sycl::malloc_device<int>(1, qt);
    if(deviceTotalValues == NULL) return;

    deviceNewInserts = sycl::malloc_device<int>(MAX_VALUES, qt);
    if(deviceNewInserts == NULL) return;

    deviceTotalNewInserts = sycl::malloc_device<int>(1, qt);
    if(deviceTotalNewInserts == NULL) return;

    // ***
    
    hostPositions = sycl::malloc_host<sycl::float4>(MAX_VALUES * HOST_BUFFER, qt);
    if(hostPositions == NULL) return;

    hostValues = sycl::malloc_host<int>(MAX_VALUES * HOST_BUFFER, qt);
    if(hostValues == NULL) return;
    
    hostInputData = sycl::malloc_host<int>(MAX_INPUT_DATA * settings.input.size(), qt);
    if(hostInputData == NULL) return;

    hostInserts = sycl::malloc_host<int>(MAX_INSERTS * HOST_BUFFER, qt);
    if(hostInserts == NULL) return;

    hostMovements = sycl::malloc_host<sycl::float4>(MAX_MOVEMENTS * HOST_BUFFER, qt);
    if(hostMovements == NULL) return;

    hostCollisions = sycl::malloc_host<sycl::float4>(MAX_COLLISIONS * HOST_BUFFER, qt);
    if(hostCollisions == NULL) return;

    hostClient = sycl::malloc_host<int>(MAX_VALUES * HOST_BUFFER, qt);
    if(hostClient == NULL) return;

    hostTotalValues = sycl::malloc_host<int>(1, qt);
    if(hostTotalValues == NULL) return;

    hostTotalNewInserts = sycl::malloc_host<int>(1, qt);
    if(hostTotalNewInserts == NULL) return;

    // ***

/*
    deviceOutput = sycl::malloc_device<int>(length, q);
    if(deviceOutput == NULL) return;

    deviceOutputIteration = sycl::malloc_device<int>(length, q);
    if(deviceOutputIteration == NULL) return;


    deviceOutputEndPtr = sycl::malloc_device<int>(clients, q);
    if(deviceOutputEndPtr == NULL) return;

    deviceReadPositionsA = sycl::malloc_device<sycl::float4>(length, q);
    if(deviceReadPositionsA == NULL) return;

    deviceReadPositionsB = sycl::malloc_device<sycl::float4>(length, q);
    if(deviceReadPositionsB == NULL) return;

    deviceReadPositionsEndPtr = sycl::malloc_device<int>(clients, q);
    if(deviceReadPositionsEndPtr == NULL) return;

    // ***

    //deviceSourceReadPositions = sycl::malloc_device<sycl::float4>(settings.epochs, q);
    //if(deviceSourceReadPositions == NULL) return;

    //hostSourceReadPositions = sycl::malloc_host<sycl::float4>(settings.epochs, q);
    //if(hostSourceReadPositions == NULL) return;

    // ***

    hostOutput = sycl::malloc_host<int>(length, q);
    if(hostOutput == NULL) return;

    hostOutputIteration = sycl::malloc_host<int>(length, q);
    if(hostOutputIteration == NULL) return;

    hostOutputEndPtr = sycl::malloc_host<int>(clients, q);
    if(hostOutputEndPtr == NULL) return;
*/
    init = true;
}

void organisation::parallel::program::clear()
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);

    std::vector<sycl::event> events;

    events.push_back(qt.memset(devicePositions, 0, sizeof(sycl::float4) * MAX_VALUES * clients));
    events.push_back(qt.memset(deviceNextPositions, 0, sizeof(sycl::float4) * MAX_VALUES * clients));
    events.push_back(qt.memset(deviceNextHalfPositions, 0, sizeof(sycl::float4) * MAX_VALUES * clients));
    events.push_back(qt.memset(deviceValues, 0, sizeof(int) * MAX_VALUES * clients));
    events.push_back(qt.memset(deviceInserts, -1, sizeof(int) * MAX_VALUES * clients));
    events.push_back(qt.memset(deviceInputData, -1, sizeof(int) * MAX_INPUT_DATA * params.input.size()));
    events.push_back(qt.memset(deviceMovementIdx, 0, sizeof(int) * clients));
    events.push_back(qt.memset(deviceInsertsIdx, 0, sizeof(int) * clients));
    events.push_back(qt.memset(deviceClient, 0, sizeof(int) * MAX_VALUES * clients));
    events.push_back(qt.memset(deviceMovements, 0, sizeof(sycl::float4) * MAX_MOVEMENTS * clients));
    events.push_back(qt.memset(deviceCollisions, 0, sizeof(sycl::float4) * MAX_COLLISIONS * clients));

    sycl::event::wait(events);
}

void organisation::parallel::program::run(organisation::data &mappings)
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)MAX_VALUES * clients};

    qt.memcpy(deviceInsertsClone, deviceInserts, sizeof(int) * MAX_INPUT_DATA * params.input.size()).wait();

    for(int epoch = 0; epoch < params.input.size(); ++epoch)
    {        
        qt.memset(deviceInsertsIdx, 0, sizeof(int) * clients).wait();
        qt.memset(deviceInputIdx, 0, sizeof(int) * clients).wait();

        int iterations = 0;
        while(iterations++ < ITERATIONS)
        {
            insert(epoch);
            update();
        };
    }
    /*
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, q);
    sycl::range num_items{(size_t)length};

    sycl::float4 *source = deviceReadPositionsA;
    int *sourceEpoch = deviceReadPositionsEpochA;

    sycl::float4 *destination = deviceReadPositionsB;
    int *destinationEpoch = deviceReadPositionsEpochB;    

    qt.memset(deviceOutputEndPtr, 0, sizeof(int) * clients).wait();

    int iterations = 0;
    while(iterations++ < ITERATIONS)
    {
        auto msA = qt.memset(destination, 0, sizeof(sycl::float4) * length);
        auto msB = qt.memset(destinationEpoch, 0, sizeof(int) * length);
        auto msC = qt.memset(deviceReadPositionsEndPtr, 0 , sizeof(int) * clients);

        msA.wait();
        msB.wait();
        msC.wait();

        qt.submit([&](auto &h) 
        {        
            auto _values = deviceValues;
            auto _inGates = deviceInGates;
            auto _outGates = deviceOutGates;
            auto _magnitude = deviceMagnitudes;
            auto _output = deviceOutput;
            auto _outputIteration = deviceOutputIteration;
            auto _outputEpoch = deviceOutputEpoch;
            auto _outputEndPtr = deviceOutputEndPtr;
            
            auto _readPositionsSource = source;
            auto _readPositionsSourceEpoch = sourceEpoch;
            auto _readPositionsDest = destination;
            auto _readPositionsDestEpoch = destinationEpoch;

            auto _readPositionsEndPtr = deviceReadPositionsEndPtr;
            
            auto _width = params.width;
            auto _height = params.height;
            auto _depth = params.depth;

            auto _in = params.in;
            auto _out = params.out;

            auto _iteration = iterations;
            auto _stride = params.size();
            auto _length = length;

            h.parallel_for(num_items, [=](auto i) 
            {
                if((_readPositionsSource[i].x() != 0)||(_readPositionsSource[i].y() != 0)||(_readPositionsSource[i].z()!= 0))
                {                 
                    sycl::float4 current = _readPositionsSource[i];    
                    int currentEpoch = _readPositionsSourceEpoch[i];           

                    int client = i / _stride;

                    int local_index = ((current.z() * _width * _height) + (current.y() * _width) + current.x());
                    int index = local_index + (client * _stride);

                    int inIndex = (client * _stride * _in) + (local_index * _in);

                    for(int x = 0; x < _in; ++x)
                    {                        
                        if(_inGates[inIndex + x] >= 0)
                        {                            
                            if(_inGates[inIndex + x] == current.w())
                            {
                                cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                                sycl::memory_scope::device, 
                                sycl::access::address_space::ext_intel_global_device_space> ar(_outputEndPtr[client]);

                                int value = _values[index];
                                if(value >= 0) 
                                {
                                    int vv = ar.fetch_add(1);
                                    if(vv < _stride) 
                                    {
                                        _output[vv + (client * _stride)] = value;
                                        _outputIteration[vv + (client * _stride)] = _iteration;
                                        _outputEpoch[vv + (client * _stride)] = currentEpoch;
                                    }
                                }

                                for(int y = 0; y < _out; ++y)
                                {                            
                                    int s = (client * _stride * _in * _out);
                                    s += (local_index * _out * _in);
                                    s += (y * _in) + x;
                                    int outIndex = s;

                                    if(_outGates[outIndex] >= 0)
                                    {                                        
                                        int outTemp = _outGates[outIndex];
                                        float magnitudeTemp = (float)_magnitude[outIndex];
                                        
                                        if(magnitudeTemp > 0)
                                        {
                    
                                            int r = outTemp % 9;
                                            float z1 = (float)((outTemp / 9) - 1);

                                            int j = r % 3;
                                            float y1 = (float)((r / 3) - 1);
                                            float x1 = (float)(j - 1);

                                            float z2 = (z1 * magnitudeTemp) + current.z();
                                            float y2 = (y1 * magnitudeTemp) + current.y();
                                            float x2 = (x1 * magnitudeTemp) + current.x();

                                            if((x2 >= 0)&&(x2 < _width)&&(y2 >= 0)&&(y2 < _height)&&(z2 >=0)&&(z2 < _depth))
                                            {
                                                cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                                                sycl::memory_scope::device, 
                                                sycl::access::address_space::ext_intel_global_device_space> br(_readPositionsEndPtr[client]);

                                                int tx = roundf(-x1) + 1;
                                                int ty = roundf(-y1) + 1;
                                                int tz = roundf(-z1) + 1;

                                                float w = (float)((sycl::abs(tz) * (3 * 3)) + (sycl::abs(ty) * 3) + sycl::abs(tx));

                                                int vv = br.fetch_add(1);
                                                if(vv < _stride)
                                                {
                                                    _readPositionsDest[vv + (client * _stride)] = { x2,y2,z2,w };
                                                    _readPositionsDestEpoch[vv + (client * _stride)] = currentEpoch;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            });
        }).wait();


        sycl::float4 *temp = destination;
        destination = source;
        source = temp;

        int *tempEpoch = destinationEpoch;
        destinationEpoch = sourceEpoch;
        sourceEpoch = tempEpoch;
    };
    */
}

void organisation::parallel::program::set(organisation::data &mappings, inputs::input &source)
{
    memset(hostInputData, -1, MAX_INPUT_DATA * params.input.size());

    for(int i = 0; i < source.size(); ++i)
    {
        inputs::epoch e;
        if(source.get(e, i))
        {
            std::vector<int> temp = mappings.get(e.input);
            int len = temp.size();
            if(len > MAX_INPUT_DATA) len = MAX_INPUT_DATA;

            for(int j = 0; j < len; ++j)
            {
                hostInputData[j + (i * MAX_INPUT_DATA)] = temp[j];
            }
        }
    }
    
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    qt.memcpy(deviceInputData, hostInputData, sizeof(int) * MAX_INPUT_DATA * params.input.size()).wait();
}

std::vector<organisation::output> organisation::parallel::program::get(organisation::data &mappings)
{    
    std::vector<output> results(clients);
/*
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, q);

    auto mcA = qt.memcpy(hostOutput, deviceOutput, sizeof(int) * length);
    auto mcB = qt.memcpy(hostOutputIteration, deviceOutputIteration, sizeof(int) * length);
    auto mcC = qt.memcpy(hostOutputEpoch, deviceOutputEpoch, sizeof(int) * length);
    auto mcD = qt.memcpy(hostOutputEndPtr, deviceOutputEndPtr, sizeof(int) * clients);
    
    mcA.wait();
    mcB.wait();
    mcC.wait();
    mcD.wait();

    std::unordered_map<int, std::vector<std::tuple<int,int>>> client_data;
    for(int i = 0; i < clients; ++i)
    {
        client_data[i] = std::vector<std::tuple<int,int>>();
    }

    for(int i = 0; i < length; ++i)
    {
        int client = i / params.size();;
        int index = i % params.size();

        if(index < hostOutputEndPtr[client])
        {
            client_data[client].push_back(std::tuple<int,int>(hostOutput[i],hostOutputEpoch[i]));
        }
    }

    for(int i = 0; i < clients; ++i)
    {    
        for(int j = 1; j < params.epochs + 1; ++j)
        {
            std::vector<int> temp;
            for(std::vector<std::tuple<int,int>>::iterator it = client_data[i].begin(); it != client_data[i].end(); ++it)
            {
                int value = std::get<0>(*it);
                int epoch = std::get<1>(*it);

                if(epoch == j) temp.push_back(value);
            }

            std::string out = mappings.get(temp);
            results[i].values.push_back(out);
        }
    }
*/
    return results;
}

void organisation::parallel::program::insert(int epoch)
{
    // REMMBER TO MEMSET _InputData to -1 before populatiin OK
    // REMMBER TO MEMSET _Inserts to -1 before populatiin OK

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

        // need to replace with localInsertValues!
        //auto _values = deviceValues;
        //auto _positions = devicePositions;
        //auto _client = deviceClient;

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
                                            sycl::access::address_space::ext_intel_global_device_space> ar(_totalValues[0]);

                int dest = ar.fetch_add(1);
                if(dest < _max_values)                
                {
                    _values[dest] = newValueToInsert;
                    _client[dest] = client;
                    _positions[dest] = { 10.0f, 10.0f, 10.0f, 10.0f };     
                    // HMM NEED TO ENSURE INSERT POINT IS NOT OCCUPIED ON CLIENT  
                    // NEED TO PASS IN STARTING POINT HERE             
                }
            }
        });
    }).wait();

    qt.memcpy(hostTotalNewInserts, deviceTotalNewInserts, sizeof(int)).wait();
    // hostTotalValues can be used in main run function, for num_items!!
}

void organisation::parallel::program::update()
{
    // needs to be collision detection aware here, basic implementation!!
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)hostTotalValues[0]};

// ***
// ENSURE MOVEMENTS ARRAY, END POINTS.W ARE INITALISED with -1
// ***

    qt.submit([&](auto &h) 
    {
        auto _values = deviceValues;
        auto _positions = devicePositions;
        auto _client = deviceClient;
        auto _movementIdx = deviceMovementIdx;
        auto _movements = deviceMovements;

        auto _max_movements = MAX_MOVEMENTS;

        h.parallel_for(num_items, [=](auto i) 
        {  
            int client = _client[i];
            int a = _movementIdx[client];
            int offset = _max_movements * client;

            sycl::float4 direction = _movements[a + offset];
            _positions[i] += direction;

            _movementIdx[client]++;      
            int temp = _movementIdx[client];          
            if((temp >= _max_movements)||(_movements[temp + offset].w() == -1))
                _movementIdx[client] = 0;            
        });
    }).wait();
}

void organisation::parallel::program::copy(::organisation::schema **source, int source_size)
{
    memset(hostPositions, -1, sizeof(sycl::float4) * MAX_VALUES * HOST_BUFFER);
    memset(hostValues, -1, sizeof(int) * MAX_VALUES * HOST_BUFFER);
    memset(hostInserts, -1, sizeof(int) * MAX_INSERTS * HOST_BUFFER);
    memset(hostMovements, -1, sizeof(sycl::float4) * MAX_MOVEMENTS * HOST_BUFFER);
    memset(hostCollisions, -1, sizeof(sycl::float4) * MAX_COLLISIONS * HOST_BUFFER);
    memset(hostClient, -1, sizeof(int) * MAX_VALUES * HOST_BUFFER);

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)clients};

    int dest_index = 0;
    int index = 0;

    for(int source_index = 0; source_index < source_size; ++source_index)
    {
        organisation::program *prog = &source[source_index]->prog;

        int d_count = 0;
        for(auto &it: prog->caches.values)
        {
            int value = std::get<0>(it);
            point position = std::get<1>(it);

            hostPositions[d_count + (index * MAX_VALUES)] = { (float)position.x, (float)position.y, (float)position.z, 0.0f };
            hostValues[d_count + (index * MAX_VALUES)] = value;

            ++d_count;
            if(d_count > MAX_VALUES) break;
        }

        hostTotalValues[0] = d_count;
        qt.memcpy(deviceTotalValues, hostTotalValues, sizeof(int)).wait();

        int m_count = 0;
        for(auto &it: prog->movement.directions)
        {            
            hostMovements[m_count + (index * MAX_MOVEMENTS)] = { (float)it.x, (float)it.y, (float)it.z, 0.0f };
            ++m_count;
            if(m_count > MAX_MOVEMENTS) break;            
        }

        int c_count = 0;
        for(auto &it: prog->collisions.values)        
        {
            vector temp;
            temp.decode(it);

            hostCollisions[c_count + (index * MAX_COLLISIONS)] = { (float)temp.x, (float)temp.y, (float)temp.z, 0.0f };
            ++c_count;
            if(c_count > MAX_COLLISIONS) break;            
        }

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

            events.push_back(qt.memcpy(&devicePositions[dest_index * MAX_VALUES], hostPositions, sizeof(sycl::float4) * MAX_VALUES * index));
            events.push_back(qt.memcpy(&deviceValues[dest_index * MAX_VALUES], hostValues, sizeof(int) * MAX_VALUES * index));
            events.push_back(qt.memcpy(&deviceInserts[dest_index * MAX_INSERTS], hostInserts, sizeof(int) * MAX_INSERTS * index));
            events.push_back(qt.memcpy(&deviceMovements[dest_index * MAX_MOVEMENTS], hostMovements, sizeof(int) * MAX_MOVEMENTS * index));
            events.push_back(qt.memcpy(&deviceCollisions[dest_index * MAX_COLLISIONS], hostCollisions, sizeof(int) * MAX_COLLISIONS * index));
            events.push_back(qt.memcpy(&deviceClient[dest_index], hostClient, sizeof(int) * index));

            memset(hostPositions, -1, sizeof(sycl::float4) * MAX_VALUES * HOST_BUFFER);
            memset(hostValues, -1, sizeof(int) * MAX_VALUES * HOST_BUFFER);
            memset(hostInserts, -1, sizeof(int) * MAX_INSERTS * HOST_BUFFER);
            memset(hostMovements, -1, sizeof(sycl::float4) * MAX_MOVEMENTS * HOST_BUFFER);
            memset(hostCollisions, -1, sizeof(sycl::float4) * MAX_COLLISIONS * HOST_BUFFER);
            memset(hostClient, -1, sizeof(int) * MAX_VALUES * HOST_BUFFER);

            sycl::event::wait(events);

            dest_index += HOST_BUFFER;
            index = 0;            
        }
    }

    if(index > 0)
    {
        std::vector<sycl::event> events;

        events.push_back(qt.memcpy(&devicePositions[dest_index * MAX_VALUES], hostPositions, sizeof(sycl::float4) * MAX_VALUES * index));
        events.push_back(qt.memcpy(&deviceValues[dest_index * MAX_VALUES], hostValues, sizeof(int) * MAX_VALUES * index));
        events.push_back(qt.memcpy(&deviceInserts[dest_index * MAX_INSERTS], hostInserts, sizeof(int) * MAX_INSERTS * index));
        events.push_back(qt.memcpy(&deviceMovements[dest_index * MAX_MOVEMENTS], hostMovements, sizeof(int) * MAX_MOVEMENTS * index));
        events.push_back(qt.memcpy(&deviceCollisions[dest_index * MAX_COLLISIONS], hostCollisions, sizeof(int) * MAX_COLLISIONS * index));
        events.push_back(qt.memcpy(&deviceClient[dest_index], hostClient, sizeof(int) * index));

        sycl::event::wait(events);
    }    
}

void organisation::parallel::program::outputarb(int *source, int length)
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

void organisation::parallel::program::outputarb(sycl::float4 *source, int length)
{
    sycl::float4 *temp = new sycl::float4[length];
    if (temp == NULL) return;

    sycl::queue q = ::parallel::queue(*dev).get();

    q.memcpy(temp, source, sizeof(sycl::float4) * length).wait();

    std::string result("");
	for (int i = 0; i < length; ++i)
	{
        int ix = (int)(temp[i].x() * 100.0f);
        int iy = (int)(temp[i].y() * 100.0f);
        int iz = (int)(temp[i].z() * 100.0f);

        if ((ix != 0) || (iy != 0) || (iz != 0))
        {
			result += std::string("[");
			result += std::to_string(i);
			result += std::string("]");
			result += std::to_string(temp[i].x());
			result += std::string(",");
			result += std::to_string(temp[i].y());
			result += std::string(",");
			result += std::to_string(temp[i].z());
			result += std::string(",");
            result += std::to_string(temp[i].w());
			result += std::string(",");
		}
	}
	result += std::string("\r\n");
	
    
    std::cout << result;

	delete[] temp;
}


void organisation::parallel::program::makeNull()
{
    dev = NULL;

    devicePositions = NULL;
    deviceNextPositions = NULL;
    deviceNextHalfPositions = NULL;
    deviceValues = NULL;
    deviceInputData = NULL;
    deviceInserts = NULL;
    deviceInsertsClone = NULL;
    
    deviceMovementIdx = NULL;
    deviceInsertsIdx = NULL;
    deviceInputIdx = NULL;
    deviceClient = NULL;

    deviceMovements = NULL;
    deviceCollisions = NULL;

    deviceTotalValues = NULL;

    deviceNewInserts = NULL;
    deviceTotalnewInserts = NULL;

    hostPositions = NULL;
    hostValues = NULL;
    hostInputData = NULL;
    hostInserts = NULL;
    hostMovements = NULL;
    hostCollisions = NULL;
    hostClient = NULL;
    hostTotalValues = NULL;
    hostTotalNewInserts = NULL;
    /*
    deviceOutput = NULL;
    deviceOutputIteration = NULL;
    deviceOutputEpoch = NULL;
    deviceOutputEndPtr = NULL;

    deviceReadPositionsA = NULL;
    deviceReadPositionsB = NULL;
    deviceReadPositionsEpochA = NULL;
    deviceReadPositionsEpochB = NULL;
    deviceReadPositionsEndPtr = NULL;   
    
    deviceSourceReadPositions = NULL;
    hostSourceReadPositions = NULL;

    hostOutput = NULL;
    hostOutputIteration = NULL;
    hostOutputEpoch = NULL;
    hostOutputEndPtr = NULL; 
    */
}

void organisation::parallel::program::cleanup()
{
    if(dev != NULL) 
    {   
        sycl::queue q = ::parallel::queue(*dev).get();

        if(hostTotalNewInserts != NULL) sycl::free(hostTotalNewInserts, q);
        if(hostTotalValues != NULL) sycl::free(hostTotalValues, q);
        if(hostClient != NULL) sycl::free(hostClient, q);
        if(hostCollisions != NULL) sycl::free(hostCollisions, q);
        if(hostMovements != NULL) sycl::free(hostMovements, q);
        if(hostInserts != NULL) sycl::free(hostInserts, q);
        if(hostInputData != NULL) sycl::free(hostInputData, q);
        if(hostValues != NULL) sycl::free(hostValues, q);
        if(hostPositions != NULL) sycl::free(hostPositions, q);

        if(deviceTotalNewInserts != NULL) sycl::free(deviceTotalNewInserts, q);
        if(deviceNewInserts != NULL) sycl::free(deviceNewInserts, q);        
        if(deviceTotalValues != NULL) sycl::free(deviceTotalValues, q);

        if(deviceCollisions != NULL) sycl::free(deviceCollisions, q);
        if(deviceMovements != NULL) sycl::free(deviceMovements, q);
        
        if(deviceClient != NULL) sycl::free(deviceClient, q);
        if(deviceInputIdx != NULL) sycl::free(deviceInputIdx, q);
        if(deviceInsertsIdx != NULL) sycl::free(deviceInsertsIdx, q);
        if(deviceMovementIdx != NULL) sycl::free(deviceMovementIdx, q);
        if(deviceInsertsClone != NULL) sycl::free(deviceInsertsClone, q);
        if(deviceInserts != NULL) sycl::free(deviceInserts, q);
        if(deviceInputData != NULL) sycl::free(deviceInputData, q);
        if(deviceValues != NULL) sycl::free(deviceValues, q);

        if(deviceNextHalfPositions != NULL) sycl::free(deviceNextHalfPositions, q);
        if(deviceNextPositions != NULL) sycl::free(deviceNextPositions, q);
        if(devicePositions != NULL) sycl::free(devicePositions, q);
    }
    /*
    if(dev != NULL) 
    {   
        sycl::queue q = ::parallel::queue(*dev).get();
        
        if (hostOutputEndPtr != NULL) sycl::free(hostOutputEndPtr, q);
        if (hostOutputEpoch != NULL) sycl::free(hostOutputEpoch, q);
        if (hostOutputIteration != NULL) sycl::free(hostOutputIteration, q);
        if (hostOutput != NULL) sycl::free(hostOutput, q);

        if (hostSourceReadPositions != NULL) sycl::free(hostSourceReadPositions, q);
        if (deviceSourceReadPositions != NULL) sycl::free(deviceSourceReadPositions, q);

        if (deviceReadPositionsEndPtr != NULL) sycl::free(deviceReadPositionsEndPtr, q);
        if (deviceReadPositionsEpochA != NULL) sycl::free(deviceReadPositionsEpochA, q);
        if (deviceReadPositionsEpochB != NULL) sycl::free(deviceReadPositionsEpochB, q);
        if (deviceReadPositionsA != NULL) sycl::free(deviceReadPositionsA, q);
        if (deviceReadPositionsB != NULL) sycl::free(deviceReadPositionsB, q);

        if (deviceOutputEndPtr != NULL) sycl::free(deviceOutputEndPtr, q);
        if (deviceOutputEpoch != NULL) sycl::free(deviceOutputEpoch, q);
        if (deviceOutputIteration != NULL) sycl::free(deviceOutputIteration, q);
        if (deviceOutput != NULL) sycl::free(deviceOutput, q);

    }
    */
}