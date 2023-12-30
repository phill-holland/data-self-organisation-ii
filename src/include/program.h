#include "data.h"
#include "point.h"
#include "history.h"
#include "genetic/movement.h"
#include "genetic/cache.h"
#include "genetic/collisions.h"
#include "genetic/insert.h"
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

        genetic::cache caches;
        genetic::movement movement;
        genetic::collisions collisions;
        genetic::insert insert;

        int length;

        bool init;
        
    public:
        program(int w, int h, int d) : caches(w,h,d)
        { 
            makeNull(); 
            reset(w,h,d); 
        }
        /*program(const program &source)
        : caches(w,h,d), movement(w,h,d) 
        { 
            copy(source); 
        }*/
        ~program() { cleanup(); }

        bool initalised() { return init; }
        void reset(int w, int h, int d);

    public:
        void clear();
        void generate(data &source);  
        void mutate(data &source);
        
        std::string run2(std::string input, data &source, history *destination = NULL);
        std::string run(std::string input, data &source, history *destination = NULL);

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