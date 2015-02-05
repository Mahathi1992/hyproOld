#include "ZonotopeReachability.h"
#include "util.h"

/*****************************************************************************
 *                                                                           *
 *                     Public Functions - Constructors                       *
 *                                                                           *
 *****************************************************************************/


template<typename Number>
ZonotopeReachability<Number>::ZonotopeReachability(unsigned int dimension) : mDimension(dimension), mInitialized(), mReadjusted()
{}

template<typename Number>
ZonotopeReachability<Number>::~ZonotopeReachability() {
}



/*****************************************************************************
 *                                                                           *
 *                 Public Functions - Getters and Setters                    *                             
 *                                                                           *
 *****************************************************************************/


template<typename Number>
unsigned int ZonotopeReachability<Number>::dimension() const {
    return mDimension;
}

template<typename Number>
std::vector< Zonotope<Number> > ZonotopeReachability<Number>::flowpipe() {
    return sequence_zonQ_;
}

template<typename Number>
std::map <unsigned int, std::vector< Zonotope<Number> > > ZonotopeReachability<Number>::intersections() {
    return intersect_zonotopes_;
}

template<typename Number>
std::map <unsigned int, std::vector< Zonotope<Number> > > ZonotopeReachability<Number>::resultingIntersections() {
    return resulting_intersect_;
}

template<typename Number>
std::map <unsigned int, Zonotope<Number> > ZonotopeReachability<Number>::pivotalZonotopes() {
    return pivotal_zonotopes_;
}

template<typename Number>
Zonotope<Number> ZonotopeReachability<Number>::currentSet() const {
    return Q_;
}

template<typename Number>
Zonotope<Number> ZonotopeReachability<Number>::initialInput() const {
    return U_;
}


template<typename Number>
void ZonotopeReachability<Number>::loadHybridAutomaton(hypro::HybridAutomaton<Number, Zonotope<Number> >* hybridAutomaton_) {
    mHybridAutomaton = *hybridAutomaton_;
    // pick first init state
    hypro::Location<Number> * loc =  *(mHybridAutomaton.initialLocations().begin());
    mCurrentLoc = *loc;
    A_ = loc->activityMat();
    bigB_ = loc->extInputMat();
    smallb_ = loc->activityVec();
    Q_ = mHybridAutomaton.valuation();
    U_ = mHybridAutomaton.extInputValuation();
    mInitialized = true;
}

/*****************************************************************************
 *                                                                           *
 *              Public Functions - Running the algorithm itself              *
 *                                                                           *
 *****************************************************************************/

template<typename Number>
bool ZonotopeReachability<Number>::startReachabilityAnalysis(unsigned int numIterations, 
                                            unsigned int offset, 
                                            hypro::scalar_t<Number> r_scalar, 
                                            unsigned int order_reduction_threshold,
                                            const ZUtility::Options& option)
{
    Zonotope<Number> res_V, res_S;
    return runReachabilityAnalysis(numIterations,offset, r_scalar, order_reduction_threshold, res_V, res_S, option);
}


template<typename Number>
bool ZonotopeReachability<Number>::runReachabilityAnalysis(unsigned int numIterations, 
                                            unsigned int offset,
                                            hypro::scalar_t<Number> r_scalar, 
                                            unsigned int order_reduction_threshold,
                                            Zonotope<Number>& res_V, Zonotope<Number>& res_S,
                                            const ZUtility::Options& option) 
{
    assert(mInitialized);
    hypro::vector_t<Number> null_vector;
    null_vector.resize(mDimension, Eigen::NoChange);
    null_vector.setZero();

    r_ = r_scalar;
    
    state_t cur_state = START;
    unsigned intersectCount=0;
    Zonotope<Number> guard_intersect, temp_V, temp_Q, temp_S;
    Hyperplane<Number> hp;
    hypro::Transition<Number> transition_taken;
    
    
    hypro::matrix_t<Number> minMaxOfLine;
    while (offset<=numIterations) {
        switch(cur_state) {
            case START: // Initializes the zonotope; is called everytime a zonotope is rebuilt after an intersection with another structure
                if(smallb_!=null_vector) {
                    readjust(); 
                }
                
                sequence_zonQ_.push_back(Q_); // Pushing back initial set of new state
                this->initialize(res_V, res_S);
                sequence_zonQ_.push_back(Q_); // push back second set of new state
                cur_state = NORMAL;
                offset++;
                break;
                
            case NORMAL: // executes the reachability analysis algorithm with given dynamics and checks at each step if a guard is intersected
                this->computeNextZonotope(order_reduction_threshold, Q_, res_V, res_S);   
                if (this->checkGuardJumpCondition(transition_taken, Q_, minMaxOfLine, option)) {             
                    cur_state = JUMP;
                    std::cout << "JUMP AT " << offset << std::endl;
                    temp_Q = Q_;
                    temp_V = res_V;
                    temp_S = res_S;
                   
                    // convert guard of transition to hyperplane and save in hp
                    this->guardToHyperplane(transition_taken, hp);
                    
                    // Check for first intersect
                    this->checkForIntersection(temp_Q, hp, guard_intersect, option.intersectMethod);
                    
                    if (option.intersectMethod==ZUtility::NDPROJECTION) {
                        if (mDimension>2) {
                            // Set min_intersect_ and max_intersect_ to minimum size, namely one column
                            min_intersect_[offset].resize(temp_Q.dimension()-1, 1);
                            max_intersect_[offset].resize(temp_Q.dimension()-1, 1);

                            min_intersect_[offset].col(0) = minMaxOfLine.row(0).transpose();
                            max_intersect_[offset].col(0) = minMaxOfLine.row(1).transpose();
                        }
                        else if (mDimension==2) {
                            min_intersect_[offset].resize(2,1);
                            max_intersect_[offset].resize(2,1);
                            
                            min_intersect_[offset].col(0) = minMaxOfLine.row(0).transpose();
                            max_intersect_[offset].col(0) = minMaxOfLine.row(1).transpose();
                        }
                    }
                    
                    //Save guard_intersect into set
                    resulting_intersect_[offset].push_back(guard_intersect);
                    intersect_zonotopes_[offset].push_back(temp_Q);
                    intersectCount++;
                }
                else {
                    Zonotope<Number> result;
                    // No intersection found with any guard, check if invariant is fulfilled...
                    if (fulfillsInvariant(Q_, result)) {
                        offset++;
                        sequence_zonQ_.push_back(result);
                        Q_ = result;
                    }
                    else return false;
                }
                break;
                
            case JUMP: // Jump occurs when a intersect with a guard is found. 
                // The algorithm enters the jump state and stops iterating through the given number of iterations and runs through the algorithm until no more intersections are found
                this->computeNextZonotope( order_reduction_threshold, temp_Q, temp_V, temp_S);  
                if(!this->checkForIntersection(temp_Q, hp, guard_intersect, option.intersectMethod, minMaxOfLine)) {
                    cur_state = START;
                    
                    std::cout << "Number of intersections at " << offset <<": " << intersectCount << std::endl;
                    intersectCount=0;
                    
//                    unsigned k=0;
//                     Jump to new state
//                    for (Zonotope<Number> z : resulting_intersect_[offset]) {
////                        k++;
////                        std::cout << "\tChoosing intersection " << k <<std::endl;
//                        Zonotope<Number> temp_resV = res_V, temp_resS = res_S;
//                        loadNewState(transition_taken, z);
//                        this->approxReachDiscrete(numIterations, offset, r_, order_reduction_threshold, temp_resV, temp_resS);
//                    }
//                    offset = numIterations+1;                  
////                    k=0;
                    Zonotope<Number> finalIntersect; // if constant b is added, finalIntersect's dimensionality should be dimension+1
                    switch (option.testCase) {
                        case 0: // Take first guard intersect and use that zonotope as starting point
                        {
                            finalIntersect = resulting_intersect_[offset][0];
                            break;
                        }
                        case 1: // using Girard's method used for initialization of zonotope
                        {
                            Zonotope<Number> overApproximatedIntersectZonotope = intersect_zonotopes_[offset][0];
                            for (unsigned i=0; i<intersect_zonotopes_[offset].size(); i++) {
                                std::cout << "iteration: " << i << std::endl;
                                this->overapproximatedConvexHull(overApproximatedIntersectZonotope,exp_rA_);
                                overApproximatedIntersectZonotope.minkowskiSum(overApproximatedIntersectZonotope, temp_V);
                            }
                            this->checkForIntersection(overApproximatedIntersectZonotope, hp, finalIntersect, option.intersectMethod);
                            break;
                        }
                        case 2: // min-Max method to construct hypercube
                        {
                            std::cout << "min intersect:\n " << min_intersect_[offset] << std::endl;
                            std::cout << "max intersect:\n" << max_intersect_[offset] << std::endl;

                            this->constructIntersectZonotopeFromMinMax(offset, temp_Q, finalIntersect, hp);
                            break;
                        }                   
                        case 3: // Preclustering
                        {
                            this->preclustering(intersect_zonotopes_[offset], hp, finalIntersect, option);
                            break;
                        }
                        
                        case 4:  // Post-clustering
                        {
                            this->postclustering(resulting_intersect_[offset], finalIntersect);
                            break;
                        }
                       
                    }
                    
                    this->loadNewState(transition_taken, finalIntersect);
                    

                }
                else { // if zonotopes still continue intersecting with guard
                    intersectCount++;
 
                    if (option.intersectMethod==ZUtility::NDPROJECTION) {
                        // conservatively resize min,max intersect matrices
                        unsigned numColsMinIntersect, numColsMaxIntersect;
                        numColsMinIntersect = min_intersect_[offset].cols();
                        numColsMaxIntersect = max_intersect_[offset].cols();
                                
                        min_intersect_[offset].conservativeResize(Eigen::NoChange, numColsMinIntersect+1);
                        min_intersect_[offset].col(numColsMinIntersect) = minMaxOfLine.row(0).transpose();

                        max_intersect_[offset].conservativeResize(Eigen::NoChange, numColsMaxIntersect+1);
                        max_intersect_[offset].col(numColsMaxIntersect) = minMaxOfLine.row(1).transpose();
                    }
                    
                    intersect_zonotopes_[offset].push_back(temp_Q);
                    resulting_intersect_[offset].push_back(guard_intersect);
                }
                break;
           
        }
        
    }
    std::cout << "Completed reachability analysis..." << std::endl;
    return true;

}



/*****************************************************************************
 *                                                                           *
 *                          Private Functions                                *
 *                                                                           *
 *****************************************************************************/

template<typename Number>
void ZonotopeReachability<Number>::preclustering(const std::vector< Zonotope<Number> >& intersects, 
                                const Hyperplane<Number>& hp, 
                                Zonotope<Number>& finalIntersect, 
                                const ZUtility::Options& option) 
{
    Zonotope<Number> currentConvexHull, result;
    currentConvexHull = intersects[0];
    for (unsigned i=1; i<intersects.size(); i++) { 
        currentConvexHull.convexHull(result, intersects[i]);
        currentConvexHull = result;
    }
//    pivotal_zonotopes_[offset] = currentConvexHull;
    this->checkForIntersection(currentConvexHull, hp, finalIntersect, option.intersectMethod);
}

template<typename Number>
void ZonotopeReachability<Number>::postclustering(const std::vector< Zonotope<Number> >& resultingIntersects, 
                                Zonotope<Number>& finalIntersect) 
{
    Zonotope<Number> currentConvexHull, result;
    currentConvexHull = resultingIntersects[0];
    for (unsigned i=0; i< resultingIntersects.size(); i++) {
        currentConvexHull.convexHull(result, resultingIntersects[i]);
        currentConvexHull = result;
    }
//    pivotal_zonotopes_[offset] = currentConvexHull;
    finalIntersect = currentConvexHull;
}


template<typename Number>
void ZonotopeReachability<Number>::readjustMatrices() {
    unsigned int origAdim = A_.rows(), origBdim = bigB_.rows();
    // Append b into new A matrix and hereby changing dimension of A matrix
    A_.conservativeResize(Eigen::NoChange,origAdim+1);
    A_.col(origAdim) = smallb_;
    
    // Append null row to A matrix
    A_.conservativeResize(origAdim+1, Eigen::NoChange);
    A_.row(origAdim).setZero();
    // Append null vector to B matrix
    bigB_.conservativeResize(origBdim+1, Eigen::NoChange);
    bigB_.row(origBdim).setZero(); 
}

template<typename Number>
void ZonotopeReachability<Number>::readjust() {
    unsigned int origQdim = Q_.dimension();
    readjustMatrices();
    // Change dimension of Q Zonotope
    if (!mReadjusted) {
        Q_.changeDimension(origQdim+1);
        hypro::vector_t<Number> temp_center = Q_.center();
        temp_center(Q_.dimension()-1) = 1;
        Q_.setCenter(temp_center);
        mReadjusted = true;
    }
}

template<typename Number>
void ZonotopeReachability<Number>::initialize(Zonotope<Number>& res_V, Zonotope<Number>& res_S) {
    // Variable declaration
    unsigned int dimA = A_.rows(), dimB = bigB_.cols();
    hypro::vector_t<Number> I_center = Q_.center();
    hypro::matrix_t<Number> I_generators = Q_.generators();
    hypro::matrix_t<Number> C_matrix, C_null, phiU, identity;
    Eigen::Matrix<Number, Eigen::Dynamic, Eigen::Dynamic> exp_rC, exp_rC_A, exp_rC_phiU;
    
    // Reset V_ and S_
    res_V.clear();
    res_S.clear();
    
    // Resizing dynamic matrices
    identity.resize(dimA, dimA);
    identity.setIdentity();
    phiU.resize(dimA, dimB);
    C_null.resize(dimB, dimA+dimB);
    C_null.setZero();
    C_matrix.resize(dimA+dimB, dimA+dimB);
    
    // forming C matrix for matrix exponential
    C_matrix << A_, bigB_,
                C_null;
    
    C_matrix = C_matrix * r_;
    
    exp_rC = hypro::convertMatToDouble(C_matrix).exp();
    exp_rC_A = exp_rC.block(0, 0, dimA, dimA);
    exp_rC_phiU = exp_rC.block(0, dimA, dimA, dimB);
    
    exp_rA_ = hypro::convertMatToFloatT(exp_rC_A);
    phiU = hypro::convertMatToFloatT(exp_rC_phiU);

    // Start RA algorithm: Omega0

    overapproximatedConvexHull(Q_, exp_rA_);
    
//    res_V.setCenter(phiU*U_.center());
//    res_V.addGenerators(phiU*U_.generators());
    U_.linearTransformation(res_V , phiU);
    
    hypro::vector_t<Number> c;
    c.resize(dimA,Eigen::NoChange);
    c.setZero();
    
    res_S.setCenter(c);
}

template<typename Number>
void ZonotopeReachability<Number>::overapproximatedConvexHull(Zonotope<Number>& Q, 
                                             const hypro::matrix_t<Number>& expMatrix)
{
    assert(Q.dimension() == expMatrix.cols());
    hypro::vector_t<Number> I_center = Q.center();
    hypro::matrix_t<Number> identity, 
                            I_generators = Q.generators();
    
    std::cout << "\tNum generators: " << Q.numGenerators() << std::endl;
    Number mxx, rnA, alphaR;
    if(Q.numGenerators()==0) 
        mxx = I_center.norm();
    else {
        hypro::vector_t<Number> result;
        result.resize(Q.dimension(), 1);
        result = I_center + I_generators.array().abs().matrix().rowwise().sum();
        mxx = result.norm();
    }
    // Compute alpha_r
    rnA = r_*A_.norm();
    alphaR = (std::exp((Number) rnA)-1-rnA)*mxx;
    
    identity.resize(Q.dimension(), Q.dimension());
    identity.setIdentity();
    
    // Find approximate union of zonotopes
    
    Zonotope<Number> MG(Q.dimension());
    MG.setCenter((I_center+expMatrix*I_center)/2);
    if(Q.numGenerators()==0) {
        MG.addGenerators((I_center-expMatrix*I_center)/2);
        MG.addGenerators(alphaR*identity);
    }
    else {
        MG.addGenerators(0.5*(I_generators+expMatrix*I_generators));
        MG.addGenerators((I_center-expMatrix*I_center)/2);
        MG.addGenerators(0.5*(I_generators-expMatrix*I_generators));
        MG.addGenerators(alphaR*identity);    
    }
    
    MG.removeEmptyGenerators();
    Q = MG;
}

template<typename Number>
void ZonotopeReachability<Number>::computeNextZonotope(unsigned int order_reduction_threshold,
                                            Zonotope<Number>& Q, 
                                            Zonotope<Number>& V,
                                            Zonotope<Number>& S) {
    
    Zonotope<Number> Q_previous = Q;
    
    assert(Q_previous.dimension()==exp_rA_.rows());

    Q_previous.linearTransformation(Q_previous, exp_rA_);
    V.linearTransformation(V, exp_rA_);
   
    S.minkowskiSum(S, V);
    Q_previous.minkowskiSum(Q, S);
    // Order reduction on Q
    if (Q.numGenerators() > order_reduction_threshold) {
        overapproximateZonotope(Q);
    }
}

template<typename Number>
void ZonotopeReachability<Number>::overapproximateZonotope(Zonotope<Number>& z) {
    hypro::matrix_t<Number> generators = z.generators();
    unsigned int z_rows = z.dimension();
    std::vector< hypro::vector_t<Number> > vectOfGenerators;

    for (unsigned int i=0; i<generators.cols(); i++) {
        vectOfGenerators.push_back(generators.col(i));
    }

    // Sort generators
    std::sort(vectOfGenerators.begin(), vectOfGenerators.end(), ZUtility::compareVectors<Number>);
    
    // Rowwise sum of all generators (absolute value)
    hypro::vector_t<Number> sumVector;
    for (unsigned int i=0; i<2*(z_rows); i++) {
        if (i==0) {
            sumVector.resize(z_rows, 1);
            sumVector << vectOfGenerators[i];
        }
        else {
            sumVector += vectOfGenerators[i].array().abs().matrix();
        }
    }

    hypro::matrix_t<Number> remaining_vectors;
    unsigned int numRemainingVectors = vectOfGenerators.size()-(2*z_rows+1);
    remaining_vectors.resize(z_rows, numRemainingVectors);
    
    for (unsigned int i=0; i<numRemainingVectors; i++) {
        remaining_vectors.col(i) = vectOfGenerators[i+(2*z_rows+1)];
    }
    
    // calculate interval hull of first 2n generators
    hypro::matrix_t<Number> interval_hull = sumVector.asDiagonal();

    hypro::matrix_t<Number> generators_new;
    generators_new.resize(z_rows, remaining_vectors.cols()+z_rows);

    generators_new << interval_hull, remaining_vectors;
    z.setGenerators(generators_new);
}

template<typename Number>
void ZonotopeReachability<Number>::guardToHyperplane(const hypro::Transition<Number>& transitionTaken, 
                                                    Hyperplane<Number>& hp) 
{
    hypro::vector_t<Number> d_vec = transitionTaken.guard().mat.row(0).transpose();
    hypro::scalar_t<Number> e_scalar = transitionTaken.guard().vec(0);
    if (mReadjusted) {
        d_vec.conservativeResize(d_vec.rows()+1, Eigen::NoChange);
        d_vec(d_vec.rows()-1) = 0;
    }
    hp.setVector(d_vec);
    hp.setScalar(e_scalar);
}

template<typename Number>
void ZonotopeReachability<Number>::constructIntersectZonotopeFromMinMax(unsigned iteration, 
                                                                        const Zonotope<Number>& Q, 
                                                                        Zonotope<Number>& result, 
                                                                        const Hyperplane<Number>& hp)
{
    if (mDimension!=2) {
        hypro::matrix_t<Number> kernel, 
                                finalGenerators, 
                                globalMinMaxMatrix;
        
        // Construct a global min max matrix 
        // Note to self: globalMinMaxMatrix is similar to mlMl in MATLAB implementation
                            
        globalMinMaxMatrix.resize(2,min_intersect_[iteration].rows());
        finalGenerators.resize(Q.dimension(),Q.dimension()-1);

        for (unsigned i=0; i<min_intersect_[iteration].rows(); i++) {
           globalMinMaxMatrix(0,i) =  min_intersect_[iteration].row(i).minCoeff();
           globalMinMaxMatrix(1,i) = max_intersect_[iteration].row(i).maxCoeff();
        }

        std::cout << "Global min max intersect: \n " << globalMinMaxMatrix << std::endl;

        Eigen::JacobiSVD< hypro::matrix_t<Number> > svd(hp.vector().transpose(), 
                                                            Eigen::ComputeFullU | Eigen::ComputeFullV);
        // Using SVD to calculate nullspace (kernel)
        kernel = svd.matrixV().block(0,1,svd.matrixV().rows(), svd.matrixV().cols()-1);

        for (unsigned i=0; i<min_intersect_[iteration].rows(); i++) {
            finalGenerators.col(i) << ((globalMinMaxMatrix(1,i)-globalMinMaxMatrix(0,i))/2) * kernel.col(i);
        }

        result.setCenter(kernel * ((globalMinMaxMatrix.row(1)+globalMinMaxMatrix.row(0))/2).transpose() + hp.scalar()*hp.vector());                 
        result.setGenerators(finalGenerators);
    }
    else {
        Eigen::Matrix<hypro::scalar_t<Number>, 2, 1> p1,p2,q1,q2;
        p1 = min_intersect_[iteration].col(0);
        p2 = max_intersect_[iteration].col(0);
        if (min_intersect_[iteration].cols() > 1) {
            for (unsigned i=1; i<min_intersect_[iteration].cols(); i++) {
                q1 = min_intersect_[iteration].col(i);
                q2 = max_intersect_[iteration].col(i);
                Eigen::Matrix<hypro::scalar_t<Number>, 2, 6> pmp;
                Eigen::Matrix<hypro::scalar_t<Number>, 1, 6> n1pmp;
                pmp << p1-p2, p1-q1, p1-q2, p2-q1, p2-q2, q1-q2;
                n1pmp = pmp.array().abs().colwise().sum();
                int idx;
                n1pmp.maxCoeff(&idx);

                switch(idx) {
                    case 0:
                        q1 = p1;
                        q2 = p2;
                        break;
                    case 1:
                        q2 = p2;
                        break;
                    case 2:
                        q2 = p1;
                        break;
                    case 3:
                        q2 = p2;
                        break;
                    case 4:
                        q1 = p2;
                        break;
                    default: break;
                }
            }
            result.setCenter((q1+q2)/2);
            result.setGenerators((q1-q2)/2);
            if (mReadjusted) {
                hypro::matrix_t<Number> center_new = result.center();
                center_new.conservativeResize(3,Eigen::NoChange);
                center_new(2) = 1;
                result.changeDimension(3);
                result.setCenter(center_new);
            }

        }
        else {
            result = resulting_intersect_[iteration][0];
        }

    }
}

template<typename Number>
bool ZonotopeReachability<Number>::checkGuardJumpCondition(hypro::Transition<Number>& transition_taken,
                                            const Zonotope<Number>& Q,
                                            hypro::matrix_t<Number>& minMaxOfLine,
                                            const ZUtility::Options& option) 
{
    std::set< hypro::Transition<Number>* > possibleTransitions = mCurrentLoc.transitions();
    Zonotope<Number> intersect_zonotope, tempQ(Q);
    bool res = false;
    
    for (hypro::Transition<Number>* trans : possibleTransitions) 
    {   
        hypro::vector_t<Number> d_vec;
        Number e_scalar = trans->guard().vec(0);
        d_vec = trans->guard().mat.row(0).transpose();
        if(mReadjusted) {
            d_vec.conservativeResize(d_vec.rows()+1, Eigen::NoChange);
            d_vec(d_vec.rows()-1) = 0;
        }
        
        Hyperplane<Number> hp(d_vec, e_scalar);
        if (checkForIntersection(tempQ, hp, intersect_zonotope, option.intersectMethod, minMaxOfLine)) {
            res = true;
            transition_taken = *trans;
            break;
        }
    }
    
    return res;
}

template<typename Number>
bool ZonotopeReachability<Number>::fulfillsInvariant(const Zonotope<Number>& inputZonotope, Zonotope<Number>& result)
{
    Zonotope<Number> tempZonotope = inputZonotope;
    struct hypro::Location<Number>::invariant inv = mCurrentLoc.invariant();
    hypro::vector_t<Number> dVec;
    Number e;
    
    if (mReadjusted) {
        tempZonotope.changeDimension(tempZonotope.dimension()-1);
    }
    
    dVec.resize(this->mDimension, Eigen::NoChange);
    if (inv.mat.rows() == 0 || inv.vec.rows() == 0) return true;
    else {
        // Specific case for invariant, assume second row is not used.
        dVec = inv.mat.row(0).transpose();
        e = inv.vec(0);
    }
    
    bool res = tempZonotope.intersectWithHalfspace(result, dVec, e);
    if (mReadjusted) {
        hypro::matrix_t<Number> hold_center;
        result.changeDimension(result.dimension()+1);
        hold_center = result.center();
        hold_center(hold_center.rows()-1) = 1;
        result.setCenter(hold_center);
    }
        
    return res;   
}


template<typename Number>
void ZonotopeReachability<Number>::loadNewState(hypro::Transition<Number>& transition, const Zonotope<Number>& intersect_zonotope) 
{
    assert(intersect_zonotope.dimension() != 0);
    smallb_ = transition.targetLoc()->activityVec();
    bigB_ = transition.targetLoc()->extInputMat();
    A_ = transition.targetLoc()->activityMat();
    mCurrentLoc = *transition.targetLoc();
    
    Zonotope<Number> temp = intersect_zonotope;
    hypro::matrix_t<Number> new_transformedMat, 
                            transformMat = transition.assignment().transformMat;
    assert(transformMat.rows() == mDimension && transformMat.cols()!=0);
    if (mReadjusted) {
        new_transformedMat.conservativeResize(transformMat.rows()+1, transformMat.rows()+1);
        new_transformedMat.setIdentity();
        new_transformedMat.block(0,0,transformMat.rows(),transformMat.rows()) = transformMat;
    }
    else {
        new_transformedMat = transformMat;
    }
    assert(new_transformedMat.rows()==intersect_zonotope.center().rows());
    temp.setCenter(new_transformedMat*temp.center());
    Q_ = temp; 
}

template<typename Number>
bool ZonotopeReachability<Number>::checkForIntersection(const Zonotope<Number>& inputZonotope, const Hyperplane<Number>& hp, 
                                    Zonotope<Number>& result,
                                    const ZUtility::IntersectionMethod_t& method)
{
    hypro::matrix_t<Number> EMPTY_MATRIX(0,0);
    return (checkForIntersection(inputZonotope, hp, result, method, EMPTY_MATRIX));
}

template<typename Number>
bool ZonotopeReachability<Number>::checkForIntersection(const Zonotope<Number>& inputZonotope, const Hyperplane<Number>& hp, 
                                        Zonotope<Number>& result, 
                                        const ZUtility::IntersectionMethod_t& method,
                                        hypro::matrix_t<Number>& minMaxOfLine)

{
    assert(inputZonotope.dimension()==hp.dimension() && "input zonotope and hyperplane must have same dimensions");
    assert((method==ZUtility::ALAMO || method==ZUtility::NDPROJECTION || method == ZUtility::DICHOTOMIC2D) && "Requires valid method (ALAMO or NDPROJECTION)");
    Zonotope<Number> tempZonotope(inputZonotope);
    Hyperplane<Number> tempHp(hp);
    
    bool res;
    // Reduce order of input zonotope if readjust was applied
    if (mReadjusted) {
        tempHp.changeDimension(tempHp.dimension()-1);
        tempZonotope.changeDimension(tempZonotope.dimension()-1);
        if (result.dimension()>1)
            result.changeDimension(result.dimension()-1);
    }
    res = tempZonotope.intersect(result, tempHp, minMaxOfLine, method);
    // Enlarge result zonotope if readjust was applied
    if (mReadjusted) {
        hypro::matrix_t<Number> hold_center;
        result.changeDimension(result.dimension()+1);
        hold_center = result.center();
        hold_center(hold_center.rows()-1) = 1;
        result.setCenter(hold_center);
    }
    return res;
}