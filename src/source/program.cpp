#include "program.h"
#include <stack>
#include <unordered_map>
#include <iostream>
#include <tuple>
#include <fstream>
#include <sstream>

std::mt19937_64 organisation::program::generator(std::random_device{}());

void organisation::program::reset(int w, int h, int d)
{
    init = false; cleanup();

    _width = w; _height = h; _depth = d;
    length = _width * _height * _depth;

    cells.resize(length);
    //cells = new cell[length];
    //if(cells == NULL) return;

    clear();

    init = true;
}

void organisation::program::clear()
{    
    for(int i = 0; i < length; ++i)
    {
        cells[i].clear();
    }
}

void organisation::program::generate(data &source)
{
    clear();

    for(int i = 0; i < length; ++i)
    {
        //int k = (std::uniform_int_distribution<int>{1, source.maximum()})(generator);
        int k = (std::uniform_int_distribution<int>{0, source.maximum() - 1})(generator);
        cells[i].generate(k);
    }
}

void organisation::program::mutate(data &source)
{
    int j = (std::uniform_int_distribution<int>{0, length - 1})(generator);

    cells[j].mutate(source.maximum());
}

std::string organisation::program::run(int start, data &source, history *destination)
{
    std::vector<int> result;

    int x = (_width / 2) - start;
    int y = (_height / 2);
    int z = (_depth / 2);
    
    const int MAX = 20;

    std::vector<std::tuple<vector,vector>> positions;
    positions.push_back(std::tuple<vector,vector> (vector { x,y,z },vector {0,-1,0}));

    int counter = 0;
    while ((!(positions.empty()))&&(counter < MAX))
    {        
        std::tuple<vector,vector> temp = positions.back();

        vector current = std::get<0>(temp);
        vector next = std::get<1>(temp);

        positions.pop_back();

        int index = (current.z * _width * _height) + (current.y * _width) + current.x;
        
        vector input = next.normalise().inverse();

        if(cells[index].is_input(input))
        {
            if(destination != NULL) destination->push(current, cells[index].value);

            if(!cells[index].is_empty())
            {                
                result.push_back(cells[index].value);
            }
            
            std::vector<vector> outputs = cells[index].outputs(input);
            for(std::vector<vector>::iterator ij = outputs.begin(); ij != outputs.end(); ++ij)
            {
                vector t = *ij;
                vector position = { current.x + t.x, current.y + t.y, current.z + t.z };

                if((position.x >= 0)&&(position.y >= 0)&&(position.z >= 0))
                {
                    if((position.x < _width)&&(position.y < _height)&&(position.z < _depth))
                    {
                        positions.push_back(std::tuple<vector,vector>(position, t));
                    }
                }
            }
        }

        ++counter;
    };
    
    return source.get(result);
}

int organisation::program::count()
{
    int result = 0;

    for(int i = 0; i < length; ++i)
    {
        if(cells[i].value > -1) ++result;
    }

    return result;
}

void organisation::program::set(int value, int x, int y, int z)
{
    if ((x < 0)||(x >= _width)) return;
    if ((y < 0)||(y >= _height)) return;
    if ((z < 0)||(z >= _depth)) return;

    int index = (z * _width * _height) + (y * _width) + x;

    cells[index].value = value;
}

void organisation::program::set(vector input, vector output, int magnitude, int x, int y, int z)
{
    if ((x < 0)||(x >= _width)) return;
    if ((y < 0)||(y >= _height)) return;
    if ((z < 0)||(z >= _depth)) return;

    int index = (z * _width * _height) + (y * _width) + x;

    cells[index].set(input, output, magnitude);
}

bool organisation::program::validate(data &source)
{
    if(count() <= 0) 
        return false;

    int gates = 0;
    for(int i = 0; i < length; ++i)
    {
        std::tuple<bool,bool> temp = cells[i].validate(source.maximum());
        if(!std::get<0>(temp)) return false;
        if(std::get<1>(temp)) ++gates;
    }

    if(gates <= 0) 
        return false;

    return true;
}

void organisation::program::copy(const program &source)
{
    cells.assign(source.cells.begin(), source.cells.end());
    _width = source._width;
    _height = source._height;
    _depth = source._depth;
    length = source.length;
}

bool organisation::program::equals(const program &source)
{
    for(int i = 0; i < length; ++i)
    {
        if(!cells[i].equals(source.cells[i])) return false;
    }

    return true;
}

void organisation::program::cross(program &a, program &b, int middle)
{
    if((a.length != b.length)||(length != a.length)) return;

    int a1 = 0, b1 = 0;
    if(middle < 0)
    {
        while(a1 == b1)
        {
            a1 = (std::uniform_int_distribution<int>{0, (int)(length - 1)})(generator);
            b1 = (std::uniform_int_distribution<int>{0, (int)(length - 1)})(generator);
        };

        if(a1 > b1)
        {
            int temp = a1;
            a1 = b1;
            b1 = temp;
        }
    }
    else
    {
        a1 = middle;
        b1 = length;
    }

    int index = 0;
    for(int i = 0; i < a1; ++i)
    {
        cells[index].copy(a.cells[i]);
		++index;
    }

    for(int i = a1; i < b1; ++i)
    {
        cells[index].copy(b.cells[i]);
        ++index;
    }

    for(int i = b1; i < length; ++i)
    {
        cells[index].copy(a.cells[i]);
        ++index;
    }    
}

void organisation::program::save(std::string filename)
{
     std::fstream output(filename, std::fstream::out | std::fstream::binary);

    if(output.is_open())
    {
        for(int z = 0; z < _depth; ++z)
        {    
            std::string line;        
            for(int y = 0; y < _height; ++y)
            {                
                for(int x = 0; x < _width; ++x)
                {
                    int index = (z * _width * _height) + (y * _width) + x;

                    line += "P ";
                    line += std::to_string(x) + ",";
                    line += std::to_string(y) + ",";
                    line += std::to_string(z);
                    line += "\r\n";

                    line += "V " + std::to_string(cells[index].value) + "\r\n";

                    std::vector<int> in_gates = cells[index].pull();
                    for(std::vector<int>::iterator it = in_gates.begin(); it != in_gates.end(); ++it)
                    {                     
                        line += "G " + std::to_string(*it);
                        line += " ";
                        
                        std::vector<int> out_gates = cells[index].pull(*it);
                        for(std::vector<int>::iterator jt = out_gates.begin(); jt != out_gates.end(); ++jt)
                        {
                            organisation::gate gate = cells[index].get(*it,*jt);

                            line += std::to_string(*jt);
                            line += "/" + std::to_string(gate.magnitude);
                            if(jt != out_gates.end() - 1) line += ",";
                        }

                        line += "\r\n";
                    }
                    line += "\r\n";
                }              
            }

            output.write(line.c_str(), line.size());
        }

        output.close();
    }   
}

void organisation::program::load(std::string filename)
{
    std::ifstream source(filename);
    if(source.is_open())
    {
        int x = 0, y = 0, z = 0;
        int *position[] = { &x, &y, &z };

        for(std::string value; getline(source, value); )
        {
            std::stringstream stream(value);
		    std::string type, word;
	            
            if(stream >> type)
            {
                if(type == "P")
                {
                    std::vector<std::string> words;
                    while(stream >> word) words.push_back(word);
                    
                    if(words[0].size() > 0)
                    {
                        std::stringstream ss(words[0]); 
                        std::string str;
                        int index = 0;
                        while (std::getline(ss, str, ',')) 
                        {
                            if(index < 3)
                            {                            
                                int temp = std::atoi(str.c_str());
                                *position[index] = temp;
                            }
                            ++index;                        
                        }
                    }
                }
                else if(type == "V")
                {
                    std::vector<std::string> words;
                    while(stream >> word) words.push_back(word);
                    if(words.size() > 0)
                    {
                        int index = (z * _width * _height) + (y * _width) + x;
                        cells[index].value = std::atoi(words[0].c_str());
                    }
                }
                else if(type == "G")
                {
                    std::vector<std::string> words;
                    while(stream >> word) words.push_back(word);
                    
                    if(words.size() >= 1)
                    {
                        int in_gate = std::atoi(words[0].c_str());
                    

                        int index = (z * _width * _height) + (y * _width) + x;
                        cells[index].set(in_gate);

                        if(words.size() >= 2)
                        {
                            std::stringstream ss(words[1]); 
                            std::string str;

                            while (std::getline(ss, str, ',')) 
                            {

                                std::stringstream ww(str);
                                std::string vv;
                                std::vector<std::string> values;
                                while(std::getline(ww, vv, '/'))
                                {
                                    values.push_back(vv);
                                };

                                if(values.size() > 1)
                                {
                                    int out_gate = std::atoi(values[0].c_str());
                                    int magnitude = std::atoi(values[1].c_str());

                                    int index = (z * _width * _height) + (y * _width) + x;
                                    cells[index].set(in_gate, out_gate, organisation::gate(magnitude)); 
                                }
                            }
                        }
                    }
                }
            }
        }

        source.close();
    }
}
    
void organisation::program::makeNull()
{
    
}

void organisation::program::cleanup()
{

}