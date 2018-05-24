/**
 * Wrapper class for SupportFunctionContent for easier use.
 * @file   SupportFunction.tpp
 * @author Simon Froitzheim
 *
 * @since      2015-11-23
 * @version    2015-12-16
 */

#include "SupportFunction.h"

namespace hypro{

//CONSTRUCTORS


    //-------PRIVATE---------
    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>::SupportFunctionT(const std::shared_ptr<SupportFunctionContent<Number>> _source) : content(_source){
        //handled by initializer list
    }

    //-------PUBLIC---------
    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>::SupportFunctionT() {
        //handled by initializer list
    }

    //copy constructor
    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>::SupportFunctionT( const SupportFunctionT<Number,Converter> &_orig ) : content(_orig.content){
        //handled by initializer list
    }

    //move constructor
    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>::SupportFunctionT( SupportFunctionT<Number,Converter>&& other) : SupportFunctionT<Number,Converter>(){
        swap(*this, other);
    }


    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>::SupportFunctionT(SF_TYPE _type, Number _radius, unsigned dimension ) : content(SupportFunctionContent<Number>::create(_type, _radius, dimension)){
        //handled by initializer list
    }

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>::SupportFunctionT(const std::vector<Point<Number>>& _vertices)
        : content(SupportFunctionContent<Number>::create(SF_TYPE::POLY, _vertices)) {
        //handled by initializer list
    }

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>::SupportFunctionT(const matrix_t<Number>& _directions, const vector_t<Number>& _distances) : content(SupportFunctionContent<Number>::create(SF_TYPE::POLY, _directions, _distances)){
         //handled by initializer list
    }

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>::SupportFunctionT(const std::vector<Halfspace<Number>>& _planes) : content(SupportFunctionContent<Number>::create(SF_TYPE::POLY, _planes)){
         //handled by initializer list
    }

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>::SupportFunctionT(const matrix_t<Number>& _shapeMatrix) : content(SupportFunctionContent<Number>::create(SF_TYPE::ELLIPSOID, _shapeMatrix)){
         //handled by initializer list
    }

    //destructor
    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>::~SupportFunctionT() {
    }

// OPERATOR OVERLOADING

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>& SupportFunctionT<Number,Converter>::operator=(SupportFunctionT<Number,Converter> _orig) {
        //std::cout << "SupportFunction Copy\n";
        if (this != &_orig){
            swap(*this, _orig);
        }
        return *this;
    }


// FUNCTIONS

    template<typename Number, typename Converter>
    EvaluationResult<Number> SupportFunctionT<Number,Converter>::evaluate( const vector_t<Number> &_direction, bool useExact ) const {
    	matrix_t<Number> tmpMatrix = _direction.transpose();
        EvaluationResult<Number> tmp = content->multiEvaluate(tmpMatrix, useExact).at(0);
        TRACE("hypro.representations.supportFunction", "(" << _direction << ") :" << tmp.supportValue);
        //return content->evaluate(_direction, useExact);
		return tmp;
    }

    template<typename Number, typename Converter>
    std::vector<EvaluationResult<Number>> SupportFunctionT<Number,Converter>::multiEvaluate( const matrix_t<Number> &_directions, bool useExact ) const {
        TRACE("hypro.representations.supportFunction", "Directions: " << _directions);
        std::vector<EvaluationResult<Number>> res = content->multiEvaluate(_directions, useExact);
        assert(res.size() == std::size_t(_directions.rows()));
        //TRACE("hypro.representations.supportFunction", "Distances: ");
        //for(const auto& item : res){
        //	TRACE("hypro.representations.supportFunction", item.supportValue);
        //}
        return res;
    }

    template<typename Number, typename Converter>
    std::size_t SupportFunctionT<Number,Converter>::dimension() const {
        return content->dimension();
    }

    template<typename Number, typename Converter>
    SF_TYPE SupportFunctionT<Number,Converter>::sfType() const {
        return content->type();
    }

    template<typename Number, typename Converter>
    unsigned SupportFunctionT<Number,Converter>::depth() const {
        return content->depth();
    }

    template<typename Number, typename Converter>
    unsigned SupportFunctionT<Number,Converter>::multiplicationsPerEvaluation() const {
        return content->multiplicationsPerEvaluation();
    }

	template<typename Number, typename Converter>
    void SupportFunctionT<Number,Converter>::cleanUp() const {
    	content->cleanUp();
    }

    template<typename Number, typename Converter>
    std::string SupportFunctionT<Number,Converter>::getDotRepresentation() const {
    	std::string nodes = "digraph structs \n { node [shape=record];\n";
    	std::string transitions = "";

    	content->getDotRepresentation(0,nodes,transitions);

    	return nodes + transitions + "}\n";
    }

    template<typename Number, typename Converter>
    unsigned SupportFunctionT<Number,Converter>::operationCount() const {
        return content->operationCount();
    }

    template<typename Number, typename Converter>
    sumContent<Number> *SupportFunctionT<Number,Converter>::summands() const {
       return content->summands();
    }

    template<typename Number, typename Converter>
    trafoContent<Number,SupportFunctionContentSetting> *SupportFunctionT<Number,Converter>::linearTrafoParameters() const {
        return content->linearTrafoParameters();
    }

    template<typename Number, typename Converter>
    scaleContent<Number> *SupportFunctionT<Number,Converter>::scaleParameters() const {
        return content->scaleParameters();
    }

    template<typename Number, typename Converter>
    unionContent<Number> *SupportFunctionT<Number,Converter>::unionParameters() const {
        return content->unionParameters();
    }

    template<typename Number, typename Converter>
    intersectionContent<Number> *SupportFunctionT<Number,Converter>::intersectionParameters() const {
        return content->intersectionParameters();
    }

    template<typename Number, typename Converter>
    PolytopeSupportFunction<Number,PolytopeSupportFunctionSetting> *SupportFunctionT<Number,Converter>::polytope() const {
        return content->polytope();
    }

    template<typename Number, typename Converter>
    BallSupportFunction<Number> *SupportFunctionT<Number,Converter>::ball() const {
        return content->ball();
    }

    template<typename Number, typename Converter>
    EllipsoidSupportFunction<Number> *SupportFunctionT<Number,Converter>::ellipsoid() const {
        return content->ellipsoid();
    }

	template<typename Number, typename Converter>
    matrix_t<Number> SupportFunctionT<Number,Converter>::matrix() const {
    	if(!mTemplateSet) {
    		evaluateTemplate();
    	}
    	return mMatrix;
    }

	template<typename Number, typename Converter>
	vector_t<Number> SupportFunctionT<Number,Converter>::vector() const {
		if(!mTemplateSet) {
    		evaluateTemplate();
    	}
    	return mVector;
	}

    template<typename Number, typename Converter>
    void SupportFunctionT<Number,Converter>::removeRedundancy() {
        // Support functions are already non-redundant (Polytope support functions are made non-redundant upon construction).
    }

    template<typename Number, typename Converter>
    std::vector<Point<Number>> SupportFunctionT<Number,Converter>::vertices(const matrix_t<Number>& additionalDirections) const {
		std::vector<vector_t<Number>> additionalDirectionVector;
		for(unsigned rowIndex = 0; rowIndex < additionalDirections.rows(); ++rowIndex){
			additionalDirectionVector.push_back(vector_t<Number>(additionalDirections.row(rowIndex)));
		}

		//std::cout << "Added " << additionalDirections.size() << " additional directions for evaluation." << std::endl;
		/*
		std::list<unsigned> projections = collectProjections();
		if( projections.size() == this->dimension() ){
			//std::cout << "Full vertices" << std::endl;
			auto tmp = Converter::toHPolytope(*this, additionalDirections);
			return tmp.vertices();
		} else {
			//std::cout << "Projection" << std::endl;
			std::list<unsigned> zeroDimensions;
			for(unsigned i = 0; i < this->dimension(); ++i) {
				if(std::find(projections.begin(), projections.end(), i) == projections.end()){
					//std::cout << "Dimension " << i << " is zero." << std::endl;
					zeroDimensions.emplace_back(i);
				}
			}
			std::vector<vector_t<Number>> templateDirections = computeTemplate<Number>(projections, 8, this->dimension()); // TODO: ATTENTION, 8 is hardcoded here.
			for(auto& direction : additionalDirections) {
				// project direction
				for(const auto& dir : zeroDimensions) {
					direction(dir) = 0;
				}
				// add projected direction
				if(direction != vector_t<Number>::Zero(this->dimension()) && std::find(templateDirections.begin(), templateDirections.end(), direction) == templateDirections.end()) {
					templateDirections.insert(templateDirections.end(), std::move(direction));
				}
			}

			matrix_t<Number> templateDirectionMatrix = matrix_t<Number>(templateDirections.size(), this->dimension());

			//fills the matrix with the template directions
			for (unsigned i=0; i<templateDirections.size();++i){
				templateDirectionMatrix.row(i) = templateDirections[i];
			}
			//std::cout << "TemplateDirectionMatrix: " << std::endl << templateDirectionMatrix << std::endl;

			std::vector<EvaluationResult<Number>> offsets = content->multiEvaluate(templateDirectionMatrix);
			assert(offsets.size() == unsigned(templateDirectionMatrix.rows()));

			//std::cout << "Multi-Eval done, add zero constraints" << std::endl;

			std::vector<unsigned> boundedConstraints;
			for(unsigned offsetIndex = 0; offsetIndex < offsets.size(); ++offsetIndex){
				if(offsets[offsetIndex].errorCode != SOLUTION::INFTY){
					boundedConstraints.push_back(offsetIndex);
				}
			}
			matrix_t<Number> constraints = matrix_t<Number>::Zero(boundedConstraints.size()+2*zeroDimensions.size(), this->dimension());
			vector_t<Number> constants = vector_t<Number>::Zero(boundedConstraints.size()+2*zeroDimensions.size());
			unsigned pos = boundedConstraints.size()-1;
			unsigned zeroDimensionPos = boundedConstraints.size();
			while(!boundedConstraints.empty()){
				constraints.row(pos) = templateDirectionMatrix.row(boundedConstraints.back());
				constants(pos) = offsets[boundedConstraints.back()].supportValue;
				boundedConstraints.pop_back();
				--pos;
			}

			//std::cout << "Projected polytope without zero constraints: " << std::endl << convert<Number,double>(constraints) << std::endl << convert<Number,double>(constants) << std::endl;

			// add zero dimension constraints
			while(!zeroDimensions.empty()) {
				//std::cout << "Add zero constraints for dimension " << zeroDimensions.front() << " at rows " << zeroDimensionPos << "f" << std::endl;
				vector_t<Number> zDimConstraint = vector_t<Number>::Zero(this->dimension());
				zDimConstraint(zeroDimensions.front()) = 1;
				constraints.row(zeroDimensionPos) = zDimConstraint;
				constants(zeroDimensionPos) = 0;
				//std::cout << "Positive zero constraint for dimension " << zeroDimensions.front() << ": " << zDimConstraint << std::endl;

				++zeroDimensionPos;

				zDimConstraint(zeroDimensions.front()) = -1;
				constraints.row(zeroDimensionPos) = zDimConstraint;
				constants(zeroDimensionPos) = 0;
				//std::cout << "Negative zero constraint for dimension " << zeroDimensions.front() << ": " << zDimConstraint << std::endl;

				zeroDimensions.pop_front();
				++zeroDimensionPos;
			}

			std::cout << "Projected Polytope: " << std::endl << constraints << std::endl << constants << std::endl;

			VertexEnumeration<Number> ve(constraints, constants);
			ve.enumerateVertices();
			return ve.getPoints();
		}
		*/
		auto tmp = Converter::toHPolytope(*this, additionalDirectionVector);
		return tmp.vertices();
    }

    template<typename Number, typename Converter>
    Number SupportFunctionT<Number,Converter>::supremum() const {
		Point<Number> supPoint = content->supremumPoint();
        return Point<Number>::inftyNorm(supPoint);
    }

	template<typename Number, typename Converter>
	SupportFunctionT<Number,Converter> SupportFunctionT<Number,Converter>::project(const std::vector<std::size_t>& dimensions) const {
		// check for full projection
		bool fullProjection = true;
		if(dimensions.size() == this->dimension()) {
			for(unsigned i = 0; i < this->dimension(); ++i) {
				if(dimensions.at(i) != i) {
					fullProjection = false;
					break;
				}
			}
		} else {
			fullProjection = false;
		}

		if(!fullProjection){
			DEBUG("hypro.represetations.supportFunction", "No full projection, create.");
			SupportFunctionT<Number,Converter> res = SupportFunctionT<Number,Converter>(content->project(dimensions));
			return res;
		}
		DEBUG("hypro.represetations.supportFunction", "Full projection, copy.");
		return *this;
	}

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter> SupportFunctionT<Number,Converter>::linearTransformation( const matrix_t<Number>& A ) const {
        SupportFunctionT<Number,Converter> res = SupportFunctionT<Number,Converter>(content->affineTransformation(A, vector_t<Number>::Zero(A.rows())));
        return res;
    }

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter> SupportFunctionT<Number,Converter>::affineTransformation( const matrix_t<Number>& A, const vector_t<Number>& b ) const {
        SupportFunctionT<Number,Converter> res = SupportFunctionT<Number,Converter>(content->affineTransformation(A, b));
        return res;
    }

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>  SupportFunctionT<Number,Converter>::minkowskiSum( const SupportFunctionT<Number,Converter> &_rhs ) const {
        SupportFunctionT<Number,Converter> res = SupportFunctionT<Number,Converter>(content->minkowskiSum(_rhs.content));
        return res;
    }

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter> SupportFunctionT<Number,Converter>::intersect(const SupportFunctionT<Number,Converter> &_rhs) const {
        SupportFunctionT<Number,Converter> res = SupportFunctionT<Number,Converter>(content->intersect(_rhs.content));
        return res;
    }

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter> SupportFunctionT<Number,Converter>::intersectHalfspace( const Halfspace<Number>& hs ) const{
    	matrix_t<Number> mat = matrix_t<Number>(1,hs.normal().rows());
    	for(unsigned i = 0; i < hs.normal().rows(); ++i){
    		mat(0,i) = hs.normal()(i);
    	}
    	vector_t<Number> vec = vector_t<Number>(1);
    	vec(0) = hs.offset();
        SupportFunctionT<Number,Converter> res = SupportFunctionT<Number,Converter>(content->intersect(SupportFunctionT<Number,Converter>(mat,vec).content));
        return res;
    }

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter> SupportFunctionT<Number,Converter>::intersectHalfspaces( const matrix_t<Number>& _mat, const vector_t<Number>& _vec ) const{
        SupportFunctionT<Number,Converter> res = SupportFunctionT<Number,Converter>(content->intersect(SupportFunctionT<Number,Converter>(_mat,_vec).content));
        return res;
    }

    template<typename Number, typename Converter>
    bool SupportFunctionT<Number,Converter>::contains( const Point<Number> &_point ) const {
        return content->contains(_point);
    }

    template<typename Number, typename Converter>
    bool SupportFunctionT<Number,Converter>::contains( const vector_t<Number> &_point ) const {
        return content->contains(_point);
    }

    template<typename Number, typename Converter>
    bool SupportFunctionT<Number,Converter>::contains( const SupportFunctionT<Number, Converter>& rhs, std::size_t directions ) const {
    	std::vector<vector_t<Number>> templateDirections = computeTemplate<Number>(this->dimension(), directions);
    	for(const auto& direction : templateDirections) {
    		if(this->evaluate(direction).supportValue < rhs.evaluate(direction).supportValue){
				return false;
			}
    	}
    	return true;
    }

    template<typename Number, typename Converter>
    bool SupportFunctionT<Number,Converter>::contains( const SupportFunctionT<Number, Converter>& rhs ) const {
    	std::vector<vector_t<Number>> templateDirections = computeTemplate<Number>(this->dimension(), 8);
    	for(const auto& direction : templateDirections) {
    		if(this->evaluate(direction).supportValue < rhs.evaluate(direction).supportValue){
				return false;
			}
    	}
    	return true;
    }

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter>  SupportFunctionT<Number,Converter>::unite( const SupportFunctionT<Number,Converter> &_rhs ) const {
        return SupportFunctionT<Number,Converter>(content->unite(_rhs.content));
    }

	template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter> unite( const std::vector<SupportFunctionT<Number,Converter>>& _rhs ) {
    	std::vector<SupportFunctionContent<Number>> converted;
    	for(const auto& set : _rhs) {
    		converted.push_back(set->content);
    	}
    	return SupportFunctionT<Number,Converter>(SupportFunctionContent<Number>::unite(converted));
    }

    template<typename Number, typename Converter>
    SupportFunctionT<Number,Converter> SupportFunctionT<Number,Converter>::scale( const Number &_factor ) const {
        return SupportFunctionT<Number,Converter>(content->scale( _factor));
    }

    template<typename Number, typename Converter>
    std::pair<CONTAINMENT, SupportFunctionT<Number,Converter>> SupportFunctionT<Number,Converter>::satisfiesHalfspace( const Halfspace<Number>& rhs ) const {
        //std::cout << __func__ << ": " << _mat << std::endl << " <= " << _vec <<  std::endl;

		bool limiting = false;
    	EvaluationResult<Number> planeEvalRes = content->evaluate(rhs.normal(), false);
    	if(planeEvalRes.errorCode == SOLUTION::INFEAS){
			//std::cout << "Is infeasible (should not happen)." << std::endl;
			//std::cout << "Set is (Hpoly): " << std::endl << Converter::toHPolytope(*this) << std::endl;
			assert(Converter::toHPolytope(*this).empty());
    		return std::make_pair(CONTAINMENT::NO, *this);
    	} else if(planeEvalRes.supportValue > rhs.offset()){
			//std::cout << "Object will be limited. " << std::endl;
    		// the actual object will be limited by the new plane
    		limiting = true;
			// std::cout << "evaluate(" << convert<Number,double>(-(_mat.row(rowI))) << ") <=  " << -(_vec(rowI)) << ": " << content->evaluate(-(_mat.row(rowI))).supportValue << " <= " << -(_vec(rowI)) << std::endl;
    		// std::cout << __func__ <<  ": Limiting plane " << convert<Number,double>(_mat.row(rowI)).transpose() << " <= " << carl::toDouble(_vec(rowI)) << std::endl;
            if(content->evaluate(-(rhs.normal()), false ).supportValue < -(rhs.offset())){
				//std::cout << "fullyOutside" << std::endl;
                // the object lies fully outside one of the planes -> return false
                return std::make_pair(CONTAINMENT::NO, this->intersectHalfspace(rhs) );
            }
    	}

    	if(limiting){
        	return std::make_pair(CONTAINMENT::PARTIAL, this->intersectHalfspace(rhs));
    	} else {
    		return std::make_pair(CONTAINMENT::FULL, *this);
    	}
    }

    template<typename Number, typename Converter>
    std::pair<CONTAINMENT, SupportFunctionT<Number,Converter>> SupportFunctionT<Number,Converter>::satisfiesHalfspaces( const matrix_t<Number>& _mat, const vector_t<Number>& _vec ) const {
        TRACE("hypro.representations.supportFunction","Matrix: " << _mat << std::endl << " <= " << _vec );
		if(_mat.rows() == 0) {
			return std::make_pair(CONTAINMENT::FULL, *this);
		}
		assert(_mat.rows() == _vec.rows());
        std::vector<unsigned> limitingPlanes;
        for(unsigned rowI = 0; rowI < _mat.rows(); ++rowI) {
        	TRACE("hypro.representations.supportFunction", "Evaluate against plane " << rowI );
        	EvaluationResult<Number> planeEvalRes = content->evaluate(_mat.row(rowI), false);
        	if(planeEvalRes.errorCode == SOLUTION::INFEAS){
				TRACE("hypro.representations.supportFunction", "Is infeasible (should not happen)." );
				TRACE("hypro.representations.supportFunction", "Set is (Hpoly): " << std::endl << Converter::toHPolytope(*this) );
        		return std::make_pair(CONTAINMENT::NO, *this);
        	//} else if(!carl::AlmostEqual2sComplement(planeEvalRes.supportValue, _vec(rowI), 2) && planeEvalRes.supportValue > _vec(rowI)){
        	} else if(planeEvalRes.supportValue > _vec(rowI)){
				TRACE("hypro.representations.supportFunction", "Object will be limited, as " << planeEvalRes.supportValue << " > " << _vec(rowI));
        		// the actual object will be limited by the new plane
        		limitingPlanes.push_back(rowI);
				Number invDirVal = content->evaluate(-(_mat.row(rowI)), false).supportValue;
				//TRACE("hypro.representations.supportFunction", "evaluate(" << -(_mat.row(rowI)) << ") <=  " << -(_vec(rowI)) << ": " << invDirVal << " <= " << -(_vec(rowI)));
        		//TRACE("hypro.representations.supportFunction", ": Limiting plane " << _mat.row(rowI).transpose() << " <= " << carl::toDouble(_vec(rowI)));

	            //if(!carl::AlmostEqual2sComplement(invDirVal, Number(-(_vec(rowI))), 2) && invDirVal < -(_vec(rowI))) {
	            if(invDirVal < -(_vec(rowI))) {
	            	// exact verification in case the values are close to each other
	            	if(carl::AlmostEqual2sComplement(Number(-invDirVal), planeEvalRes.supportValue, 16)) {
	            		EvaluationResult<Number> secndPosEval = content->evaluate(_mat.row(rowI), true);
	            		if(secndPosEval.supportValue > _vec(rowI)) {
	            			EvaluationResult<Number> secndNegEval = content->evaluate(-(_mat.row(rowI)), true);
	            			if(secndNegEval.supportValue < -(_vec(rowI))) {
	            				TRACE("hypro.representations.supportFunction", "fullyOutside" );
				                // the object lies fully outside one of the planes -> return false
				                return std::make_pair(CONTAINMENT::NO, this->intersectHalfspaces(_mat,_vec) );
	            			}
	            		}
	            	} else {
	            		// the values are far enough away from each other to make this result a false negative.
	            		TRACE("hypro.representations.supportFunction", "fullyOutside, as " << invDirVal << " >= " << -(_vec(rowI)) );
		                // the object lies fully outside one of the planes -> return false
		                return std::make_pair(CONTAINMENT::NO, this->intersectHalfspaces(_mat,_vec) );
	            	}
	            }
        	}
        }
    	if(limitingPlanes.size() < unsigned(_mat.rows())){
    		if(limitingPlanes.size() == 0 ){
    			TRACE("hypro.representations.supportFunction", " Object will stay the same");
    			return std::make_pair(CONTAINMENT::FULL, *this);
    		}
    		TRACE("hypro.representations.supportFunction", " Object will be limited but not empty (" << limitingPlanes.size() << " planes)");
    		// if the result is not fullyOutside, only add planes, which affect the object
        	matrix_t<Number> planes = matrix_t<Number>(limitingPlanes.size(), _mat.cols());
        	vector_t<Number> distances = vector_t<Number>(limitingPlanes.size());
        	for(unsigned i = 0; i < distances.rows(); ++i){
        		// std::cout << "Set row " << i << " to plane " << limitingPlanes.back() << std::endl;
        		planes.row(i) = _mat.row(limitingPlanes.back());
        		distances(i) = _vec(limitingPlanes.back());
        		limitingPlanes.pop_back();
        	}
			assert(limitingPlanes.empty());
        	TRACE("hypro.representations.supportFunction", "Intersect with " << planes << ", " << distances);
        	return std::make_pair(CONTAINMENT::PARTIAL, this->intersectHalfspaces(planes,distances));
    	} else {
    		TRACE("hypro.representations.supportFunction", " Object will be fully limited but not empty");
    		assert(limitingPlanes.size() == unsigned(_mat.rows()));
    		TRACE("hypro.representations.supportFunction", "Intersect with " << _mat << ", " << _vec);
    		return std::make_pair(CONTAINMENT::PARTIAL, this->intersectHalfspaces(_mat,_vec));
    	}
    }

    template<typename Number, typename Converter>
    bool SupportFunctionT<Number,Converter>::empty() const {
        return content->empty();
    }

    template<typename Number, typename Converter>
    void SupportFunctionT<Number,Converter>::print() const {
        return content->print();
    }

    template<typename Number, typename Converter>
    void SupportFunctionT<Number,Converter>::swap(SupportFunctionT<Number,Converter>& first, SupportFunctionT<Number,Converter>& second){
         first.content.swap(second.content);
     }

    template<typename Number, typename Converter>
    void SupportFunctionT<Number,Converter>::forceLinTransReduction(){
        content->forceLinTransReduction();
    }

	template<typename Number, typename Converter>
	std::vector<std::size_t> SupportFunctionT<Number,Converter>::collectProjections() const {
		assert(content->checkTreeValidity());
		return content->collectProjections();
	}

	template<typename Number, typename Converter>
	void SupportFunctionT<Number,Converter>::evaluateTemplate() const {
		if(!mTemplateSet) {
			std::vector<vector_t<Number>> templateDirections = computeTemplate<Number>(this->dimension(), defaultTemplateDirectionCount);

		    matrix_t<Number> templateDirectionMatrix = matrix_t<Number>(templateDirections.size(), this->dimension());

		    //fills the matrix with the template directions
		    for (unsigned i=0; i<templateDirections.size();++i){
		        templateDirectionMatrix.row(i) = templateDirections[i];
		    }

		    //lets the support function evaluate the offset of the halfspaces for each direction
		    std::vector<EvaluationResult<Number>> offsets = this->multiEvaluate(templateDirectionMatrix);

		    std::vector<std::size_t> boundedConstraints;
		    for(unsigned offsetIndex = 0; offsetIndex < offsets.size(); ++offsetIndex){
				//std::cout << "Result: " << offsets[offsetIndex] << std::endl;
		        if(offsets[offsetIndex].errorCode != SOLUTION::INFTY){
		            boundedConstraints.push_back(offsetIndex);
		        }
		    }
		    matrix_t<Number> constraints = matrix_t<Number>(boundedConstraints.size(), this->dimension());
		    vector_t<Number> constants = vector_t<Number>(boundedConstraints.size());
		    unsigned pos = boundedConstraints.size()-1;
		    while(!boundedConstraints.empty()){
		        constraints.row(pos) = templateDirectionMatrix.row(boundedConstraints.back());
		        constants(pos) = offsets[boundedConstraints.back()].supportValue;
		        boundedConstraints.pop_back();
		        --pos;
		    }

		    mMatrix = constraints;
		    mVector = constants;
		    mTemplateSet = true;
		}
	}


} // namespace hypro
