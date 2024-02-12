#include "schema.h"
#include "data.h"
#include <random>
#include <atomic>
#include <vector>

#ifndef _ORGANISATION_SCHEMAS
#define _ORGANISATION_SCHEMAS

namespace organisation
{
    class schemas
    {
        std::vector<schema*> data;
    
    private:
        int length;
        bool init;
        
    public:
        schemas(int width, int height, int depth, int size) { makeNull(); reset(width, height, depth, size); }
        ~schemas() { cleanup(); }

        bool initalised() { return init; }
        void reset(int width, int height, int depth, int size);

        bool clear();

        schema *get(int index)
        {
            if((index < 0)||(index >= length)) return NULL;
            return data[index];
        }

        bool set(schema &source, int index)
        {
            if((index < 0)||(index >= length)) return false;
            data[index]->copy(source);

            return true;
        }

        bool generate(organisation::data &source);

    protected:
        void makeNull();
        void cleanup();
    };
};

#endif