#include "cell.h"
#include "data.h"
#include "point.h"
#include "history.h"
#include "movement.h"
#include "vector.h"
#include <string>
#include <random>
#include <vector>
#include <unordered_map>
#include <tuple>

#ifndef _ORGANISATION_PROGRAM
#define _ORGANISATION_PROGRAM

namespace organisation
{    
    namespace parallel
    {
        class program;
    };
        
    class program
    {      
        friend class parallel::program;
          
        static std::mt19937_64 generator;

        int _width, _height, _depth;

        std::vector<std::tuple<int,point>> cache;
        std::vector<movement> movements;
        std::vector<int> collisions; // 27

        int length;

        bool init;
        
    public:
        program(int w, int h, int d) { makeNull(); reset(w,h,d); }
        program(const program &source) { copy(source); }
        ~program() { cleanup(); }

        bool initalised() { return init; }
        void reset(int w, int h, int d);

    public:
        void clear();
        void generate(data &source);  
        void mutate(data &source);
        
        std::string run(int start, data &source, history *destination = NULL);

        int count();

        //void set(int value, int x, int y, int z);
        //void set(vector input, vector output, int magnitude, int x, int y, int z);

        bool validate(data &source);

        int width() { return _width; }
        int height() { return _height; }
        int depth() { return _depth; }
        
    public:
        void copy(const program &source);
        bool equals(const program &source);

    public:
        void cross(program &a, program &b, int middle = -1);

    public:
        void save(std::string filename);
        void load(std::string filename);

    protected:
        void makeNull();
        void cleanup();
    };
};

#endif