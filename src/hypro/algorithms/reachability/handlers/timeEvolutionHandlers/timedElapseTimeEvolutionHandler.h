#pragma once
#include "../IHandler.h"
#include "ltiTimeEvolutionHandler.h"
#include "../../../../datastructures/HybridAutomaton/State.h"

namespace hypro
{
    template<typename State>
	class timedElapseTimeEvolutionHandler : public ltiTimeEvolutionHandler<State>{
		using Number = typename State::NumberType;
	protected:
		bool mComputed;
		tNumber mTimeBound;
	public:
		timedElapseTimeEvolutionHandler() = delete;
		timedElapseTimeEvolutionHandler(State* state, size_t index, tNumber timeStep, tNumber timeBound, hypro::matrix_t<Number> trafo,hypro::vector_t<Number> translation) : ltiTimeEvolutionHandler<State>(state, index, timeStep,trafo, translation){
			mComputed = false;
			mTimeBound = timeBound;
		}

		void handle();
		const char* handlerName() {return "timedElapseTimeEvolutionHandler";}
	};
} // hypro

#include "timedElapseTimeEvolutionHandler.tpp"