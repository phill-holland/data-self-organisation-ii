#include "compute.h"
//#include "genetic/cache.h"
#include <string>
#include <vector>
#include <unordered_map>

#ifndef _ORGANISATION_SCORE
#define _ORGANISATION_SCORE

namespace organisation
{
    namespace scores
    {
        class settings
        {
            static const int MAX_COLLISIONS = 10;
            static const int MAX_DISTANCE = 10;

        public:         
            int max_collisions;
            int max_distance;

        public:
            settings(int _max_collisions = MAX_COLLISIONS, int _max_distance = MAX_DISTANCE)
            {                
                max_collisions = _max_collisions;
                max_distance = _max_distance;
            }
        };

        class score
        {
            std::unordered_map<int,float> scores;

        public:
            score() { }
            score(const score &source) { copy(source); }
            ~score() { }

            void clear();
            
            //bool compute(organisation::compute value, settings params = settings());

            bool compute(organisation::compute value, std::unordered_map<std::string, std::vector<point>> &positions, settings params = settings());
        
            float sum();

            bool set(float value, int index);
            float get(int index);

            size_t size() { return scores.size(); }

        public:
            void copy(const score &source);
            
        //protected:
            //float compute_comparative_length_score(std::vector<std::string> &expected, std::vector<std::string> &value);
            
        public:
            bool operator==(const score &src) const
            {
                return (scores == src.scores);
            }
        };
    };
};

#endif