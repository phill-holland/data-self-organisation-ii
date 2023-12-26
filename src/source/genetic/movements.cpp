#include "genetic/movements.h"

std::mt19937_64 organisation::genetic::movements::generator(std::random_device{}());

void organisation::genetic::movements::generate(data &source)
{
    const int max_duplicate_insert = 2;
    int n = (std::uniform_int_distribution<int>{1, max_duplicate_insert})(generator);

    for(int i = 0; i <= n; ++i)
    {
        movement m1;
        m1.generate(_width, _height, _depth);
        values.push_back(m1);
    }
}

void organisation::genetic::movements::mutate(data &source, int offset)
{
    #warning need to account for offset + 1 for starting position
    // offset is all movemnts
    values[offset].mutate(_width, _height, _depth);
}