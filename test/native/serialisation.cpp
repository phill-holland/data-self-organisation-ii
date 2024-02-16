#include <gtest/gtest.h>
#include <string>
#include "program.h"
#include "data.h"
#include "vector.h"
#include "schema.h"
#include "general.h"
#include "point.h"
#include "genetic/cache.h"
#include "genetic/movement.h"
#include "genetic/collisions.h"
#include "genetic/insert.h"

TEST(BasicSerialisationDeserialisation, BasicAssertions)
{
    //GTEST_SKIP();

    const int width = 20, height = 20, depth = 20;
    organisation::point starting(width / 2, height / 2, depth / 2);

    organisation::program p1(width, height, depth);
    organisation::program p2(width, height, depth);

    organisation::genetic::cache cache(width, height, depth);
    cache.set(0, starting);
    
    organisation::genetic::insert insert;
    insert.values = { 1,2,3 };

    organisation::genetic::movement movement;
    movement.directions = { { 1,0,0 }, { 0,0,1 } };

    organisation::genetic::collisions collisions;

    collisions.values.resize(27);
    organisation::vector up = { 1,0,0 };
    organisation::vector rebound = { 0,1,0 };
    collisions.values[up.encode()] = rebound.encode();

    p1.set(cache);
    p1.set(insert);
    p1.set(movement);
    p1.set(collisions);

    std::string data = p1.serialise();
    p2.deserialise(data);

    EXPECT_TRUE(p1.equals(p2));
}
