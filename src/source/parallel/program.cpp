#include "parallel/program.hpp"
#include <algorithm>

void organisation::parallel::program::reset(::parallel::device &dev, parameters settings, int clients)
{
    init = false; cleanup();

    this->dev = &dev;
    this->params = settings;

    sycl::queue q = ::parallel::queue(dev).get();

    this->clients = clients;
    this->length = settings.size() * clients;

    deviceValues = sycl::malloc_device<int>(length, q);
    if(deviceValues == NULL) return;
    
    /*
    deviceInGates = sycl::malloc_device<int>(settings.size() * settings.in * clients, q);
    if(deviceInGates == NULL) return;

    deviceOutGates = sycl::malloc_device<int>(settings.size() * settings.in * settings.out * clients, q);
    if(deviceOutGates == NULL) return;

    deviceMagnitudes = sycl::malloc_device<int>(settings.size() * settings.in * settings.out * clients, q);
    if(deviceMagnitudes == NULL) return;
    */
    // ***

    hostValues = sycl::malloc_host<int>(settings.size() * HOST_BUFFER, q);
    if(hostValues == NULL) return;
    
    /*
    hostInGates = sycl::malloc_host<int>(HOST_BUFFER * settings.size() * settings.in, q);
    if(hostInGates == NULL) return;

    hostOutGates = sycl::malloc_host<int>(HOST_BUFFER * settings.size() * settings.in * settings.out, q);
    if(hostOutGates == NULL) return;

    hostMagnitudes = sycl::malloc_host<int>(HOST_BUFFER * settings.size() * settings.in * settings.out, q);
    if(hostMagnitudes == NULL) return;
    */
    // ***

    deviceOutput = sycl::malloc_device<int>(length, q);
    if(deviceOutput == NULL) return;

    deviceOutputIteration = sycl::malloc_device<int>(length, q);
    if(deviceOutputIteration == NULL) return;

    deviceOutputEpoch = sycl::malloc_device<int>(length, q);
    if(deviceOutputEpoch == NULL) return;

    deviceOutputEndPtr = sycl::malloc_device<int>(clients, q);
    if(deviceOutputEndPtr == NULL) return;

    deviceReadPositionsA = sycl::malloc_device<sycl::float4>(length, q);
    if(deviceReadPositionsA == NULL) return;

    deviceReadPositionsB = sycl::malloc_device<sycl::float4>(length, q);
    if(deviceReadPositionsB == NULL) return;

    deviceReadPositionsEpochA = sycl::malloc_device<int>(length, q);
    if(deviceReadPositionsEpochA == NULL) return;

    deviceReadPositionsEpochB = sycl::malloc_device<int>(length, q);
    if(deviceReadPositionsEpochB == NULL) return;

    deviceReadPositionsEndPtr = sycl::malloc_device<int>(clients, q);
    if(deviceReadPositionsEndPtr == NULL) return;

    // ***

    deviceSourceReadPositions = sycl::malloc_device<sycl::float4>(settings.epochs, q);
    if(deviceSourceReadPositions == NULL) return;

    hostSourceReadPositions = sycl::malloc_host<sycl::float4>(settings.epochs, q);
    if(hostSourceReadPositions == NULL) return;

    // ***

    hostOutput = sycl::malloc_host<int>(length, q);
    if(hostOutput == NULL) return;

    hostOutputIteration = sycl::malloc_host<int>(length, q);
    if(hostOutputIteration == NULL) return;

    hostOutputEpoch = sycl::malloc_host<int>(length, q);
    if(hostOutputEpoch == NULL) return;

    hostOutputEndPtr = sycl::malloc_host<int>(clients, q);
    if(hostOutputEndPtr == NULL) return;

    init = true;
}

void organisation::parallel::program::clear(::parallel::queue *q)
{
    /*
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, q);

    auto msA = qt.memset(deviceOutputEndPtr, 0, sizeof(int) * clients);
    auto msB = qt.memset(deviceReadPositionsEndPtr, 0, sizeof(int) * clients);
    
    auto msC = qt.memset(deviceValues, -1, sizeof(int) * length);
    auto msD = qt.memset(deviceOutput, 0, sizeof(int) * length);
    auto msE = qt.memset(deviceOutputIteration, 0, sizeof(int) * length);
    auto msF = qt.memset(deviceReadPositionsA, 0, sizeof(sycl::float4) * length);
    auto msG = qt.memset(deviceReadPositionsB, 0, sizeof(sycl::float4) * length);
    auto msH = qt.memset(deviceReadPositionsEpochA, 0, sizeof(int) * length);
    auto msI = qt.memset(deviceReadPositionsEpochB, 0, sizeof(int) * length);
    
    msA.wait();
    msB.wait();
    msC.wait();
    msD.wait();
    msE.wait();
    msF.wait();
    msG.wait();
    msH.wait();
    msI.wait();    
    */
}

void organisation::parallel::program::run(::parallel::queue *q)
{/*
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

void organisation::parallel::program::set(std::vector<sycl::float4> positions, ::parallel::queue *q)
{
    /*
    if(positions.size() > params.epochs) { std::cout << "set fail\r\n"; return; }

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, q);
    sycl::range num_items{(size_t)clients};

    memcpy(hostSourceReadPositions, positions.data(), sizeof(sycl::float4) * params.epochs);
    qt.memcpy(deviceSourceReadPositions, hostSourceReadPositions, sizeof(sycl::float4) * params.epochs).wait();

     qt.submit([&](auto &h) 
    {        
        auto _readPositionsSource = deviceSourceReadPositions;
        auto _readPositions = deviceReadPositionsA;
        auto _readPositionsEpoch = deviceReadPositionsEpochA;
        auto _readPositionsEndPtr = deviceReadPositionsEndPtr;
        
        auto _epochs = params.epochs;
        auto _dimLength = params.size();
        auto _length = length;

        h.parallel_for(num_items, [=](auto i) 
        {
            for(int j = 0; j < _epochs; ++j)
            {
                _readPositions[(i * _dimLength) + j] = _readPositionsSource[j];
                _readPositionsEpoch[(i * _dimLength) + j] = j + 1;
            }
            _readPositionsEndPtr[i] = _epochs;
        });
    }).wait();
    */
}

std::vector<organisation::parallel::output> organisation::parallel::program::get(organisation::data &mappings, ::parallel::queue *q)
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

void organisation::parallel::program::copy(::organisation::schema **source, int source_size, ::parallel::queue *q)
{
    /*
    memset(hostValues, -1, sizeof(int) * params.size() * HOST_BUFFER);
    memset(hostInGates, -1, sizeof(int) * params.size() * params.in * HOST_BUFFER);
    memset(hostOutGates, -1, sizeof(int) * params.size() * params.in * params.out * HOST_BUFFER);
    memset(hostMagnitudes, -1, sizeof(int) * params.size() * params.in * params.out * HOST_BUFFER);

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, q);
    sycl::range num_items{(size_t)clients};

    int dest_index = 0;
    int index = 0;
    
    for(int source_index = 0; source_index < source_size; ++source_index)
    {
        organisation::program *prog = &source[source_index]->prog;

        int length = prog->length;

        if(length != params.size()) 
            break;

        for(int i = 0; i < length; ++i)
        {
            hostValues[(index * length) + i] = prog->cells.at(i).value;

            std::vector<int> in = prog->cells.at(i).pull();
            int in_idx = 0;
            for(std::vector<int>::iterator jt = in.begin(); jt < in.end(); ++jt)
            {
                if(in_idx < params.in)
                {                    
                    int inIndex = (index * length * params.in) + (i * params.in);
                    hostInGates[inIndex + in_idx] = *jt;

                    std::vector<int> out = prog->cells.at(i).pull(*jt);
                    int out_idx = 0;
                    for(std::vector<int>::iterator ot = out.begin(); ot < out.end(); ++ot)
                    {
                        if(out_idx < params.out)
                        {
                            int s = (index * length * params.in * params.out);
                            s += (i * params.out * params.in);
                            s += (out_idx * params.in) + in_idx;
                            
                            hostOutGates[s] = *ot;
                            hostMagnitudes[s] = prog->cells.at(i).get(*jt,*ot).magnitude;
                        }

                        ++out_idx;
                    }
                }
                ++in_idx;
            }
        }

        ++index;
        if(index >= HOST_BUFFER)
        {
            auto mcA = qt.memcpy(&deviceValues[dest_index * length], hostValues, sizeof(int) * length * index);
            auto mcB = qt.memcpy(&deviceInGates[dest_index * params.in * length], hostInGates, sizeof(int) * params.in * length * index);
            auto mcC = qt.memcpy(&deviceOutGates[dest_index * params.in * params.out * length], hostOutGates, sizeof(int) * params.in * params.out * length * index);
            auto mcD = qt.memcpy(&deviceMagnitudes[dest_index * params.in * params.out * length], hostMagnitudes, sizeof(int) * params.in * params.out * length * index);

            mcA.wait();
            mcB.wait();
            mcC.wait();
            mcD.wait();

            memset(hostValues, -1, sizeof(int) * params.size() * HOST_BUFFER);
            memset(hostInGates, -1, sizeof(int) * params.size() * params.in * HOST_BUFFER);
            memset(hostOutGates, -1, sizeof(int) * params.size() * params.in * params.out * HOST_BUFFER);
            memset(hostMagnitudes, -1, sizeof(int) * params.size() * params.in * params.out * HOST_BUFFER);

            dest_index += HOST_BUFFER;
            index = 0;            
        }
    }

    if(index > 0)
    {
        int length = params.size();

        auto mcA = qt.memcpy(&deviceValues[dest_index * length], hostValues, sizeof(int) * length * index);
        auto mcB = qt.memcpy(&deviceInGates[dest_index * params.in * length], hostInGates, sizeof(int) * params.in * length * index);
        auto mcC = qt.memcpy(&deviceOutGates[dest_index * params.in * params.out * length], hostOutGates, sizeof(int) * params.in * params.out * length * index);
        auto mcD = qt.memcpy(&deviceMagnitudes[dest_index * params.in * params.out * length], hostMagnitudes, sizeof(int) * params.in * params.out * length * index);
        
        mcA.wait();
        mcB.wait();
        mcC.wait();
        mcD.wait();
    }
    */
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
    /*
    dev = NULL;

    deviceValues = NULL;
    deviceInGates = NULL;
    deviceOutGates = NULL;
    deviceMagnitudes = NULL;
    
    hostValues = NULL;
    hostInGates = NULL;
    hostOutGates = NULL;
    hostMagnitudes = NULL;

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

        if (hostMagnitudes != NULL) sycl::free(hostMagnitudes, q);
        if (hostOutGates != NULL) sycl::free(hostOutGates, q);
        if (hostInGates != NULL) sycl::free(hostInGates, q);
        if (hostValues != NULL) sycl::free(hostValues, q);

        if (deviceMagnitudes != NULL) sycl::free(deviceMagnitudes, q);
        if (deviceOutGates != NULL) sycl::free(deviceOutGates, q);
        if (deviceInGates != NULL) sycl::free(deviceInGates, q);
        if (deviceValues != NULL) sycl::free(deviceValues, q);
    }
    */
}