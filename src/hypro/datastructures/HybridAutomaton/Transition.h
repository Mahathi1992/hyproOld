/*
 * Class that describes one transition of a hybrid automaton.
 * File:   transition.h
 * Author: stefan & ckugler
 *
 * Created on April 30, 2014, 9:06 AM
 */

#pragma once

#include "Condition.h"
#include "Location.h"
#include "Reset.h"
#include "Label.h"
#include "HybridAutomaton.h"
#include "decomposition/Decomposition.h"
#include <carl/interval/Interval.h>

namespace hypro
{
enum Aggregation { none=0, boxAgg, parallelotopeAgg };

template<typename Number>
class Location;

/**
 * @brief      Class for transition.
 * @tparam     Number  The used number type.
 */
template<typename Number>
class Transition
{
  private:
    Location<Number>* mSource = nullptr; /// Pointer to the source location.
    Location<Number>* mTarget = nullptr; /// Pointer to the target location.
    Condition<Number> mGuard; /// Guard condition enabling the transition if satisfied.
    Reset<Number> mReset; /// Reset function.
    Aggregation mAggregationSetting = Aggregation::none; /// Aggregation settings.
    bool mUrgent = false; /// Flag if transition is urgent.
    Number mTriggerTime = Number(-1); /// Trigger-time: if positive acts as an additional guard.
    std::vector<Label> mLabels = std::vector<Label>();
    mutable std::size_t mHash = 0;

  public:

  	/**
  	 * @brief      Default constructor.
  	 */
    Transition() = default;

    /**
     * @brief      Copy constructor.
     * @param[in]  orig  The original.
     */
    Transition(const Transition<Number>& orig) = default;

    /**
     * @brief      Move constructor.
     * @param[in]  orig  The original.
     */
    Transition(Transition<Number>&& orig) = default;

    /**
     * @brief      Copy assignment operator.
     * @param[in]  orig  The original.
     * @return     A copy of the passed transition.
     */
    Transition& operator=(const Transition<Number>& orig) = default;

    /**
     * @brief      Move assignment operator.
     * @param[in]  orig  The original.
     * @return     Result.
     */
    Transition& operator=(Transition<Number>&& orig) = default;

    /**
     * @brief      Destroys the object.
     */
    ~Transition() {}

    /**
     * @brief      Constructor from source and target location.
     * @param      source  The source
     * @param      target  The target
     */
    Transition(Location<Number>* source, Location<Number>* target)
        : mSource(source), mTarget(target), mGuard(), mReset(), mAggregationSetting(), mUrgent(false) , mLabels()
    {}

    /**
     * @brief      Full constructor for basic transition.
     * @param      source  The source.
     * @param      target  The target.
     * @param[in]  guard   The guard.
     * @param[in]  reset   The reset.
     */
    Transition(Location<Number>* source, Location<Number>* target, const Condition<Number>& guard, const Reset<Number>& reset)
        : mSource(source), mTarget(target), mGuard(guard), mReset(reset), mAggregationSetting(), mUrgent(false), mLabels()
    {}


    Location<Number>* getSource() const { assert( mSource != nullptr ); return mSource; }
    Location<Number>* getTarget() const { assert( mTarget != nullptr ); return mTarget; }
    const Condition<Number>& getGuard() const { return mGuard; }
    const Reset<Number>& getReset() const { return mReset; }
    Aggregation getAggregation() const { return mAggregationSetting; }
    Number getTriggerTime() const { return mTriggerTime; }
    bool isUrgent() const { return mUrgent; }
    bool isTimeTriggered() const { return mTriggerTime >= 0; }
    bool hasIdentityReset() const { return mReset.isIdentity(); }
    const std::vector<Label>& getLabels() const { return mLabels; }
    std::size_t hash() const;

    void setSource(Location<Number>* source) { mSource = source; mHash = 0; }
    void setTarget(Location<Number>* target) { mTarget = target; mHash = 0; }
    void setSource(std::unique_ptr<Location<Number>>& source) { mSource = source.get(); mHash = 0; }
    void setTarget(std::unique_ptr<Location<Number>>& target) { mTarget = target.get(); mHash = 0; }
    void setGuard(const Condition<Number>& guard) { mGuard = guard; mHash = 0; }
    void setReset(const Reset<Number>& val) { mReset = val; mHash = 0; }
    void setAggregation(Aggregation agg) { mAggregationSetting = agg; mHash = 0; }
    void setUrgent(bool urgent = true) { mUrgent = urgent; mHash = 0; }
    void setTriggerTime(Number t) { mTriggerTime = t; mHash = 0; }
    void setLabels(const std::vector<Label>& labels) { mLabels = labels; mHash = 0; }
    void addLabel(const Label& lab) { mLabels.push_back(lab); mHash = 0; }

    std::string getDotRepresentation(const std::vector<std::string>& vars) const;

	/**
	 * @brief      Determines if this composed of rhs and a potential rest.
	 * @details    Checks whether this transition can be the result of a parallel composition where rhs was part of one composite.
	 *
	 * @param[in]  rhs       The right hand side
	 * @param[in]  rhsVars   The right hand side variables
	 * @param[in]  thisVars  The variables of this
	 *
	 * @return     True if composed of, False otherwise.
	 */
    bool isComposedOf(const Transition<Number>& rhs, const std::vector<std::string>& rhsVars, const std::vector<std::string>& thisVars) const;

     /**
    * decomposes guard  and reset of this transition.
    */
    void decompose(const Decomposition& decomposition){
        mGuard.decompose(decomposition);
        mReset.decompose(decomposition);
        mHash = 0;
    }

    /**
     * @brief      Outstream operator.
     * @param      ostr  The outstream.
     * @param[in]  t     The transition.
     * @return     Reference to the outstream.
     */
    friend std::ostream& operator<<(std::ostream& ostr, const Transition<Number>& t) {
#ifdef HYPRO_LOGGING
	    ostr << "transition(" << std::endl
              << "\t hash = " << t.hash() << std::endl
	          << "\t Source = " << t.getSource()->hash() << std::endl
              << "\t Target = " << t.getTarget()->hash() << std::endl
	          << "\t urgent = " << t.isUrgent() << std::endl
	          << "\t Labels = ";
	    for(const auto& label : t.getLabels()) ostr << label;
	    ostr << std::endl;
	    ostr  << "\t Guard = " << t.getGuard() << std::endl
	          << "\t Reset = " << t.getReset() << std::endl
	          << ")";
#endif
	    return ostr;
    }

    /**
     * @brief      Equality comparison operator.
     * @param[in]  lhs   The left hand side.
     * @param[in]  rhs   The right hand side.
     * @return     True if both transitions are equal, false otherwise.
     */
    friend bool operator==(const Transition<Number>& lhs, const Transition<Number>& rhs) {
    	if(lhs.hash() != rhs.hash()) {
    		return false;
    	}

    	if( (*lhs.mSource != *rhs.mSource) ||
			(*lhs.mTarget != *rhs.mTarget) ||
			(lhs.mUrgent != rhs.mUrgent) ||
			(lhs.mGuard != rhs.mGuard) ||
			(lhs.mReset != rhs.mReset) ||
            (lhs.mLabels != rhs.mLabels)) {
			return false;
		}

		return true;
    }

    /**
     * @brief       Inequality comparison operator
     * @param[in]   lhs     The left hand side
     * @param[in]   rhs     The right hand side
     * Qreturn      True if both transitions are unequal, false otherwise
     */
    friend bool operator!=(const Transition<Number>& lhs, const Transition<Number>& rhs){
        return !(lhs == rhs);
    }

    /**
     * @brief       Equality operator for sets of unique pointers to transitions.
     * @details     First tests whether the size of the sets are equal. Then compares the transitions the unique pointers point to for equality.
     * @param[in]   lhs     left hand side; first set to compare
     * @param[in]   rhs     right hand sid; second set to compare
     */
    friend bool operator==( const std::set<std::unique_ptr<Transition<Number>>>& lhs, const std::set<std::unique_ptr<Transition<Number>>>& rhs){
        if(lhs.size() != rhs.size()){
            return false;
        }
        auto itRhs = rhs.begin();
        for(auto itLhs = lhs.begin(); itLhs != lhs.end(); ++itLhs){
            if(**itLhs != **itRhs){
                return false;
            }
            ++itRhs;
        }
        return true;
    }
};

/*
template<typename Number>
//std::unique_ptr<Transition<Number>> parallelCompose(const std::unique_ptr<Transition<Number>>& lhsT
//                                , const std::unique_ptr<Transition<Number>>& rhsT

std::unique_ptr<Transition<Number>> parallelCompose(const Transition<Number>* lhsT
                                                    , const Transition<Number>* rhsT
                                                    , const std::vector<std::string>& lhsVar
                                                    , const std::vector<std::string>& rhsVar
                                                    , const std::vector<std::string>& haVar
                                                    , const HybridAutomaton<Number>& ha
                                                    , const std::set<Label> lhsLabels
                                                    , const std::set<Label> rhsLabels);
*/
}  // namespace hypro

namespace std {

	template<typename Number>
	struct hash<hypro::Transition<Number>>{
		size_t operator()(const hypro::Transition<Number>& trans) {
			size_t seed = 0;
			carl::hash_add(seed,trans.getSource()->hash());
			carl::hash_add(seed,trans.getTarget()->hash());
			carl::hash_add(seed,trans.getGuard().hash());
			carl::hash_add(seed,trans.getReset().hash());
			//carl::hash_add(seed,std::hash<hypro::Aggregation>(mAggregationSetting));
			//carl::hash_add(seed,std::hash<bool>(trans.isUrgent()));
			//carl::hash_add(seed,std::hash<Number>(mTriggerTime));
			//for(const auto& l : mLabels) {
			//	carl::hash_add(seed,std::hash<Label>(l));
			//}
			return seed;
		}
	};

} // namespace

#include "Transition.tpp"
