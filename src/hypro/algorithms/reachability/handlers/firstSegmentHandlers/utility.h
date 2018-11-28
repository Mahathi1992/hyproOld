#pragma once
#include "../../../../representations/GeometricObject.h"
#include "../../../../datastructures/HybridAutomaton/State.h"

namespace hypro {
    template <typename State>
	State bloatBox( const State& in, const hypro::Box<typename State::NumberType>& bloatBox , size_t index) {
        State bloatState;
        switch(in.getSetType(index)) {
            case hypro::representation_name::box: {
                bloatState.setSet(hypro::Converter<typename State::NumberType>::toBox(bloatBox), index);
                break;
            }
            case hypro::representation_name::polytope_h: {
                bloatState.setSet(hypro::Converter<typename State::NumberType>::toHPolytope(bloatBox), index);
                break;
            }
            case hypro::representation_name::polytope_v: {
                bloatState.setSet(hypro::Converter<typename State::NumberType>::toVPolytope(bloatBox), index);
                break;
            }
            case hypro::representation_name::support_function: {
                bloatState.setSet(hypro::Converter<typename State::NumberType>::toSupportFunction(bloatBox), index);
                break;
            }
            case hypro::representation_name::zonotope: {
                bloatState.setSet(hypro::Converter<typename State::NumberType>::toZonotope(bloatBox), index);
                break;
            }
            default: {
                bloatState.setSet(hypro::Converter<typename State::NumberType>::toBox(bloatBox), index);
                break;
            }
        }
        return in.partiallyMinkowskiSum(bloatState, index);
    }
} // hypro