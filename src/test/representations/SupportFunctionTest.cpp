/**
 * Tests for SupportFunctions.
 * @author Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *
 * @since		2015-02-24
 * @version		2015-02-25
 */

#include "../../hypro/representations/GeometricObject.h"
#include "../../hypro/util/Plotter.h"
#include "gtest/gtest.h"
#include "../defines.h"

#include "util/Plotter.h"


using namespace hypro;
using namespace carl;

template<typename Number>
class SupportFunctionTest : public ::testing::Test {
protected:
    virtual void SetUp() {
		constraints = matrix_t<Number>::Zero(3,2);
		constraints(0,0) = Number(2);
		constraints(0,1) = Number(4);
		constraints(1,0) = Number(1);
		constraints(1,1) = Number(-2);
		constraints(2,0) = Number(-4);
		constraints(2,1) = Number(1);

		constants = vector_t<Number>(3);
		constants(0) = Number(20);
		constants(1) = Number(5);
		constants(2) = Number(17);

		vec1 = vector_t<Number>(2);
		vec2 = vector_t<Number>(2);
		vec3 = vector_t<Number>(2);

		vec1(0) = Number(2);
		vec1(1) = Number(4);

		vec2(0) = Number(1);
		vec2(1) = Number(-2);

		vec3(0) = Number(-4);
		vec3(1) = Number(1);

		// construction of complete chain
		boxConstraints = matrix_t<Number>::Zero(4,2);
		boxConstraints << 1,0,-1,0,0,1,0,-1;
		vector_t<Number> boxConstants = vector_t<Number>::Zero(4);
		boxConstants << 1,1,1,1;
		vector_t<Number> boxConstants2 = vector_t<Number>::Zero(4);
		boxConstants2 << 5,5,5,5;
		vector_t<Number> boxConstants3 = vector_t<Number>::Zero(4);
		boxConstants3 << 1,1,1,1;
		matrix_t<Number> trafoMatrix = matrix_t<Number>(2,2);
		trafoMatrix << 0,1,1,0;
		vector_t<Number> trafoVector = vector_t<Number>(2);
		trafoVector << 1,1;


		SupportFunction<Number> box(boxConstraints,boxConstants);
		SupportFunction<Number> ball(SF_TYPE::TWO_BALL, 2, 2);
		SupportFunction<Number> mSum = box.minkowskiSum(ball);
		SupportFunction<Number> box2(boxConstraints,boxConstants2);
		SupportFunction<Number> united = mSum.unite(box2);
		SupportFunction<Number> box3(boxConstraints,boxConstants3);
		SupportFunction<Number> intersected = united.intersect(box3);
		sfChainComplete = intersected.affineTransformation(trafoMatrix, trafoVector);
		// At this point sfChainComplete holds the unit box shifted +1 unit towards each dimension.

    }

    virtual void TearDown(){
    }

	matrix_t<Number> constraints;
	matrix_t<Number> boxConstraints;
	vector_t<Number> constants;

	vector_t<Number> vec1;
	vector_t<Number> vec2;
	vector_t<Number> vec3;

	SupportFunction<Number> sfChainComplete;
};

/**
 * Tests only a single constructor which holds a concrete representation. Other constructors are tested by operations.
 */
TYPED_TEST(SupportFunctionTest, constructor) {
	SupportFunction<TypeParam> psf1 = SupportFunction<TypeParam>(this->constraints, this->constants);

	// Ball support function constructor
	BallSupportFunction<TypeParam> ball(TypeParam(3), SF_TYPE::TWO_BALL);
	ball.setDimension(3);
	EXPECT_EQ(unsigned(3), ball.dimension());
	EXPECT_FALSE(ball.empty());

	SupportFunction<TypeParam> copied(this->sfChainComplete);
	EXPECT_EQ(this->sfChainComplete.evaluate(this->vec1), copied.evaluate(this->vec1));
	EXPECT_EQ(this->sfChainComplete.evaluate(this->vec2), copied.evaluate(this->vec2));
	EXPECT_EQ(this->sfChainComplete.evaluate(this->vec3), copied.evaluate(this->vec3));

	SUCCEED();
}

TYPED_TEST(SupportFunctionTest, assignment) {
	SupportFunction<TypeParam> psf1Assigned = this->sfChainComplete;
	EXPECT_EQ(this->sfChainComplete.evaluate(this->vec1), psf1Assigned.evaluate(this->vec1));
	EXPECT_EQ(this->sfChainComplete.evaluate(this->vec2), psf1Assigned.evaluate(this->vec2));
	EXPECT_EQ(this->sfChainComplete.evaluate(this->vec3), psf1Assigned.evaluate(this->vec3));
}

TYPED_TEST(SupportFunctionTest, simpleEvaluation) {
	SupportFunction<TypeParam> psf1 = SupportFunction<TypeParam>(this->constraints, this->constants);
	matrix_t<TypeParam> vec1 = matrix_t<TypeParam>(2,1);
	matrix_t<TypeParam> vec2 = matrix_t<TypeParam>(2,1);
	matrix_t<TypeParam> vec3 = matrix_t<TypeParam>(2,1);

	EXPECT_LE(TypeParam(20), psf1.evaluate(this->vec1).supportValue);
	EXPECT_LE(TypeParam(5), psf1.evaluate(this->vec2).supportValue);
	EXPECT_TRUE(carl::AlmostEqual2sComplement(TypeParam(17), psf1.evaluate(this->vec3).supportValue));

	BallSupportFunction<TypeParam> ball(TypeParam(3), SF_TYPE::TWO_BALL);
	vector_t<TypeParam> dir = vector_t<TypeParam>::Zero(2);
	dir(0) = TypeParam(1);
	EvaluationResult<TypeParam> res = ball.evaluate(dir);
	EXPECT_TRUE(carl::AlmostEqual2sComplement(res.supportValue, TypeParam(3),4));
	//EXPECT_EQ(res.supportValue, TypeParam(3));

	BallSupportFunction<TypeParam> ball2(TypeParam(3), SF_TYPE::INFTY_BALL);
	res = ball2.evaluate(dir);
	EXPECT_TRUE(carl::AlmostEqual2sComplement(res.supportValue, TypeParam(3),4));
	//EXPECT_EQ(res.supportValue, TypeParam(3));
}

TYPED_TEST(SupportFunctionTest, Supremum) {
	SupportFunction<TypeParam> psf1 = SupportFunction<TypeParam>(this->constraints, this->constants);
	TypeParam supremum = psf1.supremum();

	// compute intersection points.
	HPolytope<TypeParam> hpt1 = HPolytope<TypeParam>(this->constraints, this->constants);
	EXPECT_EQ(hpt1.supremum(), supremum);

	matrix_t<TypeParam> trafoMatrix = matrix_t<TypeParam>(2,2);
	trafoMatrix << 1,2,3,4;
	vector_t<TypeParam> trafoVector = vector_t<TypeParam>(2);
	trafoVector << 5,6;

	SupportFunction<TypeParam> psf2 = psf1.affineTransformation(trafoMatrix, trafoVector);
	HPolytope<TypeParam> hpt2 = hpt1.affineTransformation(trafoMatrix, trafoVector);

	EXPECT_TRUE(carl::AlmostEqual2sComplement(psf2.supremum(), hpt2.supremum(), 4));

	// Ball support function supremum point
	BallSupportFunction<TypeParam> ball(TypeParam(3), SF_TYPE::TWO_BALL);
	ball.setDimension(3);
	vector_t<TypeParam> expectedPointCoordinates = vector_t<TypeParam>::Zero(3);
	expectedPointCoordinates(0) = 3;
	EXPECT_EQ(ball.supremumPoint().rawCoordinates(), expectedPointCoordinates);
}


TYPED_TEST(SupportFunctionTest, linearTransformation) {
	SupportFunction<TypeParam> psf1 = SupportFunction<TypeParam>(this->constraints, this->constants);
	matrix_t<TypeParam> rotation = matrix_t<TypeParam>(2,2);
	rotation(0,0) = 0;
	rotation(0,1) = 1;
	rotation(1,0) = -1;
	rotation(1,1) = 0;

	vector_t<TypeParam> v1Rot = rotation*(this->vec1);
	vector_t<TypeParam> v2Rot = rotation*(this->vec2);
	vector_t<TypeParam> v3Rot = rotation*(this->vec3);

	SupportFunction<TypeParam> res = psf1.linearTransformation(rotation);

	EXPECT_EQ(rotation, res.linearTrafoParameters()->parameters->matrix());
	EXPECT_EQ(vector_t<TypeParam>::Zero(2), res.linearTrafoParameters()->parameters->vector());

	EXPECT_LE(TypeParam(20), res.evaluate(v1Rot).supportValue);
	EXPECT_LE(TypeParam(5), res.evaluate(v2Rot).supportValue);
	EXPECT_TRUE(carl::AlmostEqual2sComplement(TypeParam(17), res.evaluate(v3Rot).supportValue) || TypeParam(17) <= res.evaluate(v3Rot).supportValue);

	// execute many linear transformations to test reduction

	for( std::size_t i = 1; i < std::size_t(pow(2,res.linearTrafoParameters()->parameters->power*2)); ++i) {
		res = res.linearTransformation(rotation);
	}
	EXPECT_EQ(res.linearTrafoParameters()->currentExponent, unsigned(16));
	EXPECT_EQ(res.linearTrafoParameters()->parameters->parameters.size(), std::size_t(1));
	// reduction is only invoked when using the support function ->evaluate
	res.evaluate(v2Rot);
	EXPECT_EQ(res.linearTrafoParameters()->parameters->parameters.size(), std::size_t(3));
}

TYPED_TEST(SupportFunctionTest, scale) {
	SupportFunction<TypeParam> psf1 = SupportFunction<TypeParam>(this->constraints, this->constants);
	TypeParam factor = 2;

	SupportFunction<TypeParam> res =  psf1.scale(factor);

	EXPECT_LE(TypeParam(factor) * TypeParam(20), res.evaluate(this->vec1).supportValue);
	EXPECT_LE(TypeParam(factor) * TypeParam(5), res.evaluate(this->vec2).supportValue);
	EXPECT_TRUE(carl::AlmostEqual2sComplement(TypeParam(17), res.evaluate(this->vec3).supportValue) || TypeParam(17) <= res.evaluate(this->vec3).supportValue);
}

TYPED_TEST(SupportFunctionTest, minkowskiSum) {
	matrix_t<TypeParam> constraints1 = matrix_t<TypeParam>(3,2);
	matrix_t<TypeParam> constraints2 = matrix_t<TypeParam>(3,2);
	vector_t<TypeParam> constants1 = vector_t<TypeParam>(3);
	vector_t<TypeParam> constants2 = vector_t<TypeParam>(3);

	// Triangle (-1,0) - (1,0) - (0,2)
	constraints1(0,0) = TypeParam(2);
	constraints1(0,1) = TypeParam(1);
	constraints1(1,0) = TypeParam(-2);
	constraints1(1,1) = TypeParam(1);
	constraints1(2,0) = TypeParam(0);
	constraints1(2,1) = TypeParam(-1);

	constants1(0) = TypeParam(2);
	constants1(1) = TypeParam(2);
	constants1(2) = TypeParam(0);

	// Triangle (0,2) - (-1,4) - (1,4)
	constraints2(0,0) = TypeParam(-2);
	constraints2(0,1) = TypeParam(-1);
	constraints2(1,0) = TypeParam(2);
	constraints2(1,1) = TypeParam(-1);
	constraints2(2,0) = TypeParam(0);
	constraints2(2,1) = TypeParam(1);

	constants2(0) = TypeParam(-2);
	constants2(1) = TypeParam(-2);
	constants2(2) = TypeParam(4);

	SupportFunction<TypeParam> tri1 = SupportFunction<TypeParam>(constraints1, constants1);
	SupportFunction<TypeParam> tri2 = SupportFunction<TypeParam>(constraints2, constants2);

	// Result directions
	vector_t<TypeParam> vec1 = vector_t<TypeParam>(2);
	vec1(0) = TypeParam(2);
	vec1(1) = TypeParam(1);

	vector_t<TypeParam> vec2 = vector_t<TypeParam>(2);
	vec2(0) = TypeParam(-2);
	vec2(1) = TypeParam(1);

	vector_t<TypeParam> vec3 = vector_t<TypeParam>(2);
	vec3(0) = TypeParam(0);
	vec3(1) = TypeParam(-1);

	vector_t<TypeParam> vec4 = vector_t<TypeParam>(2);
	vec4(0) = TypeParam(2);
	vec4(1) = TypeParam(-1);

	vector_t<TypeParam> vec5 = vector_t<TypeParam>(2);
	vec5(0) = TypeParam(-2);
	vec5(1) = TypeParam(-1);

	vector_t<TypeParam> vec6 = vector_t<TypeParam>(2);
	vec6(0) = TypeParam(0);
	vec6(1) = TypeParam(1);

	SupportFunction<TypeParam> res = tri1.minkowskiSum(tri2);

	EXPECT_LE(TypeParam(8), res.evaluate(vec1).supportValue);
	EXPECT_LE(TypeParam(8), res.evaluate(vec2).supportValue);
	EXPECT_LE(TypeParam(-2), res.evaluate(vec3).supportValue);
	EXPECT_LE(TypeParam(0), res.evaluate(vec4).supportValue);
	EXPECT_LE(TypeParam(0), res.evaluate(vec5).supportValue);
	EXPECT_LE(TypeParam(6), res.evaluate(vec6).supportValue);
}

TYPED_TEST(SupportFunctionTest, intersect) {
	matrix_t<TypeParam> constraints1 = matrix_t<TypeParam>::Zero(4,2);
	constraints1 << 1,0,
					-1,0,
					0,1,
					0,-1;
	vector_t<TypeParam> constants1 = vector_t<TypeParam>::Zero(4);
	constants1 << 1,1,1,1;

	vector_t<TypeParam> constants2 =  vector_t<TypeParam>::Zero(4);
	constants2 << 0,0,1,1;

	SupportFunction<TypeParam> psf1 = SupportFunction<TypeParam>(constraints1, constants1);
	SupportFunction<TypeParam> psf2 = SupportFunction<TypeParam>(constraints1, constants2);

	SupportFunction<TypeParam> result = psf1.intersect(psf2);

	vector_t<TypeParam> dir1 = vector_t<TypeParam>::Zero(2);
	dir1 << 1,0;
	vector_t<TypeParam> dir2 = vector_t<TypeParam>::Zero(2);
	dir2 << -1,0;
	vector_t<TypeParam> dir3 = vector_t<TypeParam>::Zero(2);
	dir3 << 0,1;
	vector_t<TypeParam> dir4 = vector_t<TypeParam>::Zero(2);
	dir4 << 0,-1;

	EXPECT_EQ(result.evaluate(dir1).supportValue, TypeParam(0));
	EXPECT_EQ(result.evaluate(dir2).supportValue, TypeParam(0));
	EXPECT_EQ(result.evaluate(dir3).supportValue, TypeParam(1));
	EXPECT_EQ(result.evaluate(dir4).supportValue, TypeParam(1));
}

TYPED_TEST(SupportFunctionTest, satisfiesHalfspaces) {
	matrix_t<TypeParam> constraints1 = matrix_t<TypeParam>::Zero(4,2);
	constraints1 << 1,0,
					-1,0,
					0,1,
					0,-1;
	vector_t<TypeParam> constants1 = vector_t<TypeParam>::Zero(4);
	constants1 << 1,1,1,1;

	SupportFunction<TypeParam> psf1 = SupportFunction<TypeParam>(constraints1, constants1);

	matrix_t<TypeParam> normal = matrix_t<TypeParam>::Zero(1,2);
	normal << 1,1;
	vector_t<TypeParam> normalVector = vector_t<TypeParam>::Zero(2);
	normalVector << 1,1;
	vector_t<TypeParam> offset = vector_t<TypeParam>::Zero(1);
	offset << 2;

	std::pair<bool,SupportFunction<TypeParam>> fullyContained = psf1.satisfiesHalfspaces(normal, offset);
	std::pair<bool,SupportFunction<TypeParam>> fullyContained2 = psf1.satisfiesHalfspace(Halfspace<TypeParam>(normalVector, 2));
	EXPECT_TRUE(fullyContained.first);
	EXPECT_TRUE(fullyContained2.first);
	vector_t<TypeParam> evalDir = normal.row(0);
	EXPECT_TRUE(fullyContained.second.evaluate(evalDir).supportValue >= 1);
	EXPECT_TRUE(fullyContained2.second.evaluate(evalDir).supportValue >= 1);

	normal << 1,0;
	normalVector << 1,0;
	offset << 1;
	std::pair<bool,SupportFunction<TypeParam>> onBorder = psf1.satisfiesHalfspaces(normal, offset);
	std::pair<bool,SupportFunction<TypeParam>> onBorder2 = psf1.satisfiesHalfspace(Halfspace<TypeParam>(normalVector, 1));
	EXPECT_TRUE(onBorder.first);
	EXPECT_TRUE(onBorder2.first);
	evalDir = normal.row(0);
	EXPECT_TRUE(onBorder.second.evaluate(evalDir).supportValue == 1);
	EXPECT_TRUE(onBorder2.second.evaluate(evalDir).supportValue == 1);

	offset << 0;
	std::pair<bool,SupportFunction<TypeParam>> cutHalf = psf1.satisfiesHalfspaces(normal, offset);
	std::pair<bool,SupportFunction<TypeParam>> cutHalf2 = psf1.satisfiesHalfspace(Halfspace<TypeParam>(normalVector, 0));
	EXPECT_TRUE(cutHalf.first);
	EXPECT_TRUE(cutHalf.second.evaluate(evalDir).supportValue == 0);
	EXPECT_TRUE(cutHalf2.first);
	EXPECT_TRUE(cutHalf2.second.evaluate(evalDir).supportValue == 0);

	offset << -1;
	std::pair<bool,SupportFunction<TypeParam>> barelyContained = psf1.satisfiesHalfspaces(normal, offset);
	std::pair<bool,SupportFunction<TypeParam>> barelyContained2 = psf1.satisfiesHalfspace(Halfspace<TypeParam>(normalVector, -1));
	EXPECT_TRUE(barelyContained.first);
	EXPECT_EQ(barelyContained.second.evaluate(evalDir).supportValue, -1);
	EXPECT_TRUE(barelyContained2.first);
	EXPECT_EQ(barelyContained2.second.evaluate(evalDir).supportValue, -1);

	offset << -1.1;
	std::pair<bool,SupportFunction<TypeParam>> notContained = psf1.satisfiesHalfspaces(normal, offset);
	std::pair<bool,SupportFunction<TypeParam>> notContained2 = psf1.satisfiesHalfspace(Halfspace<TypeParam>(normalVector, -1.1));
	EXPECT_FALSE(notContained.first);
	EXPECT_TRUE(notContained.second.empty());
	EXPECT_FALSE(notContained2.first);
	EXPECT_TRUE(notContained2.second.empty());
}

TYPED_TEST(SupportFunctionTest, intersectHalfspaces) {
	matrix_t<TypeParam> constraints1 = matrix_t<TypeParam>::Zero(4,2);
	constraints1 << 1,0,
					-1,0,
					0,1,
					0,-1;
	vector_t<TypeParam> constants1 = vector_t<TypeParam>::Zero(4);
	constants1 << 1,1,1,1;

	SupportFunction<TypeParam> psf1 = SupportFunction<TypeParam>(constraints1, constants1);

	matrix_t<TypeParam> normal = matrix_t<TypeParam>::Zero(1,2);
	normal << 1,1;
	vector_t<TypeParam> offset = vector_t<TypeParam>::Zero(1);
	offset << 2;

	vector_t<TypeParam> normalVector = vector_t<TypeParam>::Zero(2);
	normalVector << 1,1;

	SupportFunction<TypeParam> fullyContained = psf1.intersectHalfspaces(normal, offset);
	SupportFunction<TypeParam> fullyContained2 = psf1.intersectHalfspace(Halfspace<TypeParam>(normalVector, 2));
	vector_t<TypeParam> evalDir = normal.row(0);
	EXPECT_TRUE(fullyContained.evaluate(evalDir).supportValue >= 1);
	EXPECT_TRUE(fullyContained2.evaluate(evalDir).supportValue >= 1);

	normal << 1,0;
	normalVector << 1,0;
	offset << 1;
	SupportFunction<TypeParam> onBorder = psf1.intersectHalfspaces(normal, offset);
	SupportFunction<TypeParam> onBorder2 = psf1.intersectHalfspace(Halfspace<TypeParam>(normalVector, 1));
	evalDir = normal.row(0);
	EXPECT_TRUE(onBorder.evaluate(evalDir).supportValue == 1);
	EXPECT_TRUE(onBorder2.evaluate(evalDir).supportValue == 1);

	offset << 0;
	SupportFunction<TypeParam> cutHalf = psf1.intersectHalfspaces(normal, offset);
	SupportFunction<TypeParam> cutHalf2 = psf1.intersectHalfspace(Halfspace<TypeParam>(normalVector, 0));
	EXPECT_TRUE(cutHalf.evaluate(evalDir).supportValue == 0);
	EXPECT_TRUE(cutHalf2.evaluate(evalDir).supportValue == 0);

	offset << -1;
	SupportFunction<TypeParam> barelyContained = psf1.intersectHalfspaces(normal, offset);
	SupportFunction<TypeParam> barelyContained2 = psf1.intersectHalfspace(Halfspace<TypeParam>(normalVector, -1));
	EXPECT_TRUE(barelyContained.evaluate(evalDir).supportValue == -1);
	EXPECT_TRUE(barelyContained2.evaluate(evalDir).supportValue == -1);

	offset << -1.1;
	SupportFunction<TypeParam> notContained = psf1.intersectHalfspaces(normal, offset);
	SupportFunction<TypeParam> notContained2 = psf1.intersectHalfspace(Halfspace<TypeParam>(normalVector, -1.1));
	EXPECT_TRUE(notContained.empty());
	EXPECT_TRUE(notContained2.empty());
}

TYPED_TEST(SupportFunctionTest, unite) {
	matrix_t<TypeParam> constraints1 = matrix_t<TypeParam>(3,2);
	matrix_t<TypeParam> constraints2 = matrix_t<TypeParam>(3,2);
	vector_t<TypeParam> constants1 = vector_t<TypeParam>(3);
	vector_t<TypeParam> constants2 = vector_t<TypeParam>(3);

	// Triangle (-1,0) - (1,0) - (0,2)
	constraints1(0,0) = TypeParam(2);
	constraints1(0,1) = TypeParam(1);
	constraints1(1,0) = TypeParam(-2);
	constraints1(1,1) = TypeParam(1);
	constraints1(2,0) = TypeParam(0);
	constraints1(2,1) = TypeParam(-1);

	constants1(0) = TypeParam(2);
	constants1(1) = TypeParam(2);
	constants1(2) = TypeParam(0);

	// Triangle (0,2) - (-1,4) - (1,4)
	constraints2(0,0) = TypeParam(-2);
	constraints2(0,1) = TypeParam(-1);
	constraints2(1,0) = TypeParam(2);
	constraints2(1,1) = TypeParam(-1);
	constraints2(2,0) = TypeParam(0);
	constraints2(2,1) = TypeParam(1);

	constants2(0) = TypeParam(-2);
	constants2(1) = TypeParam(-2);
	constants2(2) = TypeParam(4);

	SupportFunction<TypeParam> tri1 = SupportFunction<TypeParam>(constraints1, constants1);
	SupportFunction<TypeParam> tri2 = SupportFunction<TypeParam>(constraints2, constants2);

	SupportFunction<TypeParam> res = tri1.unite(tri2);

	// Result directions
	vector_t<TypeParam> vec1 = vector_t<TypeParam>(2);
	vec1(0) = TypeParam(1);
	vec1(1) = TypeParam(0);

	vector_t<TypeParam> vec2 = vector_t<TypeParam>(2);
	vec2(0) = TypeParam(0);
	vec2(1) = TypeParam(1);

	vector_t<TypeParam> vec3 = vector_t<TypeParam>(2);
	vec3(0) = TypeParam(-1);
	vec3(1) = TypeParam(0);

	vector_t<TypeParam> vec4 = vector_t<TypeParam>(2);
	vec4(0) = TypeParam(0);
	vec4(1) = TypeParam(-1);

	EXPECT_LE(TypeParam(1), res.evaluate(vec1).supportValue);
	EXPECT_LE(TypeParam(4), res.evaluate(vec2).supportValue);
	EXPECT_LE(TypeParam(1), res.evaluate(vec3).supportValue);
	EXPECT_LE(TypeParam(0), res.evaluate(vec4).supportValue);

	// Box and open set
	constraints1 = matrix_t<TypeParam>::Zero(4,2);
	constraints1 << 1,0,
					-1,0,
					0,1,
					0,-1;
	constants1 = vector_t<TypeParam>::Zero(4);
	constants1 << 1,1,1,1;

	SupportFunction<TypeParam> psf1 = SupportFunction<TypeParam>(constraints1, constants1);

	constraints2 = matrix_t<TypeParam>::Zero(2,2);
	constraints2 << -1,0,
					0,-1;
	constants2 = vector_t<TypeParam>::Zero(2);
	constants2 << 1,1;

	SupportFunction<TypeParam> psf2 = SupportFunction<TypeParam>(constraints2, constants2);

	SupportFunction<TypeParam> res2 = psf1.unite(psf2);

	EXPECT_EQ(res2.evaluate(vector_t<TypeParam>(constraints1.row(0))).errorCode, SOLUTION::INFTY);
	EXPECT_EQ(res2.evaluate(vector_t<TypeParam>(constraints1.row(2))).errorCode, SOLUTION::INFTY);

}

TYPED_TEST(SupportFunctionTest, contains) {
	SupportFunction<TypeParam> psf1 = SupportFunction<TypeParam>(this->constraints, this->constants);
	vector_t<TypeParam> p = vector_t<TypeParam>::Zero(2);
	EXPECT_TRUE(psf1.contains(Point<TypeParam>({0,0})));
	p << 0,0;
	EXPECT_TRUE(psf1.contains(p));
	EXPECT_TRUE(psf1.contains(Point<TypeParam>({-2,-2})));
	p << -2,-2;
	EXPECT_TRUE(psf1.contains(p));
	EXPECT_TRUE(psf1.contains(Point<TypeParam>({3,3})));
	p << 3,3;
	EXPECT_TRUE(psf1.contains(p));

	TypeParam xCoord = TypeParam(-12)/carl::rationalize<TypeParam>(4.5)-carl::rationalize<TypeParam>(0.0001);
	TypeParam yCoord = TypeParam(4)*xCoord + TypeParam(17)-carl::rationalize<TypeParam>(0.0001);

	EXPECT_TRUE(psf1.contains(Point<TypeParam>({xCoord,yCoord})));
	p << xCoord, yCoord;
	EXPECT_TRUE(psf1.contains(p));

	EXPECT_FALSE(psf1.contains(Point<TypeParam>({xCoord+carl::rationalize<TypeParam>(0.001),yCoord+carl::rationalize<TypeParam>(0.001)})));

	EXPECT_TRUE(this->sfChainComplete.contains(Point<TypeParam>({1,1})));
	EXPECT_FALSE(this->sfChainComplete.contains(Point<TypeParam>({-1,-1})));

	//Plotter<TypeParam>::getInstance().addObject(this->sfChainComplete.vertices());
	//std::cout << "vertices: " << std::endl;
	//for(const auto& vertex : this->sfChainComplete.vertices()) {
	//	std::cout << vertex << std::endl;
	//}
	//Plotter<TypeParam>::getInstance().plot2d();
}

TYPED_TEST(SupportFunctionTest, projection) {
	SupportFunction<TypeParam> psf1 = SupportFunction<TypeParam>(this->constraints, this->constants);
	std::vector<std::size_t> dims;
	dims.push_back(0);

	psf1 = psf1.project(dims);

	vector_t<TypeParam> dir1 = vector_t<TypeParam>::Zero(2);
	dir1(0) = this->constraints(0,0);
	vector_t<TypeParam> dir2 = vector_t<TypeParam>::Zero(2);
	dir2(0) = this->constraints(1,0);
	vector_t<TypeParam> dir3 = vector_t<TypeParam>::Zero(2);
	dir3(0) = this->constraints(2,0);

	EXPECT_EQ(psf1.evaluate(vector_t<TypeParam>(this->constraints.row(0))).supportValue, psf1.evaluate(vector_t<TypeParam>(dir1)).supportValue );
	EXPECT_EQ(psf1.evaluate(vector_t<TypeParam>(this->constraints.row(1))).supportValue, psf1.evaluate(vector_t<TypeParam>(dir2)).supportValue );
	EXPECT_EQ(psf1.evaluate(vector_t<TypeParam>(this->constraints.row(2))).supportValue, psf1.evaluate(vector_t<TypeParam>(dir3)).supportValue );

	EXPECT_EQ(psf1.collectProjections().size(), dims.size());

	SupportFunction<TypeParam> projected = this->sfChainComplete.project(dims);

	dir1 << 1,0;
	dir2 << -1,0;

	EXPECT_EQ(projected.evaluate(vector_t<TypeParam>(dir1)).supportValue, this->sfChainComplete.evaluate(dir1).supportValue );
	EXPECT_EQ(projected.evaluate(vector_t<TypeParam>(dir2)).supportValue, this->sfChainComplete.evaluate(dir2).supportValue );

	EXPECT_EQ(projected.collectProjections().size(), dims.size());

	// Todo: Test projection where dimensions are exchanged, i.e. {1,0}
}

TYPED_TEST(SupportFunctionTest, plotting) {
	std::vector<std::size_t> projectionDimensions;
	projectionDimensions.push_back(0);
	projectionDimensions.push_back(1);

	Plotter<TypeParam>::getInstance().addObject(this->sfChainComplete.project(projectionDimensions).vertices());
}

TYPED_TEST(SupportFunctionTest, dotRep) {
	SupportFunction<TypeParam> psf1Assigned = this->sfChainComplete;
	matrix_t<TypeParam> vec1 = matrix_t<TypeParam>(2,1);
	vec1 << 1,0;
	psf1Assigned.evaluate(vec1);
	//std::cout << psf1Assigned.getDotRepresentation();
}
