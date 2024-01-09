#include "score.h"
#include <iostream>
#include <tuple>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>

namespace organisation {

class score {
private:
    std::unordered_map<int, float> scores;
    bool init = false;

    int countWords(const std::string& source);
    std::vector<std::tuple<std::string, int>> splitIntoWords(const std::string& source);
    int findClosestOccurrence(const std::tuple<std::string, int>& word, const std::string& value, int alphabet_length);
    float calculateDistance(float f1, float f2, float offset, float max);

public:
    void reset();
    void clear();
    bool compute(const std::string& expected, const std::string& value);
    float sum();
    bool set(float value, int index);
    float get(int index);
    void copy(const score& source);
};

void score::reset() {
    init = false;
    clear();
    init = true;
}

void score::clear() {
    scores.clear();
}

int score::countWords(const std::string& source) {
    std::istringstream stream(source);
    return std::distance(std::istream_iterator<std::string>(stream), std::istream_iterator<std::string>());
}

std::vector<std::tuple<std::string, int>> score::splitIntoWords(const std::string& source) {
    std::vector<std::tuple<std::string, int>> result;
    std::istringstream stream(source);
    std::string word;
    int index = 0;
    while (stream >> word) {
        result.emplace_back(word, index++);
    }
    return result;
}

int score::findClosestOccurrence(const std::tuple<std::string, int>& word, const std::string& value, int alphabet_length) {
    std::istringstream stream(value);
    std::string currentWord;
    int position = std::get<1>(word);
    int closestPosition = -1;
    int distance = alphabet_length + 1;
    int index = 0;
    while (stream >> currentWord) {
        if (std::get<0>(word) == currentWord) {
            int currentDistance = std::abs(index - position);
            if (currentDistance < distance) {
                distance = currentDistance;
                closestPosition = index;
            }
        }
        ++index;
    }
    return closestPosition;
}

float score::calculateDistance(float f1, float f2, float offset, float max) {
    float distance = std::min(std::abs(f1 - f2) + offset, max);
    if (f1 == -1 || f2 == -1) {
        distance = max;
    }
    return std::clamp((1.0f - (distance / max)), 0.0f, 1.0f);
}

}
