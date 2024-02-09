#include "genetic/movement.h"
#include <sstream>
#include <functional>
#include <iostream>

std::mt19937_64 organisation::genetic::movement::generator(std::random_device{}());

std::string organisation::genetic::movement::serialise()
{
    std::string result;

    for(auto &it: directions)
    {
        result += "M " + it.serialise() + "\n";
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

bool organisation::genetic::movement::validate(data &source)
{
    if(directions.empty()) { std::cout << "movement::validate(false): directions is empty\r\n"; return false; }

    for(auto &it: directions)
    {
/*        if(it.isempty())
        { 
            std::cout << "movement::validate(false): direction going nowhere\r\n"; 
            return false; 
        }
*/
        if((it.x < -1)||(it.x > 1)
            ||(it.y < -1)||(it.y > 1)
            ||(it.z < -1)||(it.z > 1))
        {
            std::cout << "movement::validate(false): direction out of bounds (" << it.x << "," << it.y << "," << it.z << ")\r\n"; 
            return false;
        }
    }

    return true;
}

void organisation::genetic::movement::generate(data &source)
{
    int n = (std::uniform_int_distribution<int>{MIN, MAX})(generator);

    for(int i = 0; i < n; ++i)
    {
        int value = (std::uniform_int_distribution<int>{0, 26})(generator);
        vector v1;
        if(v1.decode(value))
        {
            directions.push_back(v1);
        }
    }
}

bool organisation::genetic::movement::mutate(data &source)
{
    const int COUNTER = 15;

    int n = 0; 
    int value = 0, old = 0;
    int counter = 0;

    do
    {
        n = (std::uniform_int_distribution<int>{0, (int)(directions.size() - 1)})(generator);
        value = (std::uniform_int_distribution<int>{0, 26})(generator);

        old = directions[n].encode();
        vector v1;
        v1.decode(value);
        directions[n] = v1;   
    } while((old == value)&&(counter++<COUNTER));    
    
    if(old==value) return false;

    return true;
}

void organisation::genetic::movement::append(genetic *source, int src_start, int src_end)
{
    movement *s = dynamic_cast<movement*>(source);
    int length = src_end - src_start;
    
    for(int i = 0; i < length; ++i)
    {
        directions.push_back(s->directions[src_start + i]);
    }   
}

void organisation::genetic::movement::copy(const movement &source)
{
    directions.assign(source.directions.begin(), source.directions.end());
}

bool organisation::genetic::movement::equals(const movement &source)
{
    if(directions.size() != source.directions.size()) 
        return false;

    for(int i = 0; i < directions.size(); ++i)
    {
        if(directions[i] != source.directions[i]) 
            return false;
    }

    return true;
}