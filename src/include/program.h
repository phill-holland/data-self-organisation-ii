#include "data.h"
#include "point.h"
#include "genetic/movement.h"
#include "genetic/cache.h"
#include "genetic/collisions.h"
#include "genetic/insert.h"
#include "vector.h"
#include "parameters.h"
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
        class inserts;
        class collisions;
    };
        
    class program
    {      
        friend class parallel::program;
        friend class parallel::inserts;
        friend class parallel::collisions;
          
        static std::mt19937_64 generator;

        static const int MAX = 30;

        int _width, _height, _depth;

        genetic::cache caches;
        genetic::movement movement;
        genetic::collisions collisions;
        genetic::insert insert;

        int length;

        bool init;
        
    public:
        program(parameters &settings) : caches(settings), collisions(settings)
        { 
            makeNull(); 
            reset(settings); 
        }
        ~program() { cleanup(); }

        bool initalised() { return init; }
        void reset(parameters &settings);

    public:
        void set(genetic::cache &source) { caches = source; }
        void set(genetic::movement &source) { movement = source; }
        void set(genetic::collisions &source) { collisions = source; }
        void set(genetic::insert &source) { insert = source; }
        
    public:
        void clear();

        bool empty();

        void generate(data &source);  
        bool mutate(data &source);

        std::string run(std::string input, data &source, int max = MAX);

        bool validate(data &source);

        int width() { return _width; }
        int height() { return _height; }
        int depth() { return _depth; }
        
        std::unordered_map<std::string, std::vector<point>> get(data &source)
        {
            return caches.get(source);
        }

    public:
        void copy(const program &source);
        bool equals(const program &source);

    public:
        void cross(program &a, program &b);

    public:
        std::string serialise();
        void deserialise(std::string source);

    public:
        void save(std::string filename);
        void load(std::string filename);
        
    protected:
        void makeNull();
        void cleanup();
    };
};

#endif