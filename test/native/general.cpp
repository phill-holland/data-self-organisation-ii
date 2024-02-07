#include <gtest/gtest.h>
#include <string>
#include "program.h"
#include "data.h"
#include "vector.h"
#include "schema.h"
#include "general.h"
#include "point.h"
#include "genetic/cache.h"
#include "genetic/movement.h"
#include "genetic/collisions.h"
#include "genetic/insert.h"

TEST(BasicMovementAndCollisionDetection, BasicAssertions)
{
    GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;
    organisation::point starting(width / 2, height / 2, depth / 2);

    std::vector<std::tuple<organisation::point,organisation::vector,organisation::vector>> directions = {
        { 
            organisation::point(starting.x,18,starting.z), 
            organisation::vector(0,1,0), 
            organisation::vector(1,0,0)             
        },
        { 
            organisation::point(starting.x,2,starting.z), 
            organisation::vector(0,-1,0), 
            organisation::vector(1,0,0)             
        },
        { 
            organisation::point(18,starting.y,starting.z), 
            organisation::vector(1,0,0), 
            organisation::vector(0,1,0)             
        },
        { 
            organisation::point(2,starting.y,starting.z), 
            organisation::vector(-1,0,0), 
            organisation::vector(0,1,0)             
        },         
        { 
            organisation::point(starting.x,starting.y,18), 
            organisation::vector(0,0,1), 
            organisation::vector(0,1,0)             
        },       
        { 
            organisation::point(starting.x,starting.y,2), 
            organisation::vector(0,0,-1), 
            organisation::vector(0,1,0)             
        }
    };

    for(auto &it: directions)
    {        
        organisation::program p(width, height, depth);

        std::string input("daisy daisy give me your answer do .");
        std::vector<std::string> expected = organisation::split("daisy daisy daisy daisy me daisy daisy do");

        std::vector<std::string> strings = organisation::split(input);
        organisation::data d(strings);
        
        organisation::genetic::cache cache(width, height, depth);
        cache.set(0, std::get<0>(it));
        
        organisation::genetic::insert insert;
        insert.values = { 1,2,3 };

        organisation::genetic::movement movement;
        movement.directions = { std::get<1>(it) };

        organisation::genetic::collisions collisions;

        collisions.values.resize(27);
        organisation::vector up = std::get<1>(it);
        organisation::vector rebound = std::get<2>(it);
        collisions.values[up.encode()] = rebound.encode();

        p.set(cache);
        p.set(insert);
        p.set(movement);
        p.set(collisions);

        std::string output = p.run(input, d);
        std::vector<std::string> outputs = organisation::split(output);
        
        EXPECT_EQ(outputs, expected);
    }
}

/*
TEST(BasicProgramScores, BasicAssertions)
{
    GTEST_SKIP();

std::string source = R"(daisy daisy give me your answer do .
but you'll look sweet upon the seat .
)";

    const int width = 10, height = 10, depth = 10;

    std::string expected1 = "daisy daisy give me your answer do .";
    std::string expected2 = "but you'll look sweet upon the seat .";

    std::vector<std::string> s1 = organisation::split(expected1);
    std::vector<std::string> s2 = organisation::split(expected2);

    //std::vector<int> lengths = { (int)(s1.size() * 2) + 1, (int)(s2.size() * 2) + 1 };

    organisation::schema schema(width,height,depth);

    std::vector<std::string> strings = organisation::split(source);
    organisation::data d(strings);

    organisation::vector up = { 0,1,0 } ,left = { 1,0,0 };
    organisation::vector down = { 0,-1,0 } ,right = { -1,0,0 };

    std::vector<organisation::vector> in1 = { up, up, left, up, left, up, left, up };
    std::vector<organisation::vector> out1 = { down, right, down, right, down, right, down, right };

    std::vector<organisation::vector> in2 = { up, left, left, left, left, left, left, left };
    std::vector<organisation::vector> out2 = { right, right, right, right, right, right, right, right };

    int x1 = width - 1;
    int y1 = height - 1;
    int z1 = depth - 1;

    int x2 = width - 2;
    int y2 = height - 1;
    int z2 = depth - 1;

    int magnitude = 1;

    for(int i = 0; i < s1.size(); ++i)
    {
        int value1 = d.map(s1[i]);
        
        organisation::vector out = out1[i];

        //schema.prog.set(in1[i], out, magnitude, x1, y1, z1);
        //schema.prog.set(value1, x1, y1, z1);

        x1 += out.x;
        y1 += out.y;
        z1 += out.z;

        out = out2[i];

        int value2 = d.map(s2[i]);
        //schema.prog.set(in2[i], out, magnitude, x2, y2, z2);
        //schema.prog.set(value2, x2, y2, z2);
          
        x2 += out.x;
        y2 += out.y;
        z2 += out.z;
    }

    std::string output1;// = schema.run(0, expected1, d);
    std::string output2;// = schema.run(1, expected2, d);

    EXPECT_STREQ(output1.c_str(), expected1.c_str());
    EXPECT_STREQ(output2.c_str(), expected2.c_str());

    EXPECT_FLOAT_EQ(schema.sum(), 1.0f);
}
*/