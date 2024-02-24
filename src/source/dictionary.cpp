#include "dictionary.h"
#include <algorithm>

std::mt19937_64 organisation::dictionary::generator(std::random_device{}());

/*
std::string source = R"(daisy daisy give me your answer do .
I'm half crazy for the love of you .
it won't be a stylish marriage .
I can't afford a carriage .
but you'll look sweet upon the seat .
of a bicycle built for two .
)";
*/

organisation::dictionary::dictionary()
{
    words = { "daisy", "give", "me", "your", "answer", "do", "I'm", "half", "crazy", "for", "the", "love", "of",
              "you", "it", "won't", "be", "a", "stylish", "marriage", "I", "can't", "afford", "carriage",
              "but", "you'll", "look", "sweet", "upon", "seat", "bicycle", "built", "two", "." };
}

std::string organisation::dictionary::random(int length, std::vector<std::string> excluded) const
{
    int total = length;
    if(total == 0)
        total = (std::uniform_int_distribution<int>{ 2, 5 })(generator);

    std::string result;

    result = words[(std::uniform_int_distribution<int>{ 0, (int)(words.size() - 1) })(generator)];
    for(int i = 1; i < total; ++i)
    {
        result += " ";
        std::string temp;
        do
        {
            temp = words[(std::uniform_int_distribution<int>{ 0, (int)(words.size() - 1) })(generator)];
        }while(std::find(excluded.begin(),excluded.end(),temp)!=excluded.end());

        result += temp;
    }

    return result;
}