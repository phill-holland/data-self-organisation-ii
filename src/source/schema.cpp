#include "schema.h"
#include <iostream>
#include <tuple>

std::mt19937_64 organisation::schema::generator(std::random_device{}());

void organisation::schema::reset()
{
    init = false; cleanup();

    init = true;
}

void organisation::schema::clear()
{
    prog.clear();
    scores.clear();
}

void organisation::schema::generate(data &source)
{
    prog.clear();
    prog.generate(source);
}

bool organisation::schema::get(kdpoint &destination, int minimum, int maximum)
{
    int counter = 0;

    for(auto &it: scores)
    {     
        for(int j = 0; j < it.second.size(); ++j)
        {
            float temp = it.second.get(j);
            temp = (temp * ((float)maximum - minimum)) + minimum;
            if(!destination.set((long)temp, counter)) return false;

            ++counter;
        }        
    }

    return true;
}

std::vector<float> organisation::schema::get()
{
    std::vector<float> result;

    for(auto &it: scores)
    {     
        for(int j = 0; j < it.second.size(); ++j)
        {
            float temp = it.second.get(j);
            result.push_back(temp);
        }        
    }

    return result;
}

float organisation::schema::get(int dimension)
{
    int temp = dimension;
    int index = 0;

    while(temp >= scores[index].size())
    {
        int length = scores[index].size();
        if(length == 0) return 0.0f;
        
        temp -= length;        
        ++index;
    }

    return scores[index].get(temp);
}

float organisation::schema::sum()
{
    float result = 0.0f;
    if(scores.size() <= 0) 
        return 0.0f;

    for(auto &it: scores)
    {
        result += it.second.sum();
    }

    return result / ((float)scores.size());
}

void organisation::schema::compute(std::vector<std::tuple<std::string,std::string>> values)
{
    int i = 0;
    bool penalty = false;
    for(std::vector<std::tuple<std::string,std::string>>::iterator it = values.begin(); it != values.end(); ++it)
    {
        scores[i].compute(std::get<0>(*it),std::get<1>(*it));
        ++i;

        if(std::get<1>(*it).size() <= 0) penalty = true;
    }

    if(penalty)
    {
        int i = 0;
        for(std::vector<std::tuple<std::string,std::string>>::iterator it = values.begin(); it != values.end(); ++it)
        {
            scores[i].clear();
            ++i;
        }
    }
}

void organisation::schema::mutate(data &source)
{
	prog.mutate(source);
}

void organisation::schema::cross(schema *destination, schema *value)
{   
    destination->prog.cross(prog, value->prog);
}

std::string organisation::schema::run(int epoch, std::string input, std::string expected, data &source)
{		    
    std::string output = prog.run(input, source);
    scores[epoch].compute(expected, output);
    
    return output;
}

void organisation::schema::copy(const schema &source)
{    
    prog.copy(source.prog);
    scores = source.scores;
}

void organisation::schema::makeNull()
{

}

void organisation::schema::cleanup()
{
    
}