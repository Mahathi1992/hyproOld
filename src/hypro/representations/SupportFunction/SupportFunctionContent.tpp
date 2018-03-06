/**
 *	File which holds the implementation of support functions.
 * @file SupportFunctionContent.tpp
 * @author Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *
 * @since	2015-02-20
 * @version	2015-02-27
 */

#include "SupportFunctionContent.h"

namespace hypro {

template <typename Number>
SupportFunctionContent<Number>::SupportFunctionContent( const SupportFunctionContent<Number>& _orig )
	: mType( _orig.type() ), mDimension( _orig.dimension(), mDepth( _orig.depth()) ) {
	assert(_orig.checkTreeValidity());
	//std::cout << "Copy constructor, this->type:" << mType << std::endl;
	switch ( mType ) {
		case SF_TYPE::ELLIPSOID: {
            mEllipsoid = new EllipsoidSupportFunction<Number>(*_orig.ellipsoid());
            break;
        }
		case SF_TYPE::INFTY_BALL:
		case SF_TYPE::TWO_BALL: {
			mBall = new BallSupportFunction<Number>(*_orig.ball());
			break;
		}
		case SF_TYPE::INTERSECT: {
			mIntersectionParameters = new intersectionContent<Number>(*_orig.intersectionParameters());
			break;
		}
		case SF_TYPE::LINTRAFO: {
			mLinearTrafoParameters = new trafoContent<Number,SupportFunctionContentSetting>(*_orig.linearTrafoParameters());
			break;
		}
		case SF_TYPE::POLY: {
			mPolytope = new PolytopeSupportFunction<Number,PolytopeSupportFunctionSetting>(*_orig.polytope());
			break;
		}
		case SF_TYPE::PROJECTION: {
			mProjectionParameters = new projectionContent<Number>(*_orig.projectionParameters());
			break;
		}
		case SF_TYPE::SCALE: {
			mScaleParameters = new scaleContent<Number>(*_orig.scaleParameters());
			break;
		}
		case SF_TYPE::SUM: {
			mSummands = new sumContent<Number>(*_orig.summands());
			break;
		}
		case SF_TYPE::UNITE: {
			mUnionParameters = new unionContent<Number>(*_orig.unionParameters());
			break;
		}
		case SF_TYPE::NONE: {
			std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
			assert(false);
		}
		default:
			assert( false );
	}
	assert(checkTreeValidity());
}

template <typename Number>
SupportFunctionContent<Number>::SupportFunctionContent( const matrix_t<Number>& _shapeMatrix, SF_TYPE _type ) {
    	switch ( _type ) {
		case SF_TYPE::ELLIPSOID: {
			mEllipsoid = new EllipsoidSupportFunction<Number>( _shapeMatrix);
			mType = SF_TYPE::ELLIPSOID;
			mDimension = _shapeMatrix.cols();
			mDepth = 0;
			mOperationCount = 1;
			break;
		}
		default:
			assert( false );
	}
}

template <typename Number>
SupportFunctionContent<Number>::SupportFunctionContent( Number _radius, unsigned dimension, SF_TYPE _type ) {
	switch ( _type ) {
		case SF_TYPE::INFTY_BALL:
		case SF_TYPE::TWO_BALL: {
			mBall = new BallSupportFunction<Number>( _radius, _type );
			mType = _type;
			ball()->setDimension(dimension);
			mDimension = dimension;
			mDepth = 0;
			mOperationCount = 0;
			break;
		}
		default:
			assert( false );
	}
}

template <typename Number>
SupportFunctionContent<Number>::SupportFunctionContent( const matrix_t<Number> &_directions, const vector_t<Number> &_distances,
										  SF_TYPE _type ) {
	switch ( _type ) {
		case SF_TYPE::POLY: {
			mPolytope = new PolytopeSupportFunction<Number,PolytopeSupportFunctionSetting>( _directions, _distances );
			mType = SF_TYPE::POLY;
			mDimension = std::size_t(_directions.cols());
			mDepth = 0;
			mOperationCount = 0;
			break;
		}
		default:
			assert( false );
	}
}

template <typename Number>
SupportFunctionContent<Number>::SupportFunctionContent( const std::vector<Halfspace<Number>>& _planes, SF_TYPE _type ) {
	switch ( _type ) {
		case SF_TYPE::POLY: {
			mPolytope = new PolytopeSupportFunction<Number,PolytopeSupportFunctionSetting>( _planes );
			mType = SF_TYPE::POLY;
			mDimension = polytope()->dimension();
			mDepth = 0;
			mOperationCount = 0;
			break;
		}
		default:
			assert( false );
	}
}

template<typename Number>
SupportFunctionContent<Number>::SupportFunctionContent( const std::vector<Point<Number>>& _points, SF_TYPE _type ) {
	switch ( _type ) {
		case SF_TYPE::POLY: {
			mPolytope = new PolytopeSupportFunction<Number,PolytopeSupportFunctionSetting>( _points );
			mType = SF_TYPE::POLY;
			mDimension = polytope()->dimension();
			mDepth = 0;
			mOperationCount = 0;
			break;
		}
		default:
			assert( false );
	}
}

template <typename Number>
SupportFunctionContent<Number>::SupportFunctionContent( const std::shared_ptr<SupportFunctionContent<Number>>& _lhs,
										 const std::shared_ptr<SupportFunctionContent<Number>>& _rhs, SF_TYPE _type ) {
	// assert(_lhs.dimension() == _rhs.dimension());
	assert(_lhs->checkTreeValidity());
	assert(_rhs->checkTreeValidity());
	switch ( _type ) {
		case SF_TYPE::SUM: {
			mSummands = new sumContent<Number>( _lhs, _rhs );
			mType = SF_TYPE::SUM;
			mDimension = _lhs->dimension();
			assert( _lhs->type() == summands()->lhs->type() && _rhs->type() == summands()->rhs->type() );
			if (_rhs->depth() > _lhs->depth()){
				mDepth = _rhs->depth();
			} else {
				mDepth = _lhs->depth();
			}
			mOperationCount = _rhs->operationCount() + _lhs->operationCount()+1;
			break;
		}
		case SF_TYPE::UNITE: {
			mUnionParameters = new unionContent<Number>( _lhs, _rhs );
			mType = SF_TYPE::UNITE;
			mDimension = _lhs->dimension();
			if (_rhs->depth() > _lhs->depth()){
				mDepth = _rhs->depth();
			} else {
				mDepth = _lhs->depth();
			}
			mOperationCount = _rhs->operationCount() + _lhs->operationCount()+1;
			break;
		}
		case SF_TYPE::INTERSECT: {
			mIntersectionParameters = new intersectionContent<Number>( _lhs, _rhs );
			mType = SF_TYPE::INTERSECT;
			mDimension = _lhs->dimension();
			if (_rhs->depth() > _lhs->depth()){
				mDepth = _rhs->depth();
			} else {
				mDepth = _lhs->depth();
			}
			mOperationCount = _rhs->operationCount() + _lhs->operationCount()+1;
			break;
		}
		default:
			assert( false );
	}
}

template <typename Number>
SupportFunctionContent<Number>::SupportFunctionContent( const std::vector<std::shared_ptr<SupportFunctionContent<Number>>>& rhs, SF_TYPE type ) {
	// assert(_lhs.dimension() == _rhs.dimension());
	assert(rhs->checkTreeValidity());
	switch ( type ) {
		case SF_TYPE::UNITE: {
			assert(!rhs.empty());
			mUnionParameters = new unionContent<Number>( rhs );
			mType = SF_TYPE::UNITE;
			mDimension = rhs.begin()->dimension();
			mDepth = 0;
			mOperationCount = 1;
			for(const auto& sf : rhs ) {
				assert(sf->checkTreeValidity());
				assert(sf->dimension() == mDimension);
				if(mDepth < sf->depth()) {
					mDepth = sf->depth();
				}
				mOperationCount += sf->operationCount();
			}
			break;
		}
		default:
			assert( false );
	}
}

template <typename Number>
SupportFunctionContent<Number>::SupportFunctionContent( const std::shared_ptr<SupportFunctionContent<Number>>& _origin, const matrix_t<Number>& A, const vector_t<Number>& b
			, SF_TYPE _type ) {
	assert(_origin->checkTreeValidity());
	switch ( _type ) {
		case SF_TYPE::LINTRAFO: {
			mLinearTrafoParameters = new trafoContent<Number,SupportFunctionContentSetting>( _origin, A, b );
			mType = SF_TYPE::LINTRAFO;
			mDimension = _origin->dimension();
			mDepth = linearTrafoParameters()->origin->depth() +1;
            mOperationCount = linearTrafoParameters()->origin->operationCount() + 1;
			break;
		}
		default:
			assert( false );
	}
}

template <typename Number>
SupportFunctionContent<Number>::SupportFunctionContent( const std::shared_ptr<SupportFunctionContent<Number>>& _origin, const Number &_factor,
										  SF_TYPE _type ) {
	assert(_origin->checkTreeValidity());
	switch ( _type ) {
		case SF_TYPE::SCALE: {
			mScaleParameters = new scaleContent<Number>( _origin, _factor );
			mType = SF_TYPE::SCALE;
			mDimension = _origin->dimension();
			mDepth = _origin->depth() +1;
			mOperationCount = _origin->operationCount() + 1;
			break;
		}
		default:
			assert( false );
	}
}

template<typename Number>
SupportFunctionContent<Number>::SupportFunctionContent( const std::shared_ptr<SupportFunctionContent<Number>>& _origin, const std::vector<std::size_t>& dimensions, SF_TYPE _type ) {
	assert(_origin->checkTreeValidity());
	switch ( _type ) {
		case SF_TYPE::PROJECTION: {
			mProjectionParameters = new projectionContent<Number>(_origin,dimensions);
			mType = SF_TYPE::PROJECTION;
			mDimension = _origin->dimension();
			mDepth = _origin->depth() + 1;
			mOperationCount = _origin->operationCount() + 1;
			break;
		}
		default:
			assert( false );
	}
}

template <typename Number>
SupportFunctionContent<Number>::~SupportFunctionContent() {
	switch ( mType ) {
		case SF_TYPE::INFTY_BALL:
		case SF_TYPE::TWO_BALL:
			delete mBall;
			break;
		case SF_TYPE::LINTRAFO:
			delete linearTrafoParameters();
			break;
		case SF_TYPE::POLY:
			delete mPolytope;
			break;
		case SF_TYPE::PROJECTION:
			delete mProjectionParameters;
			break;
		case SF_TYPE::SCALE:
			delete mScaleParameters;
			break;
		case SF_TYPE::SUM:
			delete mSummands;
			break;
		case SF_TYPE::UNITE:
			delete mUnionParameters;
			break;
		case SF_TYPE::INTERSECT:
			delete mIntersectionParameters;
			break;
		case SF_TYPE::ELLIPSOID:
			delete ellipsoid();
			break;
		case SF_TYPE::NONE: {
			std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
			assert(false);
		}
		default:
			assert(false);
			break;
	}
}

template <typename Number>
std::shared_ptr<SupportFunctionContent<Number>>& SupportFunctionContent<Number>::operator=(
	  const std::shared_ptr<SupportFunctionContent<Number>>& _other ){
        // std::cout << "SupportFunctionContent Copy\n";
	//std::cout << "Assignment, this->type:" << _other->type() << std::endl;
	assert(_other->checkTreeValidity());
	mType = _other->type();
	switch ( mType ) {
		case SF_TYPE::ELLIPSOID:
			mEllipsoid = new EllipsoidSupportFunction<Number>(*_other->ellipsoid());
			break;
		case SF_TYPE::INFTY_BALL:
		case SF_TYPE::TWO_BALL:
			mBall = new BallSupportFunction<Number>(*_other->ball());
			break;
		case SF_TYPE::LINTRAFO:
			mLinearTrafoParameters = new trafoContent<Number,SupportFunctionContentSetting>(*_other->linearTrafoParameters());
			break;
		case SF_TYPE::POLY:
			// explicitly invoke copy constructor to avoid pointer copy
			mPolytope = new PolytopeSupportFunction<Number,PolytopeSupportFunctionSetting>(*_other->polytope());
			break;
		case SF_TYPE::PROJECTION:
			mProjectionParameters = new projectionContent<Number>(*_other->projectionParameters());
			break;
		case SF_TYPE::SCALE:
			mScaleParameters = new scaleContent<Number>(*_other->scaleParameters());
			break;
		case SF_TYPE::SUM:
			mSummands = new sumContent<Number>(*_other->summands());
			break;
		case SF_TYPE::UNITE:
			mUnionParameters = new unionContent<Number>(*_other->unionParameters());
			break;
		case SF_TYPE::INTERSECT:
			mIntersectionParameters = new intersectionContent<Number>(*_other->intersectionParameters());
			break;
		case SF_TYPE::NONE: {
			std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
			assert(false);
		}
		default:
			assert( false );
	}
	assert(checkTreeValidity());
	return std::shared_ptr<SupportFunctionContent<Number>>( this->pThis );
}

template <typename Number>
EvaluationResult<Number> SupportFunctionContent<Number>::evaluate( const vector_t<Number> &_direction, bool useExact ) const {
	matrix_t<Number> tmp = matrix_t<Number>::Zero(1,_direction.rows());
	tmp << _direction.transpose();
	return multiEvaluate(tmp,useExact).at(0);

	/*
	assert(checkTreeValidity());

	using Node = std::shared_ptr<SupportFunctionContent<Number>>;
	using Param = vector_t<Number>;
	using Res = EvaluationResult<Number>;

	std::vector<Node> callStack;
	std::vector<Param> paramStack;
	std::vector<std::pair<int,std::vector<Res>>> resultStack; // The first value is an iterator to the calling frame

	callStack.push_back(getThis());
	paramStack.push_back(_direction);
	resultStack.push_back(std::make_pair(-1, std::vector<Res>()));

	while(!callStack.empty()) {
		Node cur = callStack.back();
		Param currentParam = paramStack.back();

		if(cur->originCount() == 0) {
			// Do computation and write results in case recursion ends.

			std::pair<int,std::vector<Res>> currentResult = resultStack.back();

			// update result
			// special case: When the node is a leaf, we directly return the result.
			if(currentResult.first == -1) {
				// we reached the top, exit (or return result or whatever)
				switch ( cur->type() ) {
					case SF_TYPE::ELLIPSOID: {
						return ellipsoid()->evaluate( currentParam );
					}
					case SF_TYPE::INFTY_BALL:
					case SF_TYPE::TWO_BALL: {
						return ball()->evaluate( currentParam );
					}
					case SF_TYPE::POLY: {
						return polytope()->evaluate( currentParam, useExact );
					}
					default:
						assert(false);
						FATAL("hypro.representations.supportFunction","Wrong type.");
				}
			} else {
				switch ( cur->type() ) {
					case SF_TYPE::ELLIPSOID: {
						resultStack.at(currentResult.first).second.push_back(cur->ellipsoid()->evaluate( currentParam ));
						break;
					}
					case SF_TYPE::INFTY_BALL:
					case SF_TYPE::TWO_BALL: {
						resultStack.at(currentResult.first).second.push_back(cur->ball()->evaluate( currentParam ));
						break;
					}
					case SF_TYPE::POLY: {
						resultStack.at(currentResult.first).second.push_back(cur->polytope()->evaluate( currentParam, useExact ));
						break;
					}
					default:
						assert(false);
						FATAL("hypro.representations.supportFunction","Wrong type.");
				}
			}

			// leave recursive call.
			callStack.pop_back();
			paramStack.pop_back();
			resultStack.pop_back();

		} else {

			// Detect, if this call is finished or new.
			if(resultStack.back().second.size() == cur->originCount()) {
				// the call is finished, perform accumulating operations and forward result.

				// accumulate results - in this case sum.
				Res accumulatedResult;
				switch( cur->type() ) {
					case SF_TYPE::LINTRAFO: {
						assert(resultStack.back().second.size() == 1);
						std::pair<matrix_t<Number>, vector_t<Number>> parameterPair = cur->linearTrafoParameters()->parameters->getParameterSet(cur->linearTrafoParameters()->currentExponent);
						accumulatedResult = resultStack.back().second.front();
						switch(accumulatedResult.errorCode){
							case SOLUTION::INFTY:
							case SOLUTION::INFEAS:{
								break;
							}
							default:{
								//TRACE("hypro.representations.supportFunction","opt val rows: " << res.optimumValue.rows() << " and direction rows: " << _direction.rows());
								assert(accumulatedResult.errorCode == SOLUTION::FEAS);
								assert(accumulatedResult.optimumValue.rows() == currentParam.rows());
								accumulatedResult.optimumValue = parameterPair.first * accumulatedResult.optimumValue + parameterPair.second;
								assert(accumulatedResult.optimumValue.rows() == currentParam.rows());
								// As we know, that the optimal vertex lies on the supporting hyperplane, we can obtain the distance by dot product.
								accumulatedResult.supportValue = accumulatedResult.optimumValue.dot(currentParam);
							}
						}
						break;
					}
					case SF_TYPE::PROJECTION: {
						assert(resultStack.back().second.size() == 1);
						// simply forward the results
						accumulatedResult = resultStack.back().second.front();
						break;
					}
					case SF_TYPE::SCALE: {
						assert(resultStack.back().second.size() == 1);
						accumulatedResult = resultStack.back().second.front();
						accumulatedResult.optimumValue *= scaleParameters()->factor;
						accumulatedResult.supportValue *= scaleParameters()->factor;
						break;
					}
					case SF_TYPE::SUM: {
						assert( resultStack.back().second.size() == 2);
						Res& resA = resultStack.back().second.at(0);
						Res& resB = resultStack.back().second.at(1);
						accumulatedResult = resA;
						if(resB.errorCode == SOLUTION::INFEAS || resB.errorCode == SOLUTION::INFTY){
							accumulatedResult = resB;
							break;
						} else {
							if(resA.errorCode != SOLUTION::INFEAS && resA.errorCode != SOLUTION::INFTY) {
								accumulatedResult.optimumValue += resB.optimumValue;
								accumulatedResult.supportValue += resB.supportValue;
							}
						}
						break;
					}
					case SF_TYPE::UNITE: {
						assert(resultStack.back().second.size() > 0);
						assert(resultStack.back().second.size() == cur->unionParameters()->items.size());
						accumulatedResult = resultStack.back().second.front();

						if(accumulatedResult.errorCode == SOLUTION::INFEAS) {
							break;
						} else if(accumulatedResult.errorCode == SOLUTION::INFTY) {
							accumulatedResult.supportValue = 1;
							break;
						} else {
							for(auto resIt = resultStack.back().second.begin(); resIt != resultStack.back().second.end(); ++resIt) {
								if(resIt->errorCode == SOLUTION::INFEAS) {
									accumulatedResult = *resIt;
									break;
								} else if(resIt->errorCode == SOLUTION::INFTY) {
									resIt->supportValue = 1;
									break;
								} else {
									accumulatedResult = accumulatedResult > *resIt ? accumulatedResult : *resIt;
								}
							}
							assert(accumulatedResult.errorCode == SOLUTION::FEAS);
						}
						break;
					}
					case SF_TYPE::INTERSECT: {
						assert(resultStack.back().second.size() == 2);
						Res& resA = resultStack.back().second.at(0);
						Res& resB = resultStack.back().second.at(1);

						// easy checks for infeasibility and unboundedness first
						if(resA.errorCode == SOLUTION::INFEAS){
							accumulatedResult = resA;
							break;
						} else if(resB.errorCode == SOLUTION::INFEAS){
							accumulatedResult = resB;
							break;
						}
						assert(resA.errorCode != SOLUTION::INFEAS && resB.errorCode != SOLUTION::INFEAS);
						if(resA.errorCode == SOLUTION::INFTY){
							accumulatedResult = resB;
							break;
						} else if(resB.errorCode == SOLUTION::INFTY){
							accumulatedResult = resA;
							break;
						}
						// complete checks -> real containment TODO!!
						assert(resA.errorCode == SOLUTION::FEAS && resB.errorCode == SOLUTION::FEAS);
						accumulatedResult = resA.supportValue <= resB.supportValue ? resA : resB;
						break;
					}
					case SF_TYPE::NONE: {
						std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
						assert(false);
						break;
					}
					default:{
						assert(false);
						return EvaluationResult<Number>();
					}
				}

				if(resultStack.back().first == -1) {
					// we reached the top, exit
					TRACE("hypro.representations.supportFunction","Direction: " << _direction << ", result: " << accumulatedResult);
					return accumulatedResult;
				}

				// forward result.

				resultStack.at(resultStack.back().first).second.push_back(accumulatedResult);

				// delete result frame and close recursive call
				callStack.pop_back();
				paramStack.pop_back();
				resultStack.pop_back();

			} else {
				// this is the branch for calling recursively

				// here we create the new stack levels.
				std::size_t callingFrame = callStack.size() - 1 ;
				// Do some parameter transformation, i.e. create passed parameters
				switch( cur->type() ) {
					case SF_TYPE::ELLIPSOID:
					case SF_TYPE::INFTY_BALL:
					case SF_TYPE::POLY:
					case SF_TYPE::TWO_BALL: {
						assert(false);
						FATAL("hypro.representations.supportFunction","Leaf node cannot be an intermediate case.");
						break;
					}
					case SF_TYPE::LINTRAFO: {
						std::pair<matrix_t<Number>, vector_t<Number>> parameterPair = cur->linearTrafoParameters()->parameters->getParameterSet(cur->linearTrafoParameters()->currentExponent);
						#ifndef HYPRO_USE_VECTOR_CACHING
						matrix_t<Number> tmp = parameterPair.first.transpose();
						currentParam = tmp * _direction;
						#else
						currentParam = cur->linearTrafoParameters()->parameters->getTransformedDirection(currentParam, cur->linearTrafoParameters()->currentExponent);
						#endif
						callStack.push_back(cur->linearTrafoParameters()->origin);
						paramStack.push_back(currentParam);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::PROJECTION: {
						vector_t<Number> projectedDirection = vector_t<Number>::Zero(mDimension);
						// reduce evaluation to projection dimensions
						int entryIndex = 0;
						for(const auto& projectionDimension : cur->projectionParameters()->dimensions) {
							if(projectionDimension < cur->dimension()){
								projectedDirection(entryIndex) = currentParam(projectionDimension);
								++entryIndex;
							}
						}
						currentParam = projectedDirection;
						callStack.push_back(cur->projectionParameters()->origin);
						paramStack.push_back(currentParam);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::SCALE: {
						// Do nothing for scaling -> processing is done afterwards.
						callStack.push_back(cur->scaleParameters()->origin);
						paramStack.push_back(currentParam);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::SUM: {
						// Do nothing for sum -> processing is done afterwards.
						callStack.push_back(cur->summands()->rhs);
						callStack.push_back(cur->summands()->lhs);
						paramStack.push_back(currentParam);
						paramStack.push_back(currentParam);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::UNITE: {
						// Do nothing for union -> processing is done afterwards.
						for(unsigned i = 0; i < cur->unionParameters()->items.size(); ++i) {
							callStack.push_back(cur->unionParameters()->items.at(i));
							paramStack.push_back(currentParam);
							resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						}
						break;
					}
					case SF_TYPE::INTERSECT: {
						// Do nothing for intersection -> processing is done afterwards.
						callStack.push_back(cur->intersectionParameters()->rhs);
						callStack.push_back(cur->intersectionParameters()->lhs);
						paramStack.push_back(currentParam);
						paramStack.push_back(currentParam);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::NONE: {
						std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
						assert(false);
						break;
					}
					default:{
						assert(false);
						return EvaluationResult<Number>();
					}
				}
			}
		}
	}
	assert(false);
	std::cout << "THIS SHOULD NOT HAPPEN." << std::endl;
	return EvaluationResult<Number>();
	*/
}

template <typename Number>
std::vector<EvaluationResult<Number>> SupportFunctionContent<Number>::multiEvaluate( const matrix_t<Number> &_directions, bool useExact ) const {
	//std::cout << "Multi-evaluate, type: " << mType << std::endl;
	assert(checkTreeValidity());

	using Node = std::shared_ptr<SupportFunctionContent<Number>>;
	using Param = matrix_t<Number>;
	using Res = std::vector<EvaluationResult<Number>>;

	std::vector<Node> callStack;
	std::vector<Param> paramStack;
	std::vector<std::pair<int,std::vector<Res>>> resultStack; // The first value is an iterator to the calling frame

	callStack.push_back(getThis());
	paramStack.push_back(_directions);
	resultStack.push_back(std::make_pair(-1, std::vector<Res>()));

	while(!callStack.empty()) {
		Node cur = callStack.back();
		Param currentParam = paramStack.back();

		if(cur->originCount() == 0) {
			// Do computation and write results in case recursion ends.

			std::pair<int,std::vector<Res>> currentResult = resultStack.back();

			// update result
			// special case: When the node is a leaf, we directly return the result.
			if(currentResult.first == -1) {
				// we reached the top, exit (or return result or whatever)
				switch ( cur->type() ) {
					case SF_TYPE::ELLIPSOID: {
						return ellipsoid()->multiEvaluate( currentParam );
					}
					case SF_TYPE::INFTY_BALL:
					case SF_TYPE::TWO_BALL: {
						return ball()->multiEvaluate( currentParam );
					}
					case SF_TYPE::POLY: {
						return polytope()->multiEvaluate( currentParam, useExact );
					}
					default:
						assert(false);
						FATAL("hypro.representations.supportFunction","Wrong type.");
				}
			} else {
				switch ( cur->type() ) {
					case SF_TYPE::ELLIPSOID: {
						resultStack.at(currentResult.first).second.emplace_back(cur->ellipsoid()->multiEvaluate( currentParam ));
						break;
					}
					case SF_TYPE::INFTY_BALL:
					case SF_TYPE::TWO_BALL: {
						resultStack.at(currentResult.first).second.emplace_back(cur->ball()->multiEvaluate( currentParam ));
						break;
					}
					case SF_TYPE::POLY: {
						resultStack.at(currentResult.first).second.emplace_back(cur->polytope()->multiEvaluate( currentParam, useExact ));
						break;
					}
					default:
						assert(false);
						FATAL("hypro.representations.supportFunction","Wrong type.");
				}
			}

			// leave recursive call.
			callStack.pop_back();
			paramStack.pop_back();
			resultStack.pop_back();

		} else {

			// Detect, if this call is finished or new.
			if(resultStack.back().second.size() == cur->originCount()) {
				// the call is finished, perform accumulating operations and forward result.
				//TRACE("hypro.representations.supportFunction", ": Accumulate results.");
				// accumulate results - in this case sum.
				Res accumulatedResult;
				switch( cur->type() ) {
					case SF_TYPE::LINTRAFO: {
						//TRACE("hypro.representations.supportFunction", ": LINTRAFO, accumulate results.")
						assert(resultStack.back().second.size() == 1);
						const std::pair<matrix_t<Number>, vector_t<Number>>& parameterPair = cur->linearTrafoParameters()->parameters->getParameterSet(cur->linearTrafoParameters()->currentExponent);
						//TRACE("hypro.representations.supportFunction", "Matrix: " << parameterPair.first);
						//TRACE("hypro.representations.supportFunction", "Vector: " << parameterPair.second);
						if(resultStack.back().second.front().begin()->errorCode != SOLUTION::INFEAS) {
							unsigned directionCnt = 0;
							for(auto& entry : resultStack.back().second.front()) {
								vector_t<Number> currentDir(currentParam.row(directionCnt));
								if(entry.errorCode == SOLUTION::INFTY) {
									entry.optimumValue = entry.optimumValue;
									entry.supportValue = 1;
								} else {
									//TRACE("hypro.representations.supportFunction", ": Entry val before trafo: " << entry.optimumValue);
									entry.optimumValue = parameterPair.first * entry.optimumValue + parameterPair.second;
									// As we know, that the optimal vertex lies on the supporting Halfspace, we can obtain the distance by dot product.
									entry.supportValue = entry.optimumValue.dot(currentDir);
								}
								//auto t = convert<Number,double>(currentParam.row(directionCnt));
								//TRACE("hypro.representations.supportFunction", "Direction: " << t << ", Entry value: " << entry.supportValue);
								++directionCnt;
							}
						}
						//TRACE("hypro.representations.supportFunction",": LINTRAFO, accumulate results done.");
						accumulatedResult = resultStack.back().second.front();
						break;
					}
					case SF_TYPE::PROJECTION: {
						//TRACE("hypro.representations.supportFunction", ": PROJECTION, accumulate results.")
						assert(resultStack.back().second.size() == 1);
						// simply forward the results
						accumulatedResult = resultStack.back().second.front();
						break;
					}
					case SF_TYPE::SCALE: {
						//TRACE("hypro.representations.supportFunction", ": SCALE, accumulate results.")
						assert(resultStack.back().second.size() == 1);
						// if one result is infeasible, the others will be too -> do not process.
						if(resultStack.back().second.front().begin()->errorCode != SOLUTION::INFEAS){
							for(auto& singleRes : resultStack.back().second.front()){
								assert(singleRes.errorCode != SOLUTION::INFEAS);
								if(singleRes.errorCode == SOLUTION::FEAS){
									singleRes.supportValue *= cur->scaleParameters()->factor;
									singleRes.optimumValue *= cur->scaleParameters()->factor;
								}
							}
						}
						accumulatedResult = resultStack.back().second.front();
						break;
					}
					case SF_TYPE::SUM: {
						//TRACE("hypro.representations.supportFunction", ": SUM, accumulate results.")
						assert( resultStack.back().second.size() == 2);
						assert( resultStack.back().second.at(0).size() == std::size_t(currentParam.rows()));
						assert( resultStack.back().second.at(0).size() == resultStack.back().second.at(1).size());
						Res& resA = resultStack.back().second.at(0);
						Res& resB = resultStack.back().second.at(1);

						// only process if both are feasible. If one result is infeasible, the others will be too, so stop processing.
						if(resA.begin()->errorCode != SOLUTION::INFEAS && resB.begin()->errorCode != SOLUTION::INFEAS) {
							for(unsigned index = 0; index < resA.size(); ++index){
								assert(resA.at(index).errorCode != SOLUTION::INFEAS && resB.at(index).errorCode != SOLUTION::INFEAS);
								EvaluationResult<Number> r;
								if(resA[index].errorCode == SOLUTION::INFTY || resB[index].errorCode == SOLUTION::INFTY){
									r.errorCode = SOLUTION::INFTY;
									r.supportValue = 1;
								} else {
									r.errorCode = SOLUTION::FEAS;
									r.supportValue = resA[index].supportValue + resB[index].supportValue;
									r.optimumValue = resA[index].optimumValue + resB[index].optimumValue;
								}
								accumulatedResult.emplace_back(r);
							}
							assert( accumulatedResult.size() == std::size_t(currentParam.rows()));
						}
						break;
					}
					case SF_TYPE::UNITE: {
						//TRACE("hypro.representations.supportFunction", ": UNITE, accumulate results.")
						assert(resultStack.back().second.size() > 0);
						assert(resultStack.back().second.size() == cur->unionParameters()->items.size());
						accumulatedResult = resultStack.back().second.front();
						for(auto resIt = resultStack.back().second.begin(); resIt != resultStack.back().second.end(); ++resIt) {
							for( unsigned resultId = 0; resultId < accumulatedResult.size(); ++resultId ) {
								if((*resIt)[resultId].errorCode == SOLUTION::INFEAS) {
									accumulatedResult[resultId] = (*resIt)[resultId];
									break;
								} else if((*resIt)[resultId].errorCode == SOLUTION::INFTY) {
									(*resIt)[resultId].supportValue = 1;
									accumulatedResult[resultId] = (*resIt)[resultId];
								} else {
									assert((*resIt)[resultId].errorCode == SOLUTION::FEAS);
									accumulatedResult[resultId] = (*resIt)[resultId] > accumulatedResult[resultId] ? (*resIt)[resultId] : accumulatedResult[resultId];
									//assert(accumulatedResult[resultId].errorCode == SOLUTION::FEAS);
								}
							}

						}
						break;
					}
					case SF_TYPE::INTERSECT: {
						//TRACE("hypro.representations.supportFunction", ": INTERSECT, accumulate results.")
						assert(resultStack.back().second.size() == 2);
						Res& resA = resultStack.back().second.at(0);
						Res& resB = resultStack.back().second.at(1);
						assert( resA.size() == resB.size() );
						// in case one of the results is infeasible (the set is empty), return this result.
						if(resA.begin()->errorCode == SOLUTION::INFEAS){
							accumulatedResult = resA;
							break;
						}
						if(resB.begin()->errorCode == SOLUTION::INFEAS){
							accumulatedResult = resB;
							break;
						}
			 			for ( unsigned i = 0; i < resA.size(); ++i ) {
			 				//std::cout << "Eval in direction " << convert<Number,double>(_directions.row(i)).transpose() << std::endl;
							assert(resA[i].errorCode != SOLUTION::INFEAS && resB[i].errorCode != SOLUTION::INFEAS);
							EvaluationResult<Number> res;
							if (resA[i].errorCode == SOLUTION::INFTY) {
								//TRACE("hypro.representations.supportFunction","resA infinite");
								res.errorCode = resB[i].errorCode;
								res.supportValue = resB[i].supportValue;
								res.optimumValue = resB[i].optimumValue;
							} else if (resB[i].errorCode == SOLUTION::INFTY) {
								//TRACE("hypro.representations.supportFunction","resB infinite");
								assert(resA[i].errorCode == SOLUTION::FEAS);
								res.errorCode = resA[i].errorCode;
								res.supportValue = resA[i].supportValue;
								res.optimumValue = resA[i].optimumValue;
							} else {
								assert(resA[i].errorCode == SOLUTION::FEAS && resB[i].errorCode == SOLUTION::FEAS);
								//TRACE("hypro.representations.supportFunction","Both finite: A " << resA[i].supportValue << " vs B " << resB[i].supportValue);
								res.errorCode = SOLUTION::FEAS;
								if(resA[i].supportValue < resB[i].supportValue){
									res.supportValue = resA[i].supportValue;
									res.optimumValue = resA[i].optimumValue;
								} else {
									res.supportValue = resB[i].supportValue;
									res.optimumValue = resB[i].optimumValue;
								}
							}
							auto t = convert<Number,double>(res.optimumValue);
							//TRACE("hypro.representations.supportFunction", ": INTERSECT, accumulated result: " << t << " and value: " << res.supportValue );
							accumulatedResult.emplace_back(res);
						}
						assert(accumulatedResult.size() == std::size_t(currentParam.rows()));
						break;
					}
					case SF_TYPE::NONE: {
						std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
						assert(false);
						break;
					}
					default:{
						assert(false);
						return std::vector<EvaluationResult<Number>>();
					}

				}

				if(resultStack.back().first == -1) {
					// we reached the top, exit
					//TRACE("hypro.representations.supportFunction","Return accumulated result.");
					return accumulatedResult;
				}

				// forward result.
				//TRACE("hypro.representations.supportFunction","Push accumulated result up.");
				resultStack.at(resultStack.back().first).second.emplace_back(accumulatedResult);

				// delete result frame and close recursive call
				callStack.pop_back();
				paramStack.pop_back();
				resultStack.pop_back();

			} else {
				// this is the branch for calling recursively

				// here we create the new stack levels.
				std::size_t callingFrame = callStack.size() - 1 ;

				// Do some parameter transformation, i.e. create passed parameters
				switch( cur->type() ) {
					case SF_TYPE::ELLIPSOID:
					case SF_TYPE::INFTY_BALL:
					case SF_TYPE::POLY:
					case SF_TYPE::TWO_BALL: {
						assert(false);
						FATAL("hypro.representations.supportFunction","Leaf node cannot be an intermediate case.");
						break;
					}
					case SF_TYPE::LINTRAFO: {
						callStack.push_back(cur->linearTrafoParameters()->origin);
						#ifndef HYPRO_USE_VECTOR_CACHING
						paramStack.push_back(currentParam * cur->linearTrafoParameters()->parameters->getParameterSet(cur->linearTrafoParameters()->currentExponent).first);
						#else
						paramStack.push_back(cur->linearTrafoParameters()->parameters->getTransformedDirections(currentParam, cur->linearTrafoParameters()->currentExponent));
						#endif
						resultStack.emplace_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::PROJECTION: {
						matrix_t<Number> projectedParameters = matrix_t<Number>::Zero(_directions.rows(), _directions.cols());
						Eigen::Index entryIndex = 0;
						for(const auto& entry : cur->projectionParameters()->dimensions) {
							TRACE("hypro.representations.supportFunction","Entry: " << entry)
							if(entry < mDimension){
								projectedParameters.col(entry) = currentParam.col(entry);
								++entryIndex;
							}
						}
						assert(entryIndex == Eigen::Index(cur->projectionParameters()->dimensions.size()));
						callStack.push_back(cur->projectionParameters()->origin);
						paramStack.push_back(projectedParameters);
						resultStack.emplace_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::SCALE: {
						// Do nothing for scaling -> processing is done afterwards.
						callStack.push_back(cur->scaleParameters()->origin);
						paramStack.push_back(currentParam);
						resultStack.emplace_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::SUM: {
						// Do nothing for sum -> processing is done afterwards.
						callStack.push_back(cur->summands()->rhs);
						callStack.push_back(cur->summands()->lhs);
						paramStack.push_back(currentParam);
						paramStack.push_back(currentParam);
						resultStack.emplace_back(std::make_pair(callingFrame,std::vector<Res>()));
						resultStack.emplace_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::UNITE: {
						// Do nothing for union -> processing is done afterwards.
						for(unsigned i = 0; i < cur->unionParameters()->items.size(); ++i) {
							callStack.push_back(cur->unionParameters()->items.at(i));
							paramStack.push_back(currentParam);
							resultStack.emplace_back(std::make_pair(callingFrame,std::vector<Res>()));
						}
						break;
					}
					case SF_TYPE::INTERSECT: {
						// Do nothing for intersection -> processing is done afterwards.
						callStack.push_back(cur->intersectionParameters()->rhs);
						callStack.push_back(cur->intersectionParameters()->lhs);
						paramStack.push_back(currentParam);
						paramStack.push_back(currentParam);
						resultStack.emplace_back(std::make_pair(callingFrame,std::vector<Res>()));
						resultStack.emplace_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::NONE: {
						std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
						assert(false);
						break;
					}
					default:{
						assert(false);
						return std::vector<EvaluationResult<Number>>();
					}
				}
			}
		}
	}
	assert(false);
	std::cout << "THIS SHOULD NOT HAPPEN." << std::endl;
	return std::vector<EvaluationResult<Number>>();
}

template <typename Number>
std::size_t SupportFunctionContent<Number>::dimension() const {
	assert(mType != SF_TYPE::NONE);
	return mDimension;
}

template <typename Number>
SF_TYPE SupportFunctionContent<Number>::type() const {
	return mType;
}

template <typename Number>
unsigned SupportFunctionContent<Number>::depth() const {
	assert(mType != SF_TYPE::NONE);
	return mDepth;
}

template <typename Number>
unsigned SupportFunctionContent<Number>::operationCount() const {
	assert(mType != SF_TYPE::NONE);
	return mOperationCount;
}

template <typename Number>
unsigned SupportFunctionContent<Number>::multiplicationsPerEvaluation() const {
	using Node = std::shared_ptr<SupportFunctionContent<Number>>;
	using Res = unsigned;
	std::vector<Node> callStack;
	std::vector<std::pair<std::size_t,std::vector<Res>>> resultStack; // The first value is an iterator to the calling frame

	callStack.push_back(getThis());
	resultStack.push_back(std::make_pair(-1, std::vector<Res>()));

	while(!callStack.empty()) {
		Node cur = callStack.back();

		if(cur->originCount() == 0) {
			// Do computation and write results in case recursion ends.

			std::pair<std::size_t,std::vector<Res>> currentResult = resultStack.back();

			// update result
			// special case: When the node is a leaf, we directly return the result.
			if(currentResult.first == -1) {
				// we reached the top
				assert(currentResult.second.size() == 1);
				return currentResult.second.front();
			}

			switch(cur->type()) {
				case SF_TYPE::INFTY_BALL:
				case SF_TYPE::TWO_BALL:
				case SF_TYPE::ELLIPSOID: {
		            resultStack.at(currentResult.first).second.push_back(1);
		            break;
		        }
		        case SF_TYPE::POLY: {
		            unsigned maxValue = std::max(polytope()->constraints().rows(), polytope()->constraints().cols());
		            resultStack.at(currentResult.first).second.push_back(carl::pow(maxValue,2)*polytope()->constraints().cols());
		            break;
		        }
		        default:
		        	// TODO: Extend for other bottom cases.
		        	assert(false);
			}

			// leave recursive call.
			callStack.pop_back();
			resultStack.pop_back();

		} else {

			// Detect, if this call is finished or new.
			if(resultStack.back().second.size() == cur->originCount()) {
				// the call is finished, perform accumulating operations and forward result.

				// accumulate results
				Res accumulatedResult = 0;
				switch(cur->type()) {
					case SF_TYPE::INTERSECT:
			        case SF_TYPE::PROJECTION:
			        case SF_TYPE::LINTRAFO: {
			            accumulatedResult += 1;
			            break;
			        }
			        case SF_TYPE::UNITE:
			        	for(auto val : resultStack.back().second) {
			        		accumulatedResult += val;
			        	}
			        	break;
			        case SF_TYPE::NONE: {
						std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
						assert(false);
						break;
					}
					default:
						std::cout << __func__ << ": SF Type unexpected!" << std::endl;
						assert(false);
				}

				if(resultStack.back().first == -1) {
					// we reached the top
					return accumulatedResult;
				}

				resultStack.at(resultStack.back().first).second.push_back(accumulatedResult);

				// delete result frame and close recursive call
				callStack.pop_back();
				resultStack.pop_back();

			} else {
				// this is the branch for calling recursively

				// here we create the new stack levels. As the parameters are all the same, we do not care for their order (could be extended).
				std::size_t callingFrame = callStack.size() - 1 ;

				switch ( cur->type() ) {
			        case SF_TYPE::SUM: {
						callStack.push_back(cur->summands()->rhs);
						callStack.push_back(cur->summands()->lhs);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::INTERSECT: {
						callStack.push_back(cur->intersectionParameters()->rhs);
						callStack.push_back(cur->intersectionParameters()->lhs);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::LINTRAFO: {
						callStack.push_back(cur->linearTrafoParameters()->origin);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::SCALE: {
						callStack.push_back(cur->scaleParameters()->origin);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::UNITE: {
						for(unsigned i = 0; i < cur->unionParameters()->items.size(); ++i){
							callStack.push_back(cur->unionParameters()->items.at(i));
							resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						}
						break;
					}
					case SF_TYPE::POLY:
					case SF_TYPE::INFTY_BALL:
					case SF_TYPE::TWO_BALL:
					case SF_TYPE::ELLIPSOID:
					case SF_TYPE::BOX:
					case SF_TYPE::ZONOTOPE: {
						assert(false);
						break;
					}
					case SF_TYPE::PROJECTION: {
						callStack.push_back(cur->projectionParameters()->origin);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::NONE:
					default:
						assert(false);
			    }
			}
		}
	}
	std::cout << __func__ << ": This should not be reachable." << std::endl;
	return 0;
}

template <typename Number>
void SupportFunctionContent<Number>::forceLinTransReduction(){
	assert(checkTreeValidity());
    switch ( mType ) {
        case SF_TYPE::LINTRAFO: {
            std::shared_ptr<SupportFunctionContent<Number>> origin = linearTrafoParameters()->origin;
            std::pair<matrix_t<Number>, vector_t<Number>> parameterPair = linearTrafoParameters()->parameters->getParameterSet(linearTrafoParameters()->currentExponent);
            std::pair<matrix_t<Number>, vector_t<Number>> nextPair;
            while(origin->type() == SF_TYPE::LINTRAFO){
                unsigned currentExponent = origin->linearTrafoParameters()->currentExponent;
                nextPair = origin->linearTrafoParameters()->parameters->getParameterSet(currentExponent);
                //parameterPair.second = parameterPair.second + parameterPair.first * nextPair.second + parameterPair.second;
				parameterPair.second = parameterPair.second + parameterPair.first * nextPair.second;
                parameterPair.first = parameterPair.first * nextPair.first;
                origin = origin->linearTrafoParameters()->origin;
            }
			mDepth = origin->depth() + 1;
			mOperationCount = origin->operationCount() +1;
            mLinearTrafoParameters = new trafoContent<Number,SupportFunctionContentSetting>( origin, std::make_shared<lintrafoParameters<Number>>(parameterPair.first, parameterPair.second) );
            break;
        }
        case SF_TYPE::SUM: {
            summands()->lhs->forceLinTransReduction();
            summands()->rhs->forceLinTransReduction();
			mDepth = std::max(summands()->lhs->operationCount(), summands()->rhs->operationCount()) +1;
			mOperationCount = summands()->lhs->operationCount() + summands()->rhs->operationCount() +1;
			break;
        }
        case SF_TYPE::INTERSECT: {
            intersectionParameters()->rhs->forceLinTransReduction();
            intersectionParameters()->lhs->forceLinTransReduction();
			mDepth = std::max(summands()->lhs->operationCount(), summands()->rhs->operationCount()) +1;
			mOperationCount = summands()->lhs->operationCount() + summands()->rhs->operationCount() +1;
			break;
        }
        case SF_TYPE::UNITE: {
        	mOperationCount = 1;
        	for(auto& item : unionParameters()->items) {
        		item->forceLinTransReduction();
        		mDepth = mDepth > item->depth() ? mDepth : item->depth();
        		mOperationCount += item->operationCount();
        	}
        	break;
        }
		case SF_TYPE::PROJECTION: {
			projectionParameters()->origin->forceLinTransReduction();
			mDepth = projectionParameters()->origin->depth() + 1;
			mOperationCount = projectionParameters()->origin->operationCount() +1;
			break;
		}
		case SF_TYPE::SCALE: {
            scaleParameters()->origin->forceLinTransReduction();
			mDepth = scaleParameters()->origin->depth() + 1;
			mOperationCount = scaleParameters()->origin->operationCount() +1;
			break;
        }
        case SF_TYPE::NONE: {
			std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
			assert(false);
		}
        default:
            break;
    }
    assert(checkTreeValidity());
}


template<typename Number>
Point<Number> SupportFunctionContent<Number>::supremumPoint() const {
	assert(checkTreeValidity());
	switch ( mType ) {
		case SF_TYPE::INFTY_BALL:
		case SF_TYPE::TWO_BALL: {
			return ball()->supremumPoint();
		}
		case SF_TYPE::LINTRAFO: {
			Point<Number> supPoint = linearTrafoParameters()->origin->supremumPoint();
			if(supPoint.dimension() == 0){
				return supPoint;
			}
			std::pair<matrix_t<Number>, vector_t<Number>> parameterPair = linearTrafoParameters()->parameters->getParameterSet(linearTrafoParameters()->currentExponent);
			return supPoint.affineTransformation(parameterPair.first, parameterPair.second);
		}
		case SF_TYPE::POLY: {
			return polytope()->supremumPoint();
		}
		case SF_TYPE::PROJECTION: {
			Point<Number> tmpRes = projectionParameters()->origin->supremumPoint();
			vector_t<Number> tmp = vector_t<Number>::Zero(mDimension);
			for(const auto& entry : projectionParameters()->dimensions){
				if(entry < mDimension) {
					tmp(entry) = tmpRes.at(entry);
				}
			}
			return Point<Number>(tmp);
		}
		case SF_TYPE::SCALE: {
			if ( scaleParameters()->factor == 0 ) {
				return Point<Number>::Zero(mDimension);
			} else {
				Point<Number> supPoint = scaleParameters()->origin->supremumPoint();
				if(supPoint.dimension() == 0){
					return supPoint;
				}
				return supPoint* scaleParameters()->factor;
			}
		}
		case SF_TYPE::SUM: {
			Point<Number> lhsPoint = summands()->lhs->supremumPoint();
			Point<Number> rhsPoint = summands()->rhs->supremumPoint();
			if(lhsPoint.dimension() == 0) {
				return rhsPoint;
			}
			if(rhsPoint.dimension() == 0) {
				return lhsPoint;
			}
			return lhsPoint+rhsPoint;
		}
		case SF_TYPE::UNITE: {
			Point<Number> resPoint = (*unionParameters()->items.begin())->supremumPoint();
			if(resPoint.dimension() == 0) {
				return resPoint;
			}
			auto itemIt = unionParameters()->items.begin();
			++itemIt;
			for(; itemIt != unionParameters()->items.end(); ++itemIt) {
				Point<Number> tmpPoint = (*itemIt)->supremumPoint();
				resPoint = Point<Number>::inftyNorm(resPoint) > Point<Number>::inftyNorm(tmpPoint) ? resPoint : tmpPoint;
			}
			return resPoint;
		}
		case SF_TYPE::INTERSECT: {
			Point<Number> lhsPoint = intersectionParameters()->lhs->supremumPoint();
			Point<Number> rhsPoint = intersectionParameters()->rhs->supremumPoint();
			if(lhsPoint.dimension() == 0) {
				return rhsPoint;
			}
			if(rhsPoint.dimension() == 0) {
				return lhsPoint;
			}
			if(Point<Number>::inftyNorm(lhsPoint) < Point<Number>::inftyNorm(rhsPoint)) {
				return lhsPoint;
			}
			return rhsPoint;
		}
		case SF_TYPE::NONE: {
			std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
			return Point<Number>();
			assert(false);
		}
		default:
			assert(false);
			return Point<Number>();
	}
}

template<typename Number>
std::vector<std::size_t> SupportFunctionContent<Number>::collectProjections() const {
	using Node = std::shared_ptr<SupportFunctionContent<Number>>;
	using Res = std::vector<std::size_t>;

	std::vector<Node> callStack;
	//std::vector<vector_t<Number>> paramStack;
	std::vector<std::pair<int,std::vector<Res>>> resultStack; // The first value is an iterator to the calling frame

	callStack.push_back(getThis());
	//paramStack.push_back(1);
	resultStack.push_back(std::make_pair(-1, std::vector<Res>()));

	//std::cout << "Initialized stacks." << std::endl;

	while(!callStack.empty()) {
		Node cur = callStack.back();
		//T currentParam = paramStack.back();

		//if(cur->children.empty()) {
		if(cur->mType == SF_TYPE::POLY || cur->mType == SF_TYPE::INFTY_BALL || cur->mType == SF_TYPE::TWO_BALL || cur->mType == SF_TYPE::ELLIPSOID ) {
			//std::cout << "Reached bottom." << std::endl;
			// Do computation and write results in case recursion ends.

			std::pair<int,std::vector<Res>> currentResult = resultStack.back();

			// update result
			Res res;
			//DEBUG("hypro.representations.supportFunction","mDimension " << mDimension);
			for(std::size_t i = 0; i < mDimension; ++i){
				//DEBUG("hypro.representations.supportFunction","Added dimension " << i);
				res.emplace_back(i);
			}

			// special case: When the node is a leaf, we directly return the result.
			if(currentResult.first == -1) {
				// we reached the top, exit (or return result or whatever)
				//std::cout << "Terminating with result " << cur->doStuff(currentParam) << std::endl;
				// std::cout << __func__ << ": resulting dimension size: " << res.size() << std::endl;
				return res;
			}

			resultStack.at(currentResult.first).second.push_back(res);
			//// std::cout << "Write result for calling node on pos " << currentResult.first << std::endl;

			// leave recursive call.
			callStack.pop_back();
			//paramStack.pop_back();
			resultStack.pop_back();

		} else {

			// Detect, if this call is finished or new.
			if(resultStack.back().second.size() == cur->originCount()) {
				//// std::cout << "intermediate node on way up." << std::endl;
				// the call is finished, perform accumulating operations and forward result.

				// accumulate results - in this case intersect sets.

				if(resultStack.back().second.size() == 1) {
					// TODO: Handle lintrafo here separately.
					// std::cout << __func__ << ": Type is projection: " << (cur->type() == SF_TYPE::PROJECTION) << std::endl;
					// We need to filter, in case the element is a projection.
					if(cur->type() == SF_TYPE::PROJECTION) {
						assert(resultStack.back().second.size() == 1);
						std::vector<std::size_t> tmp = cur->projectionParameters()->dimensions;
						for(auto dimIt = tmp.begin(); dimIt != tmp.end(); ) {
							if(std::find(resultStack.back().second.front().begin(), resultStack.back().second.front().end(), *dimIt) == resultStack.back().second.front().end()) {
								dimIt = tmp.erase(dimIt);
							} else {
								++dimIt;
							}
						}
						std::swap(resultStack.back().second.front(), tmp);
					}


					if(resultStack.back().first == -1) {
						// std::cout << __func__ << ": resulting dimension size: " << resultStack.back().second.front().size() << std::endl;
						return resultStack.back().second.front();
					}
					resultStack.at(resultStack.back().first).second.push_back(resultStack.back().second.front());
				} else {
					Res accumulatedResult;
					// we create the intersection of all results. Therefore we iterate over the first vector and check the other
					// result vectors, if the respective element is contained. Iterating over the first is sufficient, as elements
					// not in the first vector will not be in the intersection anyways.
					for(unsigned pos = 0; pos < resultStack.back().second.begin()->size(); ++pos){
						std::size_t element = resultStack.back().second.begin()->at(pos);
						bool elementInAllVectors = true;
						for(unsigned resIndex = 1; resIndex < resultStack.back().second.size(); ++resIndex) {
							if(std::find(resultStack.back().second.at(resIndex).begin(), resultStack.back().second.at(resIndex).end(), element) == resultStack.back().second.at(resIndex).end()){
								elementInAllVectors = false;
								break;
							}
						}
						if(elementInAllVectors){
							// std::cout << __func__ << ": Element " << element << " is in all result vectors." << std::endl;
							// if this is a projection, we need to remove non-projected dimensions
							// std::cout << __func__ << ": Type is projection: " << (cur->type() == SF_TYPE::PROJECTION) << std::endl;
							if(cur->type() != SF_TYPE::PROJECTION ||
								( cur->type() == SF_TYPE::PROJECTION && std::find(cur->projectionParameters()->dimensions.begin(), cur->projectionParameters()->dimensions.end(), element) != cur->projectionParameters()->dimensions.end()))
							{
								// std::cout << __func__ << ": Add element " << element <<  " to result set." << std::endl;
								accumulatedResult.emplace_back(element);
							}
						}
					}

					// push result up or return
					if(resultStack.back().first == -1){
						// std::cout << __func__ << ": resulting dimension size: " << accumulatedResult.size() << std::endl;
						return accumulatedResult;
					}
					resultStack.at(resultStack.back().first).second.push_back(accumulatedResult);
				}

				// delete result frame and close recursive call
				callStack.pop_back();
				resultStack.pop_back();

			} else {
				//std::cout << "intermediate node on way down." << std::endl;

				// NEW RECURSIVE CALLS
				int callingFrame = int(callStack.size()) - 1 ;

				switch ( cur->type() ) {
					case SF_TYPE::SUM: {
						callStack.push_back(cur->summands()->rhs);
						callStack.push_back(cur->summands()->lhs);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::INTERSECT: {
						callStack.push_back(cur->intersectionParameters()->rhs);
						callStack.push_back(cur->intersectionParameters()->lhs);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::LINTRAFO: {
						callStack.push_back(cur->linearTrafoParameters()->origin);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::SCALE: {
						callStack.push_back(cur->scaleParameters()->origin);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::UNITE: {
						for(unsigned i = 0; i < cur->unionParameters()->items.size(); ++i){
							callStack.push_back(cur->unionParameters()->items.at(i));
							resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						}
						break;
					}
					case SF_TYPE::POLY:
					case SF_TYPE::INFTY_BALL:
					case SF_TYPE::TWO_BALL:
					case SF_TYPE::ELLIPSOID:
					case SF_TYPE::BOX:
					case SF_TYPE::ZONOTOPE: {
						assert(false);
						break;
					}
					case SF_TYPE::PROJECTION: {
						callStack.push_back(cur->projectionParameters()->origin);
						resultStack.push_back(std::make_pair(callingFrame,std::vector<Res>()));
						break;
					}
					case SF_TYPE::NONE:
					default:
						assert(false);
				}
			}
		}
	}
	assert(false);
	std::cout << "THIS SHOULD NOT HAPPEN." << std::endl;
	return std::vector<std::size_t>();
}

template <typename Number>
sumContent<Number> *SupportFunctionContent<Number>::summands() const {
	assert( mType == SF_TYPE::SUM);
	return mSummands;
}

template <typename Number>
trafoContent<Number,SupportFunctionContentSetting> *SupportFunctionContent<Number>::linearTrafoParameters() const {
	assert(mType == SF_TYPE::LINTRAFO);
	return mLinearTrafoParameters;
}

template <typename Number>
scaleContent<Number> *SupportFunctionContent<Number>::scaleParameters() const {
	assert( mType == SF_TYPE::SCALE);
	return mScaleParameters;
}

template <typename Number>
unionContent<Number> *SupportFunctionContent<Number>::unionParameters() const {
	assert( mType == SF_TYPE::UNITE);
	return mUnionParameters;
}

template <typename Number>
intersectionContent<Number> *SupportFunctionContent<Number>::intersectionParameters() const {
	assert( mType == SF_TYPE::INTERSECT );
	assert(mIntersectionParameters);
	return mIntersectionParameters;
}

template<typename Number>
projectionContent<Number>* SupportFunctionContent<Number>::projectionParameters() const {
	assert( mType == SF_TYPE::PROJECTION );
	return mProjectionParameters;
}

template <typename Number>
PolytopeSupportFunction<Number,PolytopeSupportFunctionSetting> *SupportFunctionContent<Number>::polytope() const {
	assert( mType == SF_TYPE::POLY );
	return mPolytope;
}

template <typename Number>
EllipsoidSupportFunction<Number> *SupportFunctionContent<Number>::ellipsoid() const {
	assert( mType == SF_TYPE::ELLIPSOID );
	return mEllipsoid;
}

template <typename Number>
BallSupportFunction<Number> *SupportFunctionContent<Number>::ball() const {
	assert( mType == SF_TYPE::INFTY_BALL || mType == SF_TYPE::TWO_BALL);
	return mBall;
}

template<typename Number>
std::shared_ptr<SupportFunctionContent<Number>> SupportFunctionContent<Number>::project(const std::vector<std::size_t>& dimensions) const {
	return create(getThis(),dimensions);
}

template <typename Number>
std::shared_ptr<SupportFunctionContent<Number>> SupportFunctionContent<Number>::affineTransformation(
	  const matrix_t<Number>& A, const vector_t<Number>& b ) const {
	return create(getThis(), A, b);
}

template <typename Number>
std::shared_ptr<SupportFunctionContent<Number>> SupportFunctionContent<Number>::minkowskiSum(
	  const std::shared_ptr<SupportFunctionContent<Number>>& rhs ) const {
	return create(SF_TYPE::SUM, getThis(), rhs);
}

template <typename Number>
std::shared_ptr<SupportFunctionContent<Number>> SupportFunctionContent<Number>::intersect(
	  const std::shared_ptr<SupportFunctionContent<Number>>& rhs ) const {
	return create(SF_TYPE::INTERSECT, getThis() , rhs);
}

template <typename Number>
bool SupportFunctionContent<Number>::contains( const Point<Number> &_point ) const {
	assert(mType != SF_TYPE::NONE);
	return this->contains( _point.rawCoordinates() );
}

template <typename Number>
bool SupportFunctionContent<Number>::contains( const vector_t<Number> &_point ) const {
	switch ( mType ) {
		case SF_TYPE::ELLIPSOID: {
			DEBUG("hypro.representations.supportFunction","ELLIPSOID, point: " << _point);
		    return ellipsoid()->contains( _point );
		}
		case SF_TYPE::INFTY_BALL:
		case SF_TYPE::TWO_BALL: {
			DEBUG("hypro.representations.supportFunction","BALL, point: " << _point);
			return ball()->contains( _point );
		}
		case SF_TYPE::LINTRAFO: {
			// TODO: Verify.
			DEBUG("hypro.representations.supportFunction","TRANSFORMATION, point: " << _point);
			std::pair<matrix_t<Number>, vector_t<Number>> parameterPair = linearTrafoParameters()->parameters->getParameterSet(linearTrafoParameters()->currentExponent);
			return linearTrafoParameters()->origin->contains( (parameterPair.first.transpose() * _point) - parameterPair.second );
		}
		case SF_TYPE::POLY: {
			DEBUG("hypro.representations.supportFunction","POLY, point: " << _point);
			return polytope()->contains( _point );
		}
		case SF_TYPE::PROJECTION: {
			DEBUG("hypro.representations.supportFunction","PROJECTION, point: " << _point);
			return projectionParameters()->origin->contains( _point ); // TODO: Correct?
		}
		case SF_TYPE::SCALE: {
			DEBUG("hypro.representations.supportFunction","SCALE, point: " << _point);
			if ( scaleParameters()->factor == 0 )
				return false;
			else
				return scaleParameters()->origin->contains( _point / scaleParameters()->factor );
		}
		case SF_TYPE::SUM: {
			DEBUG("hypro.representations.supportFunction","MINKOWSKI-SUM, point: " << _point);
			// current approach: Use templated evaluation.
			std::vector<vector_t<Number>> templates = computeTemplate<Number>(this->dimension(), defaultTemplateDirectionCount);
			// use single evaluation, as one invalid eval is enough to determine the point is not contained.
			for(const auto& dir : templates) {
				DEBUG("hypro.representations.supportFunction","Evaluate " << dir);
				EvaluationResult<Number> rhsRes =  summands()->rhs->evaluate(dir, false);
				DEBUG("hypro.representations.supportFunction","Rhsres: " << rhsRes.supportValue);
				EvaluationResult<Number> lhsRes =  summands()->lhs->evaluate(dir, false);
				DEBUG("hypro.representations.supportFunction","Lhsres: " << lhsRes.supportValue);
				DEBUG("hypro.representations.supportFunction","Summed: " << rhsRes.supportValue + lhsRes.supportValue << " and point dist: " << dir.dot(_point));
				if(dir.dot(_point) > (lhsRes.supportValue + rhsRes.supportValue)) {
					return false;
				}
			}
			return true;
		}
		case SF_TYPE::UNITE: {
			DEBUG("hypro.representations.supportFunction","UNION, point: " << _point);
			for(const auto& set : unionParameters()->items) {
				if(set->contains(_point)) {
					return true;
				}
			}
			return false;
		}
		case SF_TYPE::INTERSECT: {
			DEBUG("hypro.representations.supportFunction","INTERSECTION, point: " << _point);
			return (intersectionParameters()->rhs->contains(_point) && intersectionParameters()->lhs->contains(_point));
		}
		case SF_TYPE::NONE: {
			std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
			assert(false);
			return false;
		}
		default:
			DEBUG("hypro.representations.supportFunction","UNKNOWN, point: " << _point);
			assert( false );
			return false;
	}
	return true;
}

template <typename Number>
std::shared_ptr<SupportFunctionContent<Number>> SupportFunctionContent<Number>::unite(
	  const std::shared_ptr<SupportFunctionContent<Number>>& rhs ) const {
	return create(SF_TYPE::UNITE, getThis(), rhs);
}

template<typename Number>
std::shared_ptr<SupportFunctionContent<Number>> SupportFunctionContent<Number>::unite( const std::vector<std::shared_ptr<SupportFunctionContent<Number>>>& rhs ) {
	return create(SF_TYPE::UNITE, rhs);
}

template <typename Number>
std::shared_ptr<SupportFunctionContent<Number>> SupportFunctionContent<Number>::scale( const Number& factor ) const {;
	return create(getThis(), factor);
}

template <typename Number>
bool SupportFunctionContent<Number>::empty() const {
	assert(checkTreeValidity());
	switch ( mType ) {
		case SF_TYPE::ELLIPSOID: {
			return ellipsoid()->empty();
		}
		case SF_TYPE::INFTY_BALL:
		case SF_TYPE::TWO_BALL: {
			return ball()->empty();
		}
		case SF_TYPE::LINTRAFO: {
			return linearTrafoParameters()->origin->empty();
		}
		case SF_TYPE::POLY: {
			return polytope()->empty();
		}
		case SF_TYPE::PROJECTION: {
			if(projectionParameters()->dimensions.empty()){
				return true;
			}
			return projectionParameters()->origin->empty();
		}
		case SF_TYPE::SCALE: {
			if ( scaleParameters()->factor == 0 )
				return true;
			else
				return scaleParameters()->origin->empty();  // Todo: What if factor is negative?
		}
		case SF_TYPE::SUM: {
			return ( summands()->lhs->empty() && summands()->rhs->empty() );
		}
		case SF_TYPE::UNITE: {
			for(const auto& item : unionParameters()->items) {
				if(item->empty()) {
					return true;
				}
			}
			return false;
		}
		case SF_TYPE::INTERSECT: {
			if (intersectionParameters()->rhs->empty() || intersectionParameters()->lhs->empty()) {
				return true;
			}
			// TODO: Current implementation uses template evaluation.
			std::vector<vector_t<Number>> directions = computeTemplate<Number>(this->dimension(), defaultTemplateDirectionCount);
			for(const auto& direction : directions){
				Number rhsPos = intersectionParameters()->rhs->evaluate(direction, false).supportValue;
				Number lhsNeg = intersectionParameters()->lhs->evaluate(-direction, false).supportValue;
				if(rhsPos < -lhsNeg) return true;
				Number rhsNeg = intersectionParameters()->rhs->evaluate(-direction, false).supportValue;
				Number lhsPos = intersectionParameters()->lhs->evaluate(direction, false).supportValue;
				if(-rhsNeg > lhsPos) return true;
			}
			return false;
		}
		case SF_TYPE::NONE: {
			std::cout << __func__ << ": SF Type not properly initialized!" << std::endl;
			assert(false);
			return false;
		}
		default:
			assert( false );
			return false;
	}
}

template <typename Number>
void SupportFunctionContent<Number>::cleanUp() const {
	using Node = std::shared_ptr<SupportFunctionContent<Number>>;
	std::vector<Node> callStack;

	callStack.push_back(getThis());

	while(!callStack.empty()) {
		Node cur = callStack.back();
		callStack.pop_back();

		if(cur->originCount() == 0) {
			switch(cur->type()) {
				case SF_TYPE::POLY: {
					cur->polytope()->cleanUp();
					break;
				}
				case SF_TYPE::INFTY_BALL:
				case SF_TYPE::TWO_BALL:
				case SF_TYPE::ELLIPSOID: {
		            break;
		        }
		        default:
		        	assert(false);
			}

		} else {
			// this is the branch for calling recursively
			switch ( cur->type() ) {
		        case SF_TYPE::SUM: {
					callStack.push_back(cur->summands()->rhs);
					callStack.push_back(cur->summands()->lhs);
					break;
				}
				case SF_TYPE::INTERSECT: {
					callStack.push_back(cur->intersectionParameters()->rhs);
					callStack.push_back(cur->intersectionParameters()->lhs);
					break;
				}
				case SF_TYPE::LINTRAFO: {
					callStack.push_back(cur->linearTrafoParameters()->origin);
					break;
				}
				case SF_TYPE::SCALE: {
					callStack.push_back(cur->scaleParameters()->origin);
					break;
				}
				case SF_TYPE::UNITE: {
					for(unsigned i = 0; i < cur->unionParameters()->items.size(); ++i){
						callStack.push_back(cur->unionParameters()->items.at(i));
					}
					break;
				}
				case SF_TYPE::POLY:
				case SF_TYPE::INFTY_BALL:
				case SF_TYPE::TWO_BALL:
				case SF_TYPE::ELLIPSOID:
				case SF_TYPE::BOX:
				case SF_TYPE::ZONOTOPE: {
					assert(false);
					break;
				}
				case SF_TYPE::PROJECTION: {
					callStack.push_back(cur->projectionParameters()->origin);
					break;
				}
				case SF_TYPE::NONE:
				default:
					assert(false);
		    }
		}
	}
}

template <typename Number>
void SupportFunctionContent<Number>::print() const {
	switch ( mType ) {
		case SF_TYPE::ELLIPSOID: {
			std::cout << "ELLIPSOID" << std::endl;
		} break;
		case SF_TYPE::INFTY_BALL: {
			std::cout << "INFTY-BALL" << std::endl;
		} break;
		case SF_TYPE::TWO_BALL: {
			std::cout << "2-BALL" << std::endl;
		} break;
		case SF_TYPE::LINTRAFO: {
			std::cout << "LINTRAFO A^" << linearTrafoParameters()->currentExponent << std::endl;
			std::cout << "of" << std::endl;
			linearTrafoParameters()->origin->print();
		} break;
		case SF_TYPE::POLY: {
			std::cout << "POLY" << std::endl;
                        polytope()->print();
		} break;
		case SF_TYPE::PROJECTION: {
			std::cout << "PROJECTION" << std::endl;
			std::cout << "of" << std::endl;
			projectionParameters()->origin->print();
		} break;
		case SF_TYPE::SCALE: {
			std::cout << "SCALE" << std::endl;
		} break;
		case SF_TYPE::SUM: {
			std::cout << "SUM" << std::endl;
			std::cout << "of: " << std::endl;
			summands()->rhs->print();
			std::cout << "and" << std::endl;
			summands()->lhs->print();
		} break;
		case SF_TYPE::UNITE: {
			std::cout << "UNITE" << std::endl;
		} break;
		case SF_TYPE::INTERSECT: {
			std::cout << "INTERSECTION " << std::endl;
			std::cout << "of" << std::endl;
			intersectionParameters()->lhs->print();
			std::cout << "and" << std::endl;
			intersectionParameters()->rhs->print();
		} break;
		default:
			std::cout << "NONE" << std::endl;
	}
}

}  // namespace
