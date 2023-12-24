#include "program.h"
#include "data.h"
#include "score.h"
#include "kdpoint.h"
#include <string>
#include <random>
#include <vector>
#include <tuple>
#include <unordered_map>

#ifndef _ORGANISATION_SCHEMA
#define _ORGANISATION_SCHEMA

namespace organisation
{
    class schema
    {
        static std::mt19937_64 generator;
        
        bool init;

    public:
        program prog;
        std::unordered_map<int, score> scores;

    public:
        schema(int width, int height, int depth) : prog(width, height, depth) { makeNull(); reset(); }        
        schema(const schema &source) : prog(1,1,1) { copy(source); }
        ~schema() { cleanup(); }

    public:
        void clear();
        
        bool initalised() { return init; }
        void reset();
        
        void generate(data &source);
    
        int count() { return prog.count(); }

        bool get(kdpoint &destination, int minimum, int maximum);

        std::vector<float> get();
        float get(int dimension);

        float sum();
        
        void compute(std::vector<std::tuple<std::string,std::string>> values);

    public:        
        void cross(schema *destination, schema *value);
        void mutate(data &source);

    public:
        std::string run(int epoch, std::string expected, data &source, history *destination = NULL);           

    public:
        void copy(const schema &source);

    protected:
        void makeNull();
        void cleanup();
    };
};

#endif