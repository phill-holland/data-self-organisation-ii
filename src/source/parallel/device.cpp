#include "parallel/device.hpp"

parallel::device::device(int index)
{
    dev = select(index);
}

uint32_t parallel::device::local_mem_size()
{
    bool has_local = dev.is_host() || (dev.get_info<sycl::info::device::local_mem_type>() != sycl::info::local_mem_type::none);
    uint32_t result = dev.get_info<sycl::info::device::local_mem_size>();
    return result;
}

std::vector<std::string> parallel::device::enumerate()
{
	std::vector<std::string> result;

	for (auto device : sycl::device::get_devices(sycl::info::device_type::gpu))
	{
		result.push_back(device.get_info<sycl::info::device::name>());
  	}

	return result;
}

std::string parallel::device::capabilities()
{
    std::string result;
	int count = 0;

	for (auto device : sycl::device::get_devices(sycl::info::device_type::gpu))//all))
	{
		std::string name = device.get_info<sycl::info::device::name>();
		pi_uint32 grids = device.get_info<sycl::info::device::max_work_item_dimensions>();
		cl::sycl::id<3> threads = device.get_info<sycl::info::device::max_work_item_sizes>();

        result += std::to_string(count);
        result += std::string(") Name ");
        result += name;
        result += std::string("\r\n");

        result += std::string("Max Grids ");
        result += std::to_string(grids);
        result += std::string("\r\n");

		result += std::string("Max Threads ");
        result += std::to_string((int)threads.get(0));
        result += std::string(", ");
        result += std::to_string((int)threads.get(1));
        result += std::string(", ");
        result += std::to_string((int)threads.get(2));
        result += std::string("\r\n");

		++count;
	}

    return result;
}

sycl::device parallel::device::select(int index)
{
    std::vector<sycl::device> devices = sycl::device::get_devices(sycl::info::device_type::gpu);
    if((index >= 0)&&(index < devices.size()))
        return devices.at(index);

    return devices.at(0);
}
