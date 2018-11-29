#include "HandlerFactory.h"

namespace hypro
{

	template<typename State>
	IFirstSegmentHandler<State>* HandlerFactory<State>::buildFirstSegmentHandler(representation_name name, State* state, size_t index, tNumber timeStep){

		switch(name){
 			case representation_name::box:
 			case representation_name::polytope_h:
 			case representation_name::polytope_v:
 			case representation_name::zonotope:
 			case representation_name::support_function:
 				return new ltiFirstSegmentHandler<State>(state,index,timeStep);
				break;
 			case representation_name::difference_bounds: {
				if(SettingsProvider<State>::getInstance().useDecider() && SettingsProvider<State>::getInstance().getLocationTypeMap().find(state->getLocation())->second == LOCATIONTYPE::TIMEDLOC){
 					if(SettingsProvider<State>::getInstance().isFullTimed() ){
	 					return new timedElapseFirstSegmentHandler<State>(state,index,timeStep);
 					}
 				}
 				return new timedFirstSegmentHandler<State>(state,index,timeStep);
 			}
 		}
 		assert(false && "SHOULD NEVER REACH THIS");
 		return nullptr;
	}


	template<typename State>
	IInvariantHandler* HandlerFactory<State>::buildInvariantHandler(representation_name name, State* state, size_t index, bool noFlow){

		if(state->getLocation()->getInvariant().size() == 0 || state->getLocation()->getInvariant().getMatrix(index).rows() == 0){
			// no constraints, always full containment
			return nullptr;
		}

		switch(name){
 			case representation_name::box:
 			case representation_name::polytope_h:
 			case representation_name::polytope_v:
 			case representation_name::zonotope:
 			case representation_name::support_function:
 				if(noFlow){
 					return new discreteInvariantHandler<State>(state,index);
 				}
 				return new ltiInvariantHandler<State>(state,index);
				break;
 			case representation_name::difference_bounds: {
 				if(noFlow){
 					return new discreteInvariantHandler<State>(state,index);
 				}
 				return new timedInvariantHandler<State>(state,index);
 			}
 		}
 		assert(false && "SHOULD NEVER REACH THIS");
 		return nullptr;
	}

	template<typename State>
	IBadStateHandler* HandlerFactory<State>::buildBadStateHandler(representation_name name, State* state, size_t index, bool noFlow){
		auto localBadState = SettingsProvider<State>::getInstance().getHybridAutomaton().getLocalBadStates().find(state->getLocation());
		// no local bad states for this location or subspace
		bool noLocalBadStates = (localBadState == SettingsProvider<State>::getInstance().getHybridAutomaton().getLocalBadStates().end() || localBadState->second.getMatrix(index).rows() == 0);

		// for each global bad state, check if it has constraints in this subspace
		bool noGlobalBadStates = true;
		for (const auto& badState : SettingsProvider<State>::getInstance().getHybridAutomaton().getGlobalBadStates()) {
	    	if(badState.getMatrix(index).rows() != 0){
	    		noGlobalBadStates = false;
	    		break;
	    	}
	    }

	    if(noLocalBadStates && noGlobalBadStates){
	    	return nullptr;
	    }

		switch(name){
 			case representation_name::box:
 			case representation_name::polytope_h:
 			case representation_name::polytope_v:
 			case representation_name::zonotope:
 			case representation_name::support_function:
 				if(noFlow){
 					return new discreteBadStateHandler<State>(state,index);
 				}
 				return new ltiBadStateHandler<State>(state,index);

 			case representation_name::difference_bounds: {
 				if(noFlow){
 					return new discreteBadStateHandler<State>(state,index);
 				}
 				return new timedBadStateHandler<State>(state,index);
 			}
 		}
 		assert(false && "SHOULD NEVER REACH THIS");
 		return nullptr;
	}

	template<typename State>
	IGuardHandler<State>* HandlerFactory<State>::buildGuardHandler(representation_name name, std::shared_ptr<State> state, size_t index, Transition<Number>* transition, bool noFlow){
		if(transition->getGuard().size() == 0 || transition->getGuard().getMatrix(index).rows() == 0){
			// no constraints for this subspace
			return nullptr;
		}

		switch(name){
 			case representation_name::box:
 			case representation_name::polytope_h:
 			case representation_name::polytope_v:
 			case representation_name::zonotope:
 			case representation_name::support_function:
 				if(noFlow){
 					return new discreteGuardHandler<State>(state,index,transition);
 				}
 				return new ltiGuardHandler<State>(state,index,transition);

 			case representation_name::difference_bounds: {
 				if(noFlow){
 					return new discreteGuardHandler<State>(state,index,transition);
 				}
 				return new timedGuardHandler<State>(state,index,transition);
 			}
 		}
 		assert(false && "SHOULD NEVER REACH THIS");
 		return nullptr;
	}

	template<typename State>
	ITimeEvolutionHandler* HandlerFactory<State>::buildContinuousEvolutionHandler(representation_name name, State* state, size_t index, tNumber timeStep, tNumber timeBound, matrix_t<Number> trafo, vector_t<Number> translation){
		if(trafo == matrix_t<Number>::Identity(trafo.rows(),trafo.rows()) && translation == vector_t<Number>::Zero(trafo.rows())){
			return nullptr;
		}

		switch(name){
 			case representation_name::box:
 			case representation_name::polytope_h:
 			case representation_name::polytope_v:
 			case representation_name::zonotope:
 			case representation_name::support_function:
 				return new ltiTimeEvolutionHandler<State>(state,index,timeStep,trafo,translation);

 			case representation_name::difference_bounds: {
 				if(SettingsProvider<State>::getInstance().useDecider() && SettingsProvider<State>::getInstance().getLocationTypeMap().find(state->getLocation())->second == LOCATIONTYPE::TIMEDLOC){
					if(SettingsProvider<State>::getInstance().isFullTimed()){
						return new timedElapseTimeEvolutionHandler<State>(state,index,timeStep,timeBound,trafo,translation);
					}
					// on mixed contexts a first segment with tick was computed
					else if(SettingsProvider<State>::getInstance().useContextSwitch()){
						return new timedElapseAfterTickTimeEvolutionHandler<State>(state,index,timeStep,timeBound,trafo,translation);
					}
				}
 				return new timedTickTimeEvolutionHandler<State>(state,index,timeStep,trafo,translation);
 			}
 		}
 		assert(false && "SHOULD NEVER REACH THIS");
 		return nullptr;
	}

	template<typename State>
	IResetHandler* HandlerFactory<State>::buildResetHandler(representation_name name, State* state, size_t index,matrix_t<Number> trafo, vector_t<Number> translation){
		if(trafo.rows() == 0 || (trafo == matrix_t<Number>::Identity(trafo.rows(),trafo.rows()) && translation == vector_t<Number>::Zero(translation.rows()))){
			return nullptr;
		}

		switch(name){
 			case representation_name::box:
 			case representation_name::polytope_h:
 			case representation_name::polytope_v:
 			case representation_name::zonotope:
 			case representation_name::support_function:
 				return new ltiResetHandler<State>(state,index,trafo,translation);

 			case representation_name::difference_bounds: {
 				return new timedResetHandler<State>(state,index,trafo,translation);
 			}
 		}
 		assert(false && "SHOULD NEVER REACH THIS");
 		return nullptr;
	}



	template<typename State>
	IJumpHandler* HandlerFactory<State>::buildDiscreteSuccessorHandler(std::vector<boost::tuple<Transition<Number>*, State>>* successorBuffer,
										representation_name representation,
										std::shared_ptr<Task<State>> task,
										Transition<Number>* transition,
										StrategyParameters sPars,
										WorkQueue<std::shared_ptr<Task<State>>>& localQueue,
										WorkQueue<std::shared_ptr<Task<State>>>& localCEXQueue){
		return new ltiJumpHandler<State>(successorBuffer, representation, task,transition, sPars, localQueue, localCEXQueue);
	}

} // hypro
