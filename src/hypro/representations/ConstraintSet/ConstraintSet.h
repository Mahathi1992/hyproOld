#pragma once
/*
#ifndef INCL_FROM_GOHEADER
	static_assert(false, "This file may only be included indirectly by GeometricObject.h");
#endif
*/
#include "../types.h"
#include "../helperMethods/isBox.h"
#include "../../datastructures/Halfspace.h"
#include "../../datastructures/Point.h"
#include "../../util/linearOptimization/Optimizer.h"

namespace hypro {

struct ConstraintSetSettings {

};

/**
 * @brief      A class representing a plain constraint set.
 * @details    A constraint set in this case is a datastructure similar to a H-Polytope but without any functionality. The idea is just to
 * 			   have a container for data received from the parser or as an intermediate type.
 * @tparam     Number     The used number type.
 * @tparam     Converter  The used converter.
 * \ingroup geoState @{
 */
template <typename Number, typename S=ConstraintSetSettings>
class ConstraintSetT {
  private:
  public:
	/***************************************************************************
	 * Members
	 **************************************************************************/

	typedef S Settings;
	typedef Number NumberType;
  protected:
    matrix_t<Number> mConstraints; /*!< Matrix describing the linear constraints.*/
    vector_t<Number> mConstants; /*!< Vector describing the constant parts for the respective constraints.*/
	mutable TRIBOOL mIsBox = TRIBOOL::NSET; /*<  Cache to store whether the constraints are axis-aligned. */

  public:
	/***************************************************************************
	 * Constructors
	 **************************************************************************/

	/**
	 * @brief      Creates an empty constraintSet.
	 * @details   The empty constraintSet is represented by a zero-dimensional point pair.
	 */
	ConstraintSetT()
		: mConstraints(matrix_t<Number>::Zero(0,0))
		, mConstants(vector_t<Number>::Zero(0))
	{}

	/**
	 * @brief      Copy constructor.
	 * @param[in]  orig  The original.
	 */
	ConstraintSetT( const ConstraintSetT& orig )
		: mConstraints(orig.matrix())
		, mConstants(orig.vector())
	{}

	/**
	 * @brief      Move constructor.
	 * @param[in]  orig  The original.
	 */
	ConstraintSetT( ConstraintSetT&& orig ) = default;

	/**
	 * @brief Constructor from a matrix and a vector.
	 * @details Constructs a constraintSet assuming each row of the matrix is the normal to a hyperplane and its corresponding
	 * entry in the given vector is the offset.
	 * @param _constraints A matrix representing the constraint normals.
	 * @param _constants A vector representing the offsets of the corresponting hyperplane.
	 */
	ConstraintSetT( const matrix_t<Number>& _constraints, const vector_t<Number>& _constants ) :
		mConstraints(_constraints),
		mConstants(_constants)
	{}

	/**
	 * @brief Destructor.
	 */
	~ConstraintSetT() {}

	/***************************************************************************
	 * Getters & setters
	 **************************************************************************/

	const matrix_t<Number>& matrix() const { return mConstraints; }
	const vector_t<Number>& vector() const { return mConstants; }

	matrix_t<Number>& rMatrix() { return mConstraints; }
	vector_t<Number>& rVector() { return mConstants; }

	bool isAxisAligned() const {
		if(mIsBox == TRIBOOL::NSET) {
			mIsBox = boost::get<0>(isBox(mConstraints, mConstants)) == true ? TRIBOOL::TRUE : TRIBOOL::FALSE;
		}
		return mIsBox == TRIBOOL::TRUE;
	}

	/**
	 * @brief Determines if the current constraintSet is empty.
	 * @details The method invokes the linear optimizer to check for the existence of a solution.
	 * @return True, if the represented set is empty, False otherwise.
	 */
	bool empty() const {
		return !Optimizer<Number>(mConstraints,mConstants).checkConsistency();
	}

	void addConstraint(const vector_t<Number>& normal, Number offset) {
		if(mConstraints.cols() == 0) {
			mConstraints = matrix_t<Number>(normal);
			mConstants = vector_t<Number>::Zero(1);
			mConstants << offset;
		} else {
			assert(mConstraints.cols() == normal.rows());
			mConstraints.conservativeResize(mConstraints.rows()+1, mConstraints.cols());
			mConstants.conservativeResize(mConstraints.rows()+1);

			mConstraints.row(mConstraints.rows()-1) = normal;
			mConstants(mConstants.rows()-1) = offset;
		}
		mIsBox = TRIBOOL::NSET;
	}


	/**
	 * @brief Getter for a vertex-representation of the current constraintSet.
	 * @details Not implemented.
	 * @return An empty vector of points.
	 */
	std::vector<Point<Number>> vertices( const matrix_t<Number>& = matrix_t<Number>::Zero(0,0) ) const { return std::vector<Point<Number>>(); }

	/**
	 * @brief Checks if two constraintSets are equal.
	 * @param b1 Contains the first constraintSet.
	 * @param b2 Contains the second constraintSet.
	 * @return True, if they are equal.
	 */
	friend bool operator==( const ConstraintSetT<Number,S>& b1, const ConstraintSetT<Number,S>& b2 ) {
		if ( b1.dimension() != b2.dimension() ) {
			return false;
		}
		return ( b1.matrix() == b2.matrix() && b1.vector() == b2.vector() );
	}

	/**
	 * @brief Determines inequality of two constraintSets.
	 * @param b1 A constraintSet.
	 * @param b2 A constraintSet.
	 * @return False, if both constraintSets are equal.
	 */
	friend bool operator!=( const ConstraintSetT<Number,S>& b1, const ConstraintSetT<Number,S>& b2 ) { return !( b1 == b2 ); }

	/**
	 * @brief Assignment operator.
	 * @param rhs A constraintSet.
	 */
	ConstraintSetT<Number,S>& operator=( const ConstraintSetT<Number,S>& rhs ) = default;

	/**
	 * @brief Move assignment operator.
	 * @param rhs A constraintSet.
	 */
	ConstraintSetT<Number,S>& operator=(ConstraintSetT<Number,S>&& rhs) = default;

	/**
	 * @brief Outstream operator.
	 * @param ostr Outstream.
	 * @param b A constraintSet.
	 */
#ifdef HYPRO_LOGGING
	friend std::ostream& operator<<( std::ostream& ostr, const ConstraintSetT<Number,S>& b ) {
		ostr << "Matrix: " << b.matrix() << ", Vector: " << b.vector();
#else
	friend std::ostream& operator<<( std::ostream& ostr, const ConstraintSetT<Number,S>& ) {
#endif
		return ostr;
	}

	/***************************************************************************
	 * General interface
	 **************************************************************************/

	 /**
	  * @brief      Getter for the space dimension.
	  * @return     The dimension of the space.
	  */
	std::size_t dimension() const { return std::size_t(mConstraints.cols()); }

	/**
	 * @brief      Removes redundancy (part of the general interface. Does nothing for constraintSets.)
	 */
	void removeRedundancy() {}

	/**
	 * @brief      Storage size determination.
	 * @return     Size of the required memory.
	 */
	std::size_t size() const { return 1; }

	static representation_name type() { return representation_name::constraint_set; }

	/**
	 * @brief      Function to reduce the number representation (over-approximate).
	 * @details    The function tries to reduce the size of each interval boundary in case it is larger than some specified limit.
	 * This is done by outward rounding.
	 *
	 * @param[in]  limit      The limit
	 */
	const ConstraintSetT<Number,S>& reduceNumberRepresentation(unsigned) { return *this; }

	std::pair<CONTAINMENT, ConstraintSetT> satisfiesHalfspace( const Halfspace<Number>&  ) const { return std::make_pair(CONTAINMENT::NO,*this); }
	std::pair<CONTAINMENT, ConstraintSetT> satisfiesHalfspaces( const matrix_t<Number>& , const vector_t<Number>&  ) const { return std::make_pair(CONTAINMENT::NO,*this); }

	ConstraintSetT<Number,S> project(const std::vector<std::size_t>& ) const { return *this; }

	ConstraintSetT<Number,S> linearTransformation( const matrix_t<Number>& ) const { return *this; }
	ConstraintSetT<Number,S> affineTransformation( const matrix_t<Number>& , const vector_t<Number>& ) const { return *this; }
	ConstraintSetT<Number,S> minkowskiSum( const ConstraintSetT<Number,S>& ) const { return *this; }

	/**
	 * @brief      Computes the intersection of two constraintSets.
	 * @param[in]  rhs   The right hand side constraintSet.
	 * @return     The resulting constraintSet.
	 */
	ConstraintSetT<Number,S> intersect( const ConstraintSetT<Number,S>& ) const { return *this; }

	ConstraintSetT<Number,S> intersectHalfspace( const Halfspace<Number>& ) const { return *this; }
	ConstraintSetT<Number,S> intersectHalfspaces( const matrix_t<Number>& , const vector_t<Number>& ) const { return *this; }
	bool contains( const Point<Number>& ) const { return true; }

	/**
	 * @brief      Containment check for a constraintSet.
	 * @param[in]  constraintSet   The constraintSet.
	 * @return     True, if the given constraintSet is contained in the current constraintSet, false otherwise.
	 */
	bool contains( const ConstraintSetT<Number,S>& ) const { assert(false); return true; }

	/**
	 * @brief      Computes the union of two constraintSets.
	 * @param[in]  rhs   The right hand side constraintSet.
	 * @return     The resulting constraintSet.
	 */
	ConstraintSetT<Number,S> unite( const ConstraintSetT<Number,S>& ) const { return *this; }

	/**
	 * @brief      Computes the union of the current constraintSet with a set of constraintSets.
	 * @param[in]  constraintSets  The constraintSets.
	 * @return     The resulting constraintSet.
	 */
	static ConstraintSetT<Number,S> unite( const std::vector<ConstraintSetT<Number,S>>& ) { return ConstraintSetT<Number,S>(); }

	/**
	 * @brief      Does nothing as a ConstraintSet is only a container.
	 */
	void reduceRepresentation() {}

	/**
	 * @brief      Makes this constraintSet the empty constraintSet.
	 */
	//void clear() { return *this; }
	void clear() {}

};
/** @} */



/**
 * @brief      Conversion function for different number types.
 * @param[in]  in         The input constraintSet.
 * @tparam     From       The current number type.
 * @tparam     To         The number type the constraintSet is to be converted to.
 * @tparam     Converter  The passed representation converter.
 * @return     The resulting constraintSet.
 */
template<typename From, typename To, typename S>
ConstraintSetT<To,S> convert(const ConstraintSetT<From,S>& in) {
	return ConstraintSetT<To,S>( convert<From,To>(in.matrix()), convert<From,To>(in.vector()));
}

} // namespace hypro
