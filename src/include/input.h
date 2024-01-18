#include <vector>
#include <tuple>
#include <string>

#ifndef _ORGANISATION_INPUT
#define _ORGANISATION_INPUT

namespace organisation
{
    namespace inputs
    {
        class epoch
        {
        public:
            std::string input;
            std::string expected;
        };

        class input
        {
            std::vector<epoch> values;

        public:
            void clear()
            {
                values.clear();
            }

            void push_back(epoch &source)
            {
                values.push_back(source);
            }
            
            void set(epoch &source, int index)
            {
                if(index < 0) return;
                if(index > values.size()) values.resize(index + 1);

                values[index] = source;
            }

            bool get(epoch &destination, int index)
            {
                if((index < 0)&&(index > values.size())) return false;

                destination = values[0];

                return true;
            }

            std::vector<std::tuple<std::string,std::string>> combine(std::vector<std::string> output);

            int dimensions();

            size_t size() { return values.size(); }
        };
    };
};


#endif