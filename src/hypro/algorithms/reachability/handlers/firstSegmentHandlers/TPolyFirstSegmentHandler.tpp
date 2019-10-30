#pragma once 

#include "TPolyFirstSegmentHandler.h"

namespace hypro {


	template<typename State>
    vector_t<typename State::NumberType> TPolyFirstSegmentHandler<State>::gradientOfLinearFct(const vector_t<Number>& linearFct){
        assert(linearFct.rows() == this->mState->getDimension() + 1);
        vector_t<Number> gradient = linearFct;
        gradient(gradient.rows()-1) = 0;
        return gradient;
    }

    template<typename State>
    vector_t<typename State::NumberType> TPolyFirstSegmentHandler<State>::lieDerivative(const vector_t<Number>& dir){
        assert(dir.rows() == this->mState->getLocation()->getLinearFlow().getFlowMatrix().transpose().rows());
        assert(this->mState->getLocation()->getLinearFlow().getFlowMatrix().transpose().rows() == this->mState->getLocation()->getLinearFlow().getFlowMatrix().transpose().cols());
        return this->mState->getLocation()->getLinearFlow().getFlowMatrix().transpose() * gradientOfLinearFct(dir);
    }

    template<typename State>
    tNumber TPolyFirstSegmentHandler<State>::maxValueAtRoots(const carl::UnivariatePolynomial<tNumber>& polynom, const carl::Interval<tNumber>& interval){
        //std::cout << "TPolyFirstSegmentHandler::maxValueAtRoots" << std::endl;
        tNumber max(-1e10);
        tNumber valueAtRoot = 0;
        std::vector<carl::RealAlgebraicNumber<tNumber>> roots = carl::rootfinder::RealRootIsolation<tNumber>(polynom, interval).get_roots();
        //std::cout << "TPolyFirstSegmentHandler::maxValueAtRoots, roots are: {";
        //for(const auto& r : roots){
        //    //std::cout << r << ",";
        //}
        //std::cout << "}" << std::endl;
        for(const carl::RealAlgebraicNumber<tNumber>& root : roots){
            tNumber rootNumber(0);
            if(root.isNumeric()){
                rootNumber = root.value();
            } else if(root.isInterval()) {
                rootNumber = (root.getInterval().lower() + root.getInterval().upper()) / tNumber(2);
            } else {
                throw std::runtime_error("TPolyFirstSegmentHandler::handle, unknown time number type.");
                exit(1);
            }
            valueAtRoot = polynom.evaluate(rootNumber);
            if(valueAtRoot > max){
                max = valueAtRoot;
            }
        }   
        return max; 
    }

	template<typename State>
	void TPolyFirstSegmentHandler<State>::handle(){

		assert(this->mState->getSetType(this->mIndex) == representation_name::polytope_t);
        auto tpoly = boost::apply_visitor(genericConvertAndGetVisitor<TemplatePolyhedron<typename State::NumberType>>(), this->mState->getSet(this->mIndex));
        vector_t<Number> newVec(tpoly.vector().rows());
		
		//For each template row:
		for(int rowI = 0; rowI < tpoly.matrix().rows(); ++rowI){

			//Compute Lie derivative to power of m from initial.matrix().row(i)
            Number factorial = 1;
            vector_t<Number> derivative(tpoly.matrix().cols()+1);
            derivative.block(0,0,tpoly.matrix().cols(),1) = tpoly.matrix().row(rowI).transpose();
            derivative(derivative.rows()-1) = 0;

            std::vector<tNumber> polynomCoeffs; //Ordering is from lowest to highest degree
            std::vector<Number> polynomCoeffsAsNumber; //Ordering is from lowest to highest degree
            polynomCoeffs.emplace_back(tpoly.vector()(rowI));
            polynomCoeffsAsNumber.emplace_back(tpoly.vector()(rowI));
            EvaluationResult<Number> evalRes;
			
			//For first m coefficients in polynom:
			for(int coeffI = 1; (unsigned)coeffI < TemplatePolyhedron<Number>::Settings::DERIVATIVE_ORDER; ++coeffI){ 
            //for(int coeffI = 1; coeffI < tpoly.matrix().rows() + 1; ++coeffI){

				factorial *= coeffI;
				derivative = lieDerivative(derivative);	
                vector_t<Number> derivVarCoeffs(derivative.rows()-1);
                derivVarCoeffs = derivative.block(0,0,derivVarCoeffs.rows(),1);

				//if index < m+1: Solve LP over tpoly to evaluate into derivative direction
				//if index == m+1: Solve LP over TPoly(matrix(),invariants) to evaluate into derivative direction (remainder term only bounded by invariants)
                if((unsigned)coeffI < TemplatePolyhedron<Number>::Settings::DERIVATIVE_ORDER - 1){
                //if(coeffI < tpoly.matrix().rows()){
                    if(derivVarCoeffs == vector_t<Number>::Zero(derivVarCoeffs.rows())){
                        evalRes = EvaluationResult<Number>(derivative(derivative.rows()-1), SOLUTION::FEAS);
                    } else {
                        evalRes = tpoly.evaluate(derivative / factorial, true);    
                    }
                } else {
                    //NOTE: How to treat remainder? Normally, use tpoly.matrix() as matrix, and use giant coeffs where bounds do not exist but evaluating then does not make any sense.
                    //If using the invariant matrix we either get the same or better results.
                    if(this->mState->getLocation()->getInvariant().empty()){
                        evalRes = EvaluationResult<Number>(SOLUTION::INFTY);
                    } else {
                        assert(this->mState->getLocation()->getInvariant().getMatrix().rows() == this->mState->getLocation()->getInvariant().getVector().rows());
                        assert(this->mState->getLocation()->getInvariant().getMatrix().cols() == this->mState->getDimension());
                        TemplatePolyhedron<Number> invTPoly(this->mState->getLocation()->getInvariant().getMatrix(), this->mState->getLocation()->getInvariant().getVector());
                        evalRes = invTPoly.evaluate(derivVarCoeffs / factorial, true);    
                    }
                }

                //Save resulting value as coeff in polynomCoeffs
                if(evalRes.errorCode == SOLUTION::FEAS){
                    if(TemplatePolyhedron<Number>::Settings::DIRECTLY_COMPUTE_ROOTS){
                        polynomCoeffs.emplace_back(carl::convert<Number,tNumber>(evalRes.supportValue));
                    } else {
                        polynomCoeffs.emplace_back(carl::convert<Number,tNumber>(evalRes.supportValue));
                        polynomCoeffsAsNumber.emplace_back(evalRes.supportValue);
                    }  
                } 
            }
            
            //Get maximum value of polynom in time interval [0,delta] save in new coeff vec
            if(TemplatePolyhedron<Number>::Settings::DIRECTLY_COMPUTE_ROOTS){
                //Compute derivative of polynom
                carl::Variable var = carl::freshRealVariable("t");
                carl::UnivariatePolynomial<tNumber> polynom(var, polynomCoeffs);
                carl::UnivariatePolynomial<tNumber> polynomDeriv = carl::derivative(polynom);
                //Compute roots that lie in interval and check for maximal value there
                tNumber max = maxValueAtRoots(polynomDeriv, carl::Interval<tNumber>(tNumber(0),this->mTimeStep));    
                //Get value for 0 and for this->mTimeStep and then somehow compute firstSegment
                tNumber valueAtRoot = polynom.evaluate(tNumber(0));
                if(valueAtRoot > max){
                    max = valueAtRoot;
                }
                valueAtRoot = polynom.evaluate(this->mTimeStep);
                if(valueAtRoot > max){
                    max = valueAtRoot;
                }  
                //Set value in coeff vec
                newVec(rowI) = carl::convert<tNumber,Number>(max);
            } else {
                //Check approximately whether interval is monotonically in-/decreasing
                //Check some values in [0,mTimeStep] and whether they in-/decrease throughoutly.
                carl::Variable var = carl::freshRealVariable("t");
                carl::UnivariatePolynomial<Number> polynom(var, polynomCoeffsAsNumber);
                std::vector<Number> values;
                Number valueAtRoot = 0;
                bool increasing = true;
                bool decreasing = true;
                for(tNumber timeStepPart = 0; timeStepPart <= this->mTimeStep; timeStepPart += (this->mTimeStep / 4)){
                    values.emplace_back(polynom.evaluate(carl::convert<tNumber,Number>(timeStepPart)));
                    //std::cout << "TPolyFirstSegmentHandler::handle, timeStepPart: " << timeStepPart << " values: {";
                    //for(const auto& v : values){
                    //    //std::cout << v << ",";
                    //}
                    //std::cout << "}" << std::endl;
                    if(values.size() >= 2){
                        if(increasing && values.at(values.size()-2) > values.at(values.size()-1)){
                            //std::cout << "TPolyFirstSegmentHandler::handle, not increasing anymore!" << std::endl;
                            increasing = false;
                        }
                        if(decreasing && values.at(values.size()-2) <= values.at(values.size()-1)){
                            //std::cout << "TPolyFirstSegmentHandler::handle, not decreasing anymore!" << std::endl;
                            decreasing = false;
                        }
                        if(!increasing && !decreasing){
                            //std::cout << "TPolyFirstSegmentHandler::handle, break!" << std::endl;
                            break;
                        }        
                    }
                }
                assert(values.size() == 5);
                //std::cout << "TPolyFirstSegmentHandler::handle, increasing: " << increasing << " decreasing: " << decreasing << std::endl;
                if(!increasing && !decreasing){
                    //If not monotonically increasing or decreasing, then there must be a maximum in the interval -> compute expensive root enumeration
                    carl::UnivariatePolynomial<tNumber> polynomAsTNumber(var, polynomCoeffs);
                    carl::UnivariatePolynomial<tNumber> polynomDerivAsTNumber = carl::derivative(polynomAsTNumber);
                    newVec(rowI) = carl::convert<tNumber,Number>(maxValueAtRoots(polynomDerivAsTNumber, carl::Interval<tNumber>(tNumber(0),this->mTimeStep)));
                } else if(!increasing && decreasing){
                    //If monotonically decreasing, then first value at t = 0 was the biggest one
                    newVec(rowI) = values.front();
                } else if(increasing && !decreasing){
                    //If monotonically increasing, then last value at t = mTimeStep was the biggest one
                    newVec(rowI) = values.back();
                } else {
                    //Should not be possible
                    throw std::runtime_error("TPolyFirstSegmentHandler::handle, polnom was both monotonically increasing as well as decreasing.");
                    exit(1);
                }
                //std::cout << "TPolyFirstSegmentHandler::handle, newVec(rowI):" << newVec(rowI) << std::endl;
            }
            
            //Set value in coeff vec
            //newVec(rowI) = carl::convert<tNumber,Number>(max);
		}
        
        //Set mComputationState vector to the new coeff vec
		tpoly.setVector(newVec);
		this->mState->setSet(boost::apply_visitor(genericInternalConversionVisitor<typename State::repVariant, TemplatePolyhedron<Number>>(tpoly), this->mState->getSet(this->mIndex)),this->mIndex);
        
        //Set flow for further computations
        this->mTrafo = this->mState->getLocation()->getLinearFlow().getFlowMatrix().block(0,0,this->mState->getDimension(),this->mState->getDimension());
        this->mTranslation = this->mState->getLocation()->getLinearFlow().getFlowMatrix().block(0,this->mState->getDimension(),this->mState->getDimension(),1);
        this->mFlow = affineFlow<Number>(this->mTrafo,this->mTranslation);
	}

}

