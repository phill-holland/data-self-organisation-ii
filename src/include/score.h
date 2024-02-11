#include <string>
#include <vector>
#include <unordered_map>

#ifndef _ORGANISATION_SCORE
#define _ORGANISATION_SCORE

namespace organisation
{
    class score
    {
        std::unordered_map<int,float> scores;
        bool init;

    public:
        score() { makeNull(); reset(); }
        score(const score &source) { copy(source); }
        ~score() { cleanup(); }

        bool intialised() { return init; }
        void reset();

        void clear();
        bool compute(std::string expected, std::string value);

        float sum();

        bool set(float value, int index);
        float get(int index);

        size_t size() { return scores.size(); }

    public:
        void copy(const score &source);
        
    protected:
        void makeNull();
        void cleanup();
    };
};

#endif