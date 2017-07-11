
#pragma once

#include "Visitors.h"
#include "lib/datastructures/ReachTree/types.h"         //???
#include "lib/utils/types.h"                            //???
#include <carl/interval/Interval.h>
//#include <hypro/representations/GeometricObject.h>
#include "../../representations/GeometricObject.h"

namespace hypro
{

template<typename Number>
class Location;

template<typename Number>
class State
{
public:
	using discreteSetRepresentation = hypro::Box<Number>;
	using clockSetRepresentation = hypro::Box<Number>;
  protected:
    const Location* mLoc = nullptr; // Todo: Check if the location pointer is really needed.
    RepresentationVariant mSet;
    hypro::representation_name mSetRepresentationName;
    RepresentationVariant mDiscreteAssignment;
    RepresentationVariant mClockAssignment;
    carl::Interval<Number> mTimestamp = carl::Interval<Number>::unboundedInterval();
    bool mHasClocks = false;
    bool mHasDiscreteVariables = false;
    bool mIsEmpty = false;

  public:
    State() = default;

    State(const State& orig) :
    	mLoc(orig.getLocation()),
    	mSet(orig.getSet()),
    	mSetRepresentationName(orig.getSetRepresentation()),
    	mDiscreteAssignment(orig.getDiscreteAssignment()),
    	mClockAssignment(orig.getClockAssignment()),
    	mTimestamp(orig.getTimestamp()),
    	mHasClocks(orig.hasClockVariables()),
    	mHasDiscreteVariables(orig.hasDiscreteVariables())
    {}

    State(State&& orig) :
    	mLoc(orig.getLocation()),
    	mSet(orig.getSet()),
    	mSetRepresentationName(orig.getSetRepresentation()),
    	mDiscreteAssignment(orig.getDiscreteAssignment()),
    	mClockAssignment(orig.getClockAssignment()),
    	mTimestamp(orig.getTimestamp()),
    	mHasClocks(orig.hasClockVariables()),
    	mHasDiscreteVariables(orig.hasDiscreteVariables())
    {}

    State& operator=(const State& orig) {
    	mLoc = orig.getLocation();
    	mSet = orig.getSet();
    	mSetRepresentationName = orig.getSetRepresentation();
    	mDiscreteAssignment = orig.getDiscreteAssignment();
    	mClockAssignment = orig.getClockAssignment();
    	mTimestamp = orig.getTimestamp();
    	mHasClocks = orig.hasClockVariables();
    	mHasDiscreteVariables = orig.hasDiscreteVariables();
    	return *this;
    }

    State& operator=(State&& orig) {
    	mLoc = orig.getLocation();
    	mSet = orig.getSet();
    	mSetRepresentationName = orig.getSetRepresentation();
    	mDiscreteAssignment = orig.getDiscreteAssignment();
    	mClockAssignment = orig.getClockAssignment();
    	mTimestamp = orig.getTimestamp();
    	mHasClocks = orig.hasClockVariables();
    	mHasDiscreteVariables = orig.hasDiscreteVariables();
    	return *this;
    }

    State(const hydra::Location* _loc) : mLoc(_loc), mSet(), mDiscreteAssignment() { assert(mLoc != nullptr); }
    State(const hydra::Location* _loc, const RepresentationVariant& _rep) : mLoc(_loc), mSet(_rep), mDiscreteAssignment() { assert(mLoc != nullptr); }
    State(const hydra::Location* _loc, const RepresentationVariant& _rep, const carl::Interval<Number>& _timestamp) : mLoc(_loc), mSet(_rep), mDiscreteAssignment(), mTimestamp(_timestamp) { assert(mLoc != nullptr); }
    State(const hydra::Location* _loc,
    		const RepresentationVariant& _rep,
    		const RepresentationVariant& discreteAssingment,
    		const RepresentationVariant& clockAssignment,
    		const carl::Interval<Number>& _timestamp)
    	: mLoc(_loc)
    	, mSet(_rep)
    	, mDiscreteAssignment(discreteAssingment)
    	, mClockAssignment(clockAssignment)
    	, mTimestamp(_timestamp)
    	, mHasClocks(true)
    	, mHasDiscreteVariables(true)
    {
    	assert(mLoc != nullptr);
    }

    const hydra::Location* getLocation() const { assert(mLoc != nullptr); return mLoc; }
    const RepresentationVariant& getSet() const { return mSet; }
    RepresentationVariant& rGetSet() { return mSet; }
    inline hypro::representation_name getSetRepresentation() const { return mSetRepresentationName; }
    const RepresentationVariant& getDiscreteAssignment() const { return mDiscreteAssignment; }
    const RepresentationVariant& getClockAssignment() const { return mClockAssignment; }
    const carl::Interval<Number>& getTimestamp() const { return mTimestamp; }

    bool hasClockVariables() const { return mHasClocks; }
    bool hasDiscreteVariables() const { return mHasDiscreteVariables; }
    bool isEmpty() const { return mIsEmpty; }

    void setLocation(const Location* l) { assert(l != nullptr); mLoc = l; }
    void setSet(const RepresentationVariant& s) { mSet = s; }
    void setSetRepresentation(hypro::representation_name n) { mSetRepresentationName = n; }
    void setTimestamp(carl::Interval<Number> t) { mTimestamp = t; }
    void setDiscreteAssignment(const RepresentationVariant& d) { mDiscreteAssignment = d; mHasDiscreteVariables = true;}
    void setClockAssignment(const RepresentationVariant& c) { mClockAssignment = c; mHasClocks = true;}

    void addTimeToClocks(Number t);
    State aggregate(const State& in) const;
    std::pair<bool,State> intersect(const State& in) const;
    State applyTimeStep(const hypro::matrix_t<Number>& trafoMatrix, const hypro::vector_t<Number>& trafoVector, Number timeStepSize ) const;

    friend ostream& operator<<(ostream& out, const State& state) {
//#ifdef HYDRA_USE_LOGGING
    	//out << "location: " << state.mLoc->getName() << " at timestamp " << state.mTimestamp << std::endl;
    	out << "Set: " << hypro::convert<Number,double>(boost::get<hypro::Box<Number>>(boost::apply_visitor(genericConversionVisitor<RepresentationVariant>(hypro::representation_name::box), state.getSet()))) << std::endl;
    	out << "Discrete Set: " << boost::get<discreteSetRepresentation>(state.mDiscreteAssignment) << std::endl;
    	out << "Clock Set: " << boost::get<clockSetRepresentation>(state.mClockAssignment);
//#endif
    	return out;
    }

    friend bool operator==(const State& lhs, const State& rhs) {
    	return ( lhs.mLoc == rhs.mLoc &&
    			 lhs.mSet == rhs.mSet &&
    			 lhs.mSetRepresentationName == rhs.mSetRepresentationName &&
    			 lhs.mDiscreteAssignment == rhs.mDiscreteAssignment &&
    			 lhs.mClockAssignment == rhs.mClockAssignment &&
    			 lhs.mTimestamp == rhs.mTimestamp);
    }

    friend bool operator!=(const State& lhs, const State& rhs) {
    	return !(lhs == rhs);
    }

};
}

#include "State.tpp"