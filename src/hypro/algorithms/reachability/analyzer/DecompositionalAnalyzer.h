#pragma once
#include <hypro/datastructures/HybridAutomaton/HybridAutomaton.h>
#include <hypro/datastructures/reachability/Flowpipe.h>
#include <hypro/datastructures/reachability/ReachTreev2.h>
#include <hypro/types.h>
#include <hypro/algorithms/reachability/workers/SingularWorker.h>
#include <hypro/algorithms/reachability/workers/LTIWorker.h>
#include <hypro/algorithms/reachability/workers/RectangularWorker.h>
#include <hypro/algorithms/reachability/analyzer/ReturnTypes.h>
#include <hypro/algorithms/reachability/analyzer/DecompositionalUtil.h>

#include <queue>


namespace hypro {
namespace detail {

/**
 * @brief       Worker-visitor to reset workers for new task.
 * @tparam      Representation      The used state set representation type.
 */
template <typename Representation>
struct resetWorkerVisitor {
    void operator()( SingularWorker<Representation>& worker ) {
        worker.reset();
    }
    void operator()( RectangularWorker<Representation>& ) {
        // worker.reset();
    }
    void operator()( LTIWorker<Representation>& ) {
        return;
    }
};

/**
 * @brief       Worker-visitor to compute the time successors for the given task.
 * @tparam      Representation      The used state set representation type.
 * @param       task                The current task.
 * @param       fixedTimeStep       The fixed time step of the analyzer using the visitor.
 * @param       timeStep            The analyzer specific time step (for using multiple analyzers).
 * @return      The time interval where the subspace satisfies the invariant.
 */
template <typename Representation>
struct computeTimeSuccessorVisitor {
  using Number = typename Representation::NumberType;
  ReachTreeNode<Representation>* task;
  tNumber fixedTimeStep;
  tNumber timeStep;
    // return time interval covered by the time successors
    carl::Interval<Number> operator()( SingularWorker<Representation>& worker ) {
        worker.computeTimeSuccessors( *task, false );
        auto& flowpipe = task->getFlowpipe();
        flowpipe.insert( flowpipe.begin(), worker.getFlowpipe().begin(), worker.getFlowpipe().end() );
        // the second segment covers the entire time interval
        if ( flowpipe.size() == 0 ) {
            // invariant is initially violated
            return carl::Interval<Number>::emptyInterval();
        }
        assert( flowpipe.size() == 2 );
        // last variable is the clock
        return getTimeInterval( flowpipe[1], flowpipe[1].dimension() - 1 );
    }
    carl::Interval<Number> operator()( LTIWorker<Representation>& worker ) {
        worker.computeTimeSuccessors( task->getInitialSet(), task->getLocation(), std::back_inserter( task->getFlowpipe() ), false );
        auto& flowpipe = task->getFlowpipe();
        assert( flowpipe.size() > 0 );
        // get global time interval
        Number timeLower = carl::convert<tNumber, Number>( task->getTimings().lower()*fixedTimeStep );
        Number timeUpper = carl::convert<tNumber, Number>( task->getTimings().upper()*fixedTimeStep + flowpipe.size()*timeStep );
        return carl::Interval<Number>( timeLower , timeUpper );
    }
    carl::Interval<Number> operator()( RectangularWorker<Representation>& ) {
        // Todo: rectangular worker. Should be very similar to singular case
        return carl::Interval<Number>::emptyInterval();
    }
};

/**
 * @brief       Worker-visitor to compute the jump successors for the given task.
 * @tparam      Representation      The used state set representation type.
 * @param       task                The current task.
 */
template <typename Representation>
struct computeSingularJumpSuccessorVisitor {
  ReachTreeNode<Representation>* task;
    void operator()( SingularWorker<Representation>& worker ) {
        worker.computeJumpSuccessors( *task );
    }
    void operator()( RectangularWorker<Representation>& ) {
        // todo
        return;
    }
    void operator()( LTIWorker<Representation>& ) {
        assert( false && "Singular jump successor computation called for lti subspace." );
    }
};


/**
 * @brief       Worker-visitor to get the jump successors for the given transition in singular subspaces.
 * @tparam      Representation      The used state set representation type.
 * @param       transition          The transition for which to get successors.
 * @return      The jump successor as Representation set.
 */
template <typename Representation>
struct getSingularJumpSuccessorVisitor {
  using Number = typename Representation::NumberType;
  Transition<Number>* transition;
    Representation operator()( SingularWorker<Representation>& worker ) {
        auto it = worker.getJumpSuccessorSets().find( transition );
        if ( it != worker.getJumpSuccessorSets().end() ) {
            auto successorList = it->second;
            // size 2 if initial set intersects guard, else 1. Either way, only need the last (jump with full flowpipe)
            assert( successorList.size() == 1 || successorList.size() == 2 );
            return successorList.size() == 1 ? successorList[ 0 ] : successorList[ 1 ];
        }
        else {
            return Representation::Empty();
        }
    }
    Representation operator()( RectangularWorker<Representation>& ) {
        //todo
        return Representation::Empty();
    }
    Representation operator()( LTIWorker<Representation>& ) {
        assert( false && "Singular jump successor getter called for lti subspace." );
        return Representation::Empty();
    }
};

/**
 * @brief       Worker-visitor to get the jump successors for the given transition in lti subspaces.
 * @tparam      Representation      The used state set representation type.
 * @param       transition          The transition for which to get successors.
 * @param       guardEnabledSets    The jump predecessors.
 * @return      The jump successor as Representation set.
 */
template <typename Representation>
struct getLtiJumpSuccessorVisitor {
  using Number = typename Representation::NumberType;
  Transition<Number>* transition;
  std::vector<IndexedValuationSet<Representation>> guardEnabledSets;
    std::vector<TimedValuationSet<Representation>> operator()( SingularWorker<Representation>& ) {
        assert( false );
        return std::vector<TimedValuationSet<Representation>>();
    }
    std::vector<TimedValuationSet<Representation>> operator()( RectangularWorker<Representation>& ) {
        assert( false );
        return std::vector<TimedValuationSet<Representation>>();
    }
    std::vector<TimedValuationSet<Representation>> operator()( LTIWorker<Representation>& worker ) {
        return worker.computeJumpSuccessorsForGuardEnabled( guardEnabledSets, transition );
    }
};

} // namespace detail

// indicates that the analysis succeeded, i.e. no intersection with bad states
struct DecompositionalSuccess {};


/**
 * @brief      Class implementing a reachability analysis algorithm for decomposed hybrid automata.
 * @tparam     Representation  The used state set representation type.
 */
template <typename Representation>
class DecompositionalAnalyzer {
    using Number = typename Representation::NumberType;
    using NodeVector = std::vector<ReachTreeNode<Representation>*>;
    using RepVector = std::vector<Representation>;
    using SingularSuccessors = std::map<std::size_t, Representation>;
    using LTIPredecessors = std::map<std::size_t, std::vector<IndexedValuationSet<Representation>>>;
    using LTISuccessors = std::map<std::size_t, std::vector<TimedValuationSet<Representation>>>;

  public:
    using DecompositionalResult = AnalysisResult<DecompositionalSuccess, Failure<Representation>>;
    using WorkerVariant = std::variant<LTIWorker<Representation>,
                                       SingularWorker<Representation>,
                                       RectangularWorker<Representation>>;

    /**
     * @brief       Construct a new DecompositionalAnalyzer object.
     * @param       ha                  The decomposed hybrid automaton to analyze.
     * @param       decomposition       The decomposition information corresponding to the ha.
     * @param       fixedParameters     The fixed analysis parameters.
     * @param       parameters          The analyzer specific parameters (for using multiple analyzers).
     * @param       roots               The roots for the analysis tree (one root for each subspace).
     */
    DecompositionalAnalyzer( HybridAutomaton<Number> const& ha,
                 Decomposition const& decomposition,
                 FixedAnalysisParameters const& fixedParameters,
                 AnalysisParameters const& parameters,
                 std::vector<std::vector<ReachTreeNode<Representation>>>& roots )
        : mHybridAutomaton( &ha )
        , mDecomposition( decomposition )
        , mFixedParameters( fixedParameters )
        , mParameters( parameters ) {
        for ( auto& subspaceRoots : roots ) {
            NodeVector root;
            for ( int subspace = decomposition.subspaces.size() - 1; subspace >= 0; --subspace ) {
                root.push_back( &subspaceRoots[ subspace ] );
            }
            mWorkQueue.push_front( root );
        }
        for ( std::size_t subspace = 0; subspace < decomposition.subspaceTypes.size(); ++subspace ) {
            if ( decomposition.subspaceTypes[ subspace ] == DynamicType::linear || decomposition.subspaceTypes[ subspace ] == DynamicType::affine ) {
                mLtiTypeSubspaces.push_back( subspace );
            } else {
                mSingularTypeSubspaces.push_back( subspace );
            }
        }
    }

    /**
     * @brief       Perform safety analysis.
     * @return      A result object indicating either success (safety) or failure (bad states reachable).
     */
    DecompositionalResult run();

  private:
    /**
     * @brief       Initialize workers for the subspaces.
     * @param       subspaceTransformationCache     A vector for caching computation in lti workers.
     * @return      The initizalized workers as vector of variants.
     */
    auto initializeWorkers(
        std::vector<TimeTransformationCache<Number>>& subspaceTransformationCache )
            -> std::vector<WorkerVariant>;

    /**
     * @brief       Compute time successors in all subspaces.
     * @param       currentNodes    The current reachtree-nodes to store the successor sets.
     * @param       workers         The vector of worker variants to use for computation.
     * @return      The time interval where all subspaces satisfy their respective invariants.
     */
    auto computeTimeSuccessorsGetEnabledTime(
        NodeVector& currentNodes,
        std::vector<WorkerVariant>& workers )
            -> carl::Interval<Number>;

    /**
     * @brief       Compute time successors in all subspaces.
     * @param       currentNodes    The current reachtree-nodes to store the constrained successor sets.
     * @param       workers         The vector of worker variants to use for computation.
     * @return      The time interval where all subspaces satisfy their respective invariants.
     */
    void intersectSubspacesWithTimeInterval(
        NodeVector& currentNodes,
        const carl::Interval<Number>& timeInterval );

    /**
     * @brief       Compute the time interval where all singular subspaces satisfy a condition as subset of a maximal time interval.
     * @param       currentNodes    The current reachtree-nodes to access the flowpipes.
     * @param       condition       The condition to intersect with.
     * @param       maxEnabledTime  The maximal time interval where the condition can be enabled (e.g. invariant satisfying time).
     * @return      The time interval where all singular subspaces satisfy the condition simultaneously.
     */
    auto getSingularEnabledTime(
        const NodeVector& currentNodes,
        const Condition<Number>& condition,
        const carl::Interval<Number>& maxEnabledTime )
            -> carl::Interval<Number>;


    /**
     * @brief       Compute the segments where all lti subspaces satisfy a condition as subset of a maximal time interval.
     * @param       currentNodes    The current reachtree-nodes to access the flowpipes.
     * @param       condition       The condition to intersect with.
     * @param       maxEnabledTime  The maximal time interval where the condition can be enabled (e.g. invariant satisfying time).
     * @return      The segments for each subspace where all lti subspaces satisfy the condition simultaneously.
     */
    auto getLtiEnabledSegments(
        const NodeVector& currentNodes,
        const Condition<Number>& condition,
        const carl::Interval<Number> maxEnabledTime )
            -> LTIPredecessors;

    /**
     * @brief       Get the singular jump successors for a transition and the time interval where all successors satisfy the invariant.
     * @param       workers         The vector of worker variants to use for computation.
     * @param       transition      The transition to get successors for.
     * @return      Pair of time interval where the jump can be taken in all subspaces and the unconstrained successors
                    for each subspace.
     */
    auto getSingularJumpSuccessors(
        std::vector<WorkerVariant>& workers,
        Transition<Number>* transition )
            -> std::pair<carl::Interval<Number>, SingularSuccessors>;

    /**
     * @brief       Compute the LTI jump successors for a transition in a given time interval.
     * @param       workers             The vector of worker variants to use for computation.
     * @param       transition          The transition to get successors for.
     * @param       singularEnabledTime A time interval to constrain successors with previous information.
     * @return      The successors for each subspace.
     */
    auto getLtiJumpSuccessors(
        NodeVector& currentNodes,
        std::vector<WorkerVariant>& workers,
        Transition<Number>* transition,
        carl::Interval<Number> singularEnabledTime )
            -> LTISuccessors;

    /**
     * @brief       Create children in all subspace-trees if a transition can be taken in a time interval
                    given by an interval of segment-indices and indicate whether the transition can be taken.
     * @param       currentNodes        The current reachtree-nodes to store child-nodes.
     * @param       transition          The transition to make child-nodes for.
     * @param       segmentInterval     An interval of segment indices corresponding to the time interval.
     * @param       singularSuccessors  The (unconstrained) successor sets of the transition for the singular subspaces.
     * @param       ltiSuccessors       The successor sets oof the transition for the lti subspaces.
     * @return      Nothing if the transition cannot be taken in the given segment-interval
                    or the created child-nodes.
     */
    auto makeChildrenForSegmentInterval(
        NodeVector& currentNodes,
        const Transition<Number>* transition,
        const carl::Interval<SegmentInd>& segmentInterval,
        SingularSuccessors singularSuccessors,
        LTISuccessors ltiSuccessors )
            -> std::optional<std::vector<ReachTreeNode<Representation>*>>;

    /**
     * @brief       Create children in all singular-subspace-trees if a transition can be taken in a time interval.
     * @param       currentNodes        The current reachtree-nodes to store child-nodes.
     * @param       transition          The transition to make child-nodes for.
     * @param       timeInterval        The time interval in which to take the transition.
     * @param       singularSuccessors  The (unconstrained) successor sets of the transition for the singular subspaces.
     * @return      The created child-nodes
     */
    auto makeChildrenForTimeInterval(
        NodeVector& currentNodes,
        const Transition<Number>* transition,
        const carl::Interval<Number>& timeInterval,
        SingularSuccessors singularSuccessors )
            -> std::vector<ReachTreeNode<Representation>*>;




  protected:
    std::deque<NodeVector> mWorkQueue;               // holds the tasks that still need to be computed
    HybridAutomaton<Number> const* mHybridAutomaton; // holds a pointer to the decomposed automaton
    Decomposition mDecomposition;                    // holds decomposition information correpsonding to the automaton
    FixedAnalysisParameters mFixedParameters;        // holds common analysis parameters for all analyzers
    AnalysisParameters mParameters;                  // holds analyzer specific parameters
    std::vector<std::size_t> mSingularTypeSubspaces; // holds the subspaces that have a clock and compute all time successors at once
    std::vector<std::size_t> mLtiTypeSubspaces;      // holds the subspaces that have no clock and have multiple segments

    tNumber const mGlobalTimeHorizon = ( mFixedParameters.jumpDepth + 1 )*mFixedParameters.localTimeHorizon;
};

}  // namespace hypro

#include "DecompositionalAnalyzer.tpp"