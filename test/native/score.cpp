#include <gtest/gtest.h>
#include <vector.h>
#include "score.h"
#include "general.h"

TEST(BasicScoreEqualsOneParallel, BasicAssertions)
{    
    //GTEST_SKIP();

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
    //GTEST_SKIP();
    
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

    std::vector<float> data = { 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                                1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
                                0.0f, 1.0f, 1.0f, 1.0f, 1.0f ,
                                1.0f, 1.0f  };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.82352942f);
}

TEST(BasicScoreOneOffErrorParallel, BasicAssertions)
{    
    //GTEST_SKIP();

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

    std::vector<float> data = { 1.0f, 1.0f, 0.714285731f, 0.100000001f, 0.100000001f, 1.0f, 1.0f, 1.0f, 
                                1.0f, 0.714285731f, 0.0f, 1.0, 0.857142866f, 1.0f, 1.0f , 1.0f, 
                                1.0f  };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.79327726f);
}

TEST(BasicScoreThreeOffErrorParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    std::string value("daisy daisy me your answer give do .");
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

    std::vector<float> data = { 1.0f, 1.0f, 0.571428537f, 0.100000001f, 0.100000001f, 0.100000001f, 1.0f, 1.0f, 
                                1.0f, 0.571428537f, 0.0f, 1.0, 1.0f, 0.857142866f, 1.0f, 1.0f, 
                                1.0f  };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.72352934f);
}

TEST(BasicScoreRepeatOneParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    std::string value("daisy daisy daisy give me your answer do .");
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

    std::vector<float> data = { 1.0f, 1.0f, 0.857142866f, 0.857142866f, 0.857142866f, 0.857142866f, 0.857142866f, 0.857142866f, 
                                1.0f, 0.857142866f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 
                                0.5f  };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.91176462f);
}

TEST(BasicScoreRepeatTwoParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    std::string value("daisy daisy daisy daisy give me your answer do .");
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

    std::vector<float> data = { 1.0f, 1.0f, 0.714285731f, 0.714285731f, 0.714285731f, 0.714285731f, 0.714285731f, 0.714285731f, 
                                1.0f, 0.714285731f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 
                                0.33333334f  };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.84313726f);
}

TEST(BasicScoreRepeatThreeParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    std::string value("daisy daisy daisy daisy daisy give me your answer do .");
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

    std::vector<float> data = { 1.0f, 1.0f, 0.571428537f, 0.571428537f, 0.571428537f, 0.571428537f, 0.571428537f, 0.571428537f, 
                                1.0f, 0.571428537f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 
                                0.200000003f };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.77647048f);
}

TEST(BasicScoreShortSentenceParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    std::string value("daisy");
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

    std::vector<float> data = { 1.0f, 0.100000001f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
                                0.0294117648f };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.066435993f);
}

TEST(BasicScoreBadOrderOneParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    std::string value("me your answer do . daisy daisy give");
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

    std::vector<float> data = { 0.285714269f, 0.428571403f, 0.285714269f, 0.100000001f, 0.100000001f, 0.100000001f, 0.100000001f, 0.100000001f,
                                0.0f, 0.857142866f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.571428537f, 
                                1.0f };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.46638653f);
}

TEST(BasicScoreIncorrectParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    std::string value("incorrect");
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

    std::vector<float> data = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                0.0294117648f };

    for(int i = 0; i < total; ++i)
    {
        EXPECT_FLOAT_EQ(values[i], data[i]);
    }

    EXPECT_FLOAT_EQ(score.sum(), 0.0017301039f);
}