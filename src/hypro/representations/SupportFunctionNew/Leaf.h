/*
 * Leaf.h
 *
 * A RootGrowNode that represents a leaf node in the tree of operations representing a SupportFunction.
 * Each tree has a pointer to a Representation: Box, H-Polytope, etc.
 *
 * @author Stefan Schupp
 * @author Phillip Tse
 */

#pragma once

#include "../../types.h"

namespace hypro {

template<typename Number, typename Converter, typename Setting>
class RootGrowNode;

//A data struct for leaves, containing all needed info to construct a leaf from it
//The infos are the address to the representation as an actual number and the type of the representation.
struct LeafData : public RGNData {
	uintptr_t addressToRep;
	representation_name typeOfRep;
	bool isNotRedundant;
	LeafData(){};
	LeafData(const uintptr_t addr, const representation_name rep, const bool redundancy) 
		: addressToRep(addr), typeOfRep(rep), isNotRedundant(redundancy)
	{ }
};

//Subclass of RootGrowNode, is a node with a representation of a state
template<typename Number, typename Converter, typename Setting, typename Representation> 
class Leaf : public RootGrowNode<Number,Converter,Setting> {
  
  private:
	
	////// General Interface

	SFNEW_TYPE type = SFNEW_TYPE::LEAF;
	unsigned originCount = 0;				//A leaf cannot have children
	std::size_t mDimension = 0;

	////// Members for this class

	std::shared_ptr<Representation> rep;

	mutable bool isNotRedundant = false; 	//A flag that tells us if removeRedundancy() has been used on the representation
	
  public:

	////// Constructors & Destructors

	Leaf() : mDimension(std::size_t(0)), rep(nullptr) {}

	Leaf(Representation& r) : mDimension(r.dimension()), rep(std::make_shared<Representation>(r)) {}

	Leaf(const std::shared_ptr<Representation>& ptr) : mDimension(ptr->dimension()), rep(ptr) {}

	Leaf(const LeafData& d){
		assert(Representation::type() == d.typeOfRep);
		rep = std::move(std::make_shared<Representation>(*(reinterpret_cast<Representation*>(d.addressToRep))));
		assert(rep != nullptr);
		mDimension = rep->dimension();
		isNotRedundant = d.isNotRedundant;
	}

	~Leaf(){} 

	////// Getters & Setters

	SFNEW_TYPE getType() const { return type; }
	unsigned getOriginCount() const { return originCount; }
	std::size_t getDimension() const { return mDimension; }
	Representation* getRepresentation() const { return rep.get(); }
	bool isRedundant() const { return !isNotRedundant; }
	RGNData* getData() const { return nullptr; }
	LeafData getLeafData() const {
		return LeafData(reinterpret_cast<uintptr_t>(rep.get()), rep->type(), isNotRedundant);
	}
	void setDimension(const std::size_t d) { mDimension = d; }

	////// Displaying

	void print(std::ostream& ostr) const {
		ostr << "RootGrowNode address: " << this << " own type: " << this->getType() << " children types(address): [";
		for(auto c : this->getChildren()){
			ostr << c->getType() << "(" << &(*c) << ")" << ",";
		}
		ostr << "]" << std::endl;
		ostr << "Leaf representation: " << *rep << std::endl;
		for(auto c : this->getChildren()){
			ostr << *c;
		}
	}
	
	////// RootGrowNode Interface

	//Leaves usually do not transform
	matrix_t<Number> transform(const matrix_t<Number>&) const {
		assert(false && "Leaf::transform should never be called\n"); 
		return matrix_t<Number>::Zero(1,1); 
	}

	//Evaluate leaf via multiEvaluate function of the representation
	std::vector<EvaluationResult<Number>> compute(const matrix_t<Number>& param, bool useExact) const { 

		assert(rep != nullptr);
		
		//Optimization: Remove redundancy only when rep is being evaluated the first time
		if(!isNotRedundant){
			rep->removeRedundancy();
			isNotRedundant = true;
		}
		
		//Optimization: If rep is only a halfspace, save yourself an expensive evaluation: Used in guard / invariant satisfaction
		auto repMatrix = rep->matrix();
		if(repMatrix.rows() == 1){
			COUNT("Single constraint evaluate.");
			auto repVector = rep->vector();
			std::vector<EvaluationResult<Number>> res;
			for(int i = 0; i < param.rows(); i++){
				// TODO: extend check to linear dependence. Here temporarily sufficient as we will initialize and evaluate with the plane normals, which should be the same vectors.
				if(param.row(i) == repMatrix.row(0)){ 
					// The vectors are EXACTLY the same -> to find point on plane and to avoid squareroot computations, return vector
					// which contains zeroes except of the position with the first non-zero coeff, which is set to the stored distance.
					vector_t<Number> pointOnPlane = vector_t<Number>::Zero(param.cols());
					unsigned j = 0;
					while(j < param.cols() && param(i,j) == 0){
						++j;
					}
					// TODO: Check if this is correct -> we evaluated something weird with a weird direction (both zero vertors).
					if(param(i,j) == 0){
						res.emplace_back(EvaluationResult<Number>());
					}
					pointOnPlane(i) = repVector(0);
					res.emplace_back(EvaluationResult<Number>(repVector(0), pointOnPlane, SOLUTION::FEAS));
				} else {
					res.emplace_back(EvaluationResult<Number>(1,SOLUTION::INFTY));
				}
			}
			return res;
		}

		//If no optimizations could be used
		COUNT("Multi constraint evaluate.");
		return rep->multiEvaluate(param, useExact);
	}

	//Leaves do not aggregate
	std::vector<EvaluationResult<Number>> aggregate(std::vector<std::vector<EvaluationResult<Number>>>& , const matrix_t<Number>& ) const {
		assert(false && "Leaf::aggregate should never be called\n");
		return std::vector<EvaluationResult<Number>>();
	}

	//Leaves call empty function of the representation
	bool empty() const { 
		return rep->empty(); 
	}

	//Compute the point that is the supremum of the representation
	Point<Number> supremumPoint() const { 
		Point<Number> max = Point<Number>::Zero(rep->dimension());
		for(auto& point : rep->vertices()){
			max = Point<Number>::inftyNorm(max) > Point<Number>::inftyNorm(point) ? max : point;
		}
		return max;
	}

	//Calls contains function of given representation
	bool contains(const vector_t<Number>& point) const { 
		return rep->contains(Point<Number>(point)); 
	}

	//matrix_t<Number> getMatrix() const { return rep->matrix(); }

};

} //namespace hypro
