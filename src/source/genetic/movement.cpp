#include "genetic/movement.h"

std::mt19937_64 organisation::genetic::movement::generator(std::random_device{}());

void organisation::genetic::movement::generate(data &source)
{
    starting.generate(_width, _height, _depth);

    const int max_movements = 10;
    int n = (std::uniform_int_distribution<int>{0, max_movements})(generator);

    for(int i = 0; i < n; ++i)
    {
        int value = (std::uniform_int_distribution<int>{0, 27})(generator);
        vector v1;
        if(v1.decode(value))
        {
            directions.push_back(v1);
        }
    }
}

void organisation::genetic::movement::mutate(data &source)
{
    int s1 = (std::uniform_int_distribution<int>{0, (int)directions.size()})(generator);
    int n = (std::uniform_int_distribution<int>{0, (int)directions.size()})(generator);

    if(n == s1) starting.generate(_width, _height, _depth);
    else
    {
        if(n > s1) n -= 1;        
        int value = (std::uniform_int_distribution<int>{0, 27})(generator);
        vector v1;
        v1.decode(value);
        directions[n] = v1;   
    }
}

void organisation::genetic::movement::copy(genetic *source, int src_start, int src_end, int dest_start)
{
    movement *s = dynamic_cast<movement*>(source);
    int length = src_end - src_start;
    if(directions.size() < (length + dest_start)) directions.resize(length + dest_start);

    for(int i = 0; i < length; ++i)
    {
        directions[dest_start + i] = s->directions[src_start + i];
    }   

    starting = s->starting;
}