#include "vector.h"
#include <string>
#include <vector>
#include <random>
#include <array>
#include <unordered_map>

#ifndef _ORGANISATION_GATE
#define _ORGANISATION_GATE

namespace organisation
{        
    class gate
    {
    public:        
        static const int MAGNITUDE = 2;
        
    public:
        int magnitude;
        int encoded;

    public:
        gate(int _magnitude = -1, int _encoded = -1) 
        { 
            magnitude = _magnitude; 
            encoded = _encoded;
        }

    public:
        void clear() { magnitude = -1; }

        bool is_empty() { return magnitude <= 0; }
    };

    class gates
    {
        int write;

    public:
        static const int IN = 15;
        static const int OUT = 10;

    public:
        int encoded;

    private:
        gate data[OUT];

    public:
        gates() { clear(); }
        gates(const gates &source) { copy(source); }

        void clear();

        bool is_empty();

        int size();

        void set(int index, gate g);
        gate get(int index);

        std::vector<organisation::vector> get();
        
        std::vector<int> pull();

    public:
        bool validate(int &count);
        
    public:
        bool equals(const gates &source);  
        void copy(const gates &source);      
    };
};

#endif