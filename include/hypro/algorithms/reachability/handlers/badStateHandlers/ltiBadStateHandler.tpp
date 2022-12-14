#include "ltiBadStateHandler.h"

namespace hypro {

template <typename Representation, typename Number>
std::pair<CONTAINMENT, Representation> ltiIntersectBadStates( const Representation& valuationSet, Location<Number> const* location, const HybridAutomaton<Number>& automaton, std::size_t I ) {
	auto localBadState = automaton.getLocalBadStates().find( location );
	if ( localBadState != automaton.getLocalBadStates().end() ) {
		if ( localBadState->second.isTrue() ) {
			return std::make_pair( CONTAINMENT::FULL, valuationSet );
		} else if ( localBadState->second.isFalse() ) {
			return std::make_pair( CONTAINMENT::NO, valuationSet );
		}
		std::pair<CONTAINMENT, Representation> badStatePair = intersect( valuationSet, localBadState->second, I );
		if ( badStatePair.first != hypro::CONTAINMENT::NO ) {
			DEBUG( "hydra.worker", "Intersection with local bad states. (intersection type " << badStatePair.first << ")" );
			return badStatePair;
		}
	}

	// check global bad states
	for ( const auto& badState : automaton.getGlobalBadStates() ) {
		// at least one global bad Representation in this subspace
		std::pair<CONTAINMENT, Representation> badStatePair = intersect( valuationSet, badState, I );
		if ( badStatePair.first != CONTAINMENT::NO ) {
			DEBUG( "hydra.worker", "Intersection with global bad states" );
			return badStatePair;
		}
	}

	return std::make_pair( CONTAINMENT::NO, valuationSet );
}

}  // namespace hypro