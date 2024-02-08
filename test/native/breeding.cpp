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

TEST(BasicProgramGenerate, BasicAssertions)
{
    GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;
    organisation::point starting(width / 2, height / 2, depth / 2);

    organisation::program p1(width, height, depth);
 
    std::string input("daisy daisy give me your answer do .");        

    std::vector<std::string> strings = organisation::split(input);
    organisation::data d(strings);

    for(int i = 0; i < 100000; ++i)
    {
        p1.generate(d);
        p1.mutate(d);

        EXPECT_TRUE(p1.validate(d));
    }
}

TEST(BasicProgramMutate, BasicAssertions)
{
    GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;
    organisation::point starting(width / 2, height / 2, depth / 2);

    organisation::program p1(width, height, depth);
 
    std::string input("daisy daisy give me your answer do .");        

    std::vector<std::string> strings = organisation::split(input);
    organisation::data d(strings);

    for(int i = 0; i < 100; ++i)
    {
        organisation::program p2(width, height, depth);

        p1.generate(d);        
        p2.copy(p1);

        EXPECT_TRUE(p1.equals(p2));

        p1.mutate(d);

        EXPECT_FALSE(p1.equals(p2));
        EXPECT_TRUE(p1.validate(d));
    }
}

TEST(BasicProgramCross, BasicAssertions)
{
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;
    organisation::point starting(width / 2, height / 2, depth / 2);
     
    std::string input("daisy daisy give me your answer do .");        

    std::vector<std::string> strings = organisation::split(input);
    organisation::data d(strings);

    for(int i = 0; i < 10000; ++i)
    {
        std::cout << "\r\n\r\n";
        organisation::program p1(width, height, depth);
        organisation::program p2(width, height, depth);
        organisation::program p3(width, height, depth);

        p1.generate(d);        
        p2.generate(d);

std::cout << "p1.move.size() " << p1.moo() << "\r\n";
std::cout << "p2.move.size() " << p2.moo() << "\r\n";

        if(p3.cross(p1, p2))
        {
            if(p3.equals(p1))
                std::cout << "doh\r\n";


            if(p3.equals(p2))
                std::cout << "ray\r\n";
            
            if(!p3.validate(d))
                std::cout << "me\r\n";

            EXPECT_FALSE(p3.equals(p1));
            EXPECT_FALSE(p3.equals(p2));
            EXPECT_TRUE(p3.validate(d));
        }
    }
}
