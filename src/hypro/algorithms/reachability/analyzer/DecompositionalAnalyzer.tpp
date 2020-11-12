#include "DecompositionalAnalyzer.h"

namespace hypro {

template <typename Representation>
auto DecompositionalAnalyzer<Representation>::run() -> DecompositionalResult {
    std::vector<WorkerVariant> workers;
    // Set up workers
    for ( std::size_t subspace = 0; subspace < mDecomposition.subspaces.size(); ++subspace ) {
        WorkerVariant worker;
        switch( mDecomposition.subspaceTypes[ subspace ] ) {
            case DynamicType::discrete:
                [[fallthrough]];
            case DynamicType::timed:
                [[fallthrough]];
            case DynamicType::singular:
                worker = SingularWorker<Representation>{ *mHybridAutomaton, mFixedParameters };
                break;
            case DynamicType::rectangular:
                // Todo: Initialize rectangular worker
                break;
            case DynamicType::affine:
                [[fallthrough]];
            case DynamicType::linear:
                worker = LTIWorker<Representation>{ *mHybridAutomaton, mParameters, mFixedParameters.localTimeHorizon, TimeTransformationCache<Number>() };
                break;
            default:
                assert( false );
                break;
        }
        workers.push_back( worker );
    }

    while ( !mWorkQueue.empty() ) {
        NodeVector currentNodes = mWorkQueue.front();
        mWorkQueue.pop_front();
        std::vector<std::size_t> unsafeSubspaces;
        int minimalSegmentCount = -1;
        for ( std::size_t subspace = 0; subspace < mDecomposition.subspaces.size(); ++subspace ) {
            /* 
             1. Call workers with currentNode->getInitialSet()[subspace]
             2. Create flowpipe for each subspace
             3. Check length of flowpipe for each subspace, take the minimum
             4. Remove all segments that need too much time elapse
            */

            // 1. + 2. Call workers; create flowpipe
            WorkerVariant subspaceWorker = workers[ subspace ];
            REACHABILITY_RESULT safetyResult = std::visit( detail::computeTimeSuccessorVisitor{ subspace, currentNodes }, subspaceWorker );
            if ( safetyResult == REACHABILITY_RESULT::UNKNOWN ) {
                unsafeSubspaces.push_back( subspace );
            }
            // Cannot determine unsafety yet, because time to elapse might be limited by other subspaces
            /* Idea for checking safety: 
                For all unsafe subspaces, make sure that at least one segment is
                removed by time constraints (as soon as an unsafe state is determined
                in the worker, it will return, so only the last segment is unsafe)
            */
            // 3. Get minimal segmentCount
            int segmentCount = currentNodes[ subspace ]->getFlowpipe().size();
            if ( minimalSegmentCount == -1 || segmentCount < minimalSegmentCount ) {
                minimalSegmentCount = segmentCount;
            }
        }
        // 4. Remove all segments that need too much time elapse
        for ( std::size_t subspace = mDecomposition.subspaces.size() - 1;; --subspace ) {
            if ( currentNodes[ subspace ]->getFlowpipe().size() <= minimalSegmentCount ) {
                // Nothing to remove
                if ( subspace == unsafeSubspaces.back() ) {
                    // Unsafe state is within time limit
                    return { Failure{ currentNodes[ subspace ] } };
                }
            }
            else {
                // todo: remove elements from flowpipe (?) or create new node for subspace
            }
        }

    }
}



} // namespace hypro