#include <CL/sycl.hpp>
#include "parallel/dimension.hpp"
#include <string>
#include <stdio.h>
#include <vector>

#ifndef _PARALLEL_PARAMETERS
#define _PARALLEL_PARAMETERS

namespace parallel
{
	class parameters
	{
	public:
		int grids, threads, length;
		::parallel::dimension dimensions;

	public:
		parameters(int _W = 100, int _H = 100, int _D = 100, 
				   int _G = 256, int _T = 256, int _N = 0)
		{ 			
			reset(_W, _H, _D, _G, _T, _N);
		}

		parameters(const parameters &source) { copy(source); }

	public:
		void reset(int _W = 100, int _H = 100, int _D = 100, 
				   int _G = 256, int _T = 256, int _N = 0)
		{
			grids = _G;
			threads = _T;
			length = _N;

			dimensions.x = _W;
			dimensions.y = _H;
			dimensions.z = _D;
			
			dimensions.w = 0;

			if (length == 0) length = grids * threads;
		}
		        
	public:
		void copy(const parameters &source)
		{
			grids = source.grids;
			threads = source.threads;
			length = source.length;

			dimensions = { source.dimensions.x, source.dimensions.y, source.dimensions.z, 0 };
		}

	public:
		bool operator==(const parameters& rhs) const
		{
			return std::tie(dimensions,grids,threads,length) == 
				   std::tie(rhs.dimensions,rhs.grids,rhs.threads,rhs.length);
		}

	public:
		parameters& operator=(const parameters& source)
		{
			this->copy((parameters&)source);
			return *this;
		}
	};
};

#endif