#include "parallel/collisions.hpp"

void organisation::parallel::collisions::reset(::parallel::device &dev, 
                                               ::parallel::queue *q, 
                                               parameters &settings)
{
    init = false; cleanup();

    this->dev = &dev;
    this->queue = q;
    this->settings = settings;
    this->length = settings.max_collisions * settings.mappings.maximum() * settings.clients();

    sycl::queue &qt = ::parallel::queue(dev).get();

    deviceCollisions = sycl::malloc_device<sycl::float4>(length, qt);
    if(deviceCollisions == NULL) return;

    hostCollisions = sycl::malloc_host<sycl::float4>(settings.max_collisions * settings.mappings.maximum() * settings.host_buffer, qt);
    if(hostCollisions == NULL) return;

    clear();

    init = true;
}

void organisation::parallel::collisions::clear()
{
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);

    std::vector<sycl::event> events;

    events.push_back(qt.memset(deviceCollisions, 0, sizeof(sycl::float4) * length));

    sycl::event::wait(events);
}

void organisation::parallel::collisions::copy(::organisation::schema **source, int source_size)
{


    memset(hostCollisions, 0, sizeof(sycl::float4) * settings.max_collisions * settings.mappings.maximum() * settings.host_buffer);
    
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, queue);
    sycl::range num_items{(size_t)settings.clients()};

    int client_offset = settings.max_collisions * settings.mappings.maximum();
    int client_index = 0;
    int dest_index = 0;
    int index = 0;

    for(int source_index = 0; source_index < source_size; ++source_index)
    {
        organisation::program *prog = &source[source_index]->prog;

        int c_count = 0;
        //for(auto &it: prog->collisions.size())        
        for(int i = 0; i < prog->collisions.size(); ++i)
        {
            int direction;
            if(prog->collisions.get(direction,i))
            {
                vector temp;                
                temp.decode(direction);

                hostCollisions[c_count + (index * client_offset)] = { (float)temp.x, (float)temp.y, (float)temp.z, 0.0f };
                ++c_count;
                if(c_count > settings.max_collisions) break;            
            }
        }

        ++index;
        ++client_index;

        if(index >= settings.host_buffer)
        {
            std::vector<sycl::event> events;

            events.push_back(qt.memcpy(&deviceCollisions[dest_index * client_offset], hostCollisions, sizeof(sycl::float4) * client_offset * index));        

            sycl::event::wait(events);
            
            memset(hostCollisions, 0, sizeof(sycl::float4) * client_offset * settings.host_buffer);
                        
            dest_index += settings.host_buffer;
            index = 0;            
        }
    }

    if(index > 0)
    {
        std::vector<sycl::event> events;

        events.push_back(qt.memcpy(&deviceCollisions[dest_index * client_offset], hostCollisions, sizeof(sycl::float4) * client_offset * index));        

        sycl::event::wait(events);
    }        
}

void organisation::parallel::collisions::outputarb(int *source, int length)
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

void organisation::parallel::collisions::makeNull()
{
    dev = NULL;

    deviceCollisions = NULL;
    hostCollisions = NULL;
}

void organisation::parallel::collisions::cleanup()
{
    if(dev != NULL) 
    {   
        sycl::queue q = ::parallel::queue(*dev).get();

        if(hostCollisions != NULL) sycl::free(hostCollisions, q);
        if(deviceCollisions != NULL) sycl::free(deviceCollisions, q);          
    }
}
