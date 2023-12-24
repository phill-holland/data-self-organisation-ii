#include <string>
#include <vector>

#ifndef _ORGANISATION_GENERAL
#define _ORGANISATION_GENERAL

namespace organisation
{
    inline std::vector<std::string> split(std::string source)
    {
        std::vector<std::string> result;
        std::string temp; 

        for(auto &ch: source)
        {
            if((ch != ' ')&&(ch != 10)&&(ch != 13))
            {
                temp += ch;
            }
            else
            {
                if(temp.size() > 0)
                {
                    result.push_back(temp);
                    temp.clear();
                }
            }
        }

        if(temp.size() > 0) result.push_back(temp);
        
        return result;    
    }
};

#endif