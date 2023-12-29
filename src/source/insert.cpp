#include "genetic/insert.h"

std::mt19937_64 organisation::genetic::insert::generator(std::random_device{}());

void organisation::genetic::insert::start()
{
    counter = 0;
    current = 0;

    counter = values[0];
}

bool organisation::genetic::insert::get()
{
    bool result = false;    
    
    counter--;
    
    if(counter <= 0) 
    {        
        ++current;
        if(current >= values.size()) current = 0;

        counter = values[current];

        result = true;
    }
    
    return result;
}

void organisation::genetic::insert::generate(data &source)
{
    const int max = 15;
    const int max_interval = 5;

    int length = (std::uniform_int_distribution<int>{1, max})(generator);

    for(int i = 0; i < length; ++i)
    {
        int value = (std::uniform_int_distribution<int>{0, max_interval})(generator);
        values.push_back(value);
    }
}

void organisation::genetic::insert::mutate(data &source)
{    
    const int max_interval = 5;

    int offset = (std::uniform_int_distribution<int>{0, (int)(values.size() - 1)})(generator);
    int value = (std::uniform_int_distribution<int>{0, max_interval})(generator);
    
    values[offset] = value;    
}

void organisation::genetic::insert::copy(genetic *source, int src_start, int src_end, int dest_start)
{
    insert *s = dynamic_cast<insert*>(source);

    int length = src_end - src_start;  
    if(values.size() < (length + dest_start)) values.resize(length + dest_start);

    for(int i = 0; i < length; ++i)
    {
        values[dest_start + i] = s->values[src_start + i];
    }   
}
