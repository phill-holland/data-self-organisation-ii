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

// OUTPUT ON COLLIDE WITH STATIONARY DATA ??

// direction by data and/or lifetime

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

    const int MAX = 20;
    int counter = 0;

    do
    {
        // ***
        // add existing to collision lookup
        // ***

        std::unordered_map<int, std::vector<position*>> lookup;
        for(auto &it: points)
        {

            vector direction = it.direction;

            point a = it.current;
            point b = a + direction;
            point min = a.min(b) / 2;

            int index = ((half_width * half_height) * min.z) + ((min.y * half_width) + min.x);

            if(lookup.find(index) == lookup.end()) 
                lookup[index] = { &it };
            else
                lookup[index].push_back(&it);
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
                if(lookup.find(index) == lookup.end())
                {
                    points.push_back(temp);                    
                    lookup[index] = { &points.back() };                    
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
                for(auto &jt: it.second)
                {
                    jt->current = jt->current + jt->direction;
                    jt->index = movement.next(jt->index);                
                    jt->direction = movement.directions[jt->index];
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
                            point d = a + v;

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

                // NEED TO CHECK FOR GENIUNE COLLISION
                // if collision, add to overall, falg collisoin = true
                // line intersection test, for each i,j point in it.second
                /*vector overall(0,0,0);
                for(auto &jt: it.second)
                {                       
                    overall.x += jt->direction.x;
                    overall.y += jt->direction.y;
                    overall.z += jt->direction.z;
                }

                int length = it.second.size() -1;
                for(auto &jt: it.second)
                {
                    vector temp = overall - jt->direction;
                    temp = temp / length;                    

                    int encoded = temp.encode();
                    vector direction;
                    direction.decode(encoded);

                    jt->direction = direction;
                } 
                */               
            }
        }

    }while(counter++<MAX);
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
                    point.current = point.next;

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

/*
void organisation::program::set(int value, int x, int y, int z)
{
    if ((x < 0)||(x >= _width)) return;
    if ((y < 0)||(y >= _height)) return;
    if ((z < 0)||(z >= _depth)) return;

    int index = (z * _width * _height) + (y * _width) + x;

    cells[index].value = value;
}

void organisation::program::set(vector input, vector output, int magnitude, int x, int y, int z)
{
    if ((x < 0)||(x >= _width)) return;
    if ((y < 0)||(y >= _height)) return;
    if ((z < 0)||(z >= _depth)) return;

    int index = (z * _width * _height) + (y * _width) + x;

    //cells[index].set(input, output, magnitude);
}
*/

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