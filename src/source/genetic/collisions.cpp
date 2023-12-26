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

void organisation::genetic::collisions::mutate(data &source, int offset)
{
    int value = (std::uniform_int_distribution<int>{0, 27})(generator);
    values[offset] = value;
}

