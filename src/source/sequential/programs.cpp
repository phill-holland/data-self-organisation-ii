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

    init = true;
}

void organisation::sequential::programs::clear()
{
}

void organisation::sequential::programs::run()
{
    for(int i = 0; i < clients; ++i)
    {
        for(int j = 0; j < params.epochs; ++j)
        {
            std::string input, expected;
            //schemas[i]->run(j, input, expected, data);
        }
    }
}

std::vector<organisation::output> organisation::sequential::programs::get(organisation::data &mappings)
{
}

void organisation::sequential::programs::copy(::organisation::schema **source, int source_size)
{
}

void organisation::sequential::programs::makeNull()
{
    schemas = NULL;
}

void organisation::sequential::programs::cleanup()
{
    if(schemas != NULL) delete schemas;
}