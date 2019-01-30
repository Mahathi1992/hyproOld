#include "gtest/gtest.h"
#include "../defines.h"
#include "util/adaptions_eigen/adaptions_eigen.h"

template<typename Number>
class EigenAdaptionsTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
		matA = hypro::matrix_t<Number>(2,2);
		matA << 1,2,3,4;

		matB = hypro::matrix_t<Number>(4,4);
		matB << 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16;
	}

    virtual void TearDown()
    {}

	hypro::matrix_t<Number> matA;
	hypro::matrix_t<Number> matB;
};

TYPED_TEST(EigenAdaptionsTest, reduceNumberRepresentation)
{
	// TODO: finish vector rounding.
}

TYPED_TEST(EigenAdaptionsTest, createCode)
{
	std::string out = hypro::createCode(this->matA,0);
	std::cout << out << std::endl;
}

TYPED_TEST(EigenAdaptionsTest, selectRows)
{
	hypro::matrix_t<TypeParam> resA = hypro::matrix_t<TypeParam>(2,4);
	resA << 1,2,3,4,13,14,15,16;
	EXPECT_EQ(resA, hypro::selectRows(this->matB,std::vector<Eigen::Index>({0,3})));

	hypro::matrix_t<TypeParam> resB = hypro::matrix_t<TypeParam>(1,4);
	resB << 5,6,7,8;
	EXPECT_EQ(resB, hypro::selectRows(this->matB,std::vector<Eigen::Index>({1})));
}

TYPED_TEST(EigenAdaptionsTest, removeRows)
{
	hypro::matrix_t<TypeParam> resA = hypro::matrix_t<TypeParam>(2,4);
	resA << 1,2,3,4,13,14,15,16;
	EXPECT_EQ(resA, hypro::removeRows(this->matB,std::vector<Eigen::Index>({1,2})));

	hypro::matrix_t<TypeParam> resB = hypro::matrix_t<TypeParam>(3,4);
	resB << 1,2,3,4,9,10,11,12,13,14,15,16;
	EXPECT_EQ(resB, hypro::removeRows(this->matB,std::vector<Eigen::Index>({1})));
}
