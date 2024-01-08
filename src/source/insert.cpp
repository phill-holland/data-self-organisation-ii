#include "genetic/insert.h"
#include <iostream>
#include <sstream>
#include <functional>

std::mt19937_64 organisation::genetic::insert::generator(std::random_device{}());

void organisation::genetic::insert::start()
{    
    current = 1;
    counter = values[0];
}

bool organisation::genetic::insert::get()
{
    bool result = false;    
    
    counter--;
    
    if(counter < 0) 
    {                        
        counter = values[current++];
        if(current >= values.size()) current = 0;

        result = true;
    }
    
    return result;
}

void organisation::genetic::insert::generate(data &source)
{
    //const int max = 15;
    //const int max_interval = 5;

    int length = (std::uniform_int_distribution<int>{1, LENGTH})(generator);

    for(int i = 0; i < length; ++i)
    {
        int value = (std::uniform_int_distribution<int>{MIN, MAX})(generator);
        values.push_back(value);
    }
}

void organisation::genetic::insert::mutate(data &source)
{    
    //const int max_interval = 5;

    int offset = (std::uniform_int_distribution<int>{0, (int)(values.size() - 1)})(generator);
    int value = (std::uniform_int_distribution<int>{MIN, MAX})(generator);
    
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

std::string organisation::genetic::insert::serialise()
{
    std::string result;

    for(auto &it: values)
    {
        result += "I " + std::to_string(it) + "\n";
    }

    return result;
}

void organisation::genetic::insert::deserialise(std::string source)
{
    std::stringstream ss(source);
    std::string str;

    int value = 0;
    int index = 0;

    while(std::getline(ss,str,' '))
    {
        if(index == 0)
        {
            if(str.compare("I")!=0) return;
        }
        else if(index == 1)
        {
            int value = std::atoi(str.c_str());
            values.push_back(value);
        }

        ++index;
    };
}

void organisation::genetic::insert::copy(const insert &source)
{
    values.assign(source.values.begin(), source.values.end());
}

bool organisation::genetic::insert::equals(const insert &source)
{
    if(values.size() != source.values.size()) return false;

    for(int i = 0; i < values.size(); ++i)
    {
        if(values[i] != source.values[i]) return false;
    }

    return true;
}