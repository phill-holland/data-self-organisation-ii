#include <gtest/gtest.h>
#include "parallel/program.hpp"
#include "parallel/map/configuration.hpp"
#include "parallel/value.hpp"
#include "general.h"
#include "data.h"
#include "schema.h"
#include "kdpoint.h"
#include "statistics.h"
#include <unordered_map>

organisation::schema getSchema1(organisation::parameters &parameters, organisation::point value)
{
    organisation::schema s1(parameters);

    organisation::genetic::insert insert;
    insert.values = { 1,2,3 };    

    organisation::genetic::movement movement;
    movement.directions = { { 1,0,0 }, { 1,0,0 } };

    organisation::genetic::cache cache(parameters);
    cache.set(value, organisation::point(18,10,10));

    organisation::genetic::collisions collisions(parameters);

    organisation::vector up(1,0,0);
    organisation::vector rebound(0,1,0);

    int offset = 0;
    for(int i = 0; i < parameters.mappings.maximum(); ++i)
    {        
        collisions.set(rebound.encode(), offset + up.encode());
        offset += parameters.max_collisions;
    }

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions);

    return s1;
}

organisation::schema getSchema2(organisation::parameters &parameters, organisation::point value)
{
    organisation::schema s1(parameters);

    organisation::genetic::insert insert;
    insert.values = { 1,2,3 };    

    organisation::genetic::movement movement;
    movement.directions = { { 0,1,0 }, { 0,1,0 } };

    organisation::genetic::cache cache(parameters);
    cache.set(value, organisation::point(10,18,10));

    organisation::genetic::collisions collisions(parameters);

    organisation::vector up(0,1,0);
    organisation::vector rebound(1,0,0);

    int offset = 0;
    for(int i = 0; i < parameters.mappings.maximum(); ++i)
    {        
        collisions.set(rebound.encode(), offset + up.encode());
        offset += parameters.max_collisions;
    }

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions);

    return s1;
}

organisation::schema getSchema3(organisation::parameters &parameters,
                                std::vector<organisation::vector> direction,
                                int delay)
{
    organisation::schema s1(parameters);

    organisation::genetic::insert insert;
    insert.values = { delay };    

    organisation::genetic::movement movement;
    movement.directions = direction;

    organisation::genetic::cache cache(parameters);    
    organisation::genetic::collisions collisions(parameters);

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions);

    return s1;
}

organisation::schema getSchema4(organisation::parameters &parameters,
                                organisation::vector direction,
                                organisation::vector rebound,
                                organisation::point wall,
                                int value,
                                int delay)
{
    organisation::schema s1(parameters);

    organisation::genetic::insert insert;
    insert.values = { delay };    

    organisation::genetic::movement movement;
    movement.directions = { direction };

    organisation::genetic::cache cache(parameters);    
    cache.set(organisation::point(value,-1,-1), wall);

    organisation::genetic::collisions collisions(parameters);

    int offset = 0;
    for(int i = 0; i < parameters.mappings.maximum(); ++i)
    {        
        collisions.set(rebound.encode(), offset + direction.encode());
        offset += parameters.max_collisions;
    }

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions);

    return s1;
}

organisation::schema getSchema5(organisation::parameters &parameters,
                                int direction,
                                int rebound,
                                int iteration,
                                int value,
                                int delay)
{
    organisation::schema s1(parameters);

    organisation::genetic::insert insert;
    insert.values = { delay };    

    organisation::genetic::movement movement;
    organisation::vector _direction;
    _direction.decode(direction);
    movement.directions = { _direction };

    organisation::point wall(parameters.width/2,parameters.height/2,parameters.depth/2);
    for(int i = 0; i < iteration; ++i)
    {
        wall.x += _direction.x;
        wall.y += _direction.y;
        wall.z += _direction.z;
    }

    organisation::genetic::cache cache(parameters);    
    if(wall != organisation::point(parameters.width/2,parameters.height/2,parameters.depth/2))
        cache.set(organisation::point(value,-1,-1), wall);

    organisation::genetic::collisions collisions(parameters);

    int offset = 0;
    for(int i = 0; i < parameters.mappings.maximum(); ++i)
    {        
        collisions.set(rebound, offset + direction);
        offset += parameters.max_collisions;
    }

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions);

    return s1;
}

TEST(BasicProgramMovementWithCollisionParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;
    organisation::point starting(width / 2, height / 2, depth / 2);

    std::string input1("daisy daisy give me your answer do .");
   
    std::vector<std::vector<std::string>> expected = {
        { 
            "daisydaisydaisydaisydaisydaisydaisydaisy"
        }
    };

    std::vector<std::string> strings = organisation::split(input1);
    organisation::data mappings(strings);

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

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    parameters.mappings = mappings;
    parameters.dim_clients = organisation::point(1,1,1);
    parameters.iterations = 30;

    organisation::inputs::epoch epoch1(input1);
    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;
    mapper.origin = organisation::point(width / 2, height / 2, depth / 2);

    for(auto &it: directions)
    {        
        organisation::parallel::program program(device, &queue, mapper, parameters);
        
        EXPECT_TRUE(program.initalised());
        
        organisation::schema s1(parameters);

        organisation::genetic::insert insert;
        insert.values = { 1,2,3 };    

        organisation::genetic::movement movement;
        movement.directions = { std::get<1>(it) };

        organisation::genetic::cache cache(parameters);
        cache.set(organisation::point(0,-1,-1), std::get<0>(it));

        organisation::genetic::collisions collisions(parameters);

        organisation::vector up = std::get<1>(it);
        organisation::vector rebound = std::get<2>(it);

        int offset = 0;
        for(int i = 0; i < parameters.mappings.maximum(); ++i)
        {        
            collisions.set(rebound.encode(), offset + up.encode());
            offset += parameters.max_collisions;
        }

        s1.prog.set(cache);
        s1.prog.set(insert);
        s1.prog.set(movement);
        s1.prog.set(collisions);
        // ***

        std::vector<organisation::schema*> source = { &s1 };
        
        program.copy(source.data(), source.size());
        program.set(mappings, parameters.input);

        program.run(mappings);

        std::vector<std::vector<std::string>> compare;
        std::vector<organisation::outputs::output> results = program.get(mappings);
        
        for(auto &epoch: results)
        {
            std::unordered_map<int,std::vector<std::string>> data;
            for(auto &output: epoch.values)
            {
                if(data.find(output.client) == data.end()) data[output.client] = { output.value };
                else data[output.client].push_back(output.value);
            }

            std::vector<std::string> temp(1);
            for(auto &value: data)
            {
                temp[value.first] = std::reduce(value.second.begin(),value.second.end(),std::string(""));
            }

            compare.push_back(temp);
        }
        
        EXPECT_EQ(compare, expected);
    }
}

TEST(BasicProgramMovementWithCollisionForDifferentWordsParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 60, height = 60, depth = 60;
    organisation::point starting(width / 2, height / 2, depth / 2);

    std::string input1("daisy daisy give me your answer do .");
    
    std::vector<std::vector<std::string>> expected = {
        { 
            "daisydaisydaisydaisydaisydaisydaisydaisy", 
            "daisydaisydaisydaisydaisydaisydaisydaisy" 
        }
    };

    std::vector<organisation::vector> rebounds1 = { 
        organisation::vector(1,0,0),
        organisation::vector(-1,0,0),
        organisation::vector(0,0,1),
        organisation::vector(0,0,-1),
        organisation::vector(1,0,1),
        organisation::vector(-1,0,-1),
        organisation::vector(1,0,0)
    };

    std::vector<organisation::vector> rebounds2 = { 
        organisation::vector(0,0,1),
        organisation::vector(0,0,-1),
        organisation::vector(1,0,0),
        organisation::vector(-1,0,0),
        organisation::vector(1,0,1),
        organisation::vector(-1,0,-1),
        organisation::vector(1,0,0)
    };

    std::vector<std::string> strings = organisation::split(input1);
    organisation::data mappings(strings);
	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    parameters.mappings = mappings;
    parameters.dim_clients = organisation::point(2,1,1);
    parameters.iterations = 35;

    organisation::inputs::epoch epoch1(input1);
    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;
    mapper.origin = organisation::point(width / 2, height / 2, depth / 2);

    organisation::parallel::program program(device, &queue, mapper, parameters);
    
    EXPECT_TRUE(program.initalised());
    
    organisation::schema s1(parameters);
    organisation::schema s2(parameters);

    organisation::genetic::insert insert;
    insert.values = { 2,2,2 };    

    organisation::vector up(0,1,0);

    organisation::genetic::movement movement;
    movement.directions = { up };

    organisation::genetic::cache cache(parameters);
    cache.set(organisation::point(0, -1, -1), organisation::point(starting.x,starting.y + 8,starting.z));

    organisation::genetic::collisions collisions1(parameters);
    organisation::genetic::collisions collisions2(parameters);

    for(int i = 0; i < rebounds1.size(); ++i)
    {
        int offset = parameters.max_collisions * i;
        collisions1.set(rebounds1[i].encode(), offset + up.encode());
    }

    for(int i = 0; i < rebounds2.size(); ++i)
    {
        int offset = parameters.max_collisions * i;
        collisions2.set(rebounds2[i].encode(), offset + up.encode());
    }

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions1);

    s2.prog.set(cache);
    s2.prog.set(insert);
    s2.prog.set(movement);
    s2.prog.set(collisions2);
    // ***

    std::vector<organisation::schema*> source = { &s1, &s2 };
    
    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<std::vector<std::string>> compare;
    std::vector<organisation::outputs::output> results = program.get(mappings);
    
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

    std::vector<organisation::parallel::value> values1 = {
        { organisation::point(31,57,30), organisation::point(0,-1,-1),  3, 0 },
        { organisation::point(29,54,30), organisation::point(1,-1,-1),  6, 0 },
        { organisation::point(30,51,31), organisation::point(2,-1,-1),  9, 0 },
        { organisation::point(30,48,29), organisation::point(3,-1,-1), 12, 0 },        
        { organisation::point(31,45,31), organisation::point(4,-1,-1), 15, 0 },        
        { organisation::point(29,42,29), organisation::point(5,-1,-1), 18, 0 },        
        { organisation::point(31,39,30), organisation::point(6,-1,-1), 21, 0 },        
    };
    
    std::vector<organisation::parallel::value> values2 = {
        { organisation::point(30,57,31), organisation::point(0,-1,-1),  3, 1 },
        { organisation::point(30,54,29), organisation::point(1,-1,-1),  6, 1 },
        { organisation::point(31,51,30), organisation::point(2,-1,-1),  9, 1 },
        { organisation::point(29,48,30), organisation::point(3,-1,-1), 12, 1 },        
        { organisation::point(31,45,31), organisation::point(4,-1,-1), 15, 1 },        
        { organisation::point(29,42,29), organisation::point(5,-1,-1), 18, 1 },        
        { organisation::point(31,39,30), organisation::point(6,-1,-1), 21, 1 },        
    };

    std::vector<organisation::parallel::value> data = program.get();

    std::unordered_map<int,std::vector<organisation::parallel::value>> lookup;
    lookup[0] = { }; lookup[1] = { };

    for(auto &it: data) 
    { 
        lookup[it.client].push_back(it);
    }

    EXPECT_EQ(lookup[0], values1);
    EXPECT_EQ(lookup[1], values2);
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
    organisation::data mappings(strings);

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    
    parameters.dim_clients = organisation::point(2,1,1);
    parameters.iterations = 30;
    parameters.mappings = mappings;

    organisation::inputs::epoch epoch1(input1);
    organisation::inputs::epoch epoch2(input2);

    parameters.input.push_back(epoch1);
    parameters.input.push_back(epoch2);

    parallel::mapper::configuration mapper;
    mapper.origin = organisation::point(width / 2, height / 2, depth / 2);

    organisation::parallel::program program(device, &queue, mapper, parameters);

    EXPECT_TRUE(program.initalised());

    organisation::schema s1 = getSchema1(parameters,organisation::point(0,-1,-1));
    organisation::schema s2 = getSchema2(parameters,organisation::point(3,-1,-1));

    std::vector<organisation::schema*> source = { &s1, &s2 };
    
    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<std::vector<std::string>> compare;
    std::vector<organisation::outputs::output> results = program.get(mappings);
    
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

TEST(BasicProgramMovementMultiCellOutputParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;

    std::string input1("daisy daisy give me your answer do .");
    std::string input2("monkey monkey eat my face .");    
    std::vector<std::vector<std::string>> expected = {
        { 
            "daisygivedaisygivedaisygivedaisygivedaisygivedaisygivedaisygivedaisygive",
            "youranswerdoyouranswerdoyouranswerdoyouranswerdoyouranswerdoyouranswerdoyouranswerdoyouranswerdo"
        },
        { 
            "daisygivedaisygivedaisygivedaisygivedaisygivedaisygive",
            "youranswerdoyouranswerdoyouranswerdoyouranswerdoyouranswerdoyouranswerdo"
        }
    };

    std::vector<std::string> strings = organisation::split(input1 + " " + input2);
    organisation::data mappings(strings);

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    
    parameters.dim_clients = organisation::point(2,1,1);
    parameters.iterations = 30;
    parameters.mappings = mappings;

    organisation::inputs::epoch epoch1(input1);
    organisation::inputs::epoch epoch2(input2);

    parameters.input.push_back(epoch1);
    parameters.input.push_back(epoch2);

    parallel::mapper::configuration mapper;
    mapper.origin = organisation::point(width / 2, height / 2, depth / 2);

    organisation::parallel::program program(device, &queue, mapper, parameters);

    EXPECT_TRUE(program.initalised());

    organisation::schema s1 = getSchema1(parameters,organisation::point(0,1,-1));
    organisation::schema s2 = getSchema2(parameters,organisation::point(3,4,5));

    std::vector<organisation::schema*> source = { &s1, &s2 };
    
    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<std::vector<std::string>> compare;
    std::vector<organisation::outputs::output> results = program.get(mappings);
    
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

TEST(BasicProgramMovementReboundDirectionSameAsMovementDirectionParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;

    std::string data1("daisy daisy give me your answer do please do .");
    std::string input1("give me your .");
    
    std::vector<std::vector<std::string>> expected = {
        { 
            "daisydaisydaisygivedaisygivedaisygivedaisygivemedaisygivemeyourdaisygivemeyourdaisy"
        }
    };

    std::vector<std::string> strings = organisation::split(data1);    
    organisation::data mappings(strings);

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    
    parameters.dim_clients = organisation::point(1,1,1);
    parameters.iterations = 20;
    parameters.mappings = mappings;

    organisation::inputs::epoch epoch1(input1);
    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;
    mapper.origin = organisation::point(width / 2, height / 2, depth / 2);

    organisation::parallel::program program(device, &queue, mapper, parameters);
    
    EXPECT_TRUE(program.initalised());

    organisation::schema s1(parameters);

    organisation::genetic::insert insert;
    insert.values = { 1,2,3 };    

    organisation::genetic::movement movement;
    movement.directions = { { 1,0,0 }, { 1,0,0 } };

    organisation::genetic::cache cache(parameters);
    cache.set(organisation::point(0,-1,-1), organisation::point(18,10,10));

    organisation::genetic::collisions collisions(parameters);

    organisation::vector up(1,0,0);
    organisation::vector rebound(1,0,0);

    int offset = 0;
    for(int i = 0; i < parameters.mappings.maximum(); ++i)
    {        
        collisions.set(rebound.encode(), offset + up.encode());
        offset += parameters.max_collisions;
    }

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions);

    std::vector<organisation::schema*> source = { &s1 };
    
    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<std::vector<std::string>> compare;
    std::vector<organisation::outputs::output> results = program.get(mappings);
    
    for(auto &epoch: results)
    {
        std::unordered_map<int,std::vector<std::string>> data;
        for(auto &output: epoch.values)
        {
            if(data.find(output.client) == data.end()) data[output.client] = { output.value };
            else data[output.client].push_back(output.value);
        }

        std::vector<std::string> temp(1);
        for(auto &value: data)
        {
            temp[value.first] = std::reduce(value.second.begin(),value.second.end(),std::string(""));
        }

        compare.push_back(temp);
    }
    
    EXPECT_EQ(compare, expected);

    std::vector<organisation::parallel::value> values = {
        { organisation::point(17,10,10), organisation::point(1,-1,-1), 2, 0 },
        { organisation::point(16,10,10), organisation::point(2,-1,-1), 5, 0 },
        { organisation::point(15,10,10), organisation::point(3,-1,-1), 9, 0 },
        { organisation::point(14,10,10), organisation::point(7,-1,-1),11, 0 },        
    };
    
    std::vector<organisation::parallel::value> data = program.get();

    EXPECT_EQ(data, values);
}

TEST(BasicProgramMovementReboundDirectionSameAsMovementDirectionOutputStationaryOnlyParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;

    std::string data1("daisy daisy give me your answer do please do .");
    std::string input1("give me your .");
    
    std::vector<std::vector<std::string>> expected = {
        { 
            "daisydaisydaisydaisydaisydaisydaisydaisydaisydaisydaisy"
        }
    };

    std::vector<std::string> strings = organisation::split(data1);    
    organisation::data mappings(strings);

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    
    parameters.dim_clients = organisation::point(1,1,1);
    parameters.iterations = 20;
    parameters.mappings = mappings;
    parameters.output_stationary_only = true;

    organisation::inputs::epoch epoch1(input1);
    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;
    mapper.origin = organisation::point(width / 2, height / 2, depth / 2);

    organisation::parallel::program program(device, &queue, mapper, parameters);
    
    EXPECT_TRUE(program.initalised());

    organisation::schema s1(parameters);

    organisation::genetic::insert insert;
    insert.values = { 1,2,3 };    

    organisation::genetic::movement movement;
    movement.directions = { { 1,0,0 }, { 1,0,0 } };

    organisation::genetic::cache cache(parameters);
    cache.set(organisation::point(0,-1,-1), organisation::point(18,10,10));

    organisation::genetic::collisions collisions(parameters);

    organisation::vector up(1,0,0);
    organisation::vector rebound(1,0,0);

    int offset = 0;
    for(int i = 0; i < parameters.mappings.maximum(); ++i)
    {        
        collisions.set(rebound.encode(), offset + up.encode());
        offset += parameters.max_collisions;
    }

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions);

    std::vector<organisation::schema*> source = { &s1 };
    
    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<std::vector<std::string>> compare;
    std::vector<organisation::outputs::output> results = program.get(mappings);
    
    for(auto &epoch: results)
    {
        std::unordered_map<int,std::vector<std::string>> data;
        for(auto &output: epoch.values)
        {
            if(data.find(output.client) == data.end()) data[output.client] = { output.value };
            else data[output.client].push_back(output.value);
        }

        std::vector<std::string> temp(1);
        for(auto &value: data)
        {
            temp[value.first] = std::reduce(value.second.begin(),value.second.end(),std::string(""));
        }

        compare.push_back(temp);
    }
    
    EXPECT_EQ(compare, expected);

    std::vector<organisation::parallel::value> values = {
        { organisation::point(17,10,10), organisation::point(1,-1,-1), 2, 0 },
        { organisation::point(16,10,10), organisation::point(2,-1,-1), 5, 0 },
        { organisation::point(15,10,10), organisation::point(3,-1,-1), 9, 0 },
        { organisation::point(14,10,10), organisation::point(7,-1,-1),11, 0 },        
    };
    
    std::vector<organisation::parallel::value> data = program.get();

    EXPECT_EQ(data, values);
}

TEST(BasicProgramMovementAllDirectionsBoundaryTestParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;

    std::string input1("daisy");
    
    std::vector<std::string> strings = organisation::split(input1);
    organisation::data mappings(strings);

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);    
    parameters.dim_clients = organisation::point(2,3,1);
    parameters.iterations = 17;
    parameters.mappings = mappings;

    organisation::inputs::epoch epoch1(input1);

    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;    
    organisation::parallel::program program(device, &queue, mapper, parameters);
    
    EXPECT_TRUE(program.initalised());

    organisation::schema s1 = getSchema3(parameters, { { 1, 0, 0 } }, 1);
    organisation::schema s2 = getSchema3(parameters, { {-1, 0, 0 } }, 2);
    organisation::schema s3 = getSchema3(parameters, { { 0, 1, 0 } }, 3);
    organisation::schema s4 = getSchema3(parameters, { { 0,-1, 0 } }, 4);
    organisation::schema s5 = getSchema3(parameters, { { 0, 0, 1 } }, 5);
    organisation::schema s6 = getSchema3(parameters, { { 0, 0,-1 } }, 6);

    std::vector<organisation::schema*> source = { &s1,&s2,&s3,&s4,&s5,&s6 };

    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<organisation::parallel::value> data = program.get();

    EXPECT_TRUE(data.size() == 0);
}

TEST(BasicProgramMovementAllDirectionsPartialBoundaryParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;

    std::string input1("daisy");
    
    std::vector<std::string> strings = organisation::split(input1);
    organisation::data mappings(strings);

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    parameters.mappings = mappings;
    parameters.dim_clients = organisation::point(2,3,1);
    parameters.iterations = 14;

    organisation::inputs::epoch epoch1(input1);

    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;    
    organisation::parallel::program program(device, &queue, mapper, parameters);

    EXPECT_TRUE(program.initalised());

    organisation::schema s1 = getSchema3(parameters, { { 1, 0, 0 } }, 1);
    organisation::schema s2 = getSchema3(parameters, { {-1, 0, 0 } }, 2);
    organisation::schema s3 = getSchema3(parameters, { { 0, 1, 0 } }, 3);
    organisation::schema s4 = getSchema3(parameters, { { 0,-1, 0 } }, 4);
    organisation::schema s5 = getSchema3(parameters, { { 0, 0, 1 } }, 5);
    organisation::schema s6 = getSchema3(parameters, { { 0, 0,-1 } }, 6);

    std::vector<organisation::schema*> source = { &s1,&s2,&s3,&s4,&s5,&s6 };

    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<organisation::parallel::value> data = program.get();

    std::vector<organisation::parallel::value> values = {
        { organisation::point(10, 1,10), organisation::point(0,-1,-1), 2, 3 },
        { organisation::point(10,10,18), organisation::point(0,-1,-1), 3, 4 },
        { organisation::point(10,10, 3), organisation::point(0,-1,-1), 4, 5 }        
    };

    EXPECT_EQ(data,values);
}

TEST(BasicProgramMovementAllDirectionsBoundaryDeleteSuccessfulAndMovementSequenceMaintainedParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;

    std::string input1("daisy");
    
    std::vector<std::string> strings = organisation::split(input1);
    organisation::data mappings(strings);

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    parameters.mappings = mappings;
    parameters.dim_clients = organisation::point(2,3,1);
    parameters.iterations = 14;

    organisation::inputs::epoch epoch1(input1);

    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;    
    organisation::parallel::program program(device, &queue, mapper, parameters);

    EXPECT_TRUE(program.initalised());

    organisation::schema s1 = getSchema3(parameters, { { 1, 0, 0 } }, 1);
    organisation::schema s2 = getSchema3(parameters, { {-1, 0, 0 } }, 2);
    organisation::schema s3 = getSchema3(parameters, { { 0, 1, 0 }, { 1, 0, 0 } }, 3);
    organisation::schema s4 = getSchema3(parameters, { { 0,-1, 0 } }, 4);
    organisation::schema s5 = getSchema3(parameters, { { 0, 0, 1 }, { 1, 1, 0 } }, 5);
    organisation::schema s6 = getSchema3(parameters, { { 0, 0,-1 } }, 6);

    std::vector<organisation::schema*> source = { &s1,&s2,&s3,&s4,&s5,&s6 };

    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<organisation::parallel::value> data = program.get();

    std::vector<organisation::parallel::value> values = {
        { organisation::point(15,15,10), organisation::point(0,-1,-1), 2, 2 },
        { organisation::point(10, 1,10), organisation::point(0,-1,-1), 3, 3 },
        { organisation::point(14,14,14), organisation::point(0,-1,-1), 4, 4 },
        { organisation::point(10,10, 3), organisation::point(0,-1,-1), 5, 5 }        
    };

    EXPECT_EQ(data,values);
}

TEST(BasicProgramTestHostBufferExceededLoadParallel, BasicAssertions)
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

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    parameters.mappings = mappings;
    parameters.dim_clients = organisation::point(2,3,1);
    parameters.iterations = 9;
    parameters.host_buffer = 2;

    organisation::inputs::epoch epoch1(input1);

    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;    
    organisation::parallel::program program(device, &queue, mapper, parameters);
        
    EXPECT_TRUE(program.initalised());

    mapper.origin = organisation::point(width / 2, height / 2, depth / 2);
    
    organisation::schema s1 = getSchema4(parameters, { 1, 0, 0 }, { 0, 1, 0 }, { 12,10,10 }, 0, 1);
    organisation::schema s2 = getSchema4(parameters, {-1, 0, 0 }, { 0,-1, 0 }, {  7,10,10 }, 1, 1);
    organisation::schema s3 = getSchema4(parameters, { 0, 1, 0 }, { 1, 0, 0 }, {10, 12,10 }, 2, 1);
    organisation::schema s4 = getSchema4(parameters, { 0,-1, 0 }, {-1, 0, 0 }, {10,  7,10 }, 3, 1);
    organisation::schema s5 = getSchema4(parameters, { 0, 0, 1 }, { 1, 0, 0 }, {10, 10,12 }, 4, 1);
    organisation::schema s6 = getSchema4(parameters, { 0, 0,-1 }, {-1, 0, 0 }, {10, 10, 7 }, 5, 1);

    std::vector<organisation::schema*> source = { &s1,&s2,&s3,&s4,&s5,&s6 };

    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<std::vector<std::string>> compare;
    std::vector<organisation::outputs::output> results = program.get(mappings);
    
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
        { organisation::point(15,11,10), organisation::point(0,-1,-1), 2, 0 },
        { organisation::point( 5, 9,10), organisation::point(0,-1,-1), 2, 1 },
        { organisation::point(11,15,10), organisation::point(0,-1,-1), 2, 2 },
        { organisation::point( 9, 5,10), organisation::point(0,-1,-1), 2, 3 },
        { organisation::point(11,10,15), organisation::point(0,-1,-1), 2, 4 },
        { organisation::point( 9,10, 5), organisation::point(0,-1,-1), 2, 5 },
    };

    EXPECT_EQ(data,values);       
}

TEST(BasicProgramTestHostBufferNotEvenLoadParallel, BasicAssertions)
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

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    parameters.mappings = mappings;
    parameters.dim_clients = organisation::point(2,3,1);
    parameters.iterations = 9;
    parameters.host_buffer = 5;

    organisation::inputs::epoch epoch1(input1);

    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;    
    organisation::parallel::program program(device, &queue, mapper, parameters);
        
    EXPECT_TRUE(program.initalised());

    mapper.origin = organisation::point(width / 2, height / 2, depth / 2);
    
    organisation::schema s1 = getSchema4(parameters, { 1, 0, 0 }, { 0, 1, 0 }, { 12,10,10 }, 0, 1);
    organisation::schema s2 = getSchema4(parameters, {-1, 0, 0 }, { 0,-1, 0 }, {  7,10,10 }, 1, 1);
    organisation::schema s3 = getSchema4(parameters, { 0, 1, 0 }, { 1, 0, 0 }, {10, 12,10 }, 2, 1);
    organisation::schema s4 = getSchema4(parameters, { 0,-1, 0 }, {-1, 0, 0 }, {10,  7,10 }, 3, 1);
    organisation::schema s5 = getSchema4(parameters, { 0, 0, 1 }, { 1, 0, 0 }, {10, 10,12 }, 4, 1);
    organisation::schema s6 = getSchema4(parameters, { 0, 0,-1 }, {-1, 0, 0 }, {10, 10, 7 }, 5, 1);

    std::vector<organisation::schema*> source = { &s1,&s2,&s3,&s4,&s5,&s6 };

    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<std::vector<std::string>> compare;
    std::vector<organisation::outputs::output> results = program.get(mappings);
    
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
        { organisation::point(15,11,10), organisation::point(0,-1,-1), 2, 0 },
        { organisation::point( 5, 9,10), organisation::point(0,-1,-1), 2, 1 },
        { organisation::point(11,15,10), organisation::point(0,-1,-1), 2, 2 },
        { organisation::point( 9, 5,10), organisation::point(0,-1,-1), 2, 3 },
        { organisation::point(11,10,15), organisation::point(0,-1,-1), 2, 4 },
        { organisation::point( 9,10, 5), organisation::point(0,-1,-1), 2, 5 },
    };

    EXPECT_EQ(data,values);       
}

TEST(BasicProgramDataSwapParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;

    std::string data1("daisy daisy give me your answer do please do .");
    std::string input1("daisy give");
    
    std::vector<std::vector<std::string>> expected = {
        { "givedaisy" },
        { "givedaisy" }
    };

    std::vector<std::string> strings = organisation::split(data1);    
    organisation::data mappings(strings);

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    parameters.mappings = mappings;
    parameters.dim_clients = organisation::point(1,1,1);
    parameters.iterations = 15;

    organisation::inputs::epoch epoch1(input1);
    parameters.input.push_back(epoch1);
    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;
    mapper.origin = organisation::point(width / 2, height / 2, depth / 2);

    organisation::parallel::program program(device, &queue, mapper, parameters);
    
    EXPECT_TRUE(program.initalised());

    organisation::schema s1(parameters);

    organisation::genetic::insert insert;
    insert.values = { 1,8 };    

    organisation::genetic::movement movement;
    movement.directions = { 
        { -1,0,0 }, 
        { -1,0,0 }, 
        { -1,0,0 }, 
        { -1,0,0 },
        { -1,0,0 },
        { -1,0,0 },
        {  0,1,0 }, 
        {  1,0,0 }, 
        { 0,-1,0 }, 
        {  1,0,0 }, 
        {  1,0,0 }, 
        {  1,0,0 }, 
        {  1,0,0 }, 
        {  1,0,0 }, 
        {  1,0,0 }
    };

    organisation::genetic::cache cache(parameters);

    organisation::genetic::collisions collisions(parameters);
    
    organisation::vector left(-1,0,0);
    organisation::vector right(1,0,0);
    organisation::vector rebound(0,1,0);

    int offset = 0;
    for(int i = 0; i < parameters.mappings.maximum(); ++i)
    {        
        collisions.set(rebound.encode(), offset + left.encode());
        collisions.set(rebound.encode(), offset + right.encode());
        offset += parameters.max_collisions;
    }

    s1.prog.set(cache);
    s1.prog.set(insert);
    s1.prog.set(movement);
    s1.prog.set(collisions);

    std::vector<organisation::schema*> source = { &s1 };
    
    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<std::vector<std::string>> compare;
    std::vector<organisation::outputs::output> results = program.get(mappings);
    
    for(auto &epoch: results)
    {
        std::unordered_map<int,std::vector<std::string>> data;
        for(auto &output: epoch.values)
        {
            if(data.find(output.client) == data.end()) data[output.client] = { output.value };
            else data[output.client].push_back(output.value);
        }

        std::vector<std::string> temp(1);
        for(auto &value: data)
        {
            temp[value.first] = std::reduce(value.second.begin(),value.second.end(),std::string(""));
        }

        compare.push_back(temp);
    }
    
    EXPECT_EQ(compare, expected);

    std::vector<organisation::parallel::value> values = {
        { organisation::point( 7,11,10), organisation::point(0,-1,-1), 2, 0 },
        { organisation::point( 8,11,10), organisation::point(1,-1,-1),11, 0 }
    };
    
    std::vector<organisation::parallel::value> data = program.get();

    EXPECT_EQ(data, values);

    std::vector<organisation::statistics::statistic> statistics = program.statistics();

    EXPECT_EQ(statistics.size(), 1);

    organisation::statistics::statistic a;

    a.epochs[0] = organisation::statistics::data(2);
    a.epochs[1] = organisation::statistics::data(2);

    EXPECT_EQ(statistics, std::vector<organisation::statistics::statistic> { a });
}

TEST(BasicProgramScaleTestParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const organisation::point clients(10,10,10);
    const int width = 20, height = 20, depth = 20;

    std::string values1("daisy give me your answer do .");
    std::string input1("daisy");
    std::vector<std::string> expected = 
    { 
        "daisy", "give", "me", "your", "answer", "do"
    };
    
    std::vector<std::string> strings = organisation::split(values1);
    organisation::data mappings(strings);

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    parameters.mappings = mappings;
    parameters.dim_clients = clients;    
    parameters.iterations = 9;
    parameters.host_buffer = 100; 
    parameters.max_inserts = 30;   

    organisation::inputs::epoch epoch1(input1);
    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;    
    organisation::parallel::program program(device, &queue, mapper, parameters);
        
    EXPECT_TRUE(program.initalised());
    
    organisation::schema s1 = getSchema4(parameters, { 1, 0, 0 }, { 0, 1, 0 }, { 12,10,10 }, 0, 1);
    organisation::schema s2 = getSchema4(parameters, {-1, 0, 0 }, { 0,-1, 0 }, {  7,10,10 }, 1, 1);
    organisation::schema s3 = getSchema4(parameters, { 0, 1, 0 }, { 1, 0, 0 }, {10, 12,10 }, 2, 1);
    organisation::schema s4 = getSchema4(parameters, { 0,-1, 0 }, {-1, 0, 0 }, {10,  7,10 }, 3, 1);
    organisation::schema s5 = getSchema4(parameters, { 0, 0, 1 }, { 1, 0, 0 }, {10, 10,12 }, 4, 1);
    organisation::schema s6 = getSchema4(parameters, { 0, 0,-1 }, {-1, 0, 0 }, {10, 10, 7 }, 5, 1);

    std::vector<organisation::schema*> schemas = { &s1,&s2,&s3,&s4,&s5,&s6 };
    std::vector<organisation::schema*> source;

    int length = clients.x * clients.y * clients.z;
    int total = schemas.size();

    for(int i = 0; i < length; ++i)
    {
        source.push_back(schemas[i % total]);
    }
    
    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<std::unordered_map<int,std::string>> compare;
    std::vector<organisation::outputs::output> results = program.get(mappings);
    
    for(auto &epoch: results)
    {
        std::unordered_map<int,std::vector<std::string>> data;
        for(auto &output: epoch.values)
        {
            if(data.find(output.client) == data.end()) data[output.client] = { output.value };
            else data[output.client].push_back(output.value);
        }

        std::unordered_map<int,std::string> temp;
        for(auto &value: data)
        {
            temp[value.first] = std::reduce(value.second.begin(),value.second.end(),std::string(""));
        }

        compare.push_back(temp);
    }

    EXPECT_EQ(compare.size(), 1);
    EXPECT_EQ(source.size(), compare[0].size());

    std::unordered_map<int,std::string> first = compare.front();    
    for(int i = 0; i < length; ++i)
    {
        EXPECT_FALSE(first.find(i) == first.end());
        EXPECT_EQ(first[i],expected[i % total]);
    }
    
    std::vector<organisation::parallel::value> data = program.get();

    EXPECT_EQ(source.size(), data.size());

    std::vector<organisation::parallel::value> values = {
        { organisation::point(15,11,10), organisation::point(0,-1,-1), 2, 0 },
        { organisation::point( 5, 9,10), organisation::point(0,-1,-1), 2, 0 },
        { organisation::point(11,15,10), organisation::point(0,-1,-1), 2, 0 },
        { organisation::point( 9, 5,10), organisation::point(0,-1,-1), 2, 0 },
        { organisation::point(11,10,15), organisation::point(0,-1,-1), 2, 0 },
        { organisation::point( 9,10, 5), organisation::point(0,-1,-1), 2, 0 },
    };

    std::unordered_map<int,organisation::parallel::value> lookup;

    for(auto &it: data) { lookup[it.client] = it; }

    for(int i = 0; i < length; ++i)
    {    
        values[i % total].client = i;
        EXPECT_EQ(lookup[i], values[i % total]);
    }
}

TEST(BasicProgramAllDirectionsParallel, BasicAssertions)
{    
    //GTEST_SKIP();

    const organisation::point clients(27,1,1);
    const int width = 20, height = 20, depth = 20;

    std::string values1("a b c d e f g h i j k l m n o p q r s t u v w x y z .");
    std::string input1("a");
    
    std::vector<std::string> strings = organisation::split(values1);
    organisation::data mappings(strings);

	::parallel::device device(0);
	::parallel::queue queue(device);

    organisation::parameters parameters(width, height, depth);
    parameters.mappings = mappings;
    parameters.dim_clients = clients;    
    parameters.iterations = 8;
    parameters.host_buffer = 100; 
    parameters.max_inserts = 30;   

    organisation::inputs::epoch epoch1(input1);
    parameters.input.push_back(epoch1);

    parallel::mapper::configuration mapper;    
    organisation::parallel::program program(device, &queue, mapper, parameters);
        
    EXPECT_TRUE(program.initalised());
    
    const int iterations = 5;
    std::vector<organisation::parallel::value> predictions;
    std::vector<organisation::schema*> source;

    for(int i = 0; i < parameters.max_collisions; ++i)
    {
        int rebound = i + 1;
        if(rebound >= parameters.max_collisions) rebound = 0;

        organisation::schema temp = getSchema5(parameters, i, rebound, iterations, i, 1);
        organisation::schema *schema = new organisation::schema(parameters);
        
        EXPECT_TRUE(schema != NULL);
        EXPECT_TRUE(schema->initalised());

        schema->copy(temp);
        source.push_back(schema);

        organisation::vector _direction, _rebound;
        _direction.decode(i);
        _rebound.decode(rebound);

        organisation::point prediction(width/2,height/2,depth/2);
        for(int i = 0; i < iterations - 1; ++i)
        {
            prediction.x += _direction.x;
            prediction.y += _direction.y;
            prediction.z += _direction.z;
        }

        if(prediction != organisation::point(width/2,height/2,depth/2))
        {
            prediction.x += _rebound.x;
            prediction.y += _rebound.y;
            prediction.z += _rebound.z;
        }

        organisation::parallel::value value = { prediction, organisation::point(0,-1,-1), 2, i };
        predictions.push_back(value);
    }
    
    program.copy(source.data(), source.size());
    program.set(mappings, parameters.input);

    program.run(mappings);

    std::vector<organisation::parallel::value> data = program.get();

    EXPECT_EQ(data, predictions);

    std::vector<std::unordered_map<int,std::string>> compare;
    std::vector<organisation::outputs::output> results = program.get(mappings);
    
    for(auto &epoch: results)
    {
        std::unordered_map<int,std::vector<std::string>> data;
        for(auto &output: epoch.values)
        {
            if(data.find(output.client) == data.end()) data[output.client] = { output.value };
            else data[output.client].push_back(output.value);
        }

        std::unordered_map<int,std::string> temp;
        for(auto &value: data)
        {
            temp[value.first] = std::reduce(value.second.begin(),value.second.end(),std::string(""));
        }

        compare.push_back(temp);
    }

    EXPECT_EQ(compare.size(), 1);
    EXPECT_EQ(strings.size() - 1, compare[0].size());

    std::unordered_map<int,std::string> first = compare.front();        

    for(int i = 0; i < strings.size(); ++i)
    {
        if(strings[i] != std::string("n"))
            EXPECT_EQ(first[i],strings[i]);
    }

   for(auto &it:source)
   {
        if(it != NULL) delete it;
   }
}
