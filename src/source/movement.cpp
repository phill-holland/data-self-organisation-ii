#include "movement.h"

std::mt19937_64 organisation::movement::generator(std::random_device{}());

void organisation::movement::generate(int width, int height, int depth)
{
    starting.generate(width, height, depth);

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

void organisation::movement::mutate(int width, int height, int depth)
{
    int s1 = (std::uniform_int_distribution<int>{0, (int)directions.size()})(generator);
    int n = (std::uniform_int_distribution<int>{0, (int)directions.size()})(generator);

    if(n == s1) starting.generate(width, height, depth);
    else
    {
        if(n > s1) n -= 1;        
        int value = (std::uniform_int_distribution<int>{0, 27})(generator);
        vector v1;
        v1.decode(value);
        directions[n] = v1;   
    }
}