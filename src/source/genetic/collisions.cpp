#include "genetic/collisions.h"

std::mt19937_64 organisation::genetic::collisions::generator(std::random_device{}());

void organisation::genetic::collisions::generate(data &source)
{
    for(int i = 0; i <= 27; ++i)
    {
        int value = (std::uniform_int_distribution<int>{0, 27})(generator);
        values.push_back(value);
    }
}

void organisation::genetic::collisions::mutate(data &source)
{
    int offset = (std::uniform_int_distribution<int>{0, (int)(values.size() - 1)})(generator);
    int value = (std::uniform_int_distribution<int>{0, 27})(generator);
    
    values[offset] = value;
}

void organisation::genetic::collisions::copy(genetic *source, int src_start, int src_end, int dest_start)
{
#warning needs fix length splicing here
    collisions *s = dynamic_cast<collisions*>(source);

    // ignore dest_start ??        
    int length = src_end - src_start;
    if(length > values.size()) length = values.size();
    //if(values.size() < (length + dest_start)) values.resize(length + dest_start);

    for(int i = 0; i < length; ++i)
    {
        directions[dest_start + i] = source->directions[src_start + i];
    }   
}
