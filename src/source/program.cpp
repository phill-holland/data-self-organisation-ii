#include "program.h"
#include "position.h"
#include "genetic/templates/genetic.h"
#include "general.h"
#include <stack>
#include <unordered_map>
#include <iostream>
#include <tuple>
#include <fstream>
#include <sstream>
#include <functional>

std::mt19937_64 organisation::program::generator(std::random_device{}());

void organisation::program::reset(int w, int h, int d)
{
    init = false; cleanup();

    _width = w; _height = h; _depth = d;
    length = _width * _height * _depth;

    clear();

    init = true;
}

void organisation::program::clear()
{    
    caches.clear();
    movement.clear();
    collisions.clear();
    insert.clear();
}

void organisation::program::generate(data &source)
{
    clear();

    templates::genetic *genes[] = 
    { 
        &caches,
        &movement,
        &collisions,
        &insert
    }; 

    const int components = sizeof(genes) / sizeof(templates::genetic*);
    for(int i = 0; i < components; ++i)
    {
        genes[i]->generate(source);
    }
/*
    const int max_repeats = 2;
    const int max_cache = source.maximum();

    std::vector<int> raw = source.all();
    
    for(auto &it: raw)
    {
        int n = (std::uniform_int_distribution<int>{0, max_repeats})(generator);
        for(int i = 0; i < n; ++i)
        {
            point p1;
            p1.generate(_width, _height, _depth);
            int index = ((_width * _height) * p1.z) + ((p1.y * _width) + p1.x);
            if(points.find(index) == points.end())
            {
                cache.push_back(std::tuple<int,point>(it,p1));
                if(cache.size() >= max_cache) return;
            }
        }
    }

    const int max_duplicate_insert = 2;
    int n = (std::uniform_int_distribution<int>{1, max_duplicate_insert})(generator);
    for(int i = 0; i <= n; ++i)
    {
        movement m1;
        m1.generate(_width, _height, _depth);
        movements.push_back(m1);
    }

    for(int i = 0; i <= 27; ++i)
    {
        int value = (std::uniform_int_distribution<int>{0, 27})(generator);
        collisions.push_back(value);
    }
*/
}

void organisation::program::mutate(data &source)
{    
    templates::genetic *genes[] = 
    { 
        &caches,
        &movement,
        &collisions,
        &insert
    }; 

    const int components = sizeof(genes) / sizeof(templates::genetic*);

    const int idx = (std::uniform_int_distribution<int>{0, components - 1})(generator);
    genes[idx]->mutate(source);


    /*
    std::vector<int> lengths = { (int)cache.size(), (int)movements.size(), (int)collisions.size() };
    int total = 0;
    for(int i = 0; i < lengths.size(); ++i)
    {
        total += lengths[i];
    }

    int j = (std::uniform_int_distribution<int>{0, total - 1})(generator);
    int index = 0;
    int running = lengths[0];
    for(int i = 1; i < lengths.size(); ++i)
    {
        if(j < running) break;
        ++index;
        running += lengths[i];
    }

    int offset = j - running;

    if(index == 0)
    {
        int value = std::get<0>(cache[offset]);
        point p1;

        p1.generate(_width, _height, _depth);
        int index = ((_width * _height) * p1.z) + ((p1.y * _width) + p1.x);

        if(points.find(index) != points.end()) p1 = std::get<1>(cache[offset]);

        std::vector<int> all = source.all();
        int t1 = (std::uniform_int_distribution<int>{0, (int)(all.size() - 1)})(generator);
        value = all[t1];
    }
    else if(index == 1)
    {
        movements[offset].mutate(_width, _height, _depth);
    }
    else if(index == 2)
    {
        int value = (std::uniform_int_distribution<int>{0, 27})(generator);
        collisions[offset] = value;
    }
    */
}
/*
class insert
{
    int insert_frame_index;
    std::vector<std::string> word;
};
*/

std::string organisation::program::run5(std::string input, data &source, int max, history *destination)
{
    auto offset = [this](point &src)
    {
        return ((this->_width * this->_height) * src.z) + ((src.y * this->_width) + src.x);
    };
    
    auto _distance = [](point &a, point &b)
    {
        float x = (a.x - b.x) * (a.x - b.x);
        float y = (a.y - b.y) * (a.y - b.y);
        float z = (a.z - b.z) * (a.z - b.z);

        int d = (int)sqrtf(x + y + z);

        return d;

    };

    std::vector<int> values = source.get(input);
    std::vector<int> results;

    point starting;

    int half_width = (_width / 2);
    int half_height = (_height / 2);
    int half_depth = (_depth / 2);

    starting.x = half_width;
    starting.y = half_height;
    starting.z = half_depth;

    std::vector<position> points;    
    std::vector<position> stationary;

    points.reserve(255);

    position a, b;

    a.current = point(5,5,5);
    a.direction = vector(1,1,0);

    b.current = point(6,6,5);
    b.direction = vector(-1,-1,0);

    points.push_back(a);
    points.push_back(b);
    
    for(auto &it: caches.values)
    {
        int value = std::get<0>(it);
                
        position temp(value);

        temp.current = std::get<1>(it);        
        temp.direction = vector(0,0,0);        
        temp.index = -1;

        if(!(temp.current == starting))
            stationary.push_back(temp);
    }
    
    int counter = 0;

    do
    {
        // ***
        // check for new values to insert
        // ***

        if(values.size() > 0)
        {
            if(insert.get())
            {
                int value = values.front();
                values.erase(values.begin());

                position temp(value);
                temp.current = starting;
                temp.time = counter;
                temp.index = 0;
                temp.direction = movement.directions[0];

                std::cout << "\r\nattempt insert! " << temp.value << "\r\n";

                // NEED TO ALSO CHECK WITH STATIONARY POINTS
                // unless, generate stationary points
                // WITHOUT STARTING POINT 
                if(find_if(points.begin(), points.end(),[starting] (const position &p) { return p.current.x == starting.x && p.current.y == starting.y && p.current.z == starting.z; })==points.end())
                {
                    std::cout << "insert! [" << temp.value << "]\r\n";
                    points.push_back(temp);
                }                                
                else
                {
                    std::cout << "failed insert!\r\n";
                }                
            }
        }

        // ***
        // create lookup tables for each dimension
        // ***

        std::vector<position> working;
        std::copy(points.begin(), points.end(), std::back_inserter(working));
        std::copy(stationary.begin(), stationary.end(), std::back_inserter(working));

        std::unordered_map<int, std::vector<position*>> lens;

        std::cout << "loop " << counter << "\r\n";
        for(auto &it: working)
        {
            std::cout << "(" << it.current.x << "," << it.current.y << "," << it.current.z << ")\r\n";

            point a = it.current + it.direction;

            float x = (it.direction.x / 2.0f) + (float)it.current.x;
            float y = (it.direction.y / 2.0f) + (float)it.current.y;
            float z = (it.direction.z / 2.0f) + (float)it.current.z;

            point b((int)x,(int)y,(int)z);

            int d = _distance(a, starting);
            if(lens.find(d) == lens.end()) lens[d] = { };
            
            lens[d].push_back(&it);

            int d2 = _distance(b, starting);
            if(d2 != d) 
            {
                if(lens.find(d2) == lens.end()) lens[d2] = { };
                lens[d2].push_back(&it);
            }
            //lens[d2].push_back(&it);
        }

        for(auto &it: lens)
        {
            if(it.second.size() > 0)
            {
                // ***
                std::vector<std::vector<position*>> relative;
                position *front = it.second.front();
                relative.resize(30);
                relative[0].push_back(front);
                point t1 = front->current + front->direction;
                for(int i = 1; i < it.second.size(); ++i)
                {
                    position *src = it.second[i];
                    point t2 = src->current + src->direction;
                    int d2 = _distance(t1, t2);
                    relative[d2].push_back(it.second[i]);
                }
                int idx = 0;
                for(auto &jt:relative)
                {
                    if(jt.size() > 0)
                    {                        
                        std::cout << "relative: " << idx << " ";
                        for(auto &moo:jt)
                        {
                            std::cout << "(" << moo->current.x << "," << moo->current.y << "," << moo->current.z << ")";
                        }
                        std::cout << "\r\n";
                    }
                    ++idx;
                }
                // ***
                for(auto &a: it.second)
                {
                    for(auto &b: it.second)
                    {
                        if(a != b)
                        {
                            point t1 = a->current + a->direction;
                            if(t1 == b->current)
                            {
                                a->collisions.push_back(a->direction);
                                b->output = true;
                            }
                            point t2 = b->current + b->direction;
                            if(t2 == a->current)
                            {
                                b->collisions.push_back(b->direction);
                                a->output = true;
                            }
                        }
                    }
                }

                std::cout << "distance: " << it.first << " ";
                for(auto &jt: it.second)
                {
                std::cout << "(" << jt->current.x << "," << jt->current.y << "," << jt->current.z << ")";
                }
                std::cout << "\r\n";
            }
        }
        // ***
        // remove points out-of-bounds and compute collision directions
        // ***

        std::vector<position> temp;
        for(auto it: working)
        {
            if(it.output) results.push_back(it.value);

            if(it.current.inside(_width,_height,_depth))
            {                    
                if(it.index >= 0)
                {
                    position output;

                    output.current = it.current;
                    output.value = it.value;
                    output.time = it.time;
                    output.index = it.index;
                    output.direction = it.direction;

                    if(it.collisions.size() == 0)
                    {
                        output.current = output.current + output.direction;
                        output.index = movement.next(output.index);                
                        output.direction = movement.directions[output.index];
                    }
                    else
                    {
                        vector temp;
                        for(auto &jt:it.collisions)
                        {
                            temp = temp + jt;
                        }

                        temp = temp / it.collisions.size();

                        int encoded = temp.encode();
                        int rebounded = collisions.values[encoded];
                        vector direction;
                        direction.decode(rebounded);
                        

                        std::cout << " COL " << encoded << "," << rebounded << " ";
                        std::cout << "(" << temp.x << "," << temp.y << "," << temp.z << ") -> ";
                        std::cout << "(" << direction.x << "," << direction.y << "," << direction.z << ")\r\n";
                        output.direction = direction;          
                    }
                    
                    temp.push_back(output);
                }                
            }
        }
        points = temp;
        
    }while(counter++<max);

    return source.get(results);
}

// *********************

std::string organisation::program::run4(std::string input, data &source, int max, history *destination)
{
    auto offset = [this](point &src)
    {
        return ((this->_width * this->_height) * src.z) + ((src.y * this->_width) + src.x);
    };
    
    std::vector<int> values = source.get(input);
    std::vector<int> results;

    point starting;

    int half_width = (_width / 2);
    int half_height = (_height / 2);
    int half_depth = (_depth / 2);

    starting.x = half_width;
    starting.y = half_height;
    starting.z = half_depth;

    std::vector<position> points;    
    std::vector<position> stationary;

    points.reserve(255);

    position a, b;

    a.current = point(5,5,5);
    a.direction = vector(1,1,0);

    b.current = point(6,6,5);
    b.direction = vector(-1,-1,0);

    points.push_back(a);
    points.push_back(b);
    
    for(auto &it: caches.values)
    {
        int value = std::get<0>(it);
                
        position temp(value);

        temp.current = std::get<1>(it);        
        temp.direction = vector(0,0,0);        
        temp.index = -1;

        if(!(temp.current == starting))
            stationary.push_back(temp);
    }
    
    int counter = 0;

    do
    {
        // ***
        // check for new values to insert
        // ***

        if(values.size() > 0)
        {
            if(insert.get())
            {
                int value = values.front();
                values.erase(values.begin());

                position temp(value);
                temp.current = starting;
                temp.time = counter;
                temp.index = 0;
                temp.direction = movement.directions[0];

                std::cout << "\r\nattempt insert! " << temp.value << "\r\n";

                // NEED TO ALSO CHECK WITH STATIONARY POINTS
                // unless, generate stationary points
                // WITHOUT STARTING POINT 
                if(find_if(points.begin(), points.end(),[starting] (const position &p) { return p.current.x == starting.x && p.current.y == starting.y && p.current.z == starting.z; })==points.end())
                {
                    std::cout << "insert! [" << temp.value << "]\r\n";
                    points.push_back(temp);
                }                                
                else
                {
                    std::cout << "failed insert!\r\n";
                }                
            }
        }

        // ***
        // create lookup tables for each dimension
        // ***

        std::vector<position> working;
        std::copy(points.begin(), points.end(), std::back_inserter(working));
        std::copy(stationary.begin(), stationary.end(), std::back_inserter(working));

std::unordered_map<int, std::vector<position*>> lens;

    std::cout << "loop " << counter << "\r\n";
    for(auto &it: working)
    {
        std::cout << "(" << it.current.x << "," << it.current.y << "," << it.current.z << ")\r\n";

        point a = it.current + it.direction;

        float x = (a.x - starting.x) * (a.x - starting.x);
        float y = (a.y - starting.y) * (a.y - starting.y);
        float z = (a.z - starting.z) * (a.z - starting.z);

        int d = (int)sqrtf(x + y + z);
        
        if(lens.find(d) == lens.end()) lens[d] = { };
        lens[d].push_back(&it);
    }

    for(auto &it: lens)
    {
        std::cout << "distance: " << it.first << " ";
        for(auto &jt: it.second)
        {
        std::cout << "(" << jt->current.x << "," << jt->current.y << "," << jt->current.z << ")";
        }
        std::cout << "\r\n";
    }
                       
        // ***
        // x-dimension collision detection
        // ***

        std::unordered_map<int, std::vector<position*>> x_lookup;
        std::sort(working.begin(), working.end(), [](const position &a, const position &b)
        {
            return a.current.x < b.current.x;
        });

        for(auto &it: working)
        {
            int index = (it.current.z * _width) + it.current.y;
            if(x_lookup.find(index) == x_lookup.end())
                x_lookup[index] = { };
            
            x_lookup[index].push_back(&it);
        }
        
        for(auto &it:x_lookup)
        {    
            const int length = it.second.size();        
            int reverse = length - 1;
            int forward = 0;

            for(int i = 0; i < length; ++i)
            {
                position *&fwd = it.second[forward];
                position *&rev = it.second[reverse];

                if(fwd->direction.x < 0)
                {
                    if(forward > 0)
                    {
                        position *&previous_fwd = it.second[forward - 1];
                        if(previous_fwd->current.x == fwd->current.x + fwd->direction.x)
                        {
                            std::cout << "collide fwd X " << previous_fwd->value << "\r\n"; 

                            fwd->collisions.push_back(vector(fwd->direction.x,0,0));
                            previous_fwd->output = true;
                        }
                        /*
                        position *&previous_fwd = it.second[forward - 1];
                        if(previous_fwd->current.x != fwd->current.x + fwd->direction.x)
                            fwd->current.x += fwd->direction.x;
                        else
                        {   
                        std::cout << "collide fwd X " << previous_fwd->value << "\r\n"; 
                            fwd->collisions.push_back(vector(fwd->direction.x,0,0));
                            previous_fwd->output = true;
                        }
                        */
                    }
                    //else fwd->current.x += fwd->direction.x;
                }

                if(rev->direction.x > 0)
                {
                    if(reverse < length - 1)
                    {
                        position *&previous_rev = it.second[reverse + 1];
                        if(previous_rev->current.x == rev->current.x + rev->direction.x)
                        {
                            std::cout << "collide rev X [" << previous_rev->value << "] ";
                        std::cout << " (" << previous_rev->current.x << "," << previous_rev->current.y << "," << previous_rev->current.z << ")";
                        std::cout << "-> (" << rev->current.x << "," << rev->current.y << "," << rev->current.z << ") ";
                        std::cout << " dir(" << rev->direction.x << "," << rev->direction.y << "," << rev->direction.z << ")\r\n";

                            rev->collisions.push_back(vector(rev->direction.x,0,0));
                            previous_rev->output = true;
                        }
                        /*
                        position *&previous_rev = it.second[reverse + 1];
                        if(previous_rev->current.x != rev->current.x + rev->direction.x)
                            rev->current.x += rev->direction.x;
                        else
                        {      
                        std::cout << "collide rev X [" << previous_rev->value << "] ";
                        std::cout << " (" << previous_rev->current.x << "," << previous_rev->current.y << "," << previous_rev->current.z << ")";
                        std::cout << "-> (" << rev->current.x << "," << rev->current.y << "," << rev->current.z << ") ";
                        std::cout << " dir(" << rev->direction.x << "," << rev->direction.y << "," << rev->direction.z << ")\r\n";
                        
                            rev->collisions.push_back(vector(rev->direction.x,0,0));
                            previous_rev->output = true;
                        }
                        */
                    }
                    //else rev->current.x += rev->direction.x;
                }

                ++forward;
                --reverse;
            }
        }

        // ***
        // y-dimension collision detection
        // ***

        std::unordered_map<int, std::vector<position*>> y_lookup;
        std::sort(working.begin(), working.end(), [](const position &a, const position &b)
        {
            return a.current.y < b.current.y;
        });

        for(auto &it: working)
        {
            int index = (it.current.z * _width) + it.current.x;
            if(y_lookup.find(index) == y_lookup.end())
                y_lookup[index] = { };
            
            y_lookup[index].push_back(&it);
        }

        for(auto &it:y_lookup)
        {           
            const int length = it.second.size();
            int reverse = length - 1;
            int forward = 0;

            for(int i = 0; i < length; ++i)
            {
                position *&fwd = it.second[forward];
                position *&rev = it.second[reverse];
                
                if(fwd->direction.y < 0)
                {                
                    if(forward > 0)
                    {
                        position *&previous_fwd = it.second[forward - 1];
                        if(previous_fwd->current.y == fwd->current.y + fwd->direction.y)
                        {
                            std::cout << "collide fwd Y " << previous_fwd->value << "\r\n";                

                            fwd->collisions.push_back(vector(0,fwd->direction.y,0));
                            previous_fwd->output = true;
                        }
                        /*
                        position *&previous_fwd = it.second[forward - 1];
                        if(previous_fwd->current.y != fwd->current.y + fwd->direction.y)
                            fwd->current.y += fwd->direction.y;
                        else
                        {             
                        std::cout << "collide fwd Y " << previous_fwd->value << "\r\n";                
                            fwd->collisions.push_back(vector(0,fwd->direction.y,0));
                            previous_fwd->output = true;
                        }
                        */
                    }
                    //else fwd->current.y += fwd->direction.y;
                }

                if(rev->direction.y > 0)
                {
                    if(reverse < length - 1)
                    {
                        position *&previous_rev = it.second[reverse + 1];
                        if(previous_rev->current.y == rev->current.y + rev->direction.y)
                        {
                            std::cout << "collide rev Y " << previous_rev->value << "\r\n"; 

                            rev->collisions.push_back(vector(0,rev->direction.y,0));
                            previous_rev->output = true;
                        }
                        /*
                        position *&previous_rev = it.second[reverse + 1];
                        if(previous_rev->current.y != rev->current.y + rev->direction.y)
                            rev->current.y += rev->direction.y;
                        else
                        {                           
                        std::cout << "collide rev Y " << previous_rev->value << "\r\n"; 
                            rev->collisions.push_back(vector(0,rev->direction.y,0));
                            previous_rev->output = true;
                        }
                        */
                    }
                    //else rev->current.y += rev->direction.y;
                }

                ++forward;
                --reverse;
            }
        }

        // ***
        // z-dimension collision detection
        // ***

        std::unordered_map<int, std::vector<position*>> z_lookup;
        std::sort(working.begin(), working.end(), [](const position &a, const position &b)
        {
            return a.current.z < b.current.z;
        });

        for(auto &it: working)
        {
            int index = (it.current.y * _width) + it.current.x;
            if(z_lookup.find(index) == z_lookup.end())
                z_lookup[index] = { };
            
            z_lookup[index].push_back(&it);
        }

        for(auto &it:z_lookup)
        {            
            const int length = it.second.size();
            int reverse = length - 1;
            int forward = 0;

            for(int i = 0; i < length; ++i)
            {
                position *&fwd = it.second[forward];
                position *&rev = it.second[reverse];

                if(fwd->direction.z < 0)
                {
                    if(forward > 0)
                    {
                        position *&previous_fwd = it.second[forward - 1];
                        if(previous_fwd->current.z == fwd->current.z + fwd->direction.z)
                        {
                            std::cout << "collide fwd Z " << previous_fwd->value << "\r\n";

                            fwd->collisions.push_back(vector(0,0,fwd->direction.z));
                            previous_fwd->output = true;
                        }
                        /*
                        position *&previous_fwd = it.second[forward - 1];
                        if(previous_fwd->current.z != fwd->current.z + fwd->direction.z)
                            fwd->current.z += fwd->direction.z;
                        else
                        {
                        std::cout << "collide fwd Z " << previous_fwd->value << "\r\n"; 
                            fwd->collisions.push_back(vector(0,0,fwd->direction.z));
                            previous_fwd->output = true;
                        } 
                        */                       
                    }
                    //else fwd->current.z += fwd->direction.z;
                }

                if(rev->direction.z > 0)
                {
                    if(reverse < length - 1)
                    {
                        position *&previous_rev = it.second[reverse + 1];
                        if(previous_rev->current.z == rev->current.z + rev->direction.z)
                        {
                            std::cout << "collide rev Z " << previous_rev->value << "\r\n"; 

                            rev->collisions.push_back(vector(0,0,rev->direction.z));
                            previous_rev->output = true;
                        }
                        /*
                        position *&previous_rev = it.second[reverse + 1];
                        if(previous_rev->current.z != rev->current.z + rev->direction.z)
                            rev->current.z += rev->direction.z;
                        else
                        {
                        std::cout << "collide rev Z " << previous_rev->value << "\r\n"; 
                            rev->collisions.push_back(vector(0,0,rev->direction.z));
                            previous_rev->output = true;
                        }
                        */
                    }
                    //else rev->current.z += rev->direction.z;
                }

                ++forward;
                --reverse;
            }
        }

        // ***
        // remove points out-of-bounds and compute collision directions
        // ***

        std::vector<position> temp;
        for(auto it: working)
        {
            if(it.output) results.push_back(it.value);

            if(it.current.inside(_width,_height,_depth))
            {                    
                if(it.index >= 0)
                {
                    position output;

                    output.current = it.current;
                    output.value = it.value;
                    output.time = it.time;
                    output.index = it.index;
                    output.direction = it.direction;

                    if(it.collisions.size() == 0)
                    {
                        output.current = output.current + output.direction;
                        output.index = movement.next(output.index);                
                        output.direction = movement.directions[output.index];
                    }
                    else
                    {
                        vector temp;
                        for(auto &jt:it.collisions)
                        {
                            temp = temp + jt;
                        }

                        temp = temp / it.collisions.size();

                        int encoded = temp.encode();
                        int rebounded = collisions.values[encoded];
                        vector direction;
                        direction.decode(rebounded);
                        

                        std::cout << " COL " << encoded << "," << rebounded << " ";
                        std::cout << "(" << temp.x << "," << temp.y << "," << temp.z << ") -> ";
                        std::cout << "(" << direction.x << "," << direction.y << "," << direction.z << ")\r\n";
                        output.direction = direction;          
                    }
                    
                    temp.push_back(output);
                }                
            }
        }
        points = temp;
        
    }while(counter++<max);

    return source.get(results);
}

// OUTPUT ON COLLIDE WITH STATIONARY DATA ??
// NEED TO CHECK BOUNDS OF CONTAINING CUBE

// direction by data and/or lifetime

std::string organisation::program::run3(std::string input, data &source, history *destination)
{
    auto offset = [this](point &src)
    {
        return ((this->_width * this->_height) * src.z) + ((src.y * this->_width) + src.x);
    };
    
    std::vector<int> values = source.get(input);

    point starting;

    int half_width = (_width / 2);
    int half_height = (_height / 2);
    int half_depth = (_depth / 2);

    starting.x = half_width;
    starting.y = half_height;
    starting.z = half_depth;

    std::vector<position> points;
    std::vector<position> stationary;

    points.reserve(255);

    const int MAX = 30;
    int counter = 0;

    const int length = _width * _height * _depth;
    position *lookup[length];

    for(int i = 0; i < length; ++i)
    {
        lookup[i] = NULL;
    }

    for(auto &it: caches.values)
    {
        int value = std::get<0>(it);
                
        position temp(value);

        temp.current = std::get<1>(it);        
        temp.direction = vector(0,0,0);        

        stationary.push_back(temp);
    }

    insert.start();

    do
    {   
        std::cout << "\r\n\r\nloop " << counter << "\r\n";
        // ***
        // clear lookup table
        // ***

        for(int i = 0; i < length; ++i)
        {
            lookup[i] = NULL;
        }

        // ***
        // populate lookup table
        // ***
        
        for(auto &it: stationary)
        {            
            int index1 = offset(it.current);
            if(lookup[index1] == NULL)            
                lookup[index1] = &it;
        }

        for(auto &it: points)  
        {
            if(it.current.inside(_width,_height,_depth))
            {
                int index1 = offset(it.current);
                if(lookup[index1] == NULL)            
                    lookup[index1] = &it;
            }
        }        

        // special case for x_width - 1, if moving off grid, set to NULL
        for(int z = 0; z < _depth; ++z)
        {     
            // edge cases for x _width bounds

            for(int y = 0; y <_height; ++y)
            {
                point p1(_width - 1,y,z);
                int index1 = offset(p1);
                if(lookup[index1] != NULL)
                {
                    if(lookup[index1]->direction.x > 0)
                    {
                        lookup[index1] = NULL;
                    }
                }

                // ***

                point p2(0,y,z);
                int index2 = offset(p2);
                if(lookup[index2] != NULL)
                {
                    if(lookup[index2]->direction.x < 0)
                    {
                        lookup[index2] = NULL;
                    }
                }
            }


            int x_reverse = _width - 2;
            int x_forward = 1;

            for(int x = _width - 2; x >= 0; x--)
            {
                for(int y = 0; y < _height; ++y)
                {
                    point p1(x_reverse,y,z), p2(x_reverse + 1,y,z);
                    int index1 = offset(p1);
                    int index2 = offset(p2);

                    if(lookup[index1] != NULL)
                    {
                        if(lookup[index1]->direction.x > 0)
                        {
                            if(lookup[index2] == NULL)
                            {
                                position *temp = lookup[index1];                    
                                temp->current.x += temp->direction.x;
                            }
                            else
                            {
                                position *temp = lookup[index1];
                                //temp->collision.x += temp->direction.x;
                                // collision occured
                                // for the point, need to push_back this collision
                                // direction, and sum, for computation of next direction
                            }
                        }                    
                    }

                    // ****
                    point p3(x_forward,y,z), p4(x_forward - 1,y,z);
                    int index3 = offset(p3);
                    int index4 = offset(p4);
                    if(lookup[index3] != NULL)
                    {
                        if(lookup[index3]->direction.x < 0)
                        {
                            if(lookup[index4] == NULL)
                            {
                                position *temp = lookup[index3];                    
                                temp->current.x += temp->direction.x;
                            }
                            else
                            {
                                // record collision
                                position *temp = lookup[index3];
                                //temp->collision.x += temp->direction.x;
                            }
                        }
                    }
                }
                x_reverse--;
                x_forward++;
            }

            // ***
            // process y 

            // y edge cases

            for(int x = 0; x <_width; ++x)
            {
                point p1(x,_height - 1,z);
                int index1 = offset(p1);
                if(lookup[index1] != NULL)
                {
                    if(lookup[index1]->direction.y > 0)
                    {
                        lookup[index1] = NULL;
                    }
                }

                // ***

                point p2(x,0,z);
                int index2 = offset(p2);
                if(lookup[index2] != NULL)
                {
                    if(lookup[index2]->direction.y < 0)
                    {
                        lookup[index2] = NULL;
                    }
                }
            }


            int y_reverse = _height - 2;
            int y_forward = 1;

            for(int y = _height - 2; y >= 0; y--)
            {
                for(int x = 0; x < _width; x++)
                {
                    point p1(x,y_reverse,z), p2(x,y_reverse + 1,z);
                    int index1 = offset(p1);
                    int index2 = offset(p2);

                    if(lookup[index1] != NULL)
                    {
                        if(lookup[index1]->direction.y > 0)
                        {
                            if(lookup[index2] == NULL)
                            {
                                position *temp = lookup[index1];                    
                                temp->current.y += temp->direction.y;
                            }
                            else
                            {
                                position *temp = lookup[index1];
                                //temp->collision.y += temp->direction.y;
                                // collision occured
                                // for the point, need to push_back this collision
                                // direction, and sum, for computation of next direction
                            }
                        }                    
                    }

                    // ****
                    point p3(x,y_forward,z), p4(x,y_forward - 1,z);
                    int index3 = offset(p3);
                    int index4 = offset(p4);
                    if(lookup[index3] != NULL)
                    {
                        if(lookup[index3]->direction.y < 0)
                        {
                            if(lookup[index4] == NULL)
                            {
                                position *temp = lookup[index3];                    
                                temp->current.y += temp->direction.y;
                            }
                            else
                            {
                                // record collision
                                position *temp = lookup[index3];
                                //temp->collision.y += temp->direction.y;
                            }
                        }
                    }
                }
                y_reverse--;
                y_forward++;
            }

        }

        // WORK OUT FOR Z DIMENSIONS

        // ADD ANY NEW POINTS INTO LOOKUP TABLE

        if(values.size() > 0)
        {
            if(insert.get())
            {
                int value = values.front();
                values.erase(values.begin());

for(auto &jt: points)
{
    std::cout << "(" << jt.current.x << "," << jt.current.y << "," << jt.current.z << ")";
}
                position temp(value);
                temp.current = starting;
                temp.time = counter;
                temp.index = 0;
                temp.direction = movement.directions[0];

                int index1 = offset(temp.current);
                std::cout << "\r\nattempt insert! " << temp.value << "\r\n";
                if(lookup[index1] == NULL)
                {
                    std::cout << "insert! [" << temp.value << "]\r\n";
                    points.push_back(temp);
                    lookup[index1] = &points.back();
                }
                else
                {
                    std::cout << "failed insert (" << lookup[index1]->current.x << "," << lookup[index1]->current.y << "," << lookup[index1]->current.z << " value:" << lookup[index1]->value << ")\r\n";
                }
            }
        }

        // RECOLLATE LOOKUP bACK INTO POINTS VECTOR,
        // ALSO COMPUTE NEW DIRECTIONS FOR COLLISIONS

        std::vector<position> working;

        for(int i = 0; i < length; ++i)
        {
            if(lookup[i] != NULL)
            {
                position temp(*lookup[i]);

/*
                if((temp.collision.x == 0)&&(temp.collision.y == 0)&&(temp.collision.z == 0))
                {
                    temp.index = movement.next(temp.index);                
                    temp.direction = movement.directions[temp.index];
                }
                else
                {
                    int encoded = temp.collision.encode();
                    int rebounded = collisions.values[encoded];
                    vector direction;
                    direction.decode(rebounded);
                    temp.direction = direction;          
                }
*/
                working.push_back(temp);
            }
        }

        points = working;

    }while(counter++<MAX);
    return std::string("");

}

std::string organisation::program::run2(std::string input, data &source, history *destination)
{
    auto intersect = [](point &a, point &b, point &c, point &d)
    {
      return 
        ((a.x <= d.x && b.x >= c.x)
        &&
        (a.y <= d.y && b.y >= c.y)
        &&
        (a.z <= d.z && b.z >= c.z));
    };

    point starting;

    int half_width = (_width / 2);
    int half_height = (_height / 2);
    int half_depth = (_depth / 2);

    starting.x = half_width;
    starting.y = half_height;
    starting.z = half_depth;

    std::vector<int> values = source.get(input);

    std::vector<position> points;
    points.reserve(255);

    const int MAX = 20;
    int counter = 0;

    insert.start();

    do
    {
std::cout << "loop\r\n\r\n";
        // ***
        // remove points that extend beyond bondary
        // ***

        std::vector<position> temp;
        for(auto it: points)
        {
            if(it.current.inside(_width,_height,_depth))
            {
                temp.push_back(it);
            }
        }
        points = temp;

        // ***
        // add existing to collision lookup
        // ***

        std::unordered_map<int, std::vector<position*>> lookup;
        for(auto &it: points)
        //for(int i = 0; i < points.size(); ++i)
        {
            //position it = points[i];
            vector direction = it.direction;

            point a = it.current;
            point b = a + direction;
            point min = a.min(b) / 2;

            int index = ((half_width * half_height) * min.z) + ((min.y * half_width) + min.x);

std::cout << "LOOKUP ins value:" << it.value << " " << a.x << "," << a.y << "," << a.z << " index:" << index << "\r\n";
std::cout << "min " << min.x << "," << min.y << "," << min.z << "\r\n";

            if(lookup.find(index) == lookup.end()) 
                lookup[index] = { };
                //lookup[index] = { &points.at(i) };
            //else
                lookup[index].push_back(&it);//(points.at(i));
        }

        // ***
        // add new points
        // ***

        if(values.size() > 0)
        {
            if(insert.get())
            {
                int value = values.front();
                values.erase(values.begin());

                position temp(value);
                temp.current = starting;
                temp.time = counter;
                temp.index = 0;
                temp.direction = movement.directions[0];

                point b = starting + temp.direction;
                point min = starting.min(b);

                int index = ((half_width * half_height) * min.z) + ((min.y * half_width) + min.x);

                std::cout << "ins value:" << temp.value << " index:" << index << "\r\n";

                if(lookup.find(index) == lookup.end())
                {
                    points.push_back(temp);
                    lookup[index] = { };
                    lookup[index].push_back(&points.back());
                    //lookup[index] = { &points.back() };                    
                } 
                else
                {
                    // todo
                    // loop through each in lookup vector
                    // and see if a true collision!!!
                }          
            }
        }

        // ***
        // update positions & directions
        // ***

        for(auto &it: lookup)
        {
            if(it.second.size() <= 1)
            {        
        std::cout << "INDEX: " << it.first << "\r\n";
                for(auto &jt: it.second)
                {
                    jt->current = jt->current + jt->direction;
                    jt->index = movement.next(jt->index);                
                    jt->direction = movement.directions[jt->index];

        std::cout << "update value:" << jt->value << " pos:(" << jt->current.x << "," << jt->current.y << "," << jt->current.z << ")\r\n";
        std::cout << "update dir_idx:" << jt->index << " dir:(" << jt->direction.x << "," << jt->direction.y << "," << jt->direction.z << ")\r\n";
                }
            }
            else
            {
                int length = it.second.size();
                for(int i = 0; i < length; ++i)
                {
                    int collided = 0;
                    vector overall(0,0,0);    

                    point a = it.second[i]->current;
                    point b = a + it.second[i]->direction;

                    for(int j = 0; j < length; ++j)
                    {
                        if(i != j)
                        {
                            vector v = it.second[j]->direction;
                            point c = it.second[j]->current;
                            point d = c + v;

                            if(intersect(a,b,c,d))
                            {
                                overall.x += v.x;
                                overall.y += v.y;
                                overall.z += v.z;

                                collided++;
                            }
                        }
                    }

                    if(collided > 0)
                    {
                        overall = overall / collided;

                        int encoded = overall.encode();

                        int rebounded = collisions.values[encoded];
                        vector direction;
                        direction.decode(rebounded);

                        it.second[i]->direction = direction;
                    }
                }
            }
        }

    }while(counter++<MAX);

    return std::string("");
}

std::string organisation::program::run(std::string input, data &source, history *destination)
{
// have too unordered_maps lookups by x,yz, index
// one for cache (already exists)
// one for current loop positions
    point starting;

    starting.x = (_width / 2);
    starting.y = (_height / 2);
    starting.z = (_depth / 2);

// ***
// COPY CACHES DATA into lookup
// ***

    std::vector<position> points;
    //std::unordered_map<int, position> lookup;    
    //int length = _width * _height * _depth;
    //position volume[length];

// COLLISOIN DETECTION
// BASED ON INTERSECTING LINES

// A-LINE;
// start = a.position
// end = a.next
// B-line;
// start = b.position
// end = b.next

    std::vector<int> values = source.get(input);

    const int MAX = 20;
    int counter = 0;

    do
    {
        // ***
        // populate lookup with points
        // ***

        // ***
        // insert new points
        // ***
        //lookup.clear();
        std::unordered_map<int, position> lookup;
        for(auto &it: points)
        {
            int index = ((_width * _height) * it.current.z) + ((it.current.y * _width) + it.current.x);
            lookup[index] = it;
        }
        /*
        for(auto &it: caches.points)
        {
            int index = ((_width * _height) * it.second.z) + ((it.second.y * _width) + it.second.x);
            
            position temp;
            temp.current = it.second;
            lookup[index] = temp;
        }
        */

        // ****
        // add new point
        // ****

        if(values.size() > 0)
        {
            int value = values.front();
            values.erase(values.begin());

            if(insert.get())
            {
                int index = ((_width * _height) * starting.z) + ((starting.y * _width) + starting.x);
                if((lookup.find(index) == lookup.end())
                    &&(caches.points.find(index) == caches.points.end()))
                {
                    position temp(value);
                    temp.current = starting;
                    temp.time = counter;
                    
                    points.push_back(temp);
                    lookup[index] = temp;
                }           
                // check starting point isn't occupied
            }
        }

        // ***
        // collision detection
        // ***

// NEED TO COPY CACHE INTO NEXT
        std::unordered_map<int, std::vector<int>> next;

        for(auto &it: caches.points)
        {
            int index = ((_width * _height) * it.second.z) + ((it.second.y * _width) + it.second.x);
            
            //position temp;
            //temp.current = it.second;
            if(next.find(index) == next.end())
                next[index] = { };
            
            next[index].push_back(-1);
        }

        int i = 0;
        for(auto &it: points)
        {
            //it.index = movement.next(it.index);                
            //it.direction = movement.directions[it.index];

            // update direction in FIRAT PART OF NLOOP
            /* 
            it.next.x = it.current.x + it.direction.x;
            it.next.y = it.current.y + it.direction.y;
            it.next.z = it.current.z + it.direction.z;

            int index = ((_width * _height) * it.next.z) + ((it.next.y * _width) + it.next.x);
            if(next.find(index) == next.end())
            {
                next[index] = { };
                //next[index].push_back(i);
            }
            //else
            //{
            next[index].push_back(i);        
            //}
            */


            ++i;
        }

        // ***
        // next position update
        // ***


        for(auto &it: next)
        {
            
            if(it.second.size() == 1)
            {
                int index = it.second.front();
                if(index >= 0)
                {
                    position &point = points[index];

                    // ***
                    // *** when does the current point get updated??
                    // ***
                    // ONLY UPDATE WHEN NO COLLISIONS FROM NEW DIRECTION
                    //point.current = point.next;

                    point.index = movement.next(point.index);                
                    point.direction = movement.directions[point.index];
                }

                //it.index = movement.next(it.index);                
            //it.direction = movement.directions[it.index];
                //int index = ((_width * _height) * point.current.z) + ((point.current.y * _width) + point.current.x);
                // update lookup
            }
            else if(it.second.size() > 1)
            {
                // compute next direction
                
                // do I still increment point.index here????
                // or just reset to 0 after collision?

                // WRONG LOOP
                for(int i = 0; i < it.second.size(); ++i)
                {
                    int idx1 = it.second[i];
                    if(idx1 >= 0)
                    {
                        position &point = points[idx1];
                        vector temp;
                        for(int j = 0; j < it.second.size(); ++j)
                        {
                            int idx2 = it.second[j];
                            //if(i != j)
                            if(idx1 != idx2)
                            {
                                temp.x += points[idx2].direction.x;
                                temp.y += points[idx2].direction.y;
                                temp.z += points[idx2].direction.z;
                            }
                        }

                        temp.x /= (it.second.size() - 1);
                        temp.y /= (it.second.size() - 1);
                        temp.z /= (it.second.size() - 1);

                        int encoded = temp.encode();
                        vector direction;
                        direction.decode(encoded);

                        point.direction = direction;
                    }
                    //
                    // COLLISIONS ALSO NEED TO OUTPUT!!!
                    // NEED AGE FOR EACH VALUE ADDED TO THE SYSTEM
                    // add time index to each added cell
                    //
                }
                
                // for each point, compare with all other points in 
                // collisions
                // compute next direction
            }
        }

    } while(counter++<MAX);

/*
    std::vector<int> result;
    std::vector<position> stack;
    
    const int MAX = 20;


    int counter = 0;
    while ((!(stack.empty()))&&(counter < MAX))
    {   
        position temp = stack.back();     

        ++counter;
    }
*/
    return std::string("");
    /*
    std::vector<int> result;

    int x = (_width / 2) - start;
    int y = (_height / 2);
    int z = (_depth / 2);
    
    const int MAX = 20;

    std::vector<std::tuple<vector,vector>> positions;
    positions.push_back(std::tuple<vector,vector> (vector { x,y,z },vector {0,-1,0}));

    int counter = 0;
    while ((!(positions.empty()))&&(counter < MAX))
    {        
        std::tuple<vector,vector> temp = positions.back();

        vector current = std::get<0>(temp);
        vector next = std::get<1>(temp);

        positions.pop_back();

        int index = (current.z * _width * _height) + (current.y * _width) + current.x;
        
        vector input = next.normalise().inverse();

        if(cells[index].is_input(input))
        {
            if(destination != NULL) destination->push(current, cells[index].value);

            if(!cells[index].is_empty())
            {                
                result.push_back(cells[index].value);
            }
            
            std::vector<vector> outputs = cells[index].outputs(input);
            for(std::vector<vector>::iterator ij = outputs.begin(); ij != outputs.end(); ++ij)
            {
                vector t = *ij;
                vector position = { current.x + t.x, current.y + t.y, current.z + t.z };

                if((position.x >= 0)&&(position.y >= 0)&&(position.z >= 0))
                {
                    if((position.x < _width)&&(position.y < _height)&&(position.z < _depth))
                    {
                        positions.push_back(std::tuple<vector,vector>(position, t));
                    }
                }
            }
        }

        ++counter;
    };
    
    return source.get(result);
    */
}

int organisation::program::count()
{
    int result = 0;

/*
    for(int i = 0; i < length; ++i)
    {
        if(cells[i].value > -1) ++result;
    }
*/
    return result;
}

bool organisation::program::validate(data &source)
{
    /*
    if(count() <= 0) 
        return false;

    int gates = 0;
    for(int i = 0; i < length; ++i)
    {
        std::tuple<bool,bool> temp = cells[i].validate(source.maximum());
        if(!std::get<0>(temp)) return false;
        if(std::get<1>(temp)) ++gates;
    }

    if(gates <= 0) 
        return false;
    */
    return true;
}

void organisation::program::copy(const program &source)
{    
    //cells.assign(source.cells.begin(), source.cells.end());
    _width = source._width;
    _height = source._height;
    _depth = source._depth;
    length = source.length;
}

bool organisation::program::equals(const program &source)
{
    /*
    for(int i = 0; i < length; ++i)
    {
        if(!cells[i].equals(source.cells[i])) return false;
    }
    */
    return true;
}

void organisation::program::cross(program &a, program &b, int middle)
{
    clear();

    templates::genetic *ag[] = 
    { 
        &a.caches,
        &a.movement,
        &a.collisions,
        &a.insert
    }; 

    templates::genetic *bg[] = 
    { 
        &b.caches,
        &b.movement,
        &b.collisions,
        &b.insert
    }; 

    templates::genetic *dest[] = 
    { 
        &caches,
        &movement,
        &collisions,
        &insert
    }; 

    const int components = sizeof(dest) / sizeof(templates::genetic*);
    
    for(int i = 0; i < components; ++i)
    {
        int length = ag[i]->size() - 1;

        int sa = 0, ea = 0;

        do
        {
            sa = (std::uniform_int_distribution<int>{ 0, length })(generator);
            ea = (std::uniform_int_distribution<int>{ 0, length })(generator);
        } while(sa == ea);
        
        if(ea < sa) 
        {
            int temp = ea;
            ea = sa;
            sa = temp;
        }

        // ***

        length = bg[i]->size() - 1;

        int sb = 0, eb = 0;

        do
        {
            sb = (std::uniform_int_distribution<int>{ 0, length })(generator);
            eb = (std::uniform_int_distribution<int>{ 0, length })(generator);
        } while(sb == eb);
        
        if(eb < sb) 
        {
            int temp = eb;
            eb = sb;
            sb = temp;
        }

        // ***

        dest[i]->copy(ag[i], 0, sa, 0); // dest idx, start = 0, len = sa        
        dest[i]->copy(bg[i], sb, eb, sa); // dest idx, start = sa, len - eb -sb
        dest[i]->copy(ag[i], eb, ag[i]->size(), (eb - sb) + sa); //(eb-sb) + sa, len, ag[i] - 
    }
    //std::vector<int> lengths1;// = { (int)cache.size(), (int)movements.size(), (int)collisions.size() };
    //std::vector<int> lengths2;
    
    /*
    lengths1.push_back(a.cache.size());    
    for(auto &it:a.movements)
    {
        lengths1.push_back(it.directions.size() + 1);
    }
    lengths1.push_back(a.collisions.size());

    lengths2.push_back(b.cache.size());    
    for(auto &it: b.movements)
    {
        lengths2.push_back(it.directions.size() + 1);
    }
    lengths2.push_back(b.collisions.size());
    */
    /*
    if((a.length != b.length)||(length != a.length)) return;

    int a1 = 0, b1 = 0;
    if(middle < 0)
    {
        while(a1 == b1)
        {
            a1 = (std::uniform_int_distribution<int>{0, (int)(length - 1)})(generator);
            b1 = (std::uniform_int_distribution<int>{0, (int)(length - 1)})(generator);
        };

        if(a1 > b1)
        {
            int temp = a1;
            a1 = b1;
            b1 = temp;
        }
    }
    else
    {
        a1 = middle;
        b1 = length;
    }

    int index = 0;
    for(int i = 0; i < a1; ++i)
    {
        cells[index].copy(a.cells[i]);
		++index;
    }

    for(int i = a1; i < b1; ++i)
    {
        cells[index].copy(b.cells[i]);
        ++index;
    }

    for(int i = b1; i < length; ++i)
    {
        cells[index].copy(a.cells[i]);
        ++index;
    } 
    */   
}

void organisation::program::save(std::string filename)
{
    /*
     std::fstream output(filename, std::fstream::out | std::fstream::binary);

    if(output.is_open())
    {
        for(int z = 0; z < _depth; ++z)
        {    
            std::string line;        
            for(int y = 0; y < _height; ++y)
            {                
                for(int x = 0; x < _width; ++x)
                {
                    int index = (z * _width * _height) + (y * _width) + x;

                    line += "P ";
                    line += std::to_string(x) + ",";
                    line += std::to_string(y) + ",";
                    line += std::to_string(z);
                    line += "\r\n";

                    line += "V " + std::to_string(cells[index].value) + "\r\n";

                    std::vector<int> in_gates = cells[index].pull();
                    for(std::vector<int>::iterator it = in_gates.begin(); it != in_gates.end(); ++it)
                    {                     
                        line += "G " + std::to_string(*it);
                        line += " ";
                        
                        std::vector<int> out_gates = cells[index].pull(*it);
                        for(std::vector<int>::iterator jt = out_gates.begin(); jt != out_gates.end(); ++jt)
                        {
                            organisation::gate gate = cells[index].get(*it,*jt);

                            line += std::to_string(*jt);
                            line += "/" + std::to_string(gate.magnitude);
                            if(jt != out_gates.end() - 1) line += ",";
                        }

                        line += "\r\n";
                    }
                    line += "\r\n";
                }              
            }

            output.write(line.c_str(), line.size());
        }

        output.close();
    }   
    */
}

void organisation::program::load(std::string filename)
{
    /*
    std::ifstream source(filename);
    if(source.is_open())
    {
        int x = 0, y = 0, z = 0;
        int *position[] = { &x, &y, &z };

        for(std::string value; getline(source, value); )
        {
            std::stringstream stream(value);
		    std::string type, word;
	            
            if(stream >> type)
            {
                if(type == "P")
                {
                    std::vector<std::string> words;
                    while(stream >> word) words.push_back(word);
                    
                    if(words[0].size() > 0)
                    {
                        std::stringstream ss(words[0]); 
                        std::string str;
                        int index = 0;
                        while (std::getline(ss, str, ',')) 
                        {
                            if(index < 3)
                            {                            
                                int temp = std::atoi(str.c_str());
                                *position[index] = temp;
                            }
                            ++index;                        
                        }
                    }
                }
                else if(type == "V")
                {
                    std::vector<std::string> words;
                    while(stream >> word) words.push_back(word);
                    if(words.size() > 0)
                    {
                        int index = (z * _width * _height) + (y * _width) + x;
                        cells[index].value = std::atoi(words[0].c_str());
                    }
                }
                else if(type == "G")
                {
                    std::vector<std::string> words;
                    while(stream >> word) words.push_back(word);
                    
                    if(words.size() >= 1)
                    {
                        int in_gate = std::atoi(words[0].c_str());
                    

                        int index = (z * _width * _height) + (y * _width) + x;
                        cells[index].set(in_gate);

                        if(words.size() >= 2)
                        {
                            std::stringstream ss(words[1]); 
                            std::string str;

                            while (std::getline(ss, str, ',')) 
                            {

                                std::stringstream ww(str);
                                std::string vv;
                                std::vector<std::string> values;
                                while(std::getline(ww, vv, '/'))
                                {
                                    values.push_back(vv);
                                };

                                if(values.size() > 1)
                                {
                                    int out_gate = std::atoi(values[0].c_str());
                                    int magnitude = std::atoi(values[1].c_str());

                                    int index = (z * _width * _height) + (y * _width) + x;
                                    cells[index].set(in_gate, out_gate, organisation::gate(magnitude)); 
                                }
                            }
                        }
                    }
                }
            }
        }

        source.close();
    }
    */
}
    
void organisation::program::makeNull()
{
    
}

void organisation::program::cleanup()
{

}