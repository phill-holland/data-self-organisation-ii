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

    results = new ::organisation::outputs::output[clients];
    if(results == NULL) return;

    init = true;
}

void organisation::sequential::programs::clear()
{
    for(int i = 0; i < clients; ++i)
    {
        results[i].clear();     
    }
}

void organisation::sequential::programs::run(organisation::data &mappings)
{
    for(int i = 0; i < clients; ++i)
    {
        for(int j = 0; j <params.epochs(); ++j)
        {
            inputs::epoch epoch;
            if(input.get(epoch, j))
            {
                std::string output = schemas[i]->run(j, epoch.input, epoch.expected, mappings);
                results[i].values.push_back(outputs::data(output));
            }
        }
    }
}

void organisation::sequential::programs::set(organisation::data &mappings, inputs::input &source)
{
    input = source;
}

std::vector<organisation::outputs::output> organisation::sequential::programs::get(organisation::data &mappings)
{
    std::vector<organisation::outputs::output> temp;

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
}

void organisation::sequential::programs::cleanup()
{
    if(results != NULL) delete results;
    if(schemas != NULL) delete schemas;
}