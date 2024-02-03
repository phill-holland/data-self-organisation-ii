#include <gtest/gtest.h>
#include "parallel/inserts.hpp"
#include "parallel/program.hpp"
#include "parallel/map/configuration.hpp"
#include "general.h"
#include "data.h"
#include "schema.h"
#include "kdpoint.h"

TEST(BasicProgramInsertParallel, BasicAssertions)
{    
    GTEST_SKIP();

    const int width = 10, height = 10, depth = 10;

    std::string input1("daisy daisy give me your answer do .");
    std::string input2("monkey monkey eat my face .");
    //std::vector<std::string> expected = organisation::split("daisy daisy daisy daisy me daisy daisy do");

    std::vector<std::string> strings = organisation::split(input1 + " " + input2);
    organisation::data d(strings);

	::parallel::device *device = new ::parallel::device(0);
	::parallel::queue *queue = new parallel::queue(*device);

    organisation::parameters parameters(width, height, depth);
    
    //parameters.clients = 1;
    parameters.dim_clients = organisation::point(1,1,1);

    organisation::inputs::epoch epoch1(input1);
    organisation::inputs::epoch epoch2(input2);

    parameters.input.push_back(epoch1);
    parameters.input.push_back(epoch2);

    organisation::parallel::inserts inserts(*device, queue, parameters, 100);

    organisation::schema s1(width, height, depth);

    organisation::genetic::insert insert;
    insert.values = { 1,2,3 };    
    s1.prog.set(insert);

    std::vector<organisation::schema*> source = { &s1 };
    
    inserts.copy(source.data(), source.size());
    inserts.set(d, parameters.input);
    
    std::vector<int> expected = { 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0 };
    std::vector<int> output;

    //std::vector<organisation::parallel::value> moo;

    for(int i = 0; i < 12; ++i)
    {
        //std::cout << "loop " << i << "\r\n";

        int count = inserts.insert(0);
        output.push_back(count);

        //auto baa = inserts.get();
        //std::copy(baa.begin(),baa.end(),std::back_inserter(moo));
        //std::cout << "inserted " << count << "\r\n";
    }
    
    EXPECT_EQ(expected, output);

    // *****
    // 1) test, insert, both epochs
    // 2) test, more than one client, with different insert schema
    // 3) test input stop after end of sentence!!!
    // 4) PROGRAM COPY FROM INSERTS INTO POPULATION
    // *****    
}

organisation::schema getSchema1(const int width, const int height, const int depth)
{
    organisation::schema s1(width, height, depth);

    organisation::genetic::insert insert;
    insert.values = { 1,2,3 };    

    organisation::genetic::movement movement;
    movement.directions = { { 1,0,0 }, { 1,0,0 } };

    organisation::genetic::cache cache(width, height, depth);
    cache.set(0, organisation::point(18,10,10));

    organisation::genetic::collisions collisions;

    collisions.values.resize(27);
    organisation::vector up(1,0,0);
    organisation::vector rebound(0,1,0);
    collisions.values[up.encode()] = rebound.encode();

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions);

    return s1;
}

organisation::schema getSchema2(const int width, const int height, const int depth)
{
    organisation::schema s1(width, height, depth);

    organisation::genetic::insert insert;
    insert.values = { 1,2,3 };    

    organisation::genetic::movement movement;
    movement.directions = { { 0,1,0 }, { 0,1,0 } };

    organisation::genetic::cache cache(width, height, depth);
    cache.set(3, organisation::point(10,18,10));

    organisation::genetic::collisions collisions;

    collisions.values.resize(27);
    organisation::vector up(0,1,0);
    organisation::vector rebound(1,0,0);
    collisions.values[up.encode()] = rebound.encode();

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions);

    return s1;
}

TEST(BasicProgramMovementParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;

    std::string input1("daisy daisy give me your answer do .");
    std::string input2("monkey monkey eat my face .");
    std::vector<std::string> expected = organisation::split("daisy daisy daisy daisy daisy daisy daisy daisy");

    std::vector<std::string> strings = organisation::split(input1 + " " + input2);
    organisation::data d(strings);

	::parallel::device *device = new ::parallel::device(0);
	::parallel::queue *queue = new parallel::queue(*device);

    organisation::parameters parameters(width, height, depth);
    
    parameters.dim_clients = organisation::point(2,1,1);//(2,1,1);
    parameters.iterations = 30;

    organisation::inputs::epoch epoch1(input1);
    organisation::inputs::epoch epoch2(input2);

    parameters.input.push_back(epoch1);
    parameters.input.push_back(epoch2);

    parallel::mapper::configuration mapper;
    // NOT SURE ABOUT ORIGIN ...??? COULD BE ORIGIN OF ALL CLIENTS
    // NOT JUST SINGLE CLIENT?
    mapper.origin = organisation::point(width / 2, height / 2, depth / 2);


    organisation::parallel::program program(*device, queue, mapper, parameters);
    
    organisation::schema s1 = getSchema1(width, height, depth);
    organisation::schema s2 = getSchema2(width, height, depth);

    std::vector<organisation::schema*> source = { &s1, &s2 };
    
    program.copy(source.data(), source.size());
    program.set(d, parameters.input);

    program.run(d);

    std::vector<organisation::outputs::output> results = program.get(d);
    int epochIdx = 0;
    for(auto &epoch: results)
    {
        std::string value;
        for(auto &output: epoch.values)
        {
            value += output.value + "(" + std::to_string(output.client) + "," + std::to_string(output.index) + ")";
        }

        std::cout << "value (" << epochIdx << ") " << value << "\r\n";
        ++epochIdx;
    }
    
}
// ***
// CACHE NEEDS TO DUPLICATE PER CLIENT
// ***
// tests
// 1) bring forward native test basicMoveAndCollDetection
// 2) multiple clients (with direction movement directions)
// 3) multiple epochs
// 4) insert ends when input words end
// 5) multiple collision directions
// 6) boundaries in all direction removed correctly, and that
// 7) movements for existing cells is correctly identified (movementIdx is correct)
// 8) large scale collision detection in ASCII console test
// 9) input word bounds checking -- does it exceed settings.max_values?
// 10) need to clean up cross breeding errors, validate() == false
// 11) test inserts overlap with existing position
// 12) check loading of schemas > HOST_BUFFER in number
// 13) test all programs are copied into device memory completely, for inserts, collisions, movements
// ***
// TODO
// 1) remove settings.max_values * clients() calculation to single max length
// 2) create new configuration value for max_outputs
// 3) cmake test suite instead?
// ***
// test if direction and rebound the same
//organisation::vector up(1,0,0);
//organisation::vector rebound(1,0,0);

/*
TEST(BasicFrontTest, BasicAssertions)
{
    GTEST_SKIP();

    int points[][2] = { {97,23},{55,77},{34,76},{80,60},{99,04},{81,05},{05,81},{30,79},{15,80},{70,65},
                        {90,40},{40,30},{30,40},{20,60},{60,50},{20,20},{30,01},{60,40},{70,25},{44,62},
                        {55,55},{55,10},{15,45},{83,22},{76,46},{56,32},{45,55},{10,70},{10,30},{97,23}
                      };
    
	const int count = 30;
    const int terms = 2;

    bool expected[count];
    for(int i = 0; i < count; ++i) expected[i] = false;

    expected[0] = true;
    expected[1] = true;
    expected[3] = true;
    expected[4] = true;
    expected[6] = true;
    expected[7] = true;
    expected[8] = true;
    expected[9] = true;
    expected[10] = true;
    expected[29] = true;

    bool result[count];
    for(int i = 0; i < count; ++i) result[i] = true;

    organisation::kdpoint a(2);
    organisation::kdpoint b(2);

    for(int i = 0; i < count; ++i)
    {
        a.set(points[i][0],0);
        a.set(points[i][1],1);

        for(int j = 0; j < count; ++j)
        {
            if(i != j)
            {
                b.set(points[j][0],0);
                b.set(points[j][1],1);

                if(b.dominates(a)) 
                {
                    result[i] = false;
                    break;
                }
            }    
        }
    }

    for(int i = 0; i < count; ++i)
    {
        EXPECT_EQ(result[i], expected[i]);
    }
}
*/