#include "parallel/map.hpp"
#include <CL/sycl.hpp>

bool MapIsZero(const sycl::float4 a)
{
        if (((int)(a.x() * 100.0)) != 0) return false;
        if (((int)(a.y() * 100.0)) != 0) return false;
        if (((int)(a.z() * 100.0)) != 0) return false;

        return true;
}

bool MapIsEquals(const sycl::float4 a, const sycl::int4 la,
                 const sycl::float4 b, const sycl::int4 lb)
{
        if ((int)(a.x() * 100.0) != (int)(b.x() * 100.0)) return false;
        if ((int)(a.y() * 100.0) != (int)(b.y() * 100.0)) return false;
        if ((int)(a.z() * 100.0) != (int)(b.z() * 100.0)) return false;

        if (la.x() != lb.x()) return false;
        if (la.y() != lb.y()) return false;
        if (la.z() != lb.z()) return false;

        return true;
}

bool MapIsOutside(const sycl::int4 value, const sycl::int4 min,
                  const sycl::int4 max)
{
        if ((value.x() < min.x()) || (value.x() >= max.x())) return true;
        if ((value.y() < min.y()) || (value.y() >= max.y())) return true;
        if ((value.z() < min.z()) || (value.z() >= max.z())) return true;

        return false;
}

void ClearMap(sycl::float4 **positions, sycl::int4 **clients, const int buckets,
              const size_t N, sycl::nd_item<3> item_ct1)
{
        
        unsigned int i =
            item_ct1.get_group(2) * item_ct1.get_local_range().get(2) +
            item_ct1.get_local_id(2);


        if (i < N)
	{
		for (int j = 0; j < buckets; ++j)
		{
			positions[j][i] = { 0.0, 0.0, 0.0, 0.0 };
			clients[j][i] = { 0, 0, 0, 0 };
		}
        }
}

void BuildMap(int *coarse, int *medium, int *bucket_indices,
              int *bucket_lengths, sycl::float4 **bucket_positions,
              sycl::int4 **bucket_clients, const sycl::float4 *values,
              const sycl::int4 *clients, const sycl::float4 coarse_scale,
              const sycl::int4 coarse_dimensions,
              const sycl::float4 medium_scale,
              const sycl::int4 medium_dimensions, const sycl::float4 fine_scale,
              const sycl::int4 fine_dimensions, const int total_buckets,
              const sycl::int4 client_dimensions,
              const sycl::int4 client_totals, const int value, const int N,
              sycl::nd_item<3> item_ct1)
{
        int idx = item_ct1.get_group(2) * item_ct1.get_local_range().get(2) +
                  item_ct1.get_local_id(2);

        sycl::float4 temp = values[idx];
        if (MapIsZero(temp)) return;

        sycl::int4 client = clients[idx];

        sycl::int4 i = {(int)sycl::floor(temp.x() * coarse_scale.x()),
                        (int)sycl::floor(temp.y() * coarse_scale.y()),
                        (int)sycl::floor(temp.z() * coarse_scale.z()), 0};

        if (MapIsOutside(i, { 0, 0, 0, 0 }, coarse_dimensions)) return;

        int client_offset =
            (client.y() * client_totals.x()) + client.x() +
            (client_totals.x() * client_totals.y() * client.z());

        int i_offset = (i.y() * coarse_dimensions.x()) + i.x() +
                       (coarse_dimensions.x() * coarse_dimensions.y() * i.z());
        int m = coarse_dimensions.x() * coarse_dimensions.y() * coarse_dimensions.z();

        coarse[i_offset + (m * client_offset)] = value;

        sycl::int4 j = {(int)sycl::floor(temp.x() * medium_scale.x()),
                        (int)sycl::floor(temp.y() * medium_scale.y()),
                        (int)sycl::floor(temp.z() * medium_scale.z()), 0};

        if (MapIsOutside(j, { 0, 0, 0, 0 }, medium_dimensions)) return;

        int j_offset = (j.y() * medium_dimensions.x()) + j.x() +
                       (medium_dimensions.x() * medium_dimensions.y() * j.z());
        int p = medium_dimensions.x() * medium_dimensions.y() * medium_dimensions.z();

        medium[j_offset + (p * client_offset)] = value;

        sycl::int4 k = {(int)sycl::floor(temp.x() * fine_scale.x()),
                        (int)sycl::floor(temp.y() * fine_scale.y()),
                        (int)sycl::floor(temp.z() * fine_scale.z()), 0};

        if (MapIsOutside(k, { 0, 0, 0, 0 }, fine_dimensions)) return;

        int bucket = (k.y() * fine_dimensions.x()) + k.x() +
                     (fine_dimensions.x() * fine_dimensions.y() * k.z());

        // ***
       cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                                sycl::memory_scope::device, 
                                sycl::access::address_space::ext_intel_global_device_space> ar(bucket_indices[bucket]);
        
        int index = ar.fetch_add(1);
        // ***
        
        //int index = sycl::atomic<int>(sycl::global_ptr<int>(&bucket_indices[bucket])).fetch_add(1);
        if (index >= bucket_lengths[bucket]) return;

        bucket_positions[bucket][index] = {temp.x(), temp.y(), temp.z(), (float)value};
        bucket_clients[bucket][index] = {client.x(), client.y(), client.z(), idx};
}

void ScanMap(int *result, const sycl::float4 *values, const sycl::int4 *clients,
             const int *coarse, int *medium, const int *bucket_indices,
             const int *bucket_lengths, const sycl::float4 **bucket_positions,
             const sycl::int4 **bucket_clients, const sycl::float4 coarse_scale,
             const sycl::int4 coarse_dimensions,
             const sycl::float4 medium_scale,
             const sycl::int4 medium_dimensions, const sycl::float4 fine_scale,
             const sycl::int4 fine_dimensions, const int total_buckets,
             const sycl::int4 client_dimensions, const sycl::int4 client_totals,
             const int value, const bool self, int *collided,
             const int collided_index, const sycl::float4 *lcompare,
             const sycl::float4 *rcompare, const int N,
             sycl::nd_item<3> item_ct1)
{
        int idx = item_ct1.get_group(2) * item_ct1.get_local_range().get(2) +
                  item_ct1.get_local_id(2);

        sycl::float4 temp = values[idx];
        if (MapIsZero(temp)) return;

        sycl::int4 client = clients[idx];

        sycl::int4 i = {(int)sycl::floor(temp.x() * coarse_scale.x()),
                        (int)sycl::floor(temp.y() * coarse_scale.y()),
                        (int)sycl::floor(temp.z() * coarse_scale.z()), 0};

        if (MapIsOutside(i, { 0, 0, 0, 0 }, coarse_dimensions)) return;

        int client_offset =
            (client.y() * client_totals.x()) + client.x() +
            (client_totals.x() * client_totals.y() * client.z());

        int i_offset = (i.y() * coarse_dimensions.x()) + i.x() +
                       (coarse_dimensions.x() * coarse_dimensions.y() * i.z());
        int m = coarse_dimensions.x() * coarse_dimensions.y() * coarse_dimensions.z();

        if (coarse[i_offset + (m * client_offset)] != value) return;

        sycl::int4 j = {(int)sycl::floor(temp.x() * medium_scale.x()),
                        (int)sycl::floor(temp.y() * medium_scale.y()),
                        (int)sycl::floor(temp.z() * medium_scale.z()), 0};

        if (MapIsOutside(j, { 0, 0, 0, 0 }, medium_dimensions)) return;

        int j_offset = (j.y() * medium_dimensions.x()) + j.x() +
                       (medium_dimensions.x() * medium_dimensions.y() * j.z());
        int p = medium_dimensions.x() * medium_dimensions.y() * medium_dimensions.z();

        if (medium[j_offset + (p * client_offset)] != value) return;

        sycl::int4 k = {(int)sycl::floor(temp.x() * fine_scale.x()),
                        (int)sycl::floor(temp.y() * fine_scale.y()),
                        (int)sycl::floor(temp.z() * fine_scale.z()), 0};

        if (MapIsOutside(k, { 0, 0, 0, 0 }, fine_dimensions)) return;

        int bucket = (k.y() * fine_dimensions.x()) + k.x() +
                     (fine_dimensions.x() * fine_dimensions.y() * k.z());

        int index = bucket_indices[bucket];
	int length = bucket_lengths[bucket];
	if (index > length) index = length;

	for (int i = 0; i < index; ++i)
	{
        sycl::float4 p1 = bucket_positions[bucket][i];
        sycl::int4 c1 = bucket_clients[bucket][i];

        if ((int)p1.w() == value)
        {
			if (MapIsEquals(temp, client, p1, c1))
			{
                if ((self) && (c1.w() == idx)) return;

                int output = 0;

				if ((lcompare != NULL) && (rcompare != NULL))
				{
                    if (MapIsEquals(p1, c1, lcompare[c1.w()], client)) output = 1;
                    if (MapIsEquals(lcompare[c1.w()], client, rcompare[idx], c1)) output = 1;
                }
				else output = 1;

				if (output == 1)
				{
					result[idx] = 1;
					if (collided != NULL) collided[collided_index] = 1;
				}

				return;
			}
		}
	}
}

void ScanMap(
    sycl::int2 *result, const sycl::float4 *values, const sycl::int4 *clients,
    const int *coarse, int *medium, const int *bucket_indices,
    const int *bucket_lengths, const sycl::float4 **bucket_positions,
    const sycl::int4 **bucket_clients, const sycl::float4 coarse_scale,
    const sycl::int4 coarse_dimensions, const sycl::float4 medium_scale,
    const sycl::int4 medium_dimensions, const sycl::float4 fine_scale,
    const sycl::int4 fine_dimensions, const int total_buckets,
    const sycl::int4 client_dimensions, const sycl::int4 client_totals,
    const int value, const bool self, int *collided, const int collided_index,
    const bool symetrical, const bool inverse, const sycl::float4 *lcompare,
    const sycl::float4 *rcompare, const int N, sycl::nd_item<3> item_ct1)
{
        int idx = item_ct1.get_group(2) * item_ct1.get_local_range().get(2) +
                  item_ct1.get_local_id(2);

        sycl::float4 temp = values[idx];
        if (MapIsZero(temp)) return;

        sycl::int4 client = clients[idx];
        int client_offset =
            (client.y() * client_totals.x()) + client.x() +
            (client_totals.x() * client_totals.y() * client.z());

        sycl::int4 i = {(int)sycl::floor(temp.x() * coarse_scale.x()),
                        (int)sycl::floor(temp.y() * coarse_scale.y()),
                        (int)sycl::floor(temp.z() * coarse_scale.z()), 0};

        if (MapIsOutside(i, { 0, 0, 0, 0 }, coarse_dimensions)) return;

        int i_offset = (i.y() * coarse_dimensions.x()) + i.x() +
                       (coarse_dimensions.x() * coarse_dimensions.y() * i.z());
        int m = coarse_dimensions.x() * coarse_dimensions.y() * coarse_dimensions.z();

        if (coarse[i_offset + (m * client_offset)] != value) return;

        sycl::int4 j = {(int)sycl::floor(temp.x() * medium_scale.x()),
                        (int)sycl::floor(temp.y() * medium_scale.y()),
                        (int)sycl::floor(temp.z() * medium_scale.z()), 0};

        if (MapIsOutside(j, { 0, 0, 0, 0 }, medium_dimensions)) return;

        int j_offset = (j.y() * medium_dimensions.x()) + j.x() +
                       (medium_dimensions.x() * medium_dimensions.y() * j.z());
        int p = medium_dimensions.x() * medium_dimensions.y() * medium_dimensions.z();

        if (medium[j_offset + (p * client_offset)] != value) return;

        sycl::int4 k = {(int)sycl::floor(temp.x() * fine_scale.x()),
                        (int)sycl::floor(temp.y() * fine_scale.y()),
                        (int)sycl::floor(temp.z() * fine_scale.z()), 0};

        if (MapIsOutside(k, { 0, 0, 0, 0 }, fine_dimensions)) return;

        int bucket = (k.y() * fine_dimensions.x()) + k.x() +
                     (fine_dimensions.x() * fine_dimensions.y() * k.z());

        int index = bucket_indices[bucket];
        int length = bucket_lengths[bucket];
        if (index > length) index = length;

        for (int i = 0; i < index; ++i)
        {
            sycl::float4 p1 = bucket_positions[bucket][i];
            sycl::int4 c1 = bucket_clients[bucket][i];

            if ((int)p1.w() == value)
            {
                if (MapIsEquals(temp, client, p1, c1))
                {
                    if ((self) && (c1.w() == idx)) return;

                    int output = 0;

                    if ((lcompare != NULL) && (rcompare != NULL))
                    {
                        if (MapIsEquals(p1, c1, lcompare[c1.w()], client)) output = 1;
                        if (MapIsEquals(lcompare[c1.w()], client, rcompare[idx], c1)) output = 1;
                    }
    				else output = 1;

                    if (output == 1)
                    {
                        if (inverse) result[c1.w()] = {1, idx};
                        else result[idx] = {1, c1.w()};

                    if (symetrical)
					{
                        if (inverse) result[idx] = {1, c1.w()};
                        else result[c1.w()] = {1, idx};
                    }

					if (collided != NULL) collided[collided_index] = 1;
				}

				return;
			}
		}
	}
}

void MapScanCollision(
    const int idx, const sycl::float4 source, const sycl::int4 client,
    const bool isCenter, sycl::int4 *result, const int *coarse, int *medium,
    const int *bucket_indices, const int *bucket_lengths,
    const sycl::float4 **bucket_positions, const sycl::int4 **bucket_clients,
    const sycl::float4 coarse_scale, const sycl::int4 coarse_dimensions,
    const sycl::float4 medium_scale, const sycl::int4 medium_dimensions,
    const sycl::float4 fine_scale, const sycl::int4 fine_dimensions,
    const int total_buckets, const sycl::int4 client_dimensions,
    const sycl::int4 client_totals, const int value, const bool self,
    int *collided, const int collided_index, const bool symetrical,
    const bool inverse, const sycl::float4 *lcompare,
    const sycl::float4 *rcompare)
{
        sycl::float4 temp = source;

        int client_offset =
            (client.y() * client_totals.x()) + client.x() +
            (client_totals.x() * client_totals.y() * client.z());

        sycl::int4 i = {(int)sycl::floor(temp.x() * coarse_scale.x()),
                        (int)sycl::floor(temp.y() * coarse_scale.y()),
                        (int)sycl::floor(temp.z() * coarse_scale.z()), 0};

        if (MapIsOutside(i, { 0, 0, 0, 0 }, coarse_dimensions)) return;

        int i_offset = (i.y() * coarse_dimensions.x()) + i.x() +
                       (coarse_dimensions.x() * coarse_dimensions.y() * i.z());
        int m = coarse_dimensions.x() * coarse_dimensions.y() * coarse_dimensions.z();

        if (coarse[i_offset + (m * client_offset)] != value) return;

        sycl::int4 j = {(int)sycl::floor(temp.x() * medium_scale.x()),
                        (int)sycl::floor(temp.y() * medium_scale.y()),
                        (int)sycl::floor(temp.z() * medium_scale.z()), 0};

        if (MapIsOutside(j, { 0, 0, 0, 0 }, medium_dimensions)) return;

        int j_offset = (j.y() * medium_dimensions.x()) + j.x() +
                       (medium_dimensions.x() * medium_dimensions.y() * j.z());
        int p = medium_dimensions.x() * medium_dimensions.y() * medium_dimensions.z();

        if (medium[j_offset + (p * client_offset)] != value) return;

        sycl::int4 k = {(int)sycl::floor(temp.x() * fine_scale.x()),
                        (int)sycl::floor(temp.y() * fine_scale.y()),
                        (int)sycl::floor(temp.z() * fine_scale.z()), 0};

        if (MapIsOutside(k, { 0, 0, 0, 0 }, fine_dimensions)) return;

        int bucket = (k.y() * fine_dimensions.x()) + k.x() +
                     (fine_dimensions.x() * fine_dimensions.y() * k.z());

        int index = bucket_indices[bucket];
	int length = bucket_lengths[bucket];
	if (index > length) index = length;

	for (int i = 0; i < index; ++i)
	{
        sycl::float4 p1 = bucket_positions[bucket][i];
        sycl::int4 c1 = bucket_clients[bucket][i];

        if ((int)p1.w() == value)
        {
			if (MapIsEquals(temp, client, p1, c1))
			{
                if ((self) && (c1.w() == idx)) return;

                int output = 0;

				if ((lcompare != NULL) && (rcompare != NULL))
				{
                    if (MapIsEquals(p1, c1, lcompare[c1.w()], client)) output = 1;
                    if (MapIsEquals(lcompare[c1.w()], client, rcompare[idx], c1)) output = 1;
                }
				else output = 1;

				if (output == 1)
				{
					int m = 1;

					if (inverse)
					{
                        result[c1.w()].x() = m;
                        result[c1.w()].y() = idx;
                        if (isCenter) result[c1.w()].z() = 1;

                        cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                                                sycl::memory_scope::device, 
                                                sycl::access::address_space::ext_intel_global_device_space> ar(result[c1.w()].w());

                        ar.fetch_add(1);
                        //sycl::atomic<int>(sycl::global_ptr<int>(&result[c1.w()].w())).fetch_add(1);
                    }
					else
					{
                        result[idx].x() = m;
                        result[idx].y() = c1.w();
                        if (isCenter) result[idx].z() = 1;

                        cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                                                sycl::memory_scope::device, 
                                                sycl::access::address_space::ext_intel_global_device_space> ar(result[idx].w());

                        ar.fetch_add(1);
                        //sycl::atomic<int>(sycl::global_ptr<int>(&result[idx].w())).fetch_add(1);
                    }

					if (symetrical)
					{
						if (inverse)
						{
                            result[idx].x() = m;
                            result[idx].y() = c1.w();
                            if (isCenter) result[idx].z() = 1;

                            cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                                                    sycl::memory_scope::device, 
                                                    sycl::access::address_space::ext_intel_global_device_space> ar(result[idx].w());

                            ar.fetch_add(1);
                            //sycl::atomic<int>(sycl::global_ptr<int>(&result[idx].w())).fetch_add(1);
                        }
						else
						{
                            result[c1.w()].x() = m;
                            result[c1.w()].y() = idx;
                            if (isCenter) result[c1.w()].z() = 1;

                            cl::sycl::atomic_ref<int, cl::sycl::memory_order::relaxed, 
                                                    sycl::memory_scope::device, 
                                                    sycl::access::address_space::ext_intel_global_device_space> ar(result[c1.w()].w());

                            ar.fetch_add(1);
                            //sycl::atomic<int>(sycl::global_ptr<int>(&result[c1.w()].w())).fetch_add(1);
                        }
					}

					if (collided != NULL) collided[collided_index] = 1;
				}

				return;
			}
		}
	}
}

void ScanMap(sycl::int4 *result, const sycl::float4 *values,
             const sycl::int4 *clients, const int *coarse, int *medium,
             const int *bucket_indices, const int *bucket_lengths,
             const sycl::float4 **bucket_positions,
             const sycl::int4 **bucket_clients, const sycl::float4 coarse_scale,
             const sycl::int4 coarse_dimensions,
             const sycl::float4 medium_scale,
             const sycl::int4 medium_dimensions, const sycl::float4 fine_scale,
             const sycl::int4 fine_dimensions, const int total_buckets,
             const sycl::int4 client_dimensions, const sycl::int4 client_totals,
             const int value, const bool self, int *collided,
             const int collided_index, const bool symetrical,
             const bool inverse, const sycl::float4 *lcompare,
             const sycl::float4 *rcompare, const sycl::int4 min,
             const sycl::int4 max, const int N, sycl::nd_item<3> item_ct1)
{
        int idx = item_ct1.get_group(2) * item_ct1.get_local_range().get(2) +
                  item_ct1.get_local_id(2);

        sycl::float4 source = values[idx];
        if (MapIsZero(source)) return;

        for (int x = min.x(); x <= max.x(); ++x)
        {
                for (int y = min.y(); y <= max.y(); ++y)
                {
                        for (int z = min.z(); z <= max.z(); ++z)
                        {
                                sycl::float4 temp = {source.x() + ((float)x),
                                                     source.y() + ((float)y),
                                                     source.z() + ((float)z),
                                                     (float)source.w()};
                                sycl::int4 client = clients[idx];

                                bool isCenter = false;
				if((x==0)&&(y==0)&&(z==0)) isCenter = true;

				MapScanCollision(idx, temp, client, isCenter,
								 result,
								 coarse, medium,
								 bucket_indices, bucket_lengths,
								 bucket_positions, bucket_clients,
								 coarse_scale, coarse_dimensions,
								 medium_scale, medium_dimensions,
								 fine_scale, fine_dimensions,
								 total_buckets,
								 client_dimensions, client_totals,
								 value,
								 self, collided, collided_index,
								 symetrical, inverse,
								 lcompare, rcompare);
			}
		}
	}
}

void parallel::map::reset(::parallel::device &dev, 
                          ::parallel::parameters c, ::parallel::parameters g, scale coarse,
                          scale medium, scale fine, scale diameter, organisation::point origin,
                          long minimum, long maximum)
{
    init = false; cleanup();

	client = c; global = g;

     	this->dev = &dev;
        ::parallel::queue q(dev); 
        sycl::queue qt = q.get();

	clientTotals = { g.dimensions.x / c.dimensions.x, g.dimensions.y / c.dimensions.y, g.dimensions.z / c.dimensions.z, 0 };
        int total_clients = clientTotals.x() * clientTotals.y() * clientTotals.z();

        coarseDimension = (sycl::int4)coarse;
        coarseScale = {
            ((float)coarseDimension.x()) / ((float)client.dimensions.x),
            ((float)coarseDimension.y()) / ((float)client.dimensions.y),
            ((float)coarseDimension.z()) / ((float)client.dimensions.z), 0.0f};

        mediumDimension = (sycl::int4)medium;
        mediumScale = {
            ((float)mediumDimension.x()) / ((float)client.dimensions.x),
            ((float)mediumDimension.y()) / ((float)client.dimensions.y),
            ((float)mediumDimension.z()) / ((float)client.dimensions.z), 0.0f};

        fineBuckets = (sycl::int4)fine;
        totalBuckets = fineBuckets.x() * fineBuckets.y() * fineBuckets.z();

        fineScale = {((float)fineBuckets.x()) / ((float)client.dimensions.x),
                     ((float)fineBuckets.y()) / ((float)client.dimensions.y),
                     ((float)fineBuckets.z()) / ((float)client.dimensions.z),
                     0.0f};

        hostBucketLengths = new int[totalBuckets];
	if (hostBucketLengths == NULL) return;
	memset(hostBucketLengths, 0, sizeof(int) * totalBuckets);

	for (int i = 0; i < totalBuckets; ++i)
	{
		hostBucketLengths[i] = minimum;
	}

        sycl::int4 k = {(int)floorf(origin.x * fineScale.x()),
                        (int)floorf(origin.y * fineScale.y()),
                        (int)floorf(origin.z * fineScale.z()), 0};

        sycl::int4 radius = {diameter.x / 2, diameter.y / 2, diameter.z / 2, 0};

        for (int z = 0; z < diameter.z; ++z)
	{
		for (int y = 0; y < diameter.y; ++y)
		{
			for (int x = 0; x < diameter.x; ++x)
			{
                sycl::int4 w = {k.x() - radius.x() + x,
                                k.y() - radius.y() + y,
                                k.z() - radius.z() + z, 0};

                if ((w.x() >= 0) && (w.x() < fineBuckets.x()))
                {
                    if ((w.y() >= 0) && (w.y() < fineBuckets.y()))
                    {
                        if ((w.z() >= 0) && (w.z() < fineBuckets.z()))
                        {
                            int bucket =
                                (w.y() *
                                    fineBuckets.x()) +
                                w.x() +
                                (fineBuckets.x() *
                                    fineBuckets.y() *
                                    w.z());
                            hostBucketLengths[bucket] = maximum;
						}
					}
				}
			}
		}
	}

    deviceHostPositions = new sycl::float4 *[totalBuckets];
    if (deviceHostPositions == NULL) return;

    deviceHostClients = new sycl::int4 *[totalBuckets];
    if (deviceHostClients == NULL) return;

    hostPinnedBucketIndices = sycl::malloc_host<int>(totalBuckets, qt);
    if(hostPinnedBucketIndices == NULL) return;

    deviceCoarseScaleMap = sycl::malloc_device<int>(sizeof(int) * (int)coarseDimension.x() * (int)coarseDimension.y() * (int)coarseDimension.z() * total_clients, qt);
    if(deviceCoarseScaleMap == NULL) return;

    deviceMediumScaleMap = sycl::malloc_device<int>(sizeof(int) * (int)mediumDimension.x() * (int)mediumDimension.y() * (int)mediumDimension.z() * total_clients, qt);
    if(deviceMediumScaleMap == NULL) return;

    deviceBucketIndices = sycl::malloc_device<int>(totalBuckets, qt);
    if(deviceBucketIndices == NULL) return;

    devicePositions = sycl::malloc_device<sycl::float4 *>(totalBuckets, qt);
    if(devicePositions == NULL) return;
    
    deviceClients = sycl::malloc_device<sycl::int4 *>(totalBuckets, qt);
    if(deviceClients == NULL) return;

    deviceBucketLengths = sycl::malloc_device<int>(totalBuckets, qt);
    if(deviceBucketLengths == NULL) return;

    qt.memcpy(deviceBucketLengths, hostBucketLengths, sizeof(int) * totalBuckets).wait();

    for (int i = 0; i < totalBuckets; ++i)
	{
	    deviceHostPositions[i] = NULL;
		deviceHostClients[i] = NULL;
	}

	for (int i = 0; i < totalBuckets; ++i)
	{
        deviceHostPositions[i] = sycl::malloc_device<sycl::float4>(hostBucketLengths[i], qt);
        if(deviceHostPositions[i] == NULL) return;

        deviceHostClients[i] = sycl::malloc_device<sycl::int4>(hostBucketLengths[i], qt);
        if(deviceHostClients[i] == NULL) return;

        qt.memcpy(&devicePositions[i], &deviceHostPositions[i], sizeof(sycl::float4 *)).wait();
        qt.memcpy(&deviceClients[i], &deviceHostClients[i], sizeof(sycl::int4 *)).wait();
    }

	clear(&q);

	init = true;
}

void parallel::map::clear(::parallel::queue *q)
{
    value = 0;

    sycl::queue& qt = ::parallel::queue::get_queue(*dev, q);
    
    qt.memset(deviceCoarseScaleMap, 0, sizeof(sycl::int2) * coarseDimension.x() * coarseDimension.y() * coarseDimension.z()).wait();
    qt.memset(deviceMediumScaleMap, 0, sizeof(sycl::int2) * mediumDimension.x() * mediumDimension.y() * mediumDimension.z()).wait();
    qt.memset(deviceBucketIndices, 0, sizeof(int) * totalBuckets).wait();

    qt.submit([&](sycl::handler &cgh) 
    {
        auto cudaPositions_ct0 = devicePositions;
        auto cudaClients_ct1 = deviceClients;
        auto totalBuckets_ct2 = totalBuckets;
        auto client_length_ct3 = client.length;

        cgh.parallel_for(
            sycl::nd_range<3>(sycl::range<3>(1, 1, client.grids) *
                                    sycl::range<3>(1, 1, client.threads),
                                sycl::range<3>(1, 1, client.threads)),
            [=](sycl::nd_item<3> item_ct1) {
                    ClearMap(cudaPositions_ct0, cudaClients_ct1,
                                totalBuckets_ct2, client_length_ct3,
                                item_ct1);
        });
    }).wait();        
}

void parallel::map::build(sycl::float4 *points, sycl::int4 *clients,
                          const int length, ::parallel::queue *q)
{        
    sycl::queue& qt = ::parallel::queue::get_queue(*dev, q);

	if (++value == INT_MAX)
	{
		value = 1;
		clear(q);
	}
	else
	{
        qt.memset(deviceBucketIndices, 0, sizeof(int) * totalBuckets).wait();
    }

    qt.submit([&](sycl::handler &cgh) 
    {
        auto cudaCoarseScaleMap_ct0 = deviceCoarseScaleMap;
        auto cudaMediumScaleMap_ct1 = deviceMediumScaleMap;
        auto cudaBucketIndices_ct2 = deviceBucketIndices;
        auto cudaBucketLengths_ct3 = deviceBucketLengths;
        auto cudaPositions_ct4 = devicePositions;
        auto cudaClients_ct5 = deviceClients;
        auto coarseScale_ct8 = coarseScale;
        auto coarseDimension_ct9 = coarseDimension;
        auto mediumScale_ct10 = mediumScale;
        auto mediumDimension_ct11 = mediumDimension;
        auto fineScale_ct12 = fineScale;
        auto fineBuckets_ct13 = fineBuckets;
        auto totalBuckets_ct14 = totalBuckets;
        auto client_dimensions_ct15 = client.dimensions;
        auto clientTotals_ct16 = clientTotals;
        auto value_ct17 = value;
        auto global_length_ct18 = global.length;

        cgh.parallel_for(
            sycl::nd_range<3>(sycl::range<3>(1, 1, global.grids) *
                                    sycl::range<3>(1, 1, global.threads),
                                sycl::range<3>(1, 1, global.threads)),
            [=](sycl::nd_item<3> item_ct1) {
                BuildMap(
                    cudaCoarseScaleMap_ct0, cudaMediumScaleMap_ct1,
                    cudaBucketIndices_ct2, cudaBucketLengths_ct3,
                    cudaPositions_ct4, cudaClients_ct5, points,
                    clients, coarseScale_ct8, coarseDimension_ct9,
                    mediumScale_ct10, mediumDimension_ct11,
                    fineScale_ct12, fineBuckets_ct13,
                    totalBuckets_ct14, client_dimensions_ct15,
                    clientTotals_ct16, value_ct17,
                    global_length_ct18, item_ct1);
        });
    }).wait();        
}

void parallel::map::search(sycl::float4 *search, sycl::int4 *clients, int *result,
                           const int length, const bool self, int *collided,
                           int index, ::parallel::queue *q)
{
        sycl::queue& qt = ::parallel::queue::get_queue(*dev, q);
        
        qt.submit([&](sycl::handler &cgh)
        {
                auto cudaCoarseScaleMap_ct3 = deviceCoarseScaleMap;
                auto cudaMediumScaleMap_ct4 = deviceMediumScaleMap;
                auto cudaBucketIndices_ct5 = deviceBucketIndices;
                auto cudaBucketLengths_ct6 = deviceBucketLengths;
                auto cudaPositions_ct7 = (const sycl::float4 **)devicePositions;
                auto cudaClients_ct8 = (const sycl::int4 **)deviceClients;
                auto coarseScale_ct9 = coarseScale;
                auto coarseDimension_ct10 = coarseDimension;
                auto mediumScale_ct11 = mediumScale;
                auto mediumDimension_ct12 = mediumDimension;
                auto fineScale_ct13 = fineScale;
                auto fineBuckets_ct14 = fineBuckets;
                auto totalBuckets_ct15 = totalBuckets;
                auto client_dimensions_ct16 = client.dimensions;
                auto clientTotals_ct17 = clientTotals;
                auto value_ct18 = value;

                cgh.parallel_for(
                    sycl::nd_range<3>(sycl::range<3>(1, 1, global.grids) *
                                          sycl::range<3>(1, 1, global.threads),
                                      sycl::range<3>(1, 1, global.threads)),
                    [=](sycl::nd_item<3> item_ct1) {
                            ScanMap(
                                result, search, clients, cudaCoarseScaleMap_ct3,
                                cudaMediumScaleMap_ct4, cudaBucketIndices_ct5,
                                cudaBucketLengths_ct6, cudaPositions_ct7,
                                cudaClients_ct8, coarseScale_ct9,
                                coarseDimension_ct10, mediumScale_ct11,
                                mediumDimension_ct12, fineScale_ct13,
                                fineBuckets_ct14, totalBuckets_ct15,
                                client_dimensions_ct16, clientTotals_ct17,
                                value_ct18, self, collided, index, NULL, NULL,
                                length, item_ct1);
                    });
        }).wait();
}

void parallel::map::search(sycl::float4 *search, sycl::int4 *clients,
                           sycl::int2 *result, const int length, const bool self,
                           const bool symetrical, const bool inverse, int *collided,
                           int index, ::parallel::queue *q)
{
        sycl::queue& qt = ::parallel::queue::get_queue(*dev, q);

        qt.submit([&](sycl::handler &cgh)
        {
                auto cudaCoarseScaleMap_ct3 = deviceCoarseScaleMap;
                auto cudaMediumScaleMap_ct4 = deviceMediumScaleMap;
                auto cudaBucketIndices_ct5 = deviceBucketIndices;
                auto cudaBucketLengths_ct6 = deviceBucketLengths;
                auto cudaPositions_ct7 = (const sycl::float4 **)devicePositions;
                auto cudaClients_ct8 = (const sycl::int4 **)deviceClients;
                auto coarseScale_ct9 = coarseScale;
                auto coarseDimension_ct10 = coarseDimension;
                auto mediumScale_ct11 = mediumScale;
                auto mediumDimension_ct12 = mediumDimension;
                auto fineScale_ct13 = fineScale;
                auto fineBuckets_ct14 = fineBuckets;
                auto totalBuckets_ct15 = totalBuckets;
                auto client_dimensions_ct16 = client.dimensions;
                auto clientTotals_ct17 = clientTotals;
                auto value_ct18 = value;

                cgh.parallel_for(
                    sycl::nd_range<3>(sycl::range<3>(1, 1, global.grids) *
                                          sycl::range<3>(1, 1, global.threads),
                                      sycl::range<3>(1, 1, global.threads)),
                    [=](sycl::nd_item<3> item_ct1) {
                            ScanMap(
                                result, search, clients, cudaCoarseScaleMap_ct3,
                                cudaMediumScaleMap_ct4, cudaBucketIndices_ct5,
                                cudaBucketLengths_ct6, cudaPositions_ct7,
                                cudaClients_ct8, coarseScale_ct9,
                                coarseDimension_ct10, mediumScale_ct11,
                                mediumDimension_ct12, fineScale_ct13,
                                fineBuckets_ct14, totalBuckets_ct15,
                                client_dimensions_ct16, clientTotals_ct17,
                                value_ct18, self, collided, index, symetrical,
                                inverse, NULL, NULL, length, item_ct1);
                    });
        }).wait();
}

std::string parallel::map::outputarb(int *source, int length)
{
    std::string result;

	int *temp = new int[length];
	if (temp == NULL) return result;

    sycl::queue q = queue(*dev).get();
    q.memcpy(temp, source, sizeof(int) * length).wait();

	for (int i = 0; i < length; ++i)
	{
		if (temp[i] != 0)
		{
			result += "[";
			result += std::to_string(i);
			result += "]";
			result += std::to_string(temp[i]);
			result += ",";
		}
	}
	result += "\r\n";

	delete[] temp;

    return result;
}

void parallel::map::makeNull()
{
    dev = NULL;

	hostBucketLengths = NULL;
	deviceHostPositions = NULL;
	deviceHostClients = NULL;
	hostPinnedBucketIndices = NULL;
	deviceCoarseScaleMap = NULL;
	deviceMediumScaleMap = NULL;
	deviceBucketIndices = NULL;
	devicePositions = NULL;
	deviceClients = NULL;
	deviceBucketLengths = NULL;
}

void parallel::map::cleanup()
{
    if(dev != NULL)
    {
        sycl::queue q = queue(*dev).get();

        if (deviceHostClients != NULL)
        {
            for (int i = 0; i < totalBuckets; ++i)
            {
                if (deviceHostClients[i] != NULL) sycl::free(deviceHostClients[i], q);
            }

            delete deviceHostClients;
        }

        if (deviceHostPositions != NULL)
        {
            for (int i = 0; i < totalBuckets; ++i)
            {
                if (deviceHostPositions[i] != NULL) sycl::free(deviceHostPositions[i], q);
            }

            delete deviceHostPositions;
        }

        if (deviceBucketLengths != NULL) sycl::free(deviceBucketLengths, q);
        if (deviceClients != NULL) sycl::free(deviceClients, q);
        if (devicePositions != NULL) sycl::free(devicePositions, q);
        if (deviceBucketIndices != NULL) sycl::free(deviceBucketIndices, q);
        if (deviceMediumScaleMap != NULL) sycl::free(deviceMediumScaleMap, q);
        if (deviceCoarseScaleMap != NULL) sycl::free(deviceCoarseScaleMap, q);
        if (hostPinnedBucketIndices != NULL) sycl::free(hostPinnedBucketIndices, q);
        if (hostBucketLengths != NULL) delete hostBucketLengths;
    }
}