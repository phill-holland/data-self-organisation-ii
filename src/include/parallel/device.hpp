#include <CL/sycl.hpp>
#include <vector>
#include <string>

#ifndef _PARALLEL_DEVICE
#define _PARALLEL_DEVICE

namespace parallel
{
    class device
    {
        sycl::device dev;

    public:
        device(int index);

    public:
        sycl::device& get() { return dev; }

    public:
		static std::vector<std::string> enumerate();
		static std::string capabilities();

    public:
        uint32_t local_mem_size();
        
    protected:
        sycl::device select(int index);
    };
};

#endif