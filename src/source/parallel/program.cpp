#include "parallel/program.hpp"
#include "parallel/parameters.hpp"
#include <algorithm>

void organisation::parallel::program::reset(::parallel::device &dev, 
                                            ::parallel::queue *q, 
                                            ::parallel::mapper::configuration &mapper,
                                            parameters settings)
{
    init = false; cleanup();

    this->dev = &dev;
    this->queue = q;
    this->settings = settings;
    
    sycl::queue &qt = ::parallel::queue(dev).get();
    
    // this settings.max_values * settings.clients is invalid, based
    // on assumed uniform structure with client[0] client[1] in order
    devicePositions = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients, qt);
    if(devicePositions == NULL) return;

    deviceNextPositions = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients, qt);
    if(deviceNextPositions == NULL) return;

    deviceNextHalfPositions = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients, qt);
    if(deviceNextHalfPositions == NULL) return;

    deviceValues = sycl::malloc_device<int>(settings.max_values * settings.clients, qt);
    if(deviceValues == NULL) return;
    
    deviceNextDirections = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients, qt);
    if(deviceNextDirections == NULL) return;

    deviceMovementIdx = sycl::malloc_device<int>(settings.max_values * settings.clients, qt);
    if(deviceMovementIdx == NULL) return;

    deviceClient = sycl::malloc_device<int>(settings.max_values * settings.clients, qt);
    if(deviceClient == NULL) return;

    // ***

    deviceMovements = sycl::malloc_device<sycl::float4>(settings.max_movements * settings.clients, qt);
    if(deviceMovements == NULL) return;

    deviceCollisions = sycl::malloc_device<sycl::float4>(settings.max_collisions * settings.clients, qt);
    if(deviceCollisions == NULL) return;

    deviceMovementsCounts = sycl::malloc_device<int>(settings.clients, qt);
    if(deviceMovementsCounts == NULL) return;

    // ***
    
    hostPositions = sycl::malloc_host<sycl::float4>(settings.max_values * settings.host_buffer, qt);
    if(hostPositions == NULL) return;

    hostValues = sycl::malloc_host<int>(settings.max_values * settings.host_buffer, qt);
    if(hostValues == NULL) return;
    
    hostMovements = sycl::malloc_host<sycl::float4>(settings.max_movements * settings.host_buffer, qt);
    if(hostMovements == NULL) return;

    hostMovementsCounts = sycl::malloc_host<int>(settings.host_buffer, qt);
    if(hostMovementsCounts == NULL) return;

    hostCollisions = sycl::malloc_host<sycl::float4>(settings.max_collisions * settings.host_buffer, qt);
    if(hostCollisions == NULL) return;

    hostClient = sycl::malloc_host<int>(settings.max_values * settings.host_buffer, qt);
    if(hostClient == NULL) return;

    // ***

    ::parallel::parameters global(settings.width * settings.clients, settings.height, settings.depth);
    ::parallel::parameters client(settings.width, settings.height, settings.depth);

    //impacter = new ::parallel::mapper::map(dev, client, global, mapper);
	//if (impacter == NULL) return;	
	//if (!impacter->initalised()) return;

    inserter = new inserts(dev, q, settings, 100);
    if(inserter == NULL) return;
    if(!inserter->initalised()) return;

    // ***

    clear();

    init = true;
}

void organisation::parallel::program::clear()
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);

    std::vector<sycl::event> events;

    events.push_back(qt.memset(devicePositions, 0, sizeof(sycl::float4) * settings.max_values * settings.clients));
    events.push_back(qt.memset(deviceNextPositions, 0, sizeof(sycl::float4) * settings.max_values * settings.clients));
    events.push_back(qt.memset(deviceNextHalfPositions, 0, sizeof(sycl::float4) * settings.max_values * settings.clients));
    events.push_back(qt.memset(deviceValues, 0, sizeof(int) * settings.max_values * settings.clients));
    events.push_back(qt.memset(deviceNextDirections, 0, sizeof(sycl::float4) * settings.max_values * settings.clients));
    events.push_back(qt.memset(deviceMovementIdx, 0, sizeof(int) * settings.max_values * settings.clients));
    events.push_back(qt.memset(deviceClient, 0, sizeof(int) * settings.max_values * settings.clients));
    events.push_back(qt.memset(deviceMovements, 0, sizeof(sycl::float4) * settings.max_movements * settings.clients));
    events.push_back(qt.memset(deviceMovementsCounts, 0, sizeof(int) * settings.clients));
    events.push_back(qt.memset(deviceCollisions, 0, sizeof(sycl::float4) * settings.max_collisions * settings.clients));

    inserter->clear();

    totalValues = 0;

    sycl::event::wait(events);
}

void organisation::parallel::program::run(organisation::data &mappings)
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)(settings.max_values * settings.clients)};

    for(int epoch = 0; epoch < settings.epochs(); ++epoch)
    {                
        // ***
        inserter->clear();
        // *** 

        int iterations = 0;
        while(iterations++ < settings.iterations)
        {

            std::cout << "iterations " << iterations << " epoch " << epoch << "\r\n";
            std::cout << "movementsIdx ";
outputarb(deviceMovementIdx,settings.max_values * settings.clients);
std::cout << "positions ";
outputarb(devicePositions,settings.max_values * settings.clients);
std::cout << "values ";
outputarb(deviceValues,settings.max_values * settings.clients);
std::cout << "clients ";
outputarb(deviceClient,settings.max_values * settings.clients);


            int count = inserter->insert(epoch);

            if(count + totalValues >= settings.max_values * settings.clients)
                count = (count + totalValues) - (settings.max_values * settings.clients);

            if(count > 0)
            {
                // ***
                // need to do some collision detection for inserted values, before
                // copying them into the main population!
                // ***
                std::vector<sycl::event> events;

std::cout << "inserters values ";
outputarb(inserter->deviceNewValues, sizeof(int) * count);

std::cout << "input data ";
outputarb(inserter->deviceInputData, settings.max_input_data * settings.epochs());

                events.push_back(qt.memcpy(&devicePositions[totalValues], inserter->deviceNewPositions, sizeof(sycl::float4) * count));
                events.push_back(qt.memcpy(&deviceValues[totalValues], inserter->deviceNewValues, sizeof(int) * count));
                events.push_back(qt.memcpy(&deviceClient[totalValues], inserter->deviceNewClient, sizeof(int) * count));

                sycl::event::wait(events);

                totalValues += count;                
            }
            
            // ***
            // compute next positions and next half positions here
            // ***

            // update deviceClient to just an integer!
            // implement deviceSearchIndices int2
            // change impacter to have int client number

            //impacter->build(deviceNextPosition, deviceClient, settings.max_values * settings.clients, q);
	        //impacter->search(deviceNextPosition, deviceClient, deviceSearchIndices, settings.max_values * settings.clients, true, true, false, NULL, 0, q);
	        //impacter->search(deviceNextHalfPosition, deviceClient, deviceSearchIndices, settings.max_values * settings.clients, true, true, false, NULL, 0, q);		

            next();
            positions();
            update();

            // ***************
            // update halfPositions
            // check to see if data cells have gone out of boundaries
            // shuffle up cells to be continious
            // reduce dataTotalValues by number removed
            // (do I need to do this...just flag them as "ignore"??)

            // ****
            // if CELL out of boundaries
            // mark as invisible (w value???)
            // and then ignore for rest of iterations
            // rather than removing and updating totalValues???
            // ****
        };
    }    
}

void organisation::parallel::program::set(organisation::data &mappings, inputs::input &source)
{
    inserter->set(mappings, source);
}

std::vector<organisation::output> organisation::parallel::program::get(organisation::data &mappings)
{    
    std::vector<output> results(settings.clients);
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

void organisation::parallel::program::update()
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)totalValues};

    qt.submit([&](auto &h) 
    {
        auto _positions = devicePositions;     
        auto _nextDirections = deviceNextDirections;

        h.parallel_for(num_items, [=](auto i) 
        {  
            _positions[i] += _nextDirections[i];
        });
    }).wait();
}

void organisation::parallel::program::positions()
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)totalValues};

    qt.submit([&](auto &h) 
    {    
        auto _position = devicePositions;
        auto _nextPosition = deviceNextPositions;
        auto _nextHalfPosition = deviceNextHalfPositions;
        auto _nextDirection = deviceNextDirections;

        h.parallel_for(num_items, [=](auto i) 
        { 
            _nextPosition[i].x() = _position[i].x() + _nextDirection[i].x();
            _nextPosition[i].y() = _position[i].y() + _nextDirection[i].y();
            _nextPosition[i].z() = _position[i].z() + _nextDirection[i].z();

            _nextHalfPosition[i].x() = sycl::round(
                _position[i].x() + (_nextDirection[i].x() / 2.0f));
            _nextHalfPosition[i].y() = sycl::round(
                _position[i].y() + (_nextDirection[i].y() / 2.0f));
            _nextHalfPosition[i].z() = sycl::round(
                _position[i].z() + (_nextDirection[i].z() / 2.0f));
        });
    }).wait();        
}

void organisation::parallel::program::next()
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)totalValues};

    qt.submit([&](auto &h) 
    {
        auto _values = deviceValues;
        auto _positions = devicePositions;
        auto _client = deviceClient;
        auto _movementIdx = deviceMovementIdx;
        auto _movements = deviceMovements;
        auto _movementsCounts = deviceMovementsCounts;
        auto _nextDirections = deviceNextDirections;

        auto _max_movements = settings.max_movements;

        h.parallel_for(num_items, [=](auto i) 
        {  
            int client = _client[i];
        int a = _movementIdx[client];
            int offset = _max_movements * client;

            sycl::float4 direction = _movements[a + offset];
            _nextDirections[i] = direction;
            //_positions[i] += direction;

            _movementIdx[i]++;      
            int temp = _movementIdx[i];          
            //if((temp >= _max_movements)||(_movements[temp + offset].w() == -1))
            if((temp >= _max_movements)||(temp >= _movementsCounts[client]))
                _movementIdx[i] = 0;            
        });
    }).wait();
}

void organisation::parallel::program::copy(::organisation::schema **source, int source_size)
{
    memset(hostPositions, 0, sizeof(sycl::float4) * settings.max_values * settings.host_buffer);
    memset(hostValues, -1, sizeof(int) * settings.max_values * settings.host_buffer);
    memset(hostMovements, 0, sizeof(sycl::float4) * settings.max_movements * settings.host_buffer);
    memset(hostMovementsCounts, 0, sizeof(int) * settings.host_buffer);
    memset(hostCollisions, 0, sizeof(sycl::float4) * settings.max_collisions * settings.host_buffer);
    memset(hostClient, -1, sizeof(int) * settings.max_values * settings.host_buffer);

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)settings.clients};

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

            hostPositions[d_count + (index * settings.max_values)] = { (float)position.x, (float)position.y, (float)position.z, 0.0f };
            hostValues[d_count + (index * settings.max_values)] = value;

            ++d_count;
            if(d_count > settings.max_values) break;
        }

        totalValues = d_count;

        int m_count = 0;
        for(auto &it: prog->movement.directions)
        {            
            hostMovements[m_count + (index * settings.max_movements)] = { (float)it.x, (float)it.y, (float)it.z, 0.0f };
            ++m_count;
            if(m_count > settings.max_movements) break;            
        }
        hostMovementsCounts[index] = m_count;

        int c_count = 0;
        for(auto &it: prog->collisions.values)        
        {
            vector temp;
            temp.decode(it);

            hostCollisions[c_count + (index * settings.max_collisions)] = { (float)temp.x, (float)temp.y, (float)temp.z, 0.0f };
            ++c_count;
            if(c_count > settings.max_collisions) break;            
        }

        hostClient[index] = dest_index;

        ++index;
        if(index >= settings.host_buffer)
        {
            std::vector<sycl::event> events;

            events.push_back(qt.memcpy(&devicePositions[dest_index * settings.max_values], hostPositions, sizeof(sycl::float4) * settings.max_values * index));
            events.push_back(qt.memcpy(&deviceValues[dest_index * settings.max_values], hostValues, sizeof(int) * settings.max_values * index));
            events.push_back(qt.memcpy(&deviceMovements[dest_index * settings.max_movements], hostMovements, sizeof(int) * settings.max_movements * index));
            events.push_back(qt.memcpy(&deviceMovementsCounts[dest_index], hostMovementsCounts, sizeof(int) * index));
            events.push_back(qt.memcpy(&deviceCollisions[dest_index * settings.max_collisions], hostCollisions, sizeof(int) * settings.max_collisions * index));
            events.push_back(qt.memcpy(&deviceClient[dest_index], hostClient, sizeof(int) * index));

            memset(hostPositions, 0, sizeof(sycl::float4) * settings.max_values * settings.host_buffer);
            memset(hostValues, -1, sizeof(int) * settings.max_values * settings.host_buffer);
            memset(hostMovements, 0, sizeof(sycl::float4) * settings.max_movements * settings.host_buffer);
            memset(hostMovementsCounts, 0, sizeof(int) * settings.host_buffer);
            memset(hostCollisions, 0, sizeof(sycl::float4) * settings.max_collisions * settings.host_buffer);
            memset(hostClient, -1, sizeof(int) * settings.max_values * settings.host_buffer);

            sycl::event::wait(events);

            dest_index += settings.host_buffer;
            index = 0;            
        }
    }

    if(index > 0)
    {
        std::vector<sycl::event> events;

        events.push_back(qt.memcpy(&devicePositions[dest_index * settings.max_values], hostPositions, sizeof(sycl::float4) * settings.max_values* index));
        events.push_back(qt.memcpy(&deviceValues[dest_index * settings.max_values], hostValues, sizeof(int) * settings.max_values * index));
        events.push_back(qt.memcpy(&deviceMovements[dest_index * settings.max_movements], hostMovements, sizeof(int) * settings.max_movements * index));
        events.push_back(qt.memcpy(&deviceMovementsCounts[dest_index], hostMovementsCounts, sizeof(int) * index));
        events.push_back(qt.memcpy(&deviceCollisions[dest_index * settings.max_collisions], hostCollisions, sizeof(int) * settings.max_collisions * index));
        events.push_back(qt.memcpy(&deviceClient[dest_index], hostClient, sizeof(int) * index));

        sycl::event::wait(events);
    }    

std::cout << "movements ";
outputarb(deviceMovements,settings.max_movements * settings.clients);
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
    deviceNextDirections = NULL;
    
    deviceMovementIdx = NULL;
    deviceClient = NULL;

    deviceMovements = NULL;
    deviceMovementsCounts = NULL;
    deviceCollisions = NULL;

    hostPositions = NULL;
    hostValues = NULL;
    hostMovements = NULL;
    hostMovementsCounts = NULL;
    hostCollisions = NULL;
    hostClient = NULL; 

    impacter = NULL;
    inserter = NULL;
}

void organisation::parallel::program::cleanup()
{
    if(dev != NULL) 
    {   
        sycl::queue q = ::parallel::queue(*dev).get();

        if(inserter != NULL) delete inserter;
        if(impacter != NULL) delete impacter;

        if(hostClient != NULL) sycl::free(hostClient, q);
        if(hostCollisions != NULL) sycl::free(hostCollisions, q);
        if(hostMovementsCounts != NULL) sycl::free(hostMovementsCounts, q);
        if(hostMovements != NULL) sycl::free(hostMovements, q);
        if(hostValues != NULL) sycl::free(hostValues, q);
        if(hostPositions != NULL) sycl::free(hostPositions, q);

        if(deviceCollisions != NULL) sycl::free(deviceCollisions, q);
        if(deviceMovementsCounts != NULL) sycl::free(deviceMovementsCounts, q);
        if(deviceMovements != NULL) sycl::free(deviceMovements, q);
        
        if(deviceClient != NULL) sycl::free(deviceClient, q);
        if(deviceMovementIdx != NULL) sycl::free(deviceMovementIdx, q);
        if(deviceValues != NULL) sycl::free(deviceValues, q);

        if(deviceNextDirections != NULL) sycl::free(deviceNextDirections, q);
        if(deviceNextHalfPositions != NULL) sycl::free(deviceNextHalfPositions, q);
        if(deviceNextPositions != NULL) sycl::free(deviceNextPositions, q);
        if(devicePositions != NULL) sycl::free(devicePositions, q);
    }    
}