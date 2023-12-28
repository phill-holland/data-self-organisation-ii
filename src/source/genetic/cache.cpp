#include "genetic/cache.h"

std::mt19937_64 organisation::genetic::cache::generator(std::random_device{}());

void organisation::genetic::cache::generate(data &source)
{
    clear();

    const int max_repeats = 2;
    const int max_cache = source.maximum();

    std::vector<int> raw = source.all();
    
    for(auto &it: raw)
    {
        int n = (std::uniform_int_distribution<int>{0, max_repeats})(generator);
        for(int i = 0; i < n; ++i)
        {
            point p1;
            p1.generate(_width, _height, _depth);
            int index = ((_width * _height) * p1.z) + ((p1.y * _width) + p1.x);
            if(points.find(index) == points.end())
            {
                values.push_back(std::tuple<int,point>(it,p1));
                if(values.size() >= max_cache) return;
            }
        }
    }
}

void organisation::genetic::cache::mutate(data &source)
{
    int offset = (std::uniform_int_distribution<int>{0, (int)(values.size() - 1)})(generator);
    
    int value = std::get<0>(values[offset]);
    point p1;

    p1.generate(_width, _height, _depth);
    int index = ((_width * _height) * p1.z) + ((p1.y * _width) + p1.x);

    if(points.find(index) != points.end()) p1 = std::get<1>(values[offset]);

    std::vector<int> all = source.all();
    int t1 = (std::uniform_int_distribution<int>{0, (int)(all.size() - 1)})(generator);
    value = all[t1];
}

void organisation::genetic::cache::copy(genetic *source, int src_start, int src_end, int dest_start)
{
    cache *s = dynamic_cast<cache*>(source);
    int length = src_end - src_start;
    if(values.size() < (length + dest_start)) values.resize(length + dest_start);

    for(int i = 0; i < length; ++i)
    {
        std::tuple<int,point> temp = s->values[src_start + i];        
        values[dest_start + i] = temp;

        point p1 = std::get<1>(temp);
        int index = ((_width * _height) * p1.z) + ((p1.y * _width) + p1.x);
        if(points.find(index) == points.end())
        {
            points[index] = p1;
        }
    }
}