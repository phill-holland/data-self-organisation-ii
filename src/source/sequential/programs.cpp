#include "sequential/programs.h"

void organisation::sequential::programs::reset(parameters settings, int clients)
{
    init = false; cleanup();
    
    this->clients = clients;
    this->params = settings;

    schemas = new ::organisation::schema*[clients];
    if(schemas == NULL) return;
    
    for(int i = 0; i < clients; ++i)
    {
        schemas[i] = NULL;
    }

    results = new ::organisation::output[clients];
    if(results == NULL) return;

    inputs = new organisation::inputs::input[clients];
    if(inputs == NULL) return;

    init = true;
}

void organisation::sequential::programs::clear()
{
    for(int i = 0; i < clients; ++i)
    {
        results[i].clear();
        inputs[i].clear();
    }
}

void organisation::sequential::programs::run(organisation::data &mappings)
{
    for(int i = 0; i < clients; ++i)
    {
        for(int j = 0; j < params.epochs; ++j)
        {
            std::string input = inputs[i].values[j].input;
            #warning pass down expected from ppopulation::parameters::expected
            std::string expected;
            //std::string expected = inputs[i].values[j].expected;            
            std::string output = schemas[i]->run(j, input, expected, mappings);

            results[i].values.push_back(output);
        }
    }
}

void organisation::sequential::programs::set(std::vector<inputs::input> &source)
{
    int temp = source.size();
    if(source.size() > clients) temp = clients;

    for(int i = 0; i < temp; ++i)
    {
        inputs[i] = source[i];
    }
}

std::vector<organisation::output> organisation::sequential::programs::get(organisation::data &mappings)
{
    std::vector<organisation::output> temp;

    for(int i = 0; i < clients; ++i)
    {
        temp.push_back(results[i]);
    }

    return temp;
}

void organisation::sequential::programs::copy(::organisation::schema **source, int source_size)
{
    int temp = source_size;
    if(source_size > clients) temp = clients;

    for(int i = 0; i < temp; ++i)
    {
        schemas[i] = source[i];
    }
}

void organisation::sequential::programs::makeNull()
{
    schemas = NULL;
    results = NULL;
    inputs = NULL;
}

void organisation::sequential::programs::cleanup()
{
    if(inputs != NULL) delete inputs;
    if(results != NULL) delete results;
    if(schemas != NULL) delete schemas;
}