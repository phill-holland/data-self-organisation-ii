#include <gtest/gtest.h>
#include "parallel/inserts.hpp"
#include "parallel/program.hpp"
#include "parallel/map/configuration.hpp"
#include "general.h"
#include "data.h"
#include "schema.h"
#include "kdpoint.h"
#include <unordered_map>

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

TEST(BasicProgramMovementWithTwoClientsAndTwoEpochsParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;

    std::string input1("daisy daisy give me your answer do .");
    std::string input2("monkey monkey eat my face .");    
    std::vector<std::vector<std::string>> expected = {
        { 
            "daisydaisydaisydaisydaisydaisydaisydaisy",
            "youryouryouryouryouryouryouryour"
        },
        { 
            "daisydaisydaisydaisydaisydaisy",
            "youryouryouryouryouryour"
        }
    };

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

    std::vector<std::vector<std::string>> compare;
    std::vector<organisation::outputs::output> results = program.get(d);
    
    for(auto &epoch: results)
    {
        std::unordered_map<int,std::vector<std::string>> data;
        for(auto &output: epoch.values)
        {
            if(data.find(output.client) == data.end()) data[output.client] = { output.value };
            else data[output.client].push_back(output.value);
        }

        std::vector<std::string> temp(2);
        for(auto &value: data)
        {
            temp[value.first] = std::reduce(value.second.begin(),value.second.end(),std::string(""));
        }

        compare.push_back(temp);
    }
    
    EXPECT_EQ(compare, expected);
}
// ***
// CACHE NEEDS TO DUPLICATE PER CLIENT
// ***
// tests
// 1) bring forward native test basicMoveAndCollDetection
// 2) multiple clients (with direction movement directions)
// 3) multiple epochs
// 4) insert ends when input words end OK
// 11) test inserts overlap with existing position **** (formal test)
// 12) inserts with multiple clients **** (formal test)
// 5) multiple collision directions
// 6) boundaries in all direction removed correctly, and that
// 7) movements for existing cells is correctly identified (movementIdx is correct)
// 8) large scale collision detection in ASCII console test
// 9) input word bounds checking -- does it exceed settings.max_values?
// 10) need to clean up cross breeding errors, validate() == false
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
