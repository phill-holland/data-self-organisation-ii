#include "genetic/movements.h"

std::mt19937_64 organisation::genetic::movements::generator(std::random_device{}());

size_t organisation::genetic::movements::size() 
{ 
    int total = 0;
    for(auto &it: values)
    {
        total += it.directions.size();
    }

    return total;
}

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

void organisation::genetic::movements::mutate(data &source)
{
    //#warning need to account for offset + 1 for starting position
    // offset is all movemnts
    /*
    int total = 0, index = 0;
    for(std::vector<movement>::iterator it = values.begin(); it != values.end(); ++it)
    {
        total += it->directions.size();
        if(offset < total) break;
        ++index;
    }
    */

    int n = (std::uniform_int_distribution<int>{1, (int)(values.size() - 1)})(generator);
    values[n].mutate(_width, _height, _depth);
}

// have a single starting position

void organisation::genetic::movements::copy(genetic *source, int src_start, int src_end, int dest_start)
{
    // take start point from source
    movements *s = dynamic_cast<movements*>(source);

    std::vector<movement> *current = &source->values[0];
    int length = src_end - src_start;
    int index = 0, offset = 0;
    int overall = 0;
    
    for()
    {

        ++index;
        if(index >= current.size())
        {
            ++offset;
            current = &source->values[offset];
            index = 0;
        }
    }
    //if(values.size() < (length + dest_start)) values.resize(length + dest_start);

}
