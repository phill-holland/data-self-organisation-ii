#include "population.h"
#include "data.h"
#include "general.h"
#include <iostream>
#include <string.h>

#include "fifo.h"
#include "schema.h"
#include "vector.h"
#include "input.h"
#include "output.h"

#include "templates/programs.h"

#include "parallel/device.hpp"
#include "parallel/queue.hpp"
#include "parallel/program.hpp"

using namespace std;

std::string source = R"(daisy daisy give me your answer do .
I'm half crazy for the love of you .
it won't be a stylish marriage .
I can't afford a carriage .
but you'll look sweet upon the seat .
of a bicycle built for two .
)";

const int width = 20, height = 20, depth = 20;
const int device_idx = 0;

organisation::parameters get_parameters(organisation::data &mappings)
{
    organisation::parameters parameters(width, height, depth);

    parameters.dim_clients = organisation::point(5,1,1);
    parameters.iterations = 30;
    parameters.max_values = 30;

    // ***
    parameters.population = 5000;
    // ***

    // ***
    parameters.output_stationary_only = true;
    // ***
    
    parameters.width = width;
    parameters.height = height;
    parameters.depth = depth;
    parameters.mappings = mappings;
        
    std::string input1("daisy daisy give me your answer do .");
    std::string expected1("I'm half crazy for the love of you .");

    std::string input2("it won't be a stylish marriage .");
    std::string expected2("but you'll look sweet upon the seat .");
    
    organisation::inputs::epoch epoch1(input1, expected1);
    organisation::inputs::epoch epoch2(input2, expected2);

    parameters.input.push_back(epoch1);
    parameters.input.push_back(epoch2);
    
    return parameters;
}

bool run(organisation::templates::programs *program, organisation::parameters &parameters, organisation::schema &result)
{         	
    organisation::populations::population p(program, parameters);
    if(!p.initalised()) return false;

    int generations = 2;
    int actual = 0;

    p.clear();
    p.generate();
    
    result.copy(p.go(actual, generations));

    if(actual <= generations) 
    {
        std::string filename("output/run");
        filename += ".txt";

        result.prog.save(filename);
    }
    
    return true;
}

int main(int argc, char *argv[])
{  
    auto strings = organisation::split(source);
    organisation::data mappings(strings);
    
    organisation::parameters parameters = get_parameters(mappings);

	::parallel::device device(device_idx);
	::parallel::queue queue(device);

    parallel::mapper::configuration mapper;

    organisation::schema result(width,height,depth);   
    organisation::parallel::program program(device, &queue, mapper, parameters);

    if(program.initalised())
    {
        run(&program, parameters, result);
    }
       
    return 0;
}