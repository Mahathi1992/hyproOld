#pragma once
#include "../../logging/Logger.h"
#include "helperFunctions.h"

#include <ClpSimplex.hpp>
#include <CoinPackedMatrix.hpp>
#include <carl/core/Relation.h>

namespace hypro {

struct clp_context {
	ClpSimplex lp;
	CoinPackedMatrix matrix;
	double* mUpperBounds;
	double* mLowerBounds;
	bool arraysCreated = false;			   // true if array variables have been created.
	mutable bool mInitialized = false;	 // true if lp instance has been created.
	mutable bool mConstraintsSet = false;  // true if lp instance exists, arrays have been set and the lp instance is set up with the current constraints.

	bool operator==( const clp_context& rhs ) {
		// TODO
	}

	void createArrays( unsigned ) {
	}

	void deleteArrays() {
	}

	void createLPInstance() {
	}

	void deleteLPInstance() {
	}

	~clp_context() {
	}

	template <typename Number>
	void setMatrix( const matrix_t<Number>& constraints, const vector_t<Number>& constants, const std::vector<carl::Relation>& relations ) {
		if ( !arraysCreated ) {
			matrix = detail::createMatrix( constraints );
			mUpperBounds = new double[constants.cols()];
			mLowerBounds = new double[constants.cols()];
			for ( int i = 0; i < constants.rows(); ++i ) {
				switch ( relations[i] ) {
					case carl::Relation::LEQ:
						mUpperBounds[i] = carl::toDouble( constants( i ) );
						mLowerBounds[i] = -COIN_DBL_MAX;
						break;
					case carl::Relation::GEQ:
						mUpperBounds[i] = COIN_DBL_MAX;
						mLowerBounds[i] = carl::toDouble( constants( i ) );
						break;
					case carl::Relation::EQ:
						mUpperBounds[i] = carl::toDouble( constants( i ) );
						mLowerBounds[i] = mUpperBounds[i];
						break;
					default:
						// glpk cannot handle strict inequalities.
						assert( false );
						std::cout << "This should not happen." << std::endl;
				}
			}
			arraysCreated = true;
			mInitialized = false;
		}
	}

	template <typename Number>
	void updateConstraints( const matrix_t<Number>& constraints, const vector_t<Number>& constants, const std::vector<carl::Relation>& relations, bool maximize ) {
		assert( maximize && "CLP can only maximize." );
		// initialize problem + bounds
		setMatrix( constraints, constants, relations );
	}
};
}  // namespace hypro
