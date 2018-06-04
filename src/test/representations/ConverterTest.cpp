/**
 * @file    ConverterTest.cpp
 * @covers  Converter
 *
 * @author  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 * @author  Simon Froitzheim
 *
 * @since	2015-05-13
 * @version     2015-12-16
 */

#include "gtest/gtest.h"
#include "../defines.h"
#include "representations/GeometricObject.h"

using namespace hypro;
using namespace carl;

template<typename Number>
class ConverterTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
		//box
		std::vector<carl::Interval<Number>> boundaries;
		boundaries.push_back(carl::Interval<Number>(2,6));
		boundaries.push_back(carl::Interval<Number>(1,3));
		box = hypro::Box<Number>(boundaries);

		// first support function
		matrix = matrix_t<Number>(3,2);
		distances = vector_t<Number>(3);
		matrix << 1,1,
                -1,1,
                0,-1;
		distances << 1,1,0;

        support = SupportFunction<Number>(this->matrix, this->distances);

        // second support function (box)
        matrix2 = matrix_t<Number>(4,2);
        distances2 = vector_t<Number>(4);
        matrix2 << 0, 1, 0, -1, 1, 0, -1, 0;
        distances2 << 2, 2, 1, 1;

        support2 = SupportFunction<Number>(this->matrix2, this->distances2);

        //first zonotope
		vector_t<Number> center = vector_t<Number>(2);
		center << 2, 2;
		matrix_t<Number> generators = matrix_t<Number>(2,2);
		generators << 0, 1, 1, -1;
		zonotope = Zonotope<Number>(center, generators);

        //second zonotope (box)
        vector_t<Number> center2 = vector_t<Number>(2);
        center2 << 1, 1;
        matrix_t<Number> generators2 = matrix_t<Number>(2,2);
        generators2 << 0, 1, 1, 0;
        zonotope2 = Zonotope<Number>(center2, generators2);

		//first v-polytope (box)
		vector_t<Number> p1 = vector_t<Number>(2);
		p1(0) = 1;
		p1(1) = 2;
		vector_t<Number> p2 = vector_t<Number>(2);
		p2(0) = 3;
		p2(1) = 2;
		vector_t<Number> p3 = vector_t<Number>(2);
		p3(0) = 3;
		p3(1) = 4;
		vector_t<Number> p4 = vector_t<Number>(2);
		p4(0) = 1;
		p4(1) = 4;
		typename VPolytope<Number>::pointVector points;
		points.push_back(Point<Number>(p1));
		points.push_back(Point<Number>(p2));
		points.push_back(Point<Number>(p3));
		points.push_back(Point<Number>(p4));
		vpolytope = VPolytope<Number>(points);

		//second v-polytope (slightly deformed box)
		vector_t<Number> p5 = vector_t<Number>(2);
		p5(0) = 0;
		p5(1) = 2;
		typename VPolytope<Number>::pointVector points2;
		points2.push_back(Point<Number>(p5));
		points2.push_back(Point<Number>(p2));
		points2.push_back(Point<Number>(p3));
		points2.push_back(Point<Number>(p4));
		vpolytope2 = VPolytope<Number>(points2);

		//third v-polytope(tilted stretched box)
		vector_t<Number> p6 = vector_t<Number>(2);
		p6(0)= 2;
		p6(1)= 1;
		vector_t<Number> p7 = vector_t<Number>(2);
		p7(0)= 1;
		p7(1)= 2;
		vector_t<Number> p8 = vector_t<Number>(2);
		p8(0)= 5;
		p8(1)= 4;
		vector_t<Number> p9 = vector_t<Number>(2);
		p9(0)= 4;
		p9(1)= 5;
		typename VPolytope<Number>::pointVector points3;
		points3.push_back(Point<Number>(p6));
		points3.push_back(Point<Number>(p7));
		points3.push_back(Point<Number>(p8));
		points3.push_back(Point<Number>(p9));
		vpolytope3 = VPolytope<Number>(points3);

        // A box (for H-Polytope)
		Halfspace<Number> hp1({1,0},2);
		Halfspace<Number> hp2({0,1},2);
		Halfspace<Number> hp3({-1,0},2);
		Halfspace<Number> hp4({0,-1},2);

        planes.push_back(hp1);
		planes.push_back(hp2);
		planes.push_back(hp3);
		planes.push_back(hp4);

		hpolytope = HPolytope<Number>(this->planes);

		//second H-Polytope
		Halfspace<Number> hp5({1,1},carl::rationalize<Number>(1.4));
		Halfspace<Number> hp6({0,-1},1);
		Halfspace<Number> hp7({-1,0},1);

		planes2.push_back(hp5);
		planes2.push_back(hp6);
		planes2.push_back(hp7);

		hpolytope2 = HPolytope<Number>(this->planes2);

	#ifdef HYPRO_USE_PPL
		//A ppl polytope (box)
		pplpolytope = Polytope<Number>(points);
		pplpolytope2 = Polytope<Number>(points2);
	#endif

    }

    virtual void TearDown()
    {
    }

    matrix_t<Number> matrix;
    matrix_t<Number> matrix2;
    vector_t<Number> distances;
    vector_t<Number> distances2;
    SupportFunction<Number> support;
    SupportFunction<Number> support2;

    hypro::Box<Number> box;

    VPolytope<Number> vpolytope;
    VPolytope<Number> vpolytope2;
    VPolytope<Number> vpolytope3;

    Zonotope<Number> zonotope;
    Zonotope<Number> zonotope2;
    Zonotope<Number> zonotope3;
    Zonotope<Number> zonotope4;

    typename HPolytope<Number>::HalfspaceVector planes;
    typename HPolytope<Number>::HalfspaceVector planes2;
    HPolytope<Number> hpolytope;
    HPolytope<Number> hpolytope2;

    #ifdef HYPRO_USE_PPL
    Polytope<Number> pplpolytope;
    Polytope<Number> pplpolytope2;
    #endif
};

TYPED_TEST(ConverterTest, toBox)
{
	Converter<TypeParam>::toBox(this->box);
	Converter<TypeParam>::toBox(this->support);
    Converter<TypeParam>::toBox(this->support2);
	Converter<TypeParam>::toBox(this->vpolytope);
    Converter<TypeParam>::toBox(this->vpolytope2);
	Converter<TypeParam>::toBox(this->zonotope);
    Converter<TypeParam>::toBox(this->zonotope2);
    Converter<TypeParam>::toBox(this->zonotope, ALTERNATIVE);
    Converter<TypeParam>::toBox(this->zonotope2, ALTERNATIVE);
    Converter<TypeParam>::toBox(this->hpolytope);
    Converter<TypeParam>::toBox(this->hpolytope2);
    auto result = Converter<TypeParam>::toBox(this->hpolytope, ALTERNATIVE);
    auto result2 = Converter<TypeParam>::toBox(this->hpolytope2, ALTERNATIVE);
    auto result3 = Converter<TypeParam>::toBox(this->hpolytope);
    auto result4 = Converter<TypeParam>::toBox(this->hpolytope2);
    #ifdef HYPRO_USE_PPL
    Converter<TypeParam>::toBox(this->pplpolytope);
    Converter<TypeParam>::toBox(this->pplpolytope2);
    #endif
    SUCCEED();
}

TYPED_TEST(ConverterTest, toHPolytope)
{
    auto result =  Converter<TypeParam>::toHPolytope(this->hpolytope);
    auto result2 = Converter<TypeParam>::toHPolytope(this->support);
    auto result3 = Converter<TypeParam>::toHPolytope(this->support2);
    auto result4 = Converter<TypeParam>::toHPolytope(this->zonotope);
    auto result5 = Converter<TypeParam>::toHPolytope(this->zonotope2);
    auto result6 = Converter<TypeParam>::toHPolytope(this->vpolytope, OVER);
    auto result7 = Converter<TypeParam>::toHPolytope(this->vpolytope2, OVER);
    auto result8 = Converter<TypeParam>::toHPolytope(this->vpolytope3, OVER);
    auto result9 = Converter<TypeParam>::toHPolytope(this->box);
    #ifdef HYPRO_USE_PPL
    auto result10 = Converter<TypeParam>::toHPolytope(this->pplpolytope);
    auto result11 = Converter<TypeParam>::toHPolytope(this->pplpolytope2);
    #endif
	SUCCEED();
}

TYPED_TEST(ConverterTest, toSupportFunction)
{
    Converter<TypeParam>::toSupportFunction(this->support);
    Converter<TypeParam>::toSupportFunction(this->hpolytope);
    Converter<TypeParam>::toSupportFunction(this->hpolytope2);
    Converter<TypeParam>::toSupportFunction(this->box);
    Converter<TypeParam>::toSupportFunction(this->vpolytope);
    Converter<TypeParam>::toSupportFunction(this->vpolytope2);
    Converter<TypeParam>::toSupportFunction(this->zonotope);
    Converter<TypeParam>::toSupportFunction(this->zonotope2);
    #ifdef HYPRO_USE_PPL
    Converter<TypeParam>::toSupportFunction(this->pplpolytope);
    Converter<TypeParam>::toSupportFunction(this->pplpolytope2);
    #endif
	SUCCEED();
}

TYPED_TEST(ConverterTest, toVPolytope)
{
    auto result =  Converter<TypeParam>::toVPolytope(this->vpolytope);
    auto result2 = Converter<TypeParam>::toVPolytope(this->support, OVER);
    auto result3 = Converter<TypeParam>::toVPolytope(this->support2, OVER);
    auto result4 = Converter<TypeParam>::toVPolytope(this->zonotope);
    auto result5 = Converter<TypeParam>::toVPolytope(this->zonotope2);
    auto result6 = Converter<TypeParam>::toVPolytope(this->hpolytope, OVER);
    auto result7 = Converter<TypeParam>::toVPolytope(this->hpolytope2, OVER);
    auto result8 = Converter<TypeParam>::toVPolytope(this->box);
    #ifdef HYPRO_USE_PPL
    auto result10 = Converter<TypeParam>::toVPolytope(this->pplpolytope);
    auto result11 = Converter<TypeParam>::toVPolytope(this->pplpolytope2);
    #endif
	SUCCEED();
}

TYPED_TEST(ConverterTest, toZonotope)
{
    auto result = Converter<TypeParam>::toZonotope(this->zonotope);
    auto result2 = Converter<TypeParam>::toZonotope(this->box);
    auto result3 = Converter<TypeParam>::toZonotope(this->vpolytope);
    auto result4 = Converter<TypeParam>::toZonotope(this->vpolytope2);
    auto result5 = Converter<TypeParam>::toZonotope(this->vpolytope3);
    auto result6 = Converter<TypeParam>::toZonotope(this->hpolytope);
    auto result7 = Converter<TypeParam>::toZonotope(this->hpolytope2);
    auto result8 = Converter<TypeParam>::toZonotope(this->support, ALTERNATIVE);
    auto result9 = Converter<TypeParam>::toZonotope(this->support2, ALTERNATIVE);
    #ifdef HYPRO_USE_PPL
    auto result10 = Converter<TypeParam>::toZonotope(this->pplpolytope);
    auto result11 = Converter<TypeParam>::toZonotope(this->pplpolytope2);
    #endif
	SUCCEED();
}

#ifdef HYPRO_USE_PPL
TYPED_TEST(ConverterTest, toPolytope){

	auto result1 = Converter<TypeParam>::toPolytope(this->box);
	auto result2 = Converter<TypeParam>::toPolytope(this->zonotope);
	auto result3 = Converter<TypeParam>::toPolytope(this->vpolytope);
	auto result4 = Converter<TypeParam>::toPolytope(this->vpolytope2);
	auto result5 = Converter<TypeParam>::toPolytope(this->vpolytope3);
	auto result6 = Converter<TypeParam>::toPolytope(this->hpolytope);
	auto result7 = Converter<TypeParam>::toPolytope(this->hpolytope2);
	auto result8 = Converter<TypeParam>::toPolytope(this->support);
	auto result9 = Converter<TypeParam>::toPolytope(this->support2);
	auto result10 = Converter<TypeParam>::toPolytope(this->pplpolytope);
	SUCCEED();
}
#endif

TYPED_TEST(ConverterTest, settingsConversion)
{
	hypro::BoxT<TypeParam,Converter<TypeParam>,BoxAllOff> res = hypro::BoxT<TypeParam,Converter<TypeParam>,BoxAllOff>(this->box);
    EXPECT_EQ(this->box, res);

    SUCCEED();
}
