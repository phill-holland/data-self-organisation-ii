#include "score.h"
#include <iostream>
#include <tuple>
#include <sstream>

void organisation::score::reset()
{
	init = false; cleanup();
	
	clear();

	init = true;
}

void organisation::score::clear()
{
	scores.clear();
}

// first half relative distance from word in front
// second half, relative distance away from words "primary place"
// last value, scoare rating on sentence length

bool organisation::score::compute(std::string expected, std::string value)
{
	auto _words = [](std::string source)
    {
        std::stringstream stream(source);  
        std::string word;

        int index = 0;
        while (stream >> word) 
        {
			++index;
        };

        return index;
    };

    auto _split = [](std::string source)
    {
        std::vector<std::tuple<std::string,int>> result;

        std::stringstream stream(source);  
        std::string word;

        int index = 0;
        while (stream >> word) 
        {
            result.push_back(std::tuple<std::string,int>(word,index));
			++index;
        };

        return result;
    };

	auto _occurances = [](std::string to_find, std::string value) 
	{ 
		std::stringstream stream(value);
		std::string word;

		int result = 0;
	
		while(stream >> word)
		{
			if(to_find.compare(word) == 0) ++result;
		}

		return result;
	};

	auto _closest = [](std::tuple<std::string,int> to_find, std::string value, int alphabet_length)
	{		
		std::stringstream stream(value);
		std::string word;

		std::string find = std::get<0>(to_find);
		int position = std::get<1>(to_find);

		int distance = alphabet_length + 1;
		int result = -1;

		int count = 0;

		while(stream >> word)
		{
			if(find.compare(word) == 0)
			{
				int temp = abs(count - position);
				if(temp < distance) 
				{
					distance = temp;
					result = count;
				}
			}
			++count;
		}

		return result;
	};

	auto _distance = [](float f1, float f2, float offset, float max)
	{
		float distance = (float)std::abs(f1 - f2) + offset;

		if(distance > max) distance = max;
		if((f1 == -1)||(f2 == -1)) distance = max;
		float result = ((distance / max) * -1.0f) + 1.0f;
		
		if(result < 0.0f) result = 0.0f;
		if(result > 1.0f) result = 1.0f;

		return result;
	};

	std::vector<std::tuple<std::string,int>> alphabet = _split(expected);
	
	//int score_len = (_words(expected) * 2) + 1;
	int score_len = (alphabet.size() * 2) + 1;
	for(int i = 0; i < score_len; ++i) set(0.0f, i);

    if(value.size() == 0) return true;

	bool valid = true;
	const int MAX_WORDS = 5;
 	
	const int alphabet_len = alphabet.size();		
	float max = (float)MAX_WORDS;
	if(((float)(alphabet_len - 1))>max) max = (float)(alphabet_len - 1);

	int l1 = expected.size();
	int l2 = value.size();

	const float fib[] = { 1.0f, 2.0f, 3.0f, 5.0f, 8.0f, 13.0f, 21.0f, 34.0f, 55.0f, 89.0f, 144.0f };
	const int max_str_len = 10;

	int d = abs(l2 - l1);
	if(d > max_str_len) d = max_str_len;

	float len_score = 1.0f / fib[d];
	if(!set(len_score, score_len - 1)) valid = false;
	
	int index = 0;
	for(auto &it : alphabet)
	{            
		int f1 = _closest(it, value, alphabet_len);
		int f3 = std::get<1>(it);
					
		if(f1 >= f3) 
		{
			float distance = _distance(f1, f3, 0.0f, max); 
			if(!set(distance, index)) valid = false;
		}
		else if(!set(0.1f, index)) valid = false;

		++index;
	}

	index = 0;
	for(std::vector<std::tuple<std::string,int>>::iterator it = alphabet.begin(); it != (alphabet.end() - 1); ++it)
	{			
		std::tuple<std::string,int> a1 = *it;
		std::tuple<std::string,int> a2 = *(it + 1);

		int f1 = _closest(a1, value, alphabet_len);
		int f2 = _closest(a2, value, alphabet_len);			
		
		if(f1 < f2) 
		{
			if(!set(_distance(f1, f2, -1.0f, max),index + alphabet_len)) valid = false;		
		}
		else set(0.0f, index+alphabet_len);

		++index;
	}

	std::tuple<std::string,int> last = *(alphabet.end() - 1);
	int f1 = _closest(last, value, alphabet_len);
	int f2 = _words(value);

	if(f1 < f2) 
	{
		if(!set(_distance(f1, f2, -1.0f, max),alphabet_len + alphabet_len - 1)) valid = false;
	}
	else set(0.0f,alphabet_len + alphabet_len - 1);
	
	
	return valid;
}

float organisation::score::sum()
{    
	if(scores.size() <= 0) return 0.0f;
	float result = 0.0f;

	for(auto &b :scores)
    {
		result += b.second;
    }
    
    return result / ((float)scores.size());
}

bool organisation::score::set(float value, int index)
{
	if(index < 0) return false;

	scores[index] = value;

	return true;
}

float organisation::score::get(int index)
{
	if((index < 0)||(index >= scores.size())) return 0.0f;

	return scores[index];
}

void organisation::score::copy(const score &source)
{
	clear();

	for(auto &a: source.scores)
    {
        scores[a.first] = a.second;
    }
}

void organisation::score::makeNull() 
{ 

}

void organisation::score::cleanup()
{

}
