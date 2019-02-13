#include "@MHyProObject/MHyProBox.h"
#include "@MHyProObject/MHyProConstraintSet.h"
#include "@MHyProObject/MHyProTaylorModel.h"
#include "@MHyProObject/MHyProEllipsoid.h"
#include "@MHyProObject/MHyProSupportFunction.h"
#include "@MHyProHAutomaton/MHybridAutomaton.h"
// #include "@MHyProCondition/MCondition.h"
// #include "@MHyProFlow/MLinearFlow.h"
// #include "@MHyProFlow/MAffineFlow.h"
// #include "@MHyProLabel/MLabel.h"
// #include "@MHyProLocation/MLocation.h"
// #include "@MHyProLocation/MLocation.h"
// #include "@MHyProState/MState.h"
// #include "@MHyProTransition/MTransition.h"
#include <iostream>
#include "mex.h"
#include "matrix.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){

    // Get type of the Matlab object
    char type[64];
    mxGetString(prhs[0], type, sizeof(type));

    if(!strcmp(type, "Box")){
        MHyProBox::process(nlhs, plhs, nrhs, prhs);
    }else if(!strcmp(type, "Ellipsoid")){
        MHyProEllipsoid::process(nlhs, plhs, nrhs, prhs);
    }else if(!strcmp(type, "ConstraintSet")){
        MHyProConstraintSet::process(nlhs, plhs, nrhs, prhs);
    }else if(!strcmp(type, "SupportFunction")){
        MHyProSupportFunction::process(nlhs, plhs, nrhs, prhs);
    }else if(!strcmp(type, "TaylorModel")){
        // MHyProTaylorModel::process(nlhs, plhs, nrhs, prhs);
    }else if(!strcmp(type, "HybridAutomaton")){
        MHybridAutomaton::process(nlhs, plhs, nrhs, prhs);
    
    // }else if(!strcmp(type, "Condition")){
    //     MCondition::process(nlhs, plhs, nrhs, prhs);
    // }else if(!strcmp(type, "Location")){
    //     MLocation::process(nlhs, plhs, nrhs, prhs);
    // }else if(!strcmp(type, "Transition")){
    //     MTransition::process(nlhs, plhs, nrhs, prhs);
    // }else if(!strcmp(type, "Label")){
    //     MLabel::process(nlhs, plhs, nrhs, prhs);
    // }else if(!strcmp(type, "LinearFlow")){
    //     MLinearFlow::process(nlhs, plhs, nrhs, prhs);
    // }else if(!strcmp(type, "AffineFlow")){
    //     MAffineFlow::process(nlhs, plhs, nrhs, prhs);
    // }else if(!strcmp(type, "State")){
    //     MState::process(nlhs, plhs, nrhs, prhs);
    }else{
        mexErrMsgTxt("MHyPro - Unknown type.");
    }
    
}