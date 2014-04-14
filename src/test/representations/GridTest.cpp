/**
 * @file    GridTest.cpp
 * 
 * @covers  Grid
 * 
 * @author  Benedikt Seidl
 * 
 * @since   2014-04-14
 * @version 2014-04-14
 */

#include "gtest/gtest.h"
#include "../defines.h"
#include "../../lib/datastructures/Point.h"
#include "../../lib/representations/OrthogonalPolyhedron/Grid.h"
#include <carl/core/VariablePool.h>

using namespace hypro;
using namespace carl;

class GridTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        // grid1
        Point<number_t>::vector_t coordinates1;
        coordinates1.insert( std::make_pair(x, FLOAT_T<number_t>(2)) );
        coordinates1.insert( std::make_pair(y, FLOAT_T<number_t>(5)) );
        grid1.insert(Point<number_t>(coordinates1), true);
        
        Point<number_t>::vector_t coordinates2;
        coordinates1.insert( std::make_pair(x, FLOAT_T<number_t>(2)) );
        coordinates1.insert( std::make_pair(y, FLOAT_T<number_t>(5)) );
        grid1.insert(Point<number_t>(coordinates2), true);
        
        // grid2
        Point<number_t>::vector_t coordinates3;
        coordinates1.insert( std::make_pair(x, FLOAT_T<number_t>(2)) );
        coordinates1.insert( std::make_pair(y, FLOAT_T<number_t>(5)) );
        grid2.insert(Point<number_t>(coordinates3), true);
        
        Point<number_t>::vector_t coordinates4;
        coordinates1.insert( std::make_pair(x, FLOAT_T<number_t>(2)) );
        coordinates1.insert( std::make_pair(y, FLOAT_T<number_t>(5)) );
        grid2.insert(Point<number_t>(coordinates4), true);
    }
	
    virtual void TearDown()
    {
    }
	
    VariablePool& pool = VariablePool::getInstance();
    Variable x = pool.getFreshVariable("x");
    Variable y = pool.getFreshVariable("y");

    Grid<number_t> grid1;
    Grid<number_t> grid2;
};

TEST_F(GridTest, Constructor)
{
    Grid<number_t> grid1();
    Grid<number_t>::gridMap map;
    Grid<number_t> grid2(map);
}