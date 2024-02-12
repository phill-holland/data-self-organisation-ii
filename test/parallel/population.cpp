#include <gtest/gtest.h>
#include "parallel/program.hpp"
#include "parallel/map/configuration.hpp"
#include "parallel/value.hpp"
#include "general.h"
#include "data.h"
#include "schema.h"
#include "kdpoint.h"
#include "population.h"
#include <unordered_map>

organisation::schema getSchema(const int width, const int height, const int depth, 
                               organisation::vector direction,
                               organisation::vector rebound,
                               organisation::point wall,
                               int value,
                               int delay)
{
    organisation::schema s1(width, height, depth);

    organisation::genetic::insert insert;
    insert.values = { delay };    

    organisation::genetic::movement movement;
    movement.directions = { direction };

    organisation::genetic::cache cache(width, height, depth);    
    cache.set(value, wall);

    organisation::genetic::collisions collisions;

    collisions.values.resize(27);
    collisions.values[direction.encode()] = rebound.encode();

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions);

    return s1;
}

TEST(BasicPopulationTestParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;

    std::string values1("daisy give me your answer do .");
    std::string input1("daisy");
     std::vector<std::vector<std::string>> expected = {
        { 
            "daisy", "give", "me", "your", "answer", "do"
        }
    };

    std::vector<std::string> strings = organisation::split(values1);
    organisation::data mappings(strings);

	::parallel::device *device = new ::parallel::device(0);
	::parallel::queue *queue = new parallel::queue(*device);

    organisation::parameters parameters(width, height, depth);
    
    parameters.dim_clients = organisation::point(2,3,1);
    parameters.iterations = 9;
    parameters.mappings = mappings;

    organisation::inputs::epoch epoch1(input1);

    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;    
    organisation::parallel::program program(*device, queue, mapper, parameters);
        
    EXPECT_TRUE(program.initalised());
        
    organisation::schema s1 = getSchema(width, height, depth, { 1, 0, 0 }, { 0, 1, 0 }, { 12,10,10 }, 0, 1);
    organisation::schema s2 = getSchema(width, height, depth, {-1, 0, 0 }, { 0,-1, 0 }, {  7,10,10 }, 1, 1);
    organisation::schema s3 = getSchema(width, height, depth, { 0, 1, 0 }, { 1, 0, 0 }, {10, 12,10 }, 2, 1);
    organisation::schema s4 = getSchema(width, height, depth, { 0,-1, 0 }, {-1, 0, 0 }, {10,  7,10 }, 3, 1);
    organisation::schema s5 = getSchema(width, height, depth, { 0, 0, 1 }, { 1, 0, 0 }, {10, 10,12 }, 4, 1);
    organisation::schema s6 = getSchema(width, height, depth, { 0, 0,-1 }, {-1, 0, 0 }, {10, 10, 7 }, 5, 1);

    std::vector<organisation::schema*> source = { &s1,&s2,&s3,&s4,&s5,&s6 };

    parameters.population = source.size() * 2;    
    
    organisation::populations::population population(&program, parameters);
    EXPECT_TRUE(population.initalised());
/*
    for(int i = 0; i < source.size(); ++i)
    {
        EXPECT_TRUE(population.set(*source[i], i));
    }
*/

population.clear();
population.generate();

    int generation = 0;
    int generations = 1;

    population.go(generation, generations);

/*
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

        std::vector<std::string> temp(6);
        for(auto &value: data)
        {
            temp[value.first] = std::reduce(value.second.begin(),value.second.end(),std::string(""));
        }

        compare.push_back(temp);
    }

    EXPECT_EQ(compare, expected);
    
    std::vector<organisation::parallel::value> data = program.get();

    std::vector<organisation::parallel::value> values = {
        { organisation::point(15,11,10),0,0 },
        { organisation::point( 5, 9,10),0,1 },
        { organisation::point(11,15,10),0,2 },
        { organisation::point( 9, 5,10),0,3 },
        { organisation::point(11,10,15),0,4 },
        { organisation::point( 9,10, 5),0,5 },
    };

    EXPECT_EQ(data,values); 
    */     
}
