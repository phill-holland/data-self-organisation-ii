#include "genetic/collisions.h"
#include <sstream>
#include <functional>
#include <iostream>

std::mt19937_64 organisation::genetic::collisions::generator(std::random_device{}());

void organisation::genetic::collisions::generate(data &source)
{
    for(int i = 0; i < 26; ++i)
    {
        int value = (std::uniform_int_distribution<int>{0, 26})(generator);
        values.push_back(value);
    }
}

void organisation::genetic::collisions::mutate(data &source)
{
    const int COUNTER = 15;

    int offset = 0;
    int value = 0, old = 0;
    int counter = 0;   

    do
    {
        offset = (std::uniform_int_distribution<int>{0, (int)(values.size() - 1)})(generator);
        value = (std::uniform_int_distribution<int>{0, 26})(generator);

        old = values[offset];
        values[offset] = value;

    }while((old == value)&&(counter++<COUNTER));
    std::cout << "collisions before " << old << " after " << value << "\r\n";    
}

void organisation::genetic::collisions::copy(genetic *source, int src_start, int src_end, int dest_start)
{
#warning needs fix length splicing here
    collisions *s = dynamic_cast<collisions*>(source);

    // ignore dest_start ??        
    int length = src_end - src_start;
    if(length + dest_start > values.size())
        length -= ((length + dest_start) - values.size());
    //if(d.size() < (length + dest_start)) directions.resize(length + dest_start);
    //if(values.size() < (length + dest_start)) values.resize(length + dest_start);

    for(int i = 0; i < length; ++i)
    {
        values[dest_start + i] = s->values[src_start + i];
    }   
}

std::string organisation::genetic::collisions::serialise()
{
    std::string result;

    for(auto &it: values)
    {
        result += "C " + std::to_string(it) + "\n";
    }

    return result;
}

void organisation::genetic::collisions::deserialise(std::string source)
{
    std::stringstream ss(source);
    std::string str;

    int value = 0;
    int index = 0;

    while(std::getline(ss,str,' '))
    {
        if(index == 0)
        {
            if(str.compare("C")!=0) return;
        }
        else if(index == 1)
        {
            int value = std::atoi(str.c_str());
            values.push_back(value);
        }

        ++index;
    };
}

bool organisation::genetic::collisions::validate(data &source)
{
    if(values.size() != 26) { std::cout << "collisions::validate(false): values.size() != 26 (" << values.size() << ")\r\n"; return false; }
    
    for(auto &it: values)
    {
        if((it < 0)||(it >= 27)) 
        { 
            std::cout << "collisions::validate(false): it < 0 || it > 27\r\n"; 
            return false; }
    }

    return true;
}

void organisation::genetic::collisions::copy(const collisions &source)
{
    values.assign(source.values.begin(), source.values.end());
}

bool organisation::genetic::collisions::equals(const collisions &source)
{
    if(values.size() != source.values.size()) 
        return false;

    for(int i = 0; i < values.size(); ++i)
    {
        if(values[i] != source.values[i]) 
            return false;
    }

    return true;
}