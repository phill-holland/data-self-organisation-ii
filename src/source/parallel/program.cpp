#include "parallel/program.hpp"
#include "parallel/parameters.hpp"
#include <algorithm>

int GetCollidedKey(const sycl::float4 a, const sycl::float4 b)
{
        sycl::float4 direction;

        direction.x() = b.x() - a.x(); direction.y() = b.y() - a.y();
            direction.z() = b.z() - a.z();

        if ((direction.x() == 0) && (direction.y() == 0) &&
            (direction.z() == 0)) return 0;

        if (direction.x() < -1.0f) direction.x() = -1.0f;
        if (direction.x() > 1.0f) direction.x() = 1.0f;

        if (direction.y() < -1.0f) direction.y() = -1.0f;
        if (direction.y() > 1.0f) direction.y() = 1.0f;

        if (direction.z() < -1.0f) direction.z() = -1.0f;
        if (direction.z() > 1.0f) direction.z() = 1.0f;

        long x = ((long)(direction.x() + 1.0f));
        long y = ((long)(direction.y() + 1.0f));
        long z = ((long)(direction.z() + 1.0f));

        return (sycl::abs(z) * (3L * 3L)) + (sycl::abs(y) * 3L) + sycl::abs(x);
}

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
//#warning change devicePoistions to just be total array size, not by client, but for all clients
//settings.max_values * settings.clients()
    devicePositions = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients(), qt);
    if(devicePositions == NULL) return;

    deviceNextPositions = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients(), qt);
    if(deviceNextPositions == NULL) return;

    deviceNextHalfPositions = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients(), qt);
    if(deviceNextHalfPositions == NULL) return;

    deviceValues = sycl::malloc_device<int>(settings.max_values * settings.clients(), qt);
    if(deviceValues == NULL) return;
    
    deviceNextDirections = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients(), qt);
    if(deviceNextDirections == NULL) return;

    deviceMovementIdx = sycl::malloc_device<int>(settings.max_values * settings.clients(), qt);
    if(deviceMovementIdx == NULL) return;

    deviceClient = sycl::malloc_device<sycl::int4>(settings.max_values * settings.clients(), qt);
    if(deviceClient == NULL) return;

    // ***

    deviceCachePositions = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients(), qt);
    if(deviceCachePositions == NULL) return;

    deviceCacheValues = sycl::malloc_device<int>(settings.max_values * settings.clients(), qt);
    if(deviceCacheValues == NULL) return;

    deviceMovements = sycl::malloc_device<sycl::float4>(settings.max_movements * settings.clients(), qt);
    if(deviceMovements == NULL) return;

    deviceMovementsCounts = sycl::malloc_device<int>(settings.clients(), qt);
    if(deviceMovementsCounts == NULL) return;

    deviceCollisions = sycl::malloc_device<sycl::float4>(settings.max_collisions * settings.clients(), qt);
    if(deviceCollisions == NULL) return;

    deviceCollisionKeys = sycl::malloc_device<sycl::int2>(settings.max_values * settings.clients(), qt);
    if(deviceCollisionKeys == NULL) return;

    // ***
    
    hostCachePositions = sycl::malloc_host<sycl::float4>(settings.max_values * settings.host_buffer, qt);
    if(hostCachePositions == NULL) return;

    hostCacheValues = sycl::malloc_host<int>(settings.max_values * settings.host_buffer, qt);
    if(hostCacheValues == NULL) return;
    
    hostMovements = sycl::malloc_host<sycl::float4>(settings.max_movements * settings.host_buffer, qt);
    if(hostMovements == NULL) return;

    hostMovementsCounts = sycl::malloc_host<int>(settings.host_buffer, qt);
    if(hostMovementsCounts == NULL) return;

    hostCollisions = sycl::malloc_host<sycl::float4>(settings.max_collisions * settings.host_buffer, qt);
    if(hostCollisions == NULL) return;

    hostClient = sycl::malloc_host<sycl::int4>(settings.max_values * settings.host_buffer, qt);
    if(hostClient == NULL) return;

    // ***
    deviceOutputValues = sycl::malloc_device<int>(settings.max_values, qt);
    if(deviceOutputValues == NULL) return;
    
    deviceOutputIndex = sycl::malloc_device<int>(settings.max_values, qt);
    if(deviceOutputIndex == NULL) return;
    
    deviceOutputClient = sycl::malloc_device<sycl::float4>(settings.max_values, qt);
    if(deviceOutputClient == NULL) return;

    deviceOutputTotalValues = sycl::malloc_device<int>(1, qt);
    if(deviceOutputTotalValues == NULL) return;


    hostOutputValues = sycl::malloc_host<int>(settings.max_values, qt);
    if(deviceOutputValues == NULL) return;
    
    hostOutputIndex = sycl::malloc_host<int>(settings.max_values, qt);
    if(hostOutputIndex == NULL) return;
    
    hostOutputClient = sycl::malloc_host<sycl::float4>(settings.max_values, qt);
    if(hostOutputClient == NULL) return;

    hostOutputTotalValues = sycl::malloc_host<int>(1, qt);
    if(hostOutputTotalValues == NULL) return;
    // ***

    deviceTotalValues = sycl::malloc_device<int>(1, qt);
    if(deviceTotalValues == NULL) return;

    hostTotalValues = sycl::malloc_host<int>(1, qt);
    if(hostTotalValues == NULL) return;

    // ***
    deviceNewPositions = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients(), qt);
    if(deviceNewPositions == NULL) return;

    deviceNewValues = sycl::malloc_device<int>(settings.max_values * settings.clients(), qt);
    if(deviceNewValues == NULL) return;

    deviceNewClient = sycl::malloc_device<sycl::int4>(settings.max_values * settings.clients(), qt);
    if(deviceNewClient == NULL) return;

    deviceNewNextDirections = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients(), qt);
    if(deviceNewNextDirections == NULL) return;

    deviceNewMovementIdx = sycl::malloc_device<int>(settings.max_values * settings.clients(), qt);
    if(deviceNewMovementIdx == NULL) return;


    // ***
    deviceOldPositions = sycl::malloc_device<sycl::float4>(settings.max_values * settings.clients(), qt);
    if(deviceOldPositions == NULL) return;

    deviceOldUpdateCounter = sycl::malloc_device<int>(1, qt);
    if(deviceOldUpdateCounter == NULL) return;

    hostOldUpdateCounter = sycl::malloc_host<int>(1, qt);
    if(hostOldUpdateCounter == NULL) return;

    // ***

    ::parallel::parameters global(settings.width * settings.clients(), settings.height, settings.depth);
    global.length = settings.max_values * settings.clients(); // global length seems to be the only variable used in this class!
    ::parallel::parameters client(settings.width, settings.height, settings.depth);
    client.length = settings.max_values;

    impacter = new ::parallel::mapper::map(dev, client, global, mapper);
	if (impacter == NULL) return;	
    if (!impacter->initalised()) return;

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

    events.push_back(qt.memset(devicePositions, 0, sizeof(sycl::float4) * settings.max_values * settings.clients()));
    events.push_back(qt.memset(deviceNextPositions, 0, sizeof(sycl::float4) * settings.max_values * settings.clients()));
    events.push_back(qt.memset(deviceNextHalfPositions, 0, sizeof(sycl::float4) * settings.max_values * settings.clients()));
    events.push_back(qt.memset(deviceValues, 0, sizeof(int) * settings.max_values * settings.clients()));
    events.push_back(qt.memset(deviceNextDirections, 0, sizeof(sycl::float4) * settings.max_values * settings.clients()));
    events.push_back(qt.memset(deviceMovementIdx, 0, sizeof(int) * settings.max_values * settings.clients()));
    events.push_back(qt.memset(deviceClient, 0, sizeof(sycl::int4) * settings.max_values * settings.clients()));
    events.push_back(qt.memset(deviceCachePositions, 0, sizeof(sycl::float4) * settings.max_values * settings.clients()));
    events.push_back(qt.memset(deviceCacheValues, 0, sizeof(int) * settings.max_values * settings.clients()));
    events.push_back(qt.memset(deviceMovements, 0, sizeof(sycl::float4) * settings.max_movements * settings.clients()));
    events.push_back(qt.memset(deviceMovementsCounts, 0, sizeof(int) * settings.clients()));
    events.push_back(qt.memset(deviceCollisions, 0, sizeof(sycl::float4) * settings.max_collisions * settings.clients()));

    inserter->clear();

    totalCacheValues = 0;
    totalOutputValues = 0;
    totalValues = 0;

    outputs.clear();

    sycl::event::wait(events);
}

void organisation::parallel::program::restart()
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);

    std::vector<sycl::event> events1, events2;

    events1.push_back(qt.memset(devicePositions, 0, sizeof(sycl::float4) * settings.max_values * settings.clients()));
    events1.push_back(qt.memset(deviceNextPositions, 0, sizeof(sycl::float4) * settings.max_values * settings.clients()));
    events1.push_back(qt.memset(deviceNextHalfPositions, 0, sizeof(sycl::float4) * settings.max_values * settings.clients()));
    events1.push_back(qt.memset(deviceValues, 0, sizeof(int) * settings.max_values * settings.clients()));
    events1.push_back(qt.memset(deviceNextDirections, 0, sizeof(sycl::float4) * settings.max_values * settings.clients()));
    events1.push_back(qt.memset(deviceMovementIdx, 0, sizeof(int) * settings.max_values * settings.clients()));
    events1.push_back(qt.memset(deviceClient, 0, sizeof(sycl::int4) * settings.max_values * settings.clients()));

    inserter->clear();

    sycl::event::wait(events1);

    events2.push_back(qt.memcpy(devicePositions, deviceCachePositions, sizeof(sycl::float4) * settings.max_values * settings.clients()));
    events2.push_back(qt.memcpy(deviceValues, deviceCacheValues, sizeof(int) * settings.max_values * settings.clients()));

    sycl::event::wait(events2);

    totalOutputValues = 0;
    totalValues = totalCacheValues;
}

void organisation::parallel::program::run(organisation::data &mappings)
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)(settings.max_values * settings.clients())};

    outputs.clear();

    for(int epoch = 0; epoch < settings.epochs(); ++epoch)
    {     
        restart();           

        int iterations = 0;
        while(iterations++ < settings.iterations)
        {
        
            std::cout << "iterations " << iterations << " epoch " << epoch << " totalValues " << totalValues << "\r\n";
            //std::cout << "movementsIdx ";
            //outputarb(deviceMovementIdx,totalValues);//settings.max_values * settings.clients());
            std::cout << "positions ";
            outputarb(devicePositions,totalValues);//settings.max_values * settings.clients());
            std::cout << "values ";
            outputarb(deviceValues,totalValues);//settings.max_values * settings.clients());
            std::cout << "clients ";
            outputarb(deviceClient,totalValues);
            //std::cout << "clients ";
            //outputarb(deviceClient,totalValues);//settings.max_values * settings.clients());


            //std::cout << "deviceInserts ";
            //outputarb(inserter->deviceInserts, settings.max_inserts * settings.clients());
            //std::cout << "InputIdx ";
            //outputarb(inserter->deviceInputIdx, settings.clients());

            insert(epoch);


            // ***
            qt.memcpy(deviceOldPositions, devicePositions, sizeof(sycl::float4) * totalValues).wait();                            
            // ***

            positions();

            //std::cout << "next positions ";
//outputarb(deviceNextPositions,totalValues);//settings.max_values * settings.clients());

//std::cout << "next half positions ";
//outputarb(deviceNextHalfPositions,totalValues);//settings.max_values * settings.clients());

            //std::cout << "old next ";
            //outputarb(deviceNextDirections, totalValues);//settings.max_values * settings.clients());

            qt.memset(deviceCollisionKeys, 0, sizeof(sycl::int2) * totalValues); /*settings.max_values * settings.clients());*/

            impacter->build(deviceNextPositions, deviceClient, totalValues, queue);
	        impacter->search(deviceNextPositions, deviceClient, deviceCollisionKeys, totalValues, true, false, false, NULL, 0, queue);
	        
            impacter->build(deviceNextHalfPositions, deviceClient, totalValues, queue);
            impacter->search(deviceNextHalfPositions, deviceClient, deviceCollisionKeys, totalValues, true, false, false, NULL, 0, queue);		

            std::cout << "collision keys ";
            outputarb(deviceCollisionKeys, totalValues);//settings.max_values * settings.clients());

            update();
            next(iterations);
            
            // push next_position into temp_next_position instead
            // check temp_next_position collision with position
            // if(collision[i].x() > 1) {
            // point_b = collision[i].y();
            // if(temp_next_position[point_b] != position[point_b])
            // {
                // destination moved out of the way
            //  _position[i] = temp_next_position[i]
            // }
            //
            // ***

            // push next_position into temp_next_position instead
            // check temp_next_position
            // collision with itself
            // and 

            // if(temp_next_position[i] collides with position[i])
            // if collision[i].y() {
            // int idx_b = collision[i].y()
            // if(position[idx_b] == temp_next_position[i])
            // {
                // it's not moved
                // this point should not move
            // } 
            // else
            // {
                // move point new_position[i] = temp_next_position[i] 
            // }
            //}
            // if(position == temp_next_position)
            //  // it didn't move
            // 

            //std::cout << "new positions ";
//outputarb(devicePositions,totalValues);//settings.max_values * settings.clients());

            //std::cout << "new next ";
            //outputarb(deviceNextDirections, totalValues);//settings.max_values * settings.clients());

corrections();
boundaries();

            std::cout << "\r\n\r\n";
        };

        std::cout << "TOTAL OUTPUTS " << totalOutputValues << "\r\n";
        move(mappings);            
        
        qt.memset(deviceTotalValues, 0, sizeof(int)).wait();
        
    }    
}

void organisation::parallel::program::set(organisation::data &mappings, inputs::input &source)
{
    inserter->set(mappings, source);
}

std::vector<organisation::outputs::output> organisation::parallel::program::get(organisation::data &mappings)
{
    return outputs;    
}

void organisation::parallel::program::move(organisation::data &mappings)
{    
    if(totalOutputValues > 0)
    {
        sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);

        int output_length = totalOutputValues;
        if(output_length > settings.max_values) output_length = settings.max_values;

        std::vector<sycl::event> events;

        events.push_back(qt.memcpy(hostOutputValues, deviceOutputValues, sizeof(int) * output_length));
        events.push_back(qt.memcpy(hostOutputIndex, deviceOutputIndex, sizeof(int) * output_length));
        events.push_back(qt.memcpy(hostOutputClient, deviceOutputClient, sizeof(sycl::float4) * output_length));

        sycl::event::wait(events);

        outputs::output out;

        for(int i = 0; i < output_length; ++i)
        {
            outputs::data temp;

            temp.value = mappings.map(hostOutputValues[i]);
            temp.client = hostOutputClient[i].w();
            temp.index = hostOutputIndex[i];

            out.values.push_back(temp);
        }

        outputs.push_back(out);

        qt.memset(deviceOutputTotalValues, 0, sizeof(int)).wait();
        totalOutputValues = 0;
    }
}

void organisation::parallel::program::update()
{
    if(totalValues == 0) return;

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)totalValues};

    qt.submit([&](auto &h) 
    {
        auto _positions = devicePositions;     
        auto _nextDirections = deviceNextDirections;
        auto _collisionKeys = deviceCollisionKeys;

        h.parallel_for(num_items, [=](auto i) 
        {  
            if(_positions[i].w() == 0)
            {
                sycl::int2 key = _collisionKeys[i];
                if(key.x() == 0)
                    _positions[i] += _nextDirections[i];
            }
        });
    }).wait();
}

void organisation::parallel::program::positions()
{
    if(totalValues == 0) return;

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

void organisation::parallel::program::next(int iteration)
{
    if(totalValues == 0) return;

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)totalValues};

    hostOutputTotalValues[0] = totalOutputValues;
    qt.memcpy(deviceOutputTotalValues, hostOutputTotalValues, sizeof(int)).wait();

    qt.submit([&](auto &h) 
    {
        auto _values = deviceValues;
        auto _positions = devicePositions;   
        auto _nextPositions = deviceNextPositions; 
        auto _client = deviceClient;
        auto _movementIdx = deviceMovementIdx;
        auto _movements = deviceMovements;
        auto _movementsCounts = deviceMovementsCounts;
        auto _collisions = deviceCollisions;
        auto _nextDirections = deviceNextDirections;
        auto _collisionKeys = deviceCollisionKeys;

        auto _outputValues = deviceOutputValues;
        auto _outputIndex = deviceOutputIndex;
        auto _outputClient = deviceOutputClient;
        auto _outputTotalValues = deviceOutputTotalValues;

        auto _outputLength = settings.max_values;

        auto _iteration = iteration;

        auto _max_movements = settings.max_movements;
        auto _max_collisions = settings.max_collisions;

        h.parallel_for(num_items, [=](auto i) 
        {  
            if(_positions[i].w() == 0)
            {            
                int client = _client[i].w();
                int a = _movementIdx[client];
                int offset = _max_movements * client;

                sycl::int2 collision = _collisionKeys[i];
                if(collision.x() > 0)
                {
                    int key = GetCollidedKey(_positions[i], _nextPositions[i]);
                    sycl::float4 direction = _collisions[(client * _max_collisions) + key];                    
                    _nextDirections[i] = direction;

                    cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                    sycl::memory_scope::device, 
                    sycl::access::address_space::ext_intel_global_device_space> ar(_outputTotalValues[0]);

                    int idx = ar.fetch_add(1);

                    if(idx < _outputLength)
                    {
                        _outputValues[idx] = _values[collision.y()];
                        _outputIndex[idx] = _iteration;
                        _outputClient[idx] = _client[i];
                    }
                }
                else
                {
                    sycl::float4 direction = _movements[a + offset];
                    _nextDirections[i] = direction;            
            
                    _movementIdx[i]++;      
                    int temp = _movementIdx[i];          
                    
                    if((temp >= _max_movements)||(temp >= _movementsCounts[client]))
                        _movementIdx[i] = 0;            
                }
            }
        });
    }).wait();

    qt.memcpy(hostOutputTotalValues, deviceOutputTotalValues, sizeof(int)).wait();
    totalOutputValues = hostOutputTotalValues[0];
}

void organisation::parallel::program::insert(int epoch)
{
    int count = inserter->insert(epoch);

    std::cout << "insert count " << count << "\r\n";
    if(count + totalValues >= settings.max_values * settings.clients())
        count = (count + totalValues) - (settings.max_values * settings.clients());

    if(count > 0)
    {
        sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
        sycl::range num_items{(size_t)count};

        qt.memset(deviceCollisionKeys, 0, sizeof(sycl::int2) * totalValues);

        impacter->build(devicePositions, deviceClient, totalValues, queue, true);
        impacter->search(inserter->deviceNewPositions, inserter->deviceNewClient, deviceCollisionKeys, count, false, false, false, NULL, 0, queue, true);
std::cout << "total values before " << totalValues << "\r\n";            
        hostTotalValues[0] = totalValues;
        qt.memcpy(deviceTotalValues, hostTotalValues, sizeof(int)).wait();


            std::cout << "insert collision keys ";
            outputarb(deviceCollisionKeys, totalValues);//settings.max_values * settings.clients());

            std::cout << "insert devicePoistions ";
            outputarb(devicePositions, totalValues);

            std::cout << "inserter positions ";
            outputarb(inserter->deviceNewPositions,count);

            std::cout << "inserter values ";
            outputarb(inserter->deviceNewValues,count);

            std::cout << "inserter client ";
            outputarb(inserter->deviceNewClient,count);

        qt.submit([&](auto &h) 
        {        
            auto _positions = devicePositions; 
            auto _values = deviceValues;
            auto _client = deviceClient;
            
            auto _srcPosition = inserter->deviceNewPositions;
            auto _srcValues = inserter->deviceNewValues;
            auto _srcClient = inserter->deviceNewClient;
            
            auto _keys = deviceCollisionKeys;

            auto _valuesLength = settings.max_values * settings.clients();

            auto _totalValues = deviceTotalValues;

            h.parallel_for(num_items, [=](auto i) 
            {  
                if(_keys[i].x() == 0)
                {                
                    cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                                sycl::memory_scope::device, 
                                sycl::access::address_space::ext_intel_global_device_space> ar(_totalValues[0]);

                    int idx = ar.fetch_add(1);

                    if(idx < _valuesLength)
                    {
                        _positions[idx] = _srcPosition[i];
                        _values[idx] = _srcValues[i];
                        _client[idx] = _srcClient[i];
                    }
                }
            });
        }).wait();
        
        qt.memcpy(hostTotalValues, deviceTotalValues, sizeof(int)).wait();
        
        totalValues = hostTotalValues[0];
std::cout << "total values after " << totalValues << "\r\n";            
        if(totalValues > settings.max_values * settings.clients())
            totalValues = settings.max_values * settings.clients();
    }
}

void organisation::parallel::program::boundaries()
{
    if(totalValues == 0) return;

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)totalValues};

    qt.memset(deviceTotalValues, 0, sizeof(int)).wait();

    qt.submit([&](auto &h) 
    {        
        auto _positions = devicePositions; 
        auto _values = deviceValues;
        auto _client = deviceClient;
        auto _nextDirection = deviceNextDirections;
        auto _movementIdx = deviceMovementIdx;

        auto _newPositions = deviceNewPositions;
        auto _newValues = deviceNewValues;
        auto _newClient = deviceNewClient;
        auto _newNextDirection = deviceNewNextDirections;
        auto _newMovementIdx = deviceNewMovementIdx;
        
        auto _newTotalValues = deviceTotalValues;

        auto _width = settings.width;
        auto _height = settings.height;
        auto _depth = settings.depth;

        h.parallel_for(num_items, [=](auto i) 
        {  
            sycl::float4 temp = _positions[i];
            if((temp.x() > 0)&&(temp.x() < _width)&&
                (temp.y() > 0)&&(temp.y() < _height)&&
                (temp.z() > 0)&&(temp.z() < _depth))
            {
                cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                            sycl::memory_scope::device, 
                            sycl::access::address_space::ext_intel_global_device_space> ar(_newTotalValues[0]);

                int idx = ar.fetch_add(1);

                _newPositions[idx] = temp;
                _newValues[idx] = _values[i];
                _newClient[idx] = _client[i];
                _newNextDirection[idx] = _nextDirection[i];
                _newMovementIdx[idx] = _movementIdx[i];
            }
        });
    }).wait();

    qt.memcpy(hostTotalValues, deviceTotalValues, sizeof(int)).wait();
    int temp = hostTotalValues[0];

    if(temp != totalValues)
    {
        if(temp > 0)
        {
            std::cout << "BING BANG new totalValues " << temp << "\r\n";
            std::vector<sycl::event> events;

            events.push_back(qt.memcpy(devicePositions, deviceNewPositions, sizeof(sycl::float4) * temp));
            events.push_back(qt.memcpy(deviceValues, deviceNewValues, sizeof(int) * temp));
            events.push_back(qt.memcpy(deviceClient, deviceNewClient, sizeof(sycl::int4) * temp));
            events.push_back(qt.memcpy(deviceNextDirections, deviceNewNextDirections, sizeof(sycl::float4) * temp));
            events.push_back(qt.memcpy(deviceMovementIdx, deviceNewMovementIdx, sizeof(int) * temp));

            sycl::event::wait(events);
        }

        totalValues = temp;
    }
}

void organisation::parallel::program::corrections()
{
    if(totalValues == 0) return;

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)totalValues};

    //qt.memset(deviceCollisionKeys, 0, sizeof(sycl::int2) * totalValues);
    //qt.memset(deviceOldUpdateCounter, 0, sizeof(int)).wait();

    int counter = 0;

    do
    {
        qt.memset(deviceCollisionKeys, 0, sizeof(sycl::int2) * totalValues);
        qt.memset(deviceOldUpdateCounter, 0, sizeof(int)).wait();

        impacter->build(devicePositions, deviceClient, totalValues, queue);
        impacter->search(devicePositions, deviceClient, deviceCollisionKeys, totalValues, true, false, false, NULL, 0, queue);
            
        qt.submit([&](auto &h) 
        {      
            auto _positions = devicePositions;     
            auto _oldPositions = deviceOldPositions;
            auto _collisionKeys = deviceCollisionKeys;

            auto _updateCounter = deviceOldUpdateCounter;

            h.parallel_for(num_items, [=](auto i) 
            {    
                if(_collisionKeys[i].x() > 0)
                {
                    _positions[i] = _oldPositions[i];
                    cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                            sycl::memory_scope::device, 
                            sycl::access::address_space::ext_intel_global_device_space> ar(_updateCounter[0]);

                    ar.fetch_add(1);
                }
            });
        }).wait();

        qt.memcpy(hostOldUpdateCounter, deviceOldUpdateCounter, sizeof(int)).wait();
        //qt.memset(deviceOldUpdateCounter, 0, sizeof(int)).wait();
        counter = hostOldUpdateCounter[0];
        
        std::cout << "COUNTER " << counter << "\r\n";

        std::cout << "THINGS positions ";
outputarb(devicePositions,totalValues);//settings.max_values * settings.clients());

std::cout << "THINGS clients ";
            outputarb(deviceClient,totalValues);

                        std::cout << "THINGS collision keys ";
            outputarb(deviceCollisionKeys, totalValues);//settings.max_values * settings.clients());

            
    } while(counter > 0);
}

void organisation::parallel::program::copy(::organisation::schema **source, int source_size)
{
    memset(hostCachePositions, 0, sizeof(sycl::float4) * settings.max_values * settings.host_buffer);
    memset(hostCacheValues, -1, sizeof(int) * settings.max_values * settings.host_buffer);
    memset(hostMovements, 0, sizeof(sycl::float4) * settings.max_movements * settings.host_buffer);
    memset(hostMovementsCounts, 0, sizeof(int) * settings.host_buffer);
    memset(hostCollisions, 0, sizeof(sycl::float4) * settings.max_collisions * settings.host_buffer);
    memset(hostClient, 0, sizeof(sycl::int4) * settings.max_values * settings.host_buffer);

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)settings.clients()};

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

            hostCachePositions[d_count + (index * settings.max_values)] = { (float)position.x, (float)position.y, (float)position.z, -2.0f };
            hostCacheValues[d_count + (index * settings.max_values)] = value;

            ++d_count;
            if(d_count > settings.max_values) break;
        }

        // ***
        totalCacheValues = d_count;
        // ***

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

        #warning it's only one insert per client, this might not be needed?
        hostClient[index].w() = dest_index;

        ++index;
        if(index >= settings.host_buffer)
        {
            std::vector<sycl::event> events;

            events.push_back(qt.memcpy(&deviceCachePositions[dest_index * settings.max_values], hostCachePositions, sizeof(sycl::float4) * settings.max_values * index));
            events.push_back(qt.memcpy(&deviceCacheValues[dest_index * settings.max_values], hostCacheValues, sizeof(int) * settings.max_values * index));
            events.push_back(qt.memcpy(&deviceMovements[dest_index * settings.max_movements], hostMovements, sizeof(int) * settings.max_movements * index));
            events.push_back(qt.memcpy(&deviceMovementsCounts[dest_index], hostMovementsCounts, sizeof(int) * index));
            events.push_back(qt.memcpy(&deviceCollisions[dest_index * settings.max_collisions], hostCollisions, sizeof(sycl::float4) * settings.max_collisions * index));
            events.push_back(qt.memcpy(&deviceClient[dest_index], hostClient, sizeof(sycl::int4) * index));

            memset(hostCachePositions, 0, sizeof(sycl::float4) * settings.max_values * settings.host_buffer);
            memset(hostCacheValues, -1, sizeof(int) * settings.max_values * settings.host_buffer);
            memset(hostMovements, 0, sizeof(sycl::float4) * settings.max_movements * settings.host_buffer);
            memset(hostMovementsCounts, 0, sizeof(int) * settings.host_buffer);
            memset(hostCollisions, 0, sizeof(sycl::float4) * settings.max_collisions * settings.host_buffer);
            memset(hostClient, 0, sizeof(sycl::int4) * settings.max_values * settings.host_buffer);

            sycl::event::wait(events);

            dest_index += settings.host_buffer;
            index = 0;            
        }
    }

    if(index > 0)
    {
        std::vector<sycl::event> events;

std::cout << "clients " <<settings.clients() << "\r\n";
std::cout << "baa " << (sizeof(sycl::float4) * settings.max_values * settings.clients()) << "\r\n";
std::cout << "laa " << (sizeof(sycl::float4) * settings.max_values * index) << "\r\n";

        events.push_back(qt.memcpy(&deviceCachePositions[dest_index * settings.max_values], hostCachePositions, sizeof(sycl::float4) * settings.max_values * index));
        events.push_back(qt.memcpy(&deviceCacheValues[dest_index * settings.max_values], hostCacheValues, sizeof(int) * settings.max_values * index));
        events.push_back(qt.memcpy(&deviceMovements[dest_index * settings.max_movements], hostMovements, sizeof(int) * settings.max_movements * index));
        events.push_back(qt.memcpy(&deviceMovementsCounts[dest_index], hostMovementsCounts, sizeof(int) * index));
        events.push_back(qt.memcpy(&deviceCollisions[dest_index * settings.max_collisions], hostCollisions, sizeof(sycl::float4) * settings.max_collisions * index));
        events.push_back(qt.memcpy(&deviceClient[dest_index], hostClient, sizeof(sycl::int4) * index));

        sycl::event::wait(events);
    }    

std::cout << "movements ";
outputarb(deviceMovements,settings.max_movements * settings.clients());
std::cout << "collisions ";
outputarb(deviceCollisions,settings.max_collisions * settings.clients());
std::cout << "cache ";
outputarb(deviceCachePositions, settings.max_values * settings.clients());

    inserter->copy(source,source_size);
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
		//if ((temp[i] != -1)&&(temp[i]!=0))
		//{
			result += std::string("[");
			result += std::to_string(i);
			result += std::string("]");
			result += std::to_string(temp[i]);
			result += std::string(",");
		//}
	}
	result += std::string("\r\n");
	
    std::cout << result;

	delete[] temp;
}

void organisation::parallel::program::outputarb(sycl::int2 *source, int length)
{
    sycl::int2 *temp = new sycl::int2[length];
    if (temp == NULL) return;

    sycl::queue q = ::parallel::queue(*dev).get();

    q.memcpy(temp, source, sizeof(sycl::int2) * length).wait();

    std::string result("");
	for (int i = 0; i < length; ++i)
	{        
        if ((temp[i].x() != 0) || (temp[i].y() != 0))
        {
			result += std::string("[");
			result += std::to_string(i);
			result += std::string("]");
			result += std::to_string(temp[i].x());
			result += std::string(",");
			result += std::to_string(temp[i].y());
		}
	}
	result += std::string("\r\n");
	
    
    std::cout << result;

	delete[] temp;
}

void organisation::parallel::program::outputarb(sycl::int4 *source, int length)
{
    sycl::int4 *temp = new sycl::int4[length];
    if (temp == NULL) return;

    sycl::queue q = ::parallel::queue(*dev).get();

    q.memcpy(temp, source, sizeof(sycl::int4) * length).wait();

    std::string result("");
	for (int i = 0; i < length; ++i)
	{
        if ((temp[i].x() != 0) || (temp[i].y() != 0) || (temp[i].z() != 0))
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

    deviceCachePositions = NULL;
    deviceCacheValues = NULL;
    deviceMovements = NULL;
    deviceMovementsCounts = NULL;
    deviceCollisions = NULL;
    deviceCollisionKeys = NULL;

    hostCachePositions = NULL;
    hostCacheValues = NULL;
    hostMovements = NULL;
    hostMovementsCounts = NULL;
    hostCollisions = NULL;
    hostClient = NULL; 

    deviceOutputValues = NULL;
    deviceOutputIndex = NULL;
    deviceOutputClient = NULL;
    deviceOutputTotalValues = NULL;

    hostOutputValues = NULL;
    hostOutputIndex = NULL;
    hostOutputClient = NULL;
    hostOutputTotalValues = NULL;

    deviceTotalValues = NULL;
    hostTotalValues = NULL;

    deviceNewPositions = NULL;
    deviceNewValues = NULL;
    deviceNewClient = NULL;
    deviceNewNextDirections = NULL;
    deviceNewMovementIdx = NULL;

    deviceOldPositions = NULL;
    deviceOldUpdateCounter = NULL;
    hostOldUpdateCounter = NULL;

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
// ***

        if(hostOldUpdateCounter != NULL) sycl::free(hostOldUpdateCounter, q);
        if(deviceOldUpdateCounter != NULL) sycl::free(deviceOldUpdateCounter, q);
        if(deviceOldPositions != NULL) sycl::free(deviceOldPositions, q);

        if(deviceNewMovementIdx != NULL) sycl::free(deviceNewMovementIdx, q);
        if(deviceNewNextDirections != NULL) sycl::free(deviceNewNextDirections, q);
        if(deviceNewClient != NULL) sycl::free(deviceNewClient, q);
        if(deviceNewValues != NULL) sycl::free(deviceNewValues, q);
        if(deviceNewPositions != NULL) sycl::free(deviceNewPositions, q);

// ***
        if(hostTotalValues != NULL) sycl::free(hostTotalValues, q);
        if(deviceTotalValues != NULL) sycl::free(deviceTotalValues, q);

// ***
        if(deviceOutputValues != NULL) sycl::free(deviceOutputValues, q);
        if(deviceOutputIndex != NULL) sycl::free(deviceOutputIndex, q);
        if(deviceOutputClient != NULL) sycl::free(deviceOutputClient, q);
        if(deviceOutputTotalValues != NULL) sycl::free(deviceOutputTotalValues, q);

        if(hostOutputValues != NULL) sycl::free(hostOutputValues, q);
        if(hostOutputIndex != NULL) sycl::free(hostOutputIndex, q);
        if(hostOutputClient != NULL) sycl::free(hostOutputClient, q);
        if(hostOutputTotalValues != NULL) sycl::free(hostOutputTotalValues, q);
        
// ***
        if(hostClient != NULL) sycl::free(hostClient, q);
        if(hostCollisions != NULL) sycl::free(hostCollisions, q);
        if(hostMovementsCounts != NULL) sycl::free(hostMovementsCounts, q);
        if(hostMovements != NULL) sycl::free(hostMovements, q);
        if(hostCacheValues != NULL) sycl::free(hostCacheValues, q);
        if(hostCachePositions != NULL) sycl::free(hostCachePositions, q);

        if(deviceCollisionKeys != NULL) sycl::free(deviceCollisionKeys, q);
        if(deviceCollisions != NULL) sycl::free(deviceCollisions, q);
        if(deviceMovementsCounts != NULL) sycl::free(deviceMovementsCounts, q);
        if(deviceMovements != NULL) sycl::free(deviceMovements, q);
        if(deviceCacheValues != NULL) sycl::free(deviceCacheValues, q);
        if(deviceCachePositions != NULL) sycl::free(deviceCachePositions, q);
        
        if(deviceClient != NULL) sycl::free(deviceClient, q);
        if(deviceMovementIdx != NULL) sycl::free(deviceMovementIdx, q);
        if(deviceValues != NULL) sycl::free(deviceValues, q);

        if(deviceNextDirections != NULL) sycl::free(deviceNextDirections, q);
        if(deviceNextHalfPositions != NULL) sycl::free(deviceNextHalfPositions, q);
        if(deviceNextPositions != NULL) sycl::free(deviceNextPositions, q);
        if(devicePositions != NULL) sycl::free(devicePositions, q);
    }    
}