#include "population.h"
#include "general.h"
#include <iostream>
#include <fcntl.h>
#include <future>
#include <algorithm>
#include <signal.h>
#include <chrono>
#include <bits/stdc++.h>

std::mt19937_64 organisation::populations::population::generator(std::random_device{}());

void organisation::populations::population::reset(templates::programs *programs, parameters &params)
{
    init = false; cleanup();

    this->programs = programs;
    settings = params;

    if(settings.input.size() == 0) return;
    dimensions = settings.input.dimensions();
    if(dimensions == 0) return;

    schemas = new organisation::schemas(settings.width, settings.height, settings.depth, settings.population);
    if (schemas == NULL) return;
    if (!schemas->initalised()) return;

    intermediateA = new organisation::schema*[settings.clients()];
    if (intermediateA == NULL) return;
    for(int i = 0; i < settings.clients(); ++i) intermediateA[i] = NULL;
    for(int i = 0; i < settings.clients(); ++i)
    {
        intermediateA[i] = new organisation::schema(settings.width, settings.height, settings.depth);
        if(intermediateA[i] == NULL) return;
        if(!intermediateA[i]->initalised()) return;
    }

    intermediateB = new organisation::schema*[settings.clients()];
    if (intermediateB == NULL) return;
    for(int i = 0; i < settings.clients(); ++i) intermediateB[i] = NULL;
    for(int i = 0; i < settings.clients(); ++i)
    {
        intermediateB[i] = new organisation::schema(settings.width, settings.height, settings.depth);
        if(intermediateB[i] == NULL) return;
        if(!intermediateB[i]->initalised()) return;
    }

    intermediateC = new organisation::schema*[settings.clients()];
    if (intermediateC == NULL) return;
    for(int i = 0; i < settings.clients(); ++i) intermediateC[i] = NULL;
    for(int i = 0; i < settings.clients(); ++i)
    {
        intermediateC[i] = new organisation::schema(settings.width, settings.height, settings.depth);
        if(intermediateC[i] == NULL) return;
        if(!intermediateC[i]->initalised()) return;
    }

    init = true;
}

void organisation::populations::population::clear()
{
    schemas->clear();
}

void organisation::populations::population::generate()
{
    schemas->generate(settings.mappings);
}

organisation::schema organisation::populations::population::go(int &count, int iterations)
{    
    float highest = 0.0f;
    bool finished = false;
    count = 0;

    schema res(settings.width, settings.height, settings.depth);

    organisation::schema **set = intermediateC, **run = intermediateA, **get = intermediateB;

    region rset = { 0, (settings.population / 2) - 1 };
    region rget = { (settings.population / 2), settings.population - 1 };

    fill(intermediateA, rset);
    pull(intermediateC, rset);
    
    do
    {
        auto r1 = std::async(&organisation::populations::population::push, this, set, rset);
        auto r2 = std::async(&organisation::populations::population::pull, this, get, rget);
        auto r3 = std::async(&organisation::populations::population::execute, this, run);

        r1.wait();
        r2.wait();
        r3.wait();

        organisation::populations::results result = r3.get();

        if(result.best > highest)
        {
            res.copy(*run[result.index]);
            highest = result.best;
        }

        if(result.best >= 0.9999f) finished = true;    
        
        std::cout << "Generation (" << count << ") Best=" << result.best;
        std::cout << " Highest=" << highest;
        std::cout << " Avg=" << result.average;
        std::cout << "\r\n";

        if((iterations > 0)&&(count >= iterations)) finished = true;

        organisation::schema **t1 = set;
        set = run;
        run = get;
        get = t1;

        region t2 = rset;
        rset = rget;
        rget = t2;

        ++count;

    } while(!finished);

    return res;
}

organisation::populations::results organisation::populations::population::execute(organisation::schema **buffer)
{  
    std::chrono::high_resolution_clock::time_point previous = std::chrono::high_resolution_clock::now();   

    programs->clear();
    programs->copy(buffer, settings.clients());
    programs->set(settings.mappings, settings.input);
    programs->run(settings.mappings);

    std::vector<organisation::outputs::output> values = programs->get(settings.mappings);
    std::unordered_map<int,std::vector<std::tuple<std::string,std::string>>> mappings;

    for(int epoch = 0; epoch < values.size(); ++epoch)
    {
        organisation::inputs::epoch e;
        if(settings.input.get(e,epoch))
        {
            std::vector<organisation::outputs::data> scores = values[epoch].values;
            for(auto &it:scores)
            {
                if(mappings.find(it.client) == mappings.end())
                    mappings[it.client] = { };
                
                mappings[it.client].push_back(std::tuple<std::string,std::string>(e.expected,it.value));
            }
        }
    }

    results result;

    for(auto &it: mappings)
    {
        if((it.first >= 0)&&(it.first < settings.clients()))
        {
            buffer[it.first]->compute(it.second);

            float score = buffer[it.first]->sum();
            if(score > result.best)
            {
                result.best = score;
                result.index = it.first;
            }
            
            result.average += score;
        }
    }
    
    if(values.size() > 0)
        result.average /= (float)values.size();
    
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now - previous);   
    std::cout << "execute " << time_span.count() << "\r\n";    

    return result;
}

bool organisation::populations::population::get(schema &destination, region r)
{
    const float mutate_rate_in_percent = 20.0f;
    const float mutation = (((float)settings.population) / 100.0f) * mutate_rate_in_percent;

    int t = (std::uniform_int_distribution<int>{0, settings.population - 1})(generator);

    if(((float)t) <= mutation) 
    {
        schema *s1 = best(r);
        if(s1 == NULL) return false;

        destination.copy(*s1);
        destination.mutate(settings.mappings);            
    }
    else
    {
        schema *s1 = best(r);
        if(s1 == NULL) return false;
        schema *s2 = best(r);
        if(s2 == NULL) return false;
                     
        s1->cross(&destination, s2);
        return destination.validate(settings.mappings);
    }

    return true;
}

bool organisation::populations::population::set(schema &source, region r)
{    
    schema *destination = worst(r);
    if(destination == NULL) return false;

    destination->copy(source);

    return true;
}

organisation::schema *organisation::populations::population::best(region r)
{
    const int samples = 10;

	std::uniform_int_distribution<int> rand{ r.start, r.end };

	kdpoint temp1(dimensions), temp2(dimensions);
	kdpoint origin(dimensions);

	temp1.set(0L);
	temp2.set(0L);
	origin.set(0L);

    int competition;

    int best = rand(generator);    	
    float score = schemas->get(best)->sum();

	for (int i = 0; i < samples; ++i)
	{
		competition = rand(generator);

        schemas->get(best)->get(temp1, minimum, maximum);
        schemas->get(competition)->get(temp2, minimum, maximum);

		float t2 = schemas->get(competition)->sum();

        if(temp2.dominates(temp1))
        {
            best = competition;
            score = t2;
        }         
	}

    return schemas->get(best);
}

organisation::schema *organisation::populations::population::worst(region r)
{
    const int samples = 10;

	std::uniform_int_distribution<int> rand{ r.start, r.end };

	kdpoint temp1(dimensions), temp2(dimensions);
	kdpoint origin(dimensions);

	temp1.set(0L);
	temp2.set(0L);
	origin.set(0L);

    int competition;
    int worst = rand(generator);

	float score = schemas->get(worst)->sum();

	for (int i = 0; i < samples; ++i)
	{
		competition = rand(generator);

        schemas->get(worst)->get(temp1, minimum, maximum);
        schemas->get(competition)->get(temp2, minimum, maximum);

		float t2 = schemas->get(competition)->sum();

        if(temp1.dominates(temp2))
        {
            worst = competition;             
            score = t2;
        }
	}

    return schemas->get(worst);
}

void organisation::populations::population::pull(organisation::schema **buffer, region r)
{
    std::chrono::high_resolution_clock::time_point previous = std::chrono::high_resolution_clock::now();   
    const int escape = 15;

    if(settings.best)
    {
        for(int i = 0; i < settings.clients(); ++i)
        {
            int counter = 0;
            while(!get(*buffer[i], r)&&(++counter<escape)) { };
            if(counter>=escape)
            {
                std::cout << "repeated get error!\r\n";
            }
        }
    }
    else
    {
        int offset = 0;
        for(int i = r.start; i <= r.end; ++i)
        { 
            organisation::schema *temp = schemas->get(i);
            if(temp != NULL)           
                buffer[offset++]->copy(*temp);
        }
    }


    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now - previous);   
    std::cout << "pull(" << settings.best << ") " << time_span.count() << "\r\n";    
}

void organisation::populations::population::push(organisation::schema **buffer, region r)
{
    std::chrono::high_resolution_clock::time_point previous = std::chrono::high_resolution_clock::now();
    
    if(settings.worst)
    {
        for(int i = 0; i < settings.clients(); ++i)
        {
            set(*buffer[i], r);
        }
    }
    else
    {
        int offset = 0;
        for(int i = r.start; i <= r.end; ++i)
        {
            organisation::schema *temp = schemas->get(i);
            if(temp != NULL)
                temp->copy(*buffer[offset++]);
        }
    }

    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now - previous);   
    std::cout << "push(" << settings.worst << ") " << time_span.count() << "\r\n";    
}

void organisation::populations::population::fill(organisation::schema **destination, region r)
{
    int offset = 0;
    for(int i = r.start; i <= r.end; ++i)
    {
        destination[offset++]->copy(*schemas->get(i));
    }
}

void organisation::populations::population::makeNull() 
{ 
    schemas = NULL;
    intermediateA = NULL;
    intermediateB = NULL;
    intermediateC = NULL;
    programs = NULL;
}

void organisation::populations::population::cleanup() 
{     
    if(intermediateC != NULL)
    {
        for(int i = settings.clients() - 1; i >= 0; --i)
        {
            if(intermediateC[i] != NULL) delete intermediateC[i];
        }
        delete[] intermediateC;
    }
 
    if(intermediateB != NULL)
    {
        for(int i = settings.clients() - 1; i >= 0; --i)
        {
            if(intermediateB[i] != NULL) delete intermediateB[i];
        }
        delete[] intermediateB;
    }

   if(intermediateA != NULL)
    {
        for(int i = settings.clients() - 1; i >= 0; --i)
        {
            if(intermediateA[i] != NULL) delete intermediateA[i];
        }
        delete[] intermediateA;
    }
 
    if(schemas != NULL) delete schemas;    
}