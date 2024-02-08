#include "genetic/cache.h"
#include <sstream>
#include <functional>
#include <iostream>

std::mt19937_64 organisation::genetic::cache::generator(std::random_device{}());

bool organisation::genetic::cache::set(int value, point position)
{
    int index = ((_width * _height) * position.z) + ((position.y * _width) + position.x);
    if(points.find(index) == points.end())
    {
        points[index] = position;
        values.push_back(std::tuple<int,point>(value,position));

        return true;
    }

    return false;
}

std::string organisation::genetic::cache::serialise()
{
    std::string result;

    for(auto &it: values)
    {     
        result += "D " + std::to_string(std::get<0>(it));
        result += " " + std::get<1>(it).serialise() + "\n";            
    }

    return result;                    
}

void organisation::genetic::cache::deserialise(std::string source)
{
    std::stringstream ss(source);
    std::string str;

    int value = 0;
    int index = 0;

    while(std::getline(ss,str,' '))
    {
        if(index == 0)
        {
            if(str.compare("D")!=0) return;
        }
        else if(index == 1)
        {
            int value = std::atoi(str.c_str());
        }
        else if(index == 2)
        {
            organisation::point temp;

            temp.deserialise(str);
            int index = ((_width * _height) * temp.z) + ((temp.y * _width) + temp.x);

            if(points.find(index) == points.end())
            {
                points[index] = temp;
                values.push_back(std::tuple<int,point>(value,temp));
            }
        }

        ++index;
    };
}

bool organisation::genetic::cache::validate(data &source)
{
    //if(values.empty()) { std::cout << "cache::validate(false): values empty\r\n"; return false; }
    //if(points.empty()) { std::cout << "cache::validate(false): points empty\r\n"; return false; }

    if(values.size() != points.size()) { std::cout << "cache::validate(false): values.size(" << values.size() << ") != points.size(" << points.size() << ")\r\n"; return false; }

    std::unordered_map<int, point> duplicates;

    for(auto &it: values)
    {
        int value = std::get<0>(it);

        if(source.map(value).empty()) { std::cout << "cache::validate(false): map empty\r\n"; return false; }

        point position = std::get<1>(it);

        if(!position.inside(_width, _height, _depth))
        {
            std::cout << "cache::validate(false): invalid position\r\n"; 
            return false; 
        }

        int index = ((_width * _height) * position.z) + ((position.y * _width) + position.x);
        if(duplicates.find(index) == duplicates.end())
            duplicates[index] = position;
        else
        {
            std::cout << "cache::validate(false): duplicate position\r\n"; 
            return false;  
        }

        if(points.find(index) == points.end()) { std::cout << "cache::validate(false): point(index)\r\n"; return false; }
        if(points[index] != position) { std::cout << "cache::validate(false): invalid position\r\n"; return false; }
    }

    return true;
}

void organisation::genetic::cache::generate(data &source)
{
    clear();

    const int max_repeats = 2;
    const int max_cache = source.maximum();

    std::vector<int> raw = source.all();
    
    for(auto &it: raw)
    {
        int n = (std::uniform_int_distribution<int>{0, max_repeats})(generator);
        for(int i = 0; i < n; ++i)
        {
            point p1;
            p1.generate(_width, _height, _depth);
            int index = ((_width * _height) * p1.z) + ((p1.y * _width) + p1.x);
            if(points.find(index) == points.end())
            {
                points[index] = p1;
                values.push_back(std::tuple<int,point>(it,p1));
                if(values.size() >= max_cache) return;
            }
        }
    }
}

void organisation::genetic::cache::mutate(data &source)
{
    if(values.empty()) { std::cout << "not mutated\r\n"; return; }

    int offset = (std::uniform_int_distribution<int>{0, (int)(values.size() - 1)})(generator);
    
    //int value = std::get<0>(values[offset]);
    point p1;

    p1.generate(_width, _height, _depth);
    int index = ((_width * _height) * p1.z) + ((p1.y * _width) + p1.x);

    //if(points.find(index) != points.end()) p1 = std::get<1>(values[offset]);

    std::vector<int> all = source.all();
    int t1 = (std::uniform_int_distribution<int>{0, (int)(all.size() - 1)})(generator);
    int value = all[t1];

    if(points.find(index) == points.end()) 
    {
        std::cout << "cache new point!\r\n";
        points[index] = { };
        points[index] = p1;
        values.push_back(std::tuple<int,point>(value,p1));
    }
    else
    {
        for(auto &it: values)
        {
            point &temp = std::get<1>(it);
            if(temp == p1) 
            {   
                std::cout << "cache before " << std::get<0>(it);             
                std::get<0>(it) = value;

                std::cout << " after " << std::get<0>(it) << "\r\n";

                return;
            }
        }
    }
}

void organisation::genetic::cache::append(genetic *source, int src_start, int src_end)
{
std::cout << "cache::append " << src_start << "," << src_end << "," << "\r\n";    

    cache *s = dynamic_cast<cache*>(source);
    int length = src_end - src_start;

    for(int i = 0; i < length; ++i)
    {
        std::tuple<int,point> temp = s->values[src_start + i];        

        point p1 = std::get<1>(temp);
        int index = ((_width * _height) * p1.z) + ((p1.y * _width) + p1.x);
        if(points.find(index) == points.end())
        {
            values.push_back(temp);
            points[index] = p1;            
        }
    }
}

void organisation::genetic::cache::copy(const cache &source)
{
    _width = source._width;
    _height = source._height;
    _depth = source._depth;

    values = source.values;
    points = source.points;
}

bool organisation::genetic::cache::equals(const cache &source)
{
    if(values.size() != source.values.size()) 
        return false;
    if(points.size() != source.points.size())
        return false;

    for(int i = 0; i < values.size(); ++i)
    {
        std::tuple<int,point> a = values[i];
        std::tuple<int,point> b = source.values[i];

        if(a != b) 
            return false;
    }

    for(auto &it: points)
    {
        if(source.points.find(it.first) == source.points.end()) 
            return false;
        if(!(source.points.at(it.first) == it.second)) 
            return false;        
    }

    return true;
}