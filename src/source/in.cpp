#include "in.h"
#include <iostream>
#include <algorithm>

std::mt19937_64 organisation::in::generator(std::random_device{}());

void organisation::in::clear()
{
    write = 0;
}

bool organisation::in::is_empty(int index)
{
    organisation::gates *result = std::find_if(std::begin(gates),std::begin(gates) + write,
    [index](organisation::gates const &source) { return source.encoded == index; });

    if(result == std::begin(gates) + write) return true;

    return false;
}

void organisation::in::set(int in)
{
    if(write >= gates::IN) 
        return;
    if(in == -1) 
        return;

    organisation::gates *result = std::find_if(std::begin(gates),std::begin(gates) + write,
    [in](organisation::gates const &source) { return source.encoded == in; });

    if(result == std::begin(gates) + write)
    {
        gates[write].clear();
        gates[write++].encoded = in;
    }    
}

void organisation::in::set(int in, int out, gate value)
{
    if(write >= gates::IN) 
        return;
    if(in == -1) 
        return;

    organisation::gates *result = std::find_if(std::begin(gates),std::begin(gates) + write,
    [in](organisation::gates const &source) { return source.encoded == in; });

    if(result == std::begin(gates) + write)
    {
        gates[write].clear();
        gates[write].encoded = in;
        gates[write].set(out, value);
        ++write;    
    }
    else
    {
        result->set(out, value);        
    }
}

organisation::gate organisation::in::get(int in, int out)
{ 
    organisation::gates *result = std::find_if(std::begin(gates),std::begin(gates) + write,
    [in](organisation::gates const &source) { return source.encoded == in; });


    if(result != std::begin(gates) + write)
    {
        return result->get(out);
    }

    return gate();
}

std::vector<organisation::vector> organisation::in::outputs(int index)
{
    organisation::gates *result = std::find_if(std::begin(gates),std::begin(gates) + write,
    [index](organisation::gates const &source) { return source.encoded == index; });

    if(result == std::begin(gates) + write) return std::vector<organisation::vector>();
    
    return result->get();
}

void organisation::in::generate()
{
    clear();

    int in = (std::uniform_int_distribution<int>{0, gates::IN})(generator);     

    for(int i = 0; i < in; ++i)
    {
        int k = (std::uniform_int_distribution<int>{0, 26})(generator);         
        int out = (std::uniform_int_distribution<int>{0, gates::OUT})(generator);     

        set(k);      

        for(int j = 0; j < out; ++j)
        {
            int magnitude = (std::uniform_int_distribution<int>{1, gate::MAGNITUDE})(generator);
            int m = (std::uniform_int_distribution<int>{0, 26})(generator);         

            set(k, m, gate(magnitude));
        }
    }
}

void organisation::in::mutate()
{
    int k = (std::uniform_int_distribution<int>{0, 26})(generator); 
    int m = (std::uniform_int_distribution<int>{0, 26})(generator); 

    if(get(k,m).magnitude == -1)
    {
        int magnitude = (std::uniform_int_distribution<int>{1, gate::MAGNITUDE})(generator);
        set(k,m,gate(magnitude));
    }
    else set(k,m,gate());
}

bool organisation::in::validate(int &in)
{
    in = 0;

    for(int i = 0; i < write; ++i)
    {
        int out = 0;
        if(!gates[i].validate(out)) 
            return false;

        if(out > 0) ++in;
    }

    return true;
}

std::vector<int> organisation::in::pull()
{
    std::vector<int> result;

    for(int i = 0; i < write; ++i) result.push_back(gates[i].encoded);

    return result;
}

std::vector<int> organisation::in::pull(int in)
{
    organisation::gates *result = std::find_if(std::begin(gates),std::begin(gates) + write,
    [in](organisation::gates const &source) { return source.encoded == in; });

    if(result != std::begin(gates) + write)
    {
        return result->pull();
    }

    return std::vector<int>();
}
    
bool organisation::in::equals(const in &source)
{
    if(source.write != write) return false;

    for(int i = 0; i < source.write; ++i)
    {
        if(!gates[i].equals(source.gates[i])) return false;
    }

    return true;
}

void organisation::in::copy(const in &source)
{
    clear();

    write = source.write;    
    for(int i = 0; i < source.write; ++i)
    {
        gates[i].copy(source.gates[i]);
    }
}
