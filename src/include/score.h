#include "compute.h"
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
            static const int MAX_WORDS = 5;
            static const int MAX_COLLISIONS = 10;

        public:
            int max_words;
            int max_collisions;

        public:
            settings(int _max_words = MAX_WORDS, int _max_collisions = MAX_COLLISIONS)
            {
                max_words = _max_words;
                max_collisions = _max_collisions;
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
            
            bool compute(organisation::compute value, settings params = settings());

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
};

#endif