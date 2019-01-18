#pragma once

#include "../../../representations/GeometricObject.h"
#include "MHyProGeometricObject.h"
#include <iostream>
#include <string>
#include "mex.h"
#include "matrix.h"
#include "../ClassHandle.h"
#include "../ObjectHandle.h"


class MHyProBox: public MHyProGeometricObject<hypro::Box<double>>{
    public:
        static void process(int, mxArray**, int, const mxArray**);
    private:
        static void emptyBox(int, mxArray**, int, const mxArray**);
        static void boxFromSingleInterval(int, mxArray**, int, const mxArray**);
        static void boxFromIntervals(int, mxArray**, int, const mxArray**);
        static void boxFromPoints(int, mxArray**, int,  const mxArray**);
        static void test(int, mxArray**, int, const mxArray**);
        static void empty(int, mxArray**, int, const mxArray**);
        static void intervals(int, mxArray**, int, const mxArray**);
        static void insert(int, mxArray**, int, const mxArray**);
        static void limits(int, mxArray**, int, const mxArray**);
        static void constraints(int, mxArray**, int, const mxArray**);
        static void interval(int, mxArray**, int, const mxArray**);
        static void at(int, mxArray**, int, const mxArray**);
        static void is_symmetric(int, mxArray**, int, const mxArray**);
        static void max(int, mxArray**, int, const mxArray**);
        static void min(int, mxArray**, int, const mxArray**);
        static void evaluate(int, mxArray**, int, const mxArray**);
        static void multiEvaluate(int, mxArray**, int, const mxArray**);
        static void outstream(int, mxArray**, int, const mxArray**);
        static void box_size(int, mxArray**, int, const mxArray**);
        static void type(int, mxArray**, int, const mxArray**);
        static void makeSymmetric(int, mxArray**, int, const mxArray**);
        static void minkowskiDecomposition(int, mxArray**, int, const mxArray**);
};

#include "MHyProBox.tpp"