#pragma once

#include "../../../types.h"
#include "mex.h"
#include "matrix.h"
#include "../Handles/ClassHandle.h"
#include "../Handles/ObjectHandle.h"

template<class T> class MFlow{
    public:
        static void new_empty(int, mxArray**, int, const mxArray**);
        static void copy(int, mxArray**, int, const mxArray**);
        static void size(int, mxArray**, int, const mxArray**);
        static void delete_flow(int, mxArray**, int, const mxArray**);
        static void type(int, mxArray**, int, const mxArray**);
        static void dimension(int, mxArray**, int, const mxArray**);
        static void isTimed(int, mxArray**, int, const mxArray**);
        static void isDiscrete(int, mxArray**, int, const mxArray**);
        static void equals(int, mxArray**, int, const mxArray**);
        static void unequals(int, mxArray**, int, const mxArray**);
};
#include "MFlow.tpp"
