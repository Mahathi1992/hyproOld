#pragma once

#include "../../representations/GeometricObject.h"
#include "../../datastructures/Halfspace.h"
#include <iostream>
#include <map>
#include <cassert>
#include "mex.h"
#include "matrix.h"
#include "ClassHandle.h"
#include "ObjectHandle.h"
#include "HyProGeometricObject.h"

class HyProSupportFunction: public HyProGeometricObject<hypro::SupportFunction<double>>{
    public:
        static void process(int, mxArray**, int, const mxArray**);
    private:
        static void new_points(int, mxArray**, int, const mxArray**);
        static void new_intervals(int, mxArray**, int, const mxArray**);
        static void new_halfspaces(int, mxArray**, int, const mxArray**);
        static void ostream(int, mxArray**, int, const mxArray**);
        static void type(int, mxArray**, int, const mxArray**);
        static void depth(int, mxArray**, int, const mxArray**);
        static void operationCount(int, mxArray**, int, const mxArray**);
        static void summands(int, mxArray**, int, const mxArray**);
        static void scaleParameters(int, mxArray**, int, const mxArray**);
        static void linearTrafoParameters(int, mxArray**, int, const mxArray**);
        static void unionParameters(int, mxArray**, int, const mxArray**);
        static void intersectionParameters(int, mxArray**, int, const mxArray**);
        static void polytope(int, mxArray**, int, const mxArray**);
        static void ball(int, mxArray**, int, const mxArray**);
        static void ellipsoid(int, mxArray**, int, const mxArray**);
        static void supremum(int, mxArray**, int, const mxArray**);
        static void contains_vec(int, mxArray**, int, const mxArray**);
        static void contains_dir(int, mxArray**, int, const mxArray**);
        static void scale(int, mxArray**, int, const mxArray**);
        static void swap(int, mxArray**, int, const mxArray**);
        static void forceLinTransReduction(int, mxArray**, int, const mxArray**);
        static void multiplicationsPerEvaluation(int, mxArray**, int, const mxArray**);
        static void cleanUp(int, mxArray**, int, const mxArray**);
        static void collectProjections(int, mxArray**, int, const mxArray**);

};

#include "HyProSupportFunction.tpp"