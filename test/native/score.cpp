#include <gtest/gtest.h>
#include <vector.h>
#include "score.h"
#include "general.h"

TEST(BasicScoreEqualsOneParallel, BasicAssertions)
{    
    GTEST_SKIP();

    std::string value("daisy daisy give me your answer do .");
    std::string expected("daisy daisy give me your answer do .");

    std::vector<std::string> strings = organisation::split(expected);

    organisation::score score;

    score.compute(expected, value);

    std::vector<float> values;
    int total = score.size();

    EXPECT_EQ(total, (strings.size() * 2) + 1);

    for(int i = 0; i < total; ++i)
    {
        values.push_back(score.get(i));
    }

    std::vector<float> data = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                1.0f, 1.0f, 1.0f, 1.0f, 1.0f ,
                                1.0f, 1.0f  };

    EXPECT_EQ(values, data);
    EXPECT_FLOAT_EQ(score.sum(), 1.0f);
}

TEST(BasicScoreOneOffParallel, BasicAssertions)
{    
    GTEST_SKIP();
    
    std::string value("daisy daisy monkey me your answer do .");
    std::string expected("daisy daisy give me your answer do .");

    std::vector<std::string> strings = organisation::split(expected);

    organisation::score score;

    score.compute(expected, value);

    std::vector<float> values;
    int total = score.size();

    EXPECT_EQ(total, (strings.size() * 2) + 1);

    for(int i = 0; i < total; ++i)
    {
        values.push_back(score.get(i));
    }

    std::vector<float> data = { 1.0f, 1.0f, 0.1f, 1.0f, 1.0f,
                                1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
                                0.0f, 1.0f, 1.0f, 1.0f, 1.0f ,
                                1.0f, 0.33333334f  };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.79019606f);
}

TEST(BasicScoreOneOffErrorParallel, BasicAssertions)
{    
    GTEST_SKIP();

    std::string value("daisy daisy me give your answer do .");
    std::string expected("daisy daisy give me your answer do .");
    
    std::vector<std::string> strings = organisation::split(expected);

    organisation::score score;

    score.compute(expected, value);

    std::vector<float> values;
    int total = score.size();

    EXPECT_EQ(total, (strings.size() * 2) + 1);

    for(int i = 0; i < total; ++i)
    {
        values.push_back(score.get(i));
    }

    std::vector<float> data = { 1.0f, 1.0f, 0.857142866f, 0.100000001f, 1.0f,
                                1.0f, 1.0f, 1.0f, 1.0f, 0.857142866f,
                                0.0f, 0.857142866f, 1.0f, 1.0f, 1.0f ,
                                1.0f, 1.0f  };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.86302519f);
}

TEST(BasicScoreTwoOffErrorParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    std::string value("daisy daisy me your give answer do .");
    std::string expected("daisy daisy give me your answer do .");
    
    std::vector<std::string> strings = organisation::split(expected);

    organisation::score score;

    score.compute(expected, value);

    std::vector<float> values;
    int total = score.size();

    EXPECT_EQ(total, (strings.size() * 2) + 1);

    for(int i = 0; i < total; ++i)
    {
        values.push_back(score.get(i));
    }

    std::vector<float> data = { 1.0f, 1.0f, 0.857142866f, 0.100000001f, 1.0f,
                                1.0f, 1.0f, 1.0f, 1.0f, 0.857142866f,
                                0.0f, 0.857142866f, 1.0f, 1.0f, 1.0f ,
                                1.0f, 1.0f  };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.86302519f);
}

// test scores with similar but shorter sentence
// test scores with similar, but longer sentence
// test score distance, word increasing away from ideal
// std::string expected("daisy daisy me give your answer do .");
//
// std::string expected("daisy daisy me your give answer do .");
// std::string expected("daisy daisy me your answer give do .");
// std::string expected("daisy daisy me your answer do give .");

// std::string expected("daisy daisy daisy");
// std::string expected("daisy daisy daisy daisy daisy");
// std::string expected("daisy daisy daisy daisy daisy give me your answer do .");

// test scores in schema, with many epochs