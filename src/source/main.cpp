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
#include "dictionary.h"

#include "templates/programs.h"

#include "parallel/device.hpp"
#include "parallel/queue.hpp"
#include "parallel/program.hpp"

using namespace std;

const organisation::dictionary dictionary;

const int width = 6, height = 6, depth = 6;
const int device_idx = 0;
const int generations = 500;

organisation::parameters get_parameters(organisation::data &mappings)
{
    organisation::parameters parameters(width, height, depth);

    parameters.dim_clients = organisation::point(10,10,10);
    parameters.iterations = 30;
    parameters.max_values = 100;
    parameters.max_cache = parameters.max_values;
    parameters.max_cache_dimension = 3;

    parameters.population = parameters.clients() * 4;

    parameters.output_stationary_only = true;
    
    parameters.width = width;
    parameters.height = height;
    parameters.depth = depth;
    parameters.mappings = mappings;        

    // ***    
    parameters.scores.max_collisions = 1;
    // ***

    //std::string input1("daisy daisy give me your answer do");
    //std::string expected1("I'm half crazy for the love of you");
            
    std::string input1("daisy daisy");
    std::string expected1("I'm half crazy for the love of");
        
    organisation::inputs::epoch epoch1(input1, expected1);
    
    parameters.input.push_back(epoch1);
    
    std::cout << "input: \"" << input1 << "\" expected: \"" << expected1 << "\"\r\n";

    return parameters;
}

bool run(organisation::templates::programs *program, organisation::parameters &parameters, organisation::schema &result)
{         	
    organisation::populations::population p(program, parameters);
    if(!p.initalised()) return false;

    int actual = 0;

    p.clear();
    p.generate();
    
    result.copy(p.go(actual, generations));

    if(actual <= generations) 
    {
        std::string filename("output/run.txt");
        result.prog.save(filename);
    }
    
    return true;
}

int main(int argc, char *argv[])
{  
    auto strings = dictionary.get();
    organisation::data mappings(strings);
    
    organisation::parameters parameters = get_parameters(mappings);

	::parallel::device device(device_idx);
	::parallel::queue queue(device);

    parallel::mapper::configuration mapper;

    organisation::schema result(parameters);   
    organisation::parallel::program program(device, &queue, mapper, parameters);

    if(program.initalised())
    {
        run(&program, parameters, result);
    }
       
    return 0;
}