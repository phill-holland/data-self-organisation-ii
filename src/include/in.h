#include "gate.h"
#include "vector.h"
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

#ifndef _ORGANISATION_IN
#define _ORGANISATION_IN

namespace organisation
{        
    class in
    {
        static std::mt19937_64 generator;
        
        organisation::gates gates[organisation::gates::IN];

        int write;

    public:
        in() { write = 0; }

        void clear();
        
        bool is_empty(int index);
        
        void set(int in);
        void set(int in, int out, gate value);     
        gate get(int in, int out);
           
        std::vector<int> pull();
        std::vector<int> pull(int in);

        std::vector<organisation::vector> outputs(int index);
        
        void generate();
        void mutate();

    public:
        bool validate(int &in);
        
    public:
        bool equals(const in &source);   
        void copy(const in &source);     
    };
};

#endif