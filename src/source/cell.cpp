#include "cell.h"
#include <stdlib.h>
#include <math.h>

std::mt19937_64 organisation::cell::generator(std::random_device{}());

void organisation::cell::clear()
{
    value = -1;
    //routes.clear();
}

void organisation::cell::generate(int value)
{
    clear();

    this->value = value;
    //routes.generate();
}

void organisation::cell::mutate(int max)
{
    int j = (std::uniform_int_distribution<int>{0, 2})(generator);     

    if(j == 0) value = -1;
    else if (j == 1)
    {
        value = (std::uniform_int_distribution<int>{0, max - 1})(generator);          
    }
    else if (j == 2)
    {
        #warning fix me
        //routes.mutate();        
    }
}

/*
bool organisation::cell::is_input(vector source)
{
    int tx = source.x + 1;
    int ty = source.y + 1;
    int tz = source.z + 1;

	int temp = (abs(tz) * (3 * 3)) + (abs(ty) * 3) + abs(tx);
    
    return !routes.is_empty(temp);
}

std::vector<organisation::vector> organisation::cell::outputs(vector input)
{
    int in = map(input);
    return routes.outputs(in);
}
*/
/*
void organisation::cell::set(vector input, vector output, int magnitude)
{
    int s1 = map(input);
    int s2 = map(output);

    routes.set(s1, s2, gate(magnitude));
}
*/
std::tuple<bool,bool> organisation::cell::validate(int max)
{
    return std::tuple<bool,bool>(false, false);
    /*
    int in = 0;

    if(!routes.validate(in)) return std::tuple<bool, bool>(false,in <= gates::IN);
    if((value < -1)||(value > max)) return std::tuple<bool, bool>(false,in <= gates::IN);

    return std::tuple<bool, bool>(true, in <= gates::IN);
    */
}

bool organisation::cell::equals(const cell &source)
{
    if(value != source.value) return false;
    
    //return routes.equals(source.routes);

    return true;

}

void organisation::cell::copy(const cell &source)
{
    clear();

    value = source.value;
    //routes.copy(source.routes);
}

int organisation::cell::map(vector source)
{
    int tx = source.x + 1;
	int ty = source.y + 1;
	int tz = source.z + 1;

	if ((tx < 0) || (tx > 2)) return 0;
	if ((ty < 0) || (ty > 2)) return 0;
	if ((tz < 0) || (tz > 2)) return 0;

	return (abs(tz) * (3L * 3L)) + (abs(ty) * 3L) + abs(tx);
}