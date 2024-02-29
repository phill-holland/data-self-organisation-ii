#include <gtest/gtest.h>
#include <vector.h>
#include <unordered_map>
#include <string>
#include "score.h"
#include "general.h"
#include "point.h"
#include "compute.h"

TEST(BasicScoreEqualsOneParallel, BasicAssertions)
{    
    GTEST_SKIP();

    std::string value("daisy daisy give me your answer do .");
    std::string expected("daisy daisy give me your answer do .");

    std::vector<std::string> value_strings = organisation::split(value);
    std::vector<std::string> expected_strings = organisation::split(expected);
        
    std::unordered_map<std::string, std::vector<organisation::point>> positions;
    organisation::point expected_position(0,0,0);
    for(auto &it: expected_strings)    
    {
        if(positions.find(it) == positions.end()) positions[it] = { };
        positions[it].push_back(expected_position);
        expected_position.x += 1;
    }

    organisation::scores::score score;
    organisation::scores::settings settings;

    organisation::compute compute;
    
    organisation::point value_position(0,0,0);
    for(auto &it: value_strings)
    {
        compute.value.push_back(std::tuple<std::string, organisation::point>(it, value_position));
        value_position.x += 1;
    }

    organisation::statistics::data statistics(settings.max_collisions);

    compute.expected = expected_strings;
    compute.stats = statistics;

    score.compute(compute, positions, settings);
    
    std::vector<float> values;
    int total = score.size();

    EXPECT_EQ(total, expected_strings.size() + 2);

    for(int i = 0; i < total; ++i)
    {
        values.push_back(score.get(i));
    }

    std::vector<float> data = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
                                
    EXPECT_EQ(values, data);
    EXPECT_FLOAT_EQ(score.sum(), 1.0f);
}

TEST(BasicScoreOneOffParallel, BasicAssertions)
{    
    GTEST_SKIP();

    std::string value("daisy daisy monkey me your answer do .");
    std::string expected("daisy daisy give me your answer do .");

    std::vector<std::string> value_strings = organisation::split(value);
    std::vector<std::string> expected_strings = organisation::split(expected);
        
    std::unordered_map<std::string, std::vector<organisation::point>> positions;
    organisation::point expected_position(0,0,0);
    for(auto &it: expected_strings)    
    {
        if(positions.find(it) == positions.end()) positions[it] = { };
        positions[it].push_back(expected_position);
        expected_position.x += 1;
    }

    organisation::scores::score score;
    organisation::scores::settings settings;

    organisation::compute compute;
    
    organisation::point value_position(0,0,0);
    for(auto &it: value_strings)
    {
        compute.value.push_back(std::tuple<std::string, organisation::point>(it, value_position));
        value_position.x += 1;
    }

    organisation::statistics::data statistics(settings.max_collisions);

    compute.expected = expected_strings;
    compute.stats = statistics;

    score.compute(compute, positions, settings);
    
    std::vector<float> values;
    int total = score.size();

    EXPECT_EQ(total, expected_strings.size() + 2);

    for(int i = 0; i < total; ++i)
    {
        values.push_back(score.get(i));
    }

    std::vector<float> data = { 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                                1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
                                
    EXPECT_EQ(values, data);
    EXPECT_FLOAT_EQ(score.sum(), 0.89999998f);
}

TEST(BasicWordsCorrectButMixedParallel, BasicAssertions)
{    
    GTEST_SKIP();

    std::string value(". do answer your me give daisy daisy");
    std::string expected("daisy daisy give me your answer do .");

    std::vector<std::string> value_strings = organisation::split(value);
    std::vector<std::string> expected_strings = organisation::split(expected);
        
    std::unordered_map<std::string, std::vector<organisation::point>> positions;
    organisation::point expected_position(0,0,0);
    for(auto &it: expected_strings)    
    {
        if(positions.find(it) == positions.end()) positions[it] = { };
        positions[it].push_back(expected_position);
        expected_position.x += 1;
    }

    organisation::scores::score score;
    organisation::scores::settings settings;

    organisation::compute compute;
    
    organisation::point value_position(0,1,0);
    for(auto &it: value_strings)
    {
        compute.value.push_back(std::tuple<std::string, organisation::point>(it, value_position));
        value_position.y += 1;
    }

    organisation::statistics::data statistics(settings.max_collisions);

    compute.expected = expected_strings;
    compute.stats = statistics;

    score.compute(compute, positions, settings);
    
    std::vector<float> values;
    int total = score.size();

    EXPECT_EQ(total, expected_strings.size() + 2);

    for(int i = 0; i < total; ++i)
    {
        values.push_back(score.get(i));
    }

    std::vector<float> data = { 0.9f, 0.8f, 0.639445f, 0.5f, 0.35968757f,
                                0.218975f, 0.078045547f, 0.0f, 1.0f, 1.0f };
                                
    EXPECT_EQ(values.size(), data.size());
    for(int i = 0; i < values.size(); ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }
    
    EXPECT_FLOAT_EQ(score.sum(), 0.54961532f);
}

TEST(BasicScoreCorrectButMixedWithGreaterLengthParallel, BasicAssertions)
{    
    GTEST_SKIP();

    std::string value(". do answer your me give daisy daisy monkey monkey chestnut");
    std::string expected("daisy daisy give me your answer do .");

    std::vector<std::string> value_strings = organisation::split(value);
    std::vector<std::string> expected_strings = organisation::split(expected);
        
    std::unordered_map<std::string, std::vector<organisation::point>> positions;
    organisation::point expected_position(0,0,0);
    for(auto &it: expected_strings)    
    {
        if(positions.find(it) == positions.end()) positions[it] = { };
        positions[it].push_back(expected_position);
        expected_position.x += 1;
    }

    organisation::scores::score score;
    organisation::scores::settings settings;

    organisation::compute compute;
    
    organisation::point value_position(0,1,0);
    for(auto &it: value_strings)
    {
        compute.value.push_back(std::tuple<std::string, organisation::point>(it, value_position));
        value_position.y += 1;
    }

    organisation::statistics::data statistics(settings.max_collisions);

    compute.expected = expected_strings;
    compute.stats = statistics;

    score.compute(compute, positions, settings);
    
    std::vector<float> values;
    int total = score.size();

    EXPECT_EQ(total, expected_strings.size() + 2);

    for(int i = 0; i < total; ++i)
    {
        values.push_back(score.get(i));
    }

    std::vector<float> data = { 0.9f, 0.8f, 0.639445f, 0.5f, 0.35968757f,
                                0.218975f, 0.078045547f, 0.0f, 1.0f, 0.625f };
                                
    EXPECT_EQ(values.size(), data.size());
    for(int i = 0; i < values.size(); ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.51211536f);
}

TEST(BasicScoreCorrectButMixedWithLessLengthParallel, BasicAssertions)
{    
    GTEST_SKIP();

    std::string value(". do answer your me give");
    std::string expected("daisy daisy give me your answer do .");

    std::vector<std::string> value_strings = organisation::split(value);
    std::vector<std::string> expected_strings = organisation::split(expected);
        
    std::unordered_map<std::string, std::vector<organisation::point>> positions;
    organisation::point expected_position(0,0,0);
    for(auto &it: expected_strings)    
    {
        if(positions.find(it) == positions.end()) positions[it] = { };
        positions[it].push_back(expected_position);
        expected_position.x += 1;
    }

    organisation::scores::score score;
    organisation::scores::settings settings;

    organisation::compute compute;
    
    organisation::point value_position(0,1,0);
    for(auto &it: value_strings)
    {
        compute.value.push_back(std::tuple<std::string, organisation::point>(it, value_position));
        value_position.y += 1;
    }

    organisation::statistics::data statistics(settings.max_collisions);

    compute.expected = expected_strings;
    compute.stats = statistics;

    score.compute(compute, positions, settings);
    
    std::vector<float> values;
    int total = score.size();

    EXPECT_EQ(total, expected_strings.size() + 2);

    for(int i = 0; i < total; ++i)
    {
        values.push_back(score.get(i));
    }

    std::vector<float> data = { 0.9f, 0.8f, 0.639445f, 0.5f, 0.35968757f,
                                0.218975f, 0.0f, 0.0f, 1.0f, 0.75f };
                                
    EXPECT_EQ(values.size(), data.size());
    for(int i = 0; i < values.size(); ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }
    EXPECT_FLOAT_EQ(score.sum(), 0.51681077f);
}

TEST(BasicScoreAllInCorrectButWithSameLengthParallel, BasicAssertions)
{    
    GTEST_SKIP();

    std::string value("monkey banana apple ape susan landmine tea hedgehog");
    std::string expected("daisy daisy give me your answer do .");

    std::vector<std::string> value_strings = organisation::split(value);
    std::vector<std::string> expected_strings = organisation::split(expected);
        
    std::unordered_map<std::string, std::vector<organisation::point>> positions;
    organisation::point expected_position(0,0,0);
    for(auto &it: expected_strings)    
    {
        if(positions.find(it) == positions.end()) positions[it] = { };
        positions[it].push_back(expected_position);
        expected_position.x += 1;
    }

    organisation::scores::score score;
    organisation::scores::settings settings;

    organisation::compute compute;
    
    organisation::point value_position(0,1,0);
    for(auto &it: value_strings)
    {
        compute.value.push_back(std::tuple<std::string, organisation::point>(it, value_position));
        value_position.y += 1;
    }

    organisation::statistics::data statistics(settings.max_collisions);

    compute.expected = expected_strings;
    compute.stats = statistics;

    score.compute(compute, positions, settings);
    
    std::vector<float> values;
    int total = score.size();

    EXPECT_EQ(total, expected_strings.size() + 2);

    for(int i = 0; i < total; ++i)
    {
        values.push_back(score.get(i));
    }

    std::vector<float> data = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f, 1.0f };
                                
    EXPECT_EQ(values, data);
    EXPECT_FLOAT_EQ(score.sum(), 0.2f);
}

