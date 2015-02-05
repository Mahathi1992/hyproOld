/**
 * Tests for SupportFunctions.
 * @author Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 * 
 * @since		2015-01-20
 * @version		2015-01-22
 */

#include "../../lib/representations/SupportFunction/PolytopeSupportfunction.h"
#include "gtest/gtest.h"
#include "../defines.h"


using namespace hypro;
using namespace carl;

template<typename Number>
class PolytopeSupportFunctionTest : public ::testing::Test {
protected:
    virtual void SetUp() {
		constraints = matrix_t<Number>::Zero(3,3);
//		constraints(0,0) = Number(0.5);
//		constraints(0,1) = Number(-1);
//		constraints(1,0) = Number(0.5);
//		constraints(1,1) = Number(1);
//		constraints(2,0) = Number(-4);
//		constraints(2,1) = Number(1);
		constraints(0,0) = Number(2);
		constraints(0,1) = Number(4);
		constraints(0,2) = Number(0);
		constraints(1,0) = Number(1);
		constraints(1,1) = Number(-2);
		constraints(1,2) = Number(0);
		constraints(2,0) = Number(-4);
		constraints(2,1) = Number(10);
		constraints(2,2) = Number(0);
		
		constants = matrix_t<Number>(3,1);
//		constants(0,0) = Number(2.5);
//		constants(1,0) = Number(5);
//		constants(2,0) = Number(17);
		constants(0,0) = Number(20);
		constants(1,0) = Number(5);
		constants(2,0) = Number(17);

		ad = new artificialDirections<Number>();
		ad->dir1 = getZeroVector<Number>(constraints.cols());
		ad->dir2 = getZeroVector<Number>(constraints.cols());

		ad->dir1(constraints.cols()-1,0) = 1;
		ad->dir2(constraints.cols()-1,0) = -1;
    }
	
    virtual void TearDown(){
		delete ad;
    }
	
	matrix_t<Number> constraints;
	matrix_t<Number> constants;
	artificialDirections<Number>* ad;
};

TYPED_TEST(PolytopeSupportFunctionTest, constructor) {
	std::cout << this->constraints << std::endl;
	std::cout << this->constants << std::endl;
	PolytopeSupportFunction<TypeParam> psf1 = PolytopeSupportFunction<TypeParam>(this->constraints, this->constants, operator_e::LEQ, 3, this->ad);
	SUCCEED();
}

TYPED_TEST(PolytopeSupportFunctionTest, access) {
	PolytopeSupportFunction<TypeParam> psf1 = PolytopeSupportFunction<TypeParam>(this->constraints, this->constants, operator_e::LEQ, 3, this->ad);
	matrix_t<TypeParam> vec1 = matrix_t<TypeParam>(3,1);
	matrix_t<TypeParam> vec2 = matrix_t<TypeParam>(3,1);
	matrix_t<TypeParam> vec3 = matrix_t<TypeParam>(3,1);
//	matrix_t<TypeParam> vec1 = matrix_t<TypeParam>(2,1);
//	matrix_t<TypeParam> vec2 = matrix_t<TypeParam>(2,1);
//	matrix_t<TypeParam> vec3 = matrix_t<TypeParam>(2,1);
	
	vec1(0,0) = TypeParam(2);
	vec1(1,0) = TypeParam(4);
	vec1(2,0) = TypeParam(0);

	vec2(0,0) = TypeParam(1);
	vec2(1,0) = TypeParam(-2);
	vec2(2,0) = TypeParam(0);
	
	vec3(0,0) = TypeParam(-4);
	vec3(1,0) = TypeParam(1);
	vec3(2,0) = TypeParam(0);
	
	EXPECT_EQ(TypeParam(20),psf1.evaluate(vec1).supportValue);
	EXPECT_EQ(TypeParam(5),psf1.evaluate(vec2).supportValue);
	EXPECT_EQ(TypeParam(17),psf1.evaluate(vec3).supportValue);
}

TYPED_TEST(PolytopeSupportFunctionTest, linearTransformation) {

}

TYPED_TEST(PolytopeSupportFunctionTest, MinkowskiSum) {

}

TYPED_TEST(PolytopeSupportFunctionTest, contains) {

}