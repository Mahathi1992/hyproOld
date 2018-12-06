/*
 * RootGrowNode.h
 *
 * An abstract class representing a node in a tree where a new node is set above the root,
 * thus making the tree grow at its root.
 * Each node knows and owns its children and has a weak_ptr to itself.
 * Every type of operation and leaf must inherit from this base class, since only then
 * this unifying base class can be used for calculations.
 *
 * @author Stefan Schupp
 * @author Phillip Tse
 */

#pragma once 

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <type_traits>
#include <tuple>
#include <cassert>
#include "../../types.h"
#include "../../util/linearOptimization/EvaluationResult.h"
#include "LinTrafoParameters.h"

namespace hypro {

//Type of nodes. Needed to fast determine which node subclass is actually calling a function. 
enum SFNEW_TYPE { NODE = 0, LEAF, TRAFO, SCALEOP, PROJECTOP, SUMOP, INTERSECTOP, UNIONOP};

template<typename Number, typename Setting>
class RootGrowNode {

  public:

  	using PointerVec = std::vector<std::shared_ptr<RootGrowNode<Number,Setting>>>;

  protected:

	////// Members

	SFNEW_TYPE mType = NODE;							//NONE since RootGrowNode should later be an abstract class
	unsigned originCount = 0;							//Least amount of children needed to function properly
	PointerVec mChildren = PointerVec();				//vector of all current children
	
  public:

	////// Constructors

	RootGrowNode() {}
	virtual ~RootGrowNode(){}	

	////// Getters and Setters

	virtual SFNEW_TYPE getType() const { return mType; }
	virtual unsigned getOriginCount() const { return originCount; }
	virtual PointerVec getChildren() const { return mChildren; }
	
	////// Modifiers

	void addToChildren(const std::shared_ptr<RootGrowNode>& rhs){ mChildren.push_back(rhs); }
	void clearChildren(){ mChildren.clear(); }

	////// Displaying

	friend std::ostream& operator<<(std::ostream& ostr, const RootGrowNode& r){
		ostr << "current address: " << &r << " type: " << r.getType() << " children types(address): [";
		for(auto c : r.getChildren()){
			ostr << c->getType() << "(" << &c << ")" << ",";
		}
		ostr << "]" << std::endl;
		for(auto c : r.getChildren()){
			ostr << *c << std::endl;
		}
		return ostr;
	}

	////// Traversal

	// The needed functions for evaluate. Virtual s.t. they can be implemented in the Operation/Leaf classes
	// Three functions are needed: transform, compute and aggregate.
	// - transform will be called by a node for every child
	// - compute will only be called once by leaf nodes
	// - aggregate will only be called once by all non leaf nodes
	//
	// NOTE: 	All functions used as the transform must have the signature A name(A param)
	//			All functions used as the compute must have the signature B name(A param)
	//			All functions used as the aggregate must have the signature std::vector<B> name(B param)

	//For everyone - transform
	virtual matrix_t<Number> transform(const matrix_t<Number>& param) const = 0;

	//For leaves - compute
	virtual std::vector<EvaluationResult<Number>> compute(const matrix_t<Number>& param, bool useExact) const = 0; 
	
	//For operations - aggregate
	virtual std::vector<EvaluationResult<Number>> aggregate(std::vector<std::vector<EvaluationResult<Number>>>& resultStackBack, const matrix_t<Number>& currentParam) const = 0;

	////// Functions for SupportFunctionNew
	
	virtual bool empty() const { assert(false && "This should only be called by Leaf\n"); return false; }
	virtual Point<Number> supremumPoint() const { assert(false && "supremumPoint() should only be called by Leaf\n"); return Point<Number>(); }
	virtual Point<Number> supremumPoint(std::vector<Point<Number>>& points) const { assert(false && "supremumPoint(vector) should only be called by operations\n"); return Point<Number>(); }
};

} //namespace hypro