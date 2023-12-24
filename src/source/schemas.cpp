#include "schemas.h"
#include <iostream>
#include <functional>
#include <limits>
#include <iostream>

void organisation::schemas::reset(int width, int height, int depth, int size)
{
    init = false; cleanup();
    this->length = size;
    
    data.resize(size);

    for(int i = 0; i < size; ++i) { data[i] = NULL; }

    for(int i = 0; i < size; ++i)   
    { 
        data[i] = new organisation::schema(width, height, depth);
        if(data[i] == NULL) return;
    }

    init = true;
}

bool organisation::schemas::clear()
{
    for(int i = 0; i < length; ++i)
    {
        data[i]->clear();
    }

    return true;
}

bool organisation::schemas::generate(organisation::data &source)
{
    for(int i = 0; i < length; ++i)
    {
        data[i]->generate(source);
    }

    return true;
}

void organisation::schemas::makeNull() 
{ 
}

void organisation::schemas::cleanup() 
{ 
    if(data.size() > 0)
    {
        for(int i = length - 1; i >= 0; i--)
        { 
            if(data[i] != NULL) delete data[i];
        }
    }
}