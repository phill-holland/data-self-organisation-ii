#include "genetic/movement.h"
#include <sstream>
#include <functional>

std::mt19937_64 organisation::genetic::movement::generator(std::random_device{}());

std::string organisation::genetic::movement::serialise()
{
    std::string result;

    for(auto &it: directions)
    {
        result += "M " + it.serialise() + "\r\n";
    }

    return result;
}

void organisation::genetic::movement::deserialise(std::string source)
{
    std::stringstream ss(source);
    std::string str;

    int value = 0;
    int index = 0;

    while(std::getline(ss,str,' '))
    {
        if(index == 0)
        {
            if(str.compare("M")!=0) return;
        }
        else if(index == 1)
        {
            organisation::vector temp;

            temp.deserialise(str);
            directions.push_back(temp);            
        }

        ++index;
    };
}

void organisation::genetic::movement::generate(data &source)
{
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
    int n = (std::uniform_int_distribution<int>{0, (int)(directions.size() - 1)})(generator);

    int value = (std::uniform_int_distribution<int>{0, 27})(generator);
    vector v1;
    v1.decode(value);
    directions[n] = v1;   
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
}

void organisation::genetic::movement::copy(const movement &source)
{
    directions.assign(source.directions.begin(), source.directions.end());
}

bool organisation::genetic::movement::equals(const movement &source)
{
    if(directions.size() != source.directions.size()) return false;

    for(int i = 0; i < directions.size(); ++i)
    {
        if(directions[i] != source.directions[i]) return false;
    }

    return true;
}