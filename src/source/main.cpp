#include "population.h"
#include "data.h"
#include "history.h"
#include "general.h"
#include <iostream>
#include <string.h>

#include "fifo.h"
#include "schema.h"
#include "vector.h"

#include "parallel/device.hpp"
#include "parallel/queue.hpp"
#include "parallel/program.hpp"

using namespace std;

std::string source = R"(daisy daisy give me your answer do .
I'm half crazy for the love of you .)";
/*
std::string source = R"(daisy daisy give me your answer do .
I'm half crazy for the love of you .
it won't be a stylish marriage .
I can't afford a carriage .
but you'll look sweet upon the seat .
of a bicycle built for two .
)";
*/

std::vector<std::string> expected = { "daisy daisy give me your answer do .", "I'm half crazy for the love of you ." };

const int device = 0;
const int rounds = 15;
const int population = 4000, clients = 3500;
const int iterations = 1000;

organisation::parallel::parameters get()
{
    const int width = 5, height = 5, depth = 5, in = 15, out = 10;

    organisation::parallel::parameters parameters(width, height, depth, in, out);
    parameters.epochs = expected.size();
    return parameters; 
}

organisation::schema run(organisation::parallel::parameters parameters, organisation::data &mappings, std::vector<std::string> expected, int round = 0)
{         
	::parallel::device *dev = new ::parallel::device(device);
	::parallel::queue *q = new parallel::queue(*dev);
    
    organisation::populations::parameters settings;
    settings.params = parameters;
    settings.dev = dev;
    settings.q = q;
    settings.expected = expected;
    settings.mappings = mappings;
    settings.clients = clients;
    settings.size = population;
    
    organisation::populations::population p(settings);

    int actual = 0;

    p.clear();
    p.generate();

    organisation::schema best(settings.params.width,settings.params.height,settings.params.depth);
    best.copy(p.go(expected, actual, iterations));

    if(actual <= iterations) 
    {
        std::string filename("output/run");
        filename += std::to_string(round);
        filename += ".txt";

        best.prog.save(filename);
    }
    
    return best;
}

bool single(organisation::schema &schema, organisation::data &mappings, organisation::parallel::parameters parameters, std::vector<std::string> expected)
{          
	::parallel::device *dev = new ::parallel::device(0);
	::parallel::queue *q = new parallel::queue(*dev);

    const int clients = 1;

    organisation::parallel::program p_program(*dev, parameters, clients);

    p_program.clear(q);

    std::vector<organisation::schema*> source = { &schema };
    p_program.copy(source.data(), source.size(), q);

    int x1 = (parameters.width / 2);
    int y1 = (parameters.height / 2);
    int z1 = (parameters.depth / 2);

    organisation::vector w {0,1,0};
    std::vector<sycl::float4> positions;

    int j = 0;
    for(std::vector<std::string>::iterator it = expected.begin(); it != expected.end(); ++it)
    {
        positions.push_back( { x1 + j, y1, z1, w.encode() } );
        ++j;
    }

    p_program.set(positions, q);

    p_program.run(q);

    std::vector<organisation::parallel::output> results = p_program.get(mappings, q);
    
    int index = 0;
    for(std::vector<std::string>::iterator it = expected.begin(); it != expected.end(); ++it)
    {
        //std::string out1 = schema.run(index, *it, mappings, NULL);
        std::string out1;
        std::cout << "CPU " << index << " [" << out1 << "]\r\n";

        if(results[0].values.size() > index)
        {
            std::string results1 = results[0].values[index];
            std::cout << "GPU " << index << " [" << results1 << "]\r\n";
            if(*it == results1) std::cout << "OK\r\n";
            else std::cout << "NOT OK\r\n";
        }
        else std::cout << "GPU " << index << " NO OUTPUT\r\n";

        ++index;
    }

    return true;
}

int main(int argc, char *argv[])
{  
    std::vector<std::string> devices = ::parallel::device::enumerate();
    for(std::vector<std::string>::iterator it = devices.begin(); it < devices.end(); ++it)
    {
        std::cout << *it << "\r\n";
    }

    auto strings = organisation::split(source);
    organisation::data mappings(strings);
    
    organisation::parallel::parameters parameters = get();
        
    for(int i = 0; i < rounds; ++i)
    {
        organisation::schema best = run(parameters, mappings, expected, i);
        single(best, mappings, parameters, expected);
    }
    
    return 0;
}