/*
 * IntersectOp.h
 *
 * A RootGrowNode that represents a intersection operation in the tree of operations representing a SupportFunction.
 *
 * @author Stefan Schupp
 * @author Phillip Tse
 */

#pragma once

#include "RootGrowNode.h"
#include "SupportFunctionNew.h"

namespace hypro {

//Forward Declaration
template<typename Number, typename Converter, typename Setting>
class SupportFunctionNewT;	

//Specialized subclass for sums as example of a binary operator
template<typename Number, typename Converter, typename Setting>
class IntersectOp : public RootGrowNode<Number,Setting> {

  private:
	
	////// General Interface

	SFNEW_TYPE type = SFNEW_TYPE::INTERSECTOP;
	unsigned originCount = 2;
	std::size_t mDimension = 0;
	//PointerVec mChildren;

	////// Special members of this class

  public:

	////// Constructors & Destructors

	IntersectOp() = delete;

	IntersectOp(const SupportFunctionNewT<Number,Converter,Setting>& lhs, const SupportFunctionNewT<Number,Converter,Setting>& rhs) : mDimension(lhs.dimension()) { 
		assert(lhs.dimension() == rhs.dimension());
		lhs.addOperation(this, std::vector<SupportFunctionNewT<Number,Converter,Setting>>{rhs}); 
	}

	IntersectOp(const SupportFunctionNewT<Number,Converter,Setting>& lhs, const std::vector<SupportFunctionNewT<Number,Converter,Setting>>& rhs) : mDimension(lhs.dimension()) { 
		for(const auto& sf : rhs){
			assert(lhs.dimension() == sf.dimension());
		}
		lhs.addOperation(this, rhs); 
	}

	~IntersectOp(){}

	////// Getters and Setters

	SFNEW_TYPE getType() const override { return type; }
	unsigned getOriginCount() const { return originCount; }
	std::size_t getDimension() const { return mDimension; }

	////// RootGrowNode Interface

	//Does nothing
	matrix_t<Number> transform(const matrix_t<Number>& param) const {
		return param;
	}

	//Should not be reached
	std::vector<EvaluationResult<Number>> compute(const matrix_t<Number>& , bool ) const { 
		assert(false && "IntersectOp::compute should never be called"); 
		return std::vector<EvaluationResult<Number>>();
	}

	//Given two result vecs, sum them coefficientwise
	std::vector<EvaluationResult<Number>> aggregate(std::vector<std::vector<EvaluationResult<Number>>>& resultStackBack, const matrix_t<Number>& ) const {
		
		TRACE("hypro.representations.supportFunction", ": INTERSECT, accumulate results.")
		assert(resultStackBack.size() >= 2);
		std::vector<EvaluationResult<Number>> accumulatedResult;

		// in case one of the results is infeasible (the set is empty), return this result.
		for(const auto& res : resultStackBack){
			if(res.begin()->errorCode == SOLUTION::INFEAS){
				accumulatedResult = res;
				return accumulatedResult;
			}
		}
		
		//For all coordinates of all results in resultStackBack, take the iteratively look for the smallest evaluation result
		for ( unsigned i = 0; i < resultStackBack.at(0).size(); ++i ) {		
			
			//Init r with some large numbers or else the loop doesn't start
			EvaluationResult<Number> r;
			r.errorCode = SOLUTION::FEAS;
			r.supportValue = Number(10000000);
			r.optimumValue = vector_t<Number>::Zero(resultStackBack.at(0).size());
			for(int j = 0; j < r.optimumValue.rows(); ++j){
				r.optimumValue[j] = Number(10000000);
			}

			//actual aggregation loop
			for(const auto& res : resultStackBack){
				assert(res[i].errorCode != SOLUTION::INFEAS);
				if(r.errorCode == SOLUTION::FEAS && res[i].errorCode == SOLUTION::INFTY){
					r.supportValue = res[i].supportValue;
					r.optimumValue = res[i].optimumValue;
				} else {
					if(r.errorCode == SOLUTION::FEAS && res[i].errorCode == SOLUTION::FEAS){
						r.supportValue = res[i].supportValue < r.supportValue ? res[i].supportValue : r.supportValue; 
						r.optimumValue = res[i].optimumValue < r.optimumValue ? res[i].optimumValue : r.optimumValue; 	
					} 
				}
			}
			accumulatedResult.emplace_back(r);
		}
		return accumulatedResult;
	}

	/* Problem: Check emptiness of intersection of n convex objects
	 * Solution 1: Template Evaluation
	 *	- For each template direction, evaluate every object except one in that direction and the one into the -direction
	 *	- If -direction is bigger in absolute value, then intersection not empty
	 *	- Needs n*t*LP worst time, since every evaluate needs to solve a LP
	 *	- Can stop early if direction directly found -> best time possible time is n*LP
	 *	- Scales bad in dimension
	 * Solution 2: Template Evaluation +
	 *  - Like Solution 1, but parallel evaluation of 2 directions
	 *	- Needs 0.5*n*t*LP time
	 * Solution 3: Center of Gravity (COG)
	 *  - For each object, generate a point within the object
	 *		- Either on a halfspace (needs an LP, how to choose which halfspace?)
	 *		- Or the center of gravity ob the object (needs to compute vertices, then averaging -> expensive as hell)
	 *	- Compute the COG of all points and determine the direction as the vector from the COG to the points
	 *	- Evaluate all directions -> but how to determine emptiness from that?
	 *	- Needs n*LP to generate points and n*LPs for evaluation
	 */
	bool empty(const std::vector<bool>& childrenEmpty) const {

		//Current implementation uses Solution 1: template evaluation.

		//Quick check
		for(auto child : childrenEmpty){
			if(child) return true;
		}

		//First: select the most negative object
		vector_t<Number> allNegativeDir = -1*vector_t<Number>::Ones(mDimension);
		std::vector<SupportFunctionNewT<Number,Converter,Setting>> sfChildren;
		std::size_t indexOfMostNegativeChild = 0;
		Number mostNegativeEvalValue = Number(100000000);
		for(int i = 0; i < this->getChildren().size(); ++i){
			sfChildren.emplace_back(SupportFunctionNewT<Number,Converter,Setting>(this->getChildren().at(i)));
			Number childEvalRes = sfChildren.back().evaluate(allNegativeDir,false).supportValue;
			indexOfMostNegativeChild = childEvalRes < mostNegativeEvalValue ? sfChildren.size()-1 : indexOfMostNegativeChild;
			mostNegativeEvalValue = childEvalRes < mostNegativeEvalValue ? childEvalRes : mostNegativeEvalValue;
		}
		assert(indexOfMostNegativeChild < sfChildren.size());

		//Erase most negative child from sfChildren
		SupportFunctionNewT<Number,Converter,Setting> mostNegative = sfChildren.at(indexOfMostNegativeChild);
		sfChildren.erase(sfChildren.begin() + indexOfMostNegativeChild);

		std::vector<vector_t<Number>> directions = computeTemplate<Number>(mDimension, defaultTemplateDirectionCount);

		for(const auto& direction : directions){
			
			//Determine reverse direction for mostNegative
			Number mostNegEvalRes = mostNegative.evaluate(-direction, false).supportValue;

			//Fill evalResults
			for(const auto& child : sfChildren){
				Number childEvalRes = child.evaluate(direction, false).supportValue;
				if(childEvalRes < -mostNegEvalRes) return true;
			}
		}
		return false;
	}

	//Select smallest supremum of given suprema
	Point<Number> supremumPoint(std::vector<Point<Number>>& points) const {
		Point<Number> smallestInftyNorm = points.front();
		for(auto& p : points){
			if(p.dimension() == 0) return p;
			smallestInftyNorm = Point<Number>::inftyNorm(smallestInftyNorm) < Point<Number>::inftyNorm(p) ? smallestInftyNorm : p;
		}
		return smallestInftyNorm;
	}

	//Only return true if all children contained the point before	
	bool contains(const std::vector<bool>& v) const {
		for(auto containedInChild : v){
			if(!containedInChild) return false;
		}
		return true;
	}
};

} //namespace hypro

