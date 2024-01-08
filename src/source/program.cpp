#include "program.h"
#include "position.h"
#include "genetic/templates/genetic.h"
#include "genetic/templates/serialiser.h"
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

std::string organisation::program::run(std::string input, data &source, int max, history *destination)
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

    /*
    position a, b;

    a.current = point(5,5,5);
    a.direction = vector(1,1,0);

    b.current = point(6,6,5);
    b.direction = vector(-1,-1,0);

    points.push_back(a);
    points.push_back(b);
    */
    
    // ***
    // load stationary positions (those that don't move per frame, direction = vector(0,0,0))
    // from the cache class
    // ***

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
        // copy stationary points, and master working point array into same array
        // ***

        std::vector<position> working;
        std::copy(points.begin(), points.end(), std::back_inserter(working));
        std::copy(stationary.begin(), stationary.end(), std::back_inserter(working));

        // ***
        // load points into collision detection lens hashmap
        // key = distance from starting point (middle of grid, defined by width,height,depth)
        // ***

        std::unordered_map<int, std::vector<position*>> lens;

        std::cout << "loop " << counter << "\r\n";
        for(auto &it: working)
        {
            std::cout << "(" << it.current.x << "," << it.current.y << "," << it.current.z << ")\r\n";

            // ***
            // hashmap key defined by current position, plus direction, any points
            // that are about to collide and overlap, will have the same distance value
            // ***

            point a = it.current + it.direction;

            // ***
            // special case, works out "half-point" for a points new direction
            // otherwise two points may cross each others diagonal, without registering
            // as a collision
            // a.point(1,1,1) a.direction(1,1,1)
            // b.point(2,2,2) a.direction(-1,-1,-1)
            // ***

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
        }

        for(auto &it: lens)
        {
            if(it.second.size() > 0)
            {
                // ***
                // begin dog water code section
                // ***

                /*
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
                */
                
                // ***
                // end dog water code section
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
        // check points are not outside bounds of grid, width, height, depth
        // if a point has not been marked as a collision with another point
        // apply direction to point, and then compute new direction
        //
        // if a point has been marked as collided, it does not move for this frame
        // iteration, and a new direction (depending on collision direction) is calculated
        // point remains stationary until it's future path is clear
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

                        // turns a vector direction, into a single encoded integer (for memory efficency!)
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
    _width = source._width;
    _height = source._height;
    _depth = source._depth;
    length = source.length;

    caches.copy(source.caches);
    movement.copy(source.movement);
    collisions.copy(source.collisions);
    insert.copy(source.insert);    
}

bool organisation::program::equals(const program &source)
{
    if(!caches.equals(source.caches)) return false;
    if(!movement.equals(source.movement)) return false;
    if(!collisions.equals(source.collisions)) return false;
    if(!insert.equals(source.insert)) return false;
    
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
    std::vector<templates::serialiser*> sources = 
    { 
        &caches,
        &movement,
        &collisions,
        &insert
    }; 

    std::fstream output(filename, std::fstream::out | std::fstream::binary);

    if(output.is_open())
    {
        for(auto &it: sources)
        {
            std::string data = it->serialise();
            output.write(data.c_str(), data.size());
        }
    }

    output.close();
}

void organisation::program::load(std::string filename)
{
    std::ifstream source(filename);
    if(source.is_open())
    {
        caches.clear();
        collisions.clear();
        movement.clear();
        insert.clear();

        for(std::string value; getline(source, value); )
        {
            std::stringstream stream(value);
		    std::string type;
	            
            if(stream >> type)
            {
                if(type == "C") caches.deserialise(value);
                else if(type == "M") movement.deserialise(value);
                else if(type == "C") collisions.deserialise(value);
                else if(type == "I") insert.deserialise(value);
            }
        }

    }

    source.close();
}
    
void organisation::program::makeNull()
{
    
}

void organisation::program::cleanup()
{

}