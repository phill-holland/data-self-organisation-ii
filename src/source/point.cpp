#include "point.h"
#include <sstream>
#include <functional>

std::mt19937_64 organisation::point::generator(std::random_device{}());

void organisation::point::generate(int max_x, int max_y, int max_z, int min_x, int min_y, int min_z)
{
    x = (std::uniform_int_distribution<int>{min_x, max_x - 1})(generator);
    y = (std::uniform_int_distribution<int>{min_y, max_y - 1})(generator);
    z = (std::uniform_int_distribution<int>{min_z, max_z - 1})(generator);
}

std::string organisation::point::serialise()
{
    std::string result("(");

    result += std::to_string(x);
    result += ",";
    result += std::to_string(y);
    result += ",";
    result += std::to_string(z);
    result += ")";

    return result;
}

void organisation::point::deserialise(std::string source)
{
    if(source.size() < 2) return;
    if(source.front() != '(') return;
    if(source.back() != ')') return;

    clear();

    int *position[] = { &x, &y, &z };

    std::string temp = source.substr(1, source.size() - 2);
    std::stringstream ss(temp);

    int index = 0;
    std::string str;

    while(std::getline(ss, str, ','))
    {
        if(index < 3)
        {
            int temp = std::atoi(str.c_str());
            *position[index++] = temp;  
        }
    }
}