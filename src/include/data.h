#include <string>
#include <vector>
#include <unordered_map>

#ifndef _ORGANISATION_DATA
#define _ORGANISATION_DATA

namespace organisation
{    
    class data
    {
        std::unordered_map<std::string, int> forward;
        std::unordered_map<int, std::string> reverse;

        int max;

    public:
        data() { max = 0; } 
        data(std::vector<std::string> &source) { reset(source); }
        data(const data &source) { copy(source); }

    public:
        void reset(std::vector<std::string> &source) { clear(); add(source); }

        void add(std::vector<std::string> &source);

        void clear()
        {
            forward.clear();
            reverse.clear();
            max = 0;
        }

        int maximum() { return max; }
        
        std::string map(int value);
        int map(std::string value);
        
        std::string get(std::vector<int> &source);  
        std::vector<int> get(std::string source);

        std::vector<int> all();
        
    public:
        void copy(const data &source);      
    };
};

#endif