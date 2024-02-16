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

    public:
        score() { }
        score(const score &source) { copy(source); }
        ~score() { }

        void clear();
        bool compute(std::string expected, std::string value);

        float sum();

        bool set(float value, int index);
        float get(int index);

        size_t size() { return scores.size(); }

    public:
        void copy(const score &source);
        
    protected:
        float compute_comparative_length_score(std::string expected, std::string value);
        
    public:
        bool operator==(const score &src) const
        {
            return (scores == src.scores);
        }
    };
};

#endif