#pragma once
#include "../../../../datastructures/HybridAutomaton/State.h"
#include "../../../../datastructures/HybridAutomaton/flow/rectangular.h"
#include "../../../../datastructures/HybridAutomaton/flow/rectangularOperations.h"
#include "../../../../representations/GeometricObjectBase.h"
#include "../../../../representations/conversion/conversionHelper.h"
#include "../IHandler.h"

#include <sstream>
#include <string>

namespace hypro {

template <typename State>
State rectangularApplyTimeEvolution( const State& initialSet, const rectangularFlow<typename State::NumberType>& flow );

template <typename State>
State rectangularApplyReverseTimeEvolution( const State& badSet, const rectangularFlow<typename State::NumberType>& flow );

template <template <typename> typename PolyhedralRepresentation, typename Number>
PolyhedralRepresentation<Number> rectangularApplyTimeEvolution( const PolyhedralRepresentation<Number>& initialSet, const rectangularFlow<Number>& flow );

template <template <typename> typename PolyhedralRepresentation, typename Number>
PolyhedralRepresentation<Number> rectangularApplyBoundedTimeEvolution( const PolyhedralRepresentation<Number>& initialSet, const rectangularFlow<Number>& flow, tNumber timeBound );

template <template <typename> typename PolyhedralRepresentation, typename Number>
PolyhedralRepresentation<Number> rectangularApplyReverseTimeEvolution( const PolyhedralRepresentation<Number>& badSet, const rectangularFlow<Number>& flow );

template <typename Number>
CarlPolytope<Number> rectangularApplyTimeEvolution( const CarlPolytope<Number>& initialSet, const rectangularFlow<Number>& flow );

template <typename Number>
CarlPolytope<Number> rectangularApplyReverseTimeEvolution( const CarlPolytope<Number>& badSet, const rectangularFlow<Number>& flow );

}  // namespace hypro

#include "rectangularTimeEvolutionHandler.tpp"
