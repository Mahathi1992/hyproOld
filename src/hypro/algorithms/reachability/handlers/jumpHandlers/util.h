#pragma once

#include "../../../../datastructures/reachability/ReachTreeNode.h"
//#include "../../../../../../include/lib/utils/DecisionEntity.h"
//#include <hypro/datastructures/HybridAutomaton/Transition.h>

namespace hypro {

template<typename State>
bool isTreeExtension(typename ReachTreeNode<State>::Node_t node, Transition<typename State::NumberType>* transition) {
	if(node->getChildren().empty()) {
		//assert(mLocalTimingUpdate.getChildren().empty());
		TRACE("hydra.worker.discrete","Regular tree extension as there are no child nodes.");
		return true;
	} else {
		std::vector<ReachTreeNode<State>*> tChildren = node->getChildrenForTransition(transition);
		if(tChildren.empty()){
			TRACE("hydra.worker.discrete","Regular tree extension as there are no child nodes for this transition.");
			return true;
		}
	}
	return false;
}

template<typename State>
inline void collectTimespansForRefinementLevel(std::vector<carl::Interval<tNumber>>& timespans, typename ReachTreeNode<State>::NodeList_t& nodes, unsigned targetLevel) {
	for(auto nodeIt = nodes.begin(); nodeIt != nodes.end(); ){
		if((*nodeIt)->getRefinements().size() == targetLevel+1 && !(*nodeIt)->getRefinements().at(targetLevel).isDummy) {
			// We only consider child nodes that are not already set up with the required btLevel. This can happen due to
			// "wavy" trajectories where a transition is enabled more than once.
			TRACE("hydra.worker.refinement", "The node " << (*nodeIt) << " has already been set up for level " << targetLevel << " and is not considered again.");
			nodeIt = nodes.erase(nodeIt);
		} else {
			TRACE("hydra.worker.refinement","Old child relevant for transition is " << (*nodeIt));
			if(timespans.empty() || !set_have_intersection(timespans.back(), (*nodeIt)->getTimestamp((*nodeIt)->getLatestBTLevel()))) {
				timespans.push_back((*nodeIt)->getTimestamp((*nodeIt)->getLatestBTLevel()));
			} else {
				timespans.back() = timespans.back().convexHull((*nodeIt)->getTimestamp((*nodeIt)->getLatestBTLevel()));
			}
			++nodeIt;
		}
	}
}

template<typename State>
inline void preProcess(typename ReachTreeNode<State>::NodeList_t& toMatch, const std::vector<carl::Interval<tNumber>>& timespans) {
	// if the nodes timestamp lies completely outside the pre-defined timespan, do not consider this.
	bool valid = false;
	for(auto nodeIt = toMatch.begin(); nodeIt != toMatch.end();) {
		for(const auto& enabledInterval : timespans) {
			if(set_have_intersection(enabledInterval, (*nodeIt)->getPath().back().timeInterval)) {
				valid = true;
				break;
			}
		}
		if(!valid) {
			TRACE("hydra.worker.refinement","Do not consider child with timestamp " << (*nodeIt)->getPath().back().timeInterval);
			nodeIt = toMatch.erase(nodeIt);
		} else {
			++nodeIt;
		}
	}
}

template<typename State>
struct nodeUpdater{
	nodeUpdater(const std::shared_ptr<Task<State>>& currentTask, WorkQueue<std::shared_ptr<Task<State>>>* localQueue, WorkQueue<std::shared_ptr<Task<State>>>* localCEXQueue, std::size_t targetLevel)
		: mTask(currentTask)
		, mLocalQueue(localQueue)
		, mLocalCEXQueue(localCEXQueue)
		, mTargetLevel(targetLevel)
	{}

	inline void createAndRaiseRefinementTask(typename ReachTreeNode<State>::Node_t node, typename ReachTreeNode<State>::Node_t update) {
		assert(mTask->btInfo.btPath.at(mTask->btInfo.currentBTPosition+1).isDiscreteStep());
		// update refinements in case we continue backtracking
		node->updateContent(update);

		// also set intermediate refinements
		for(std::size_t i = 0; i < node->getRefinements().size(); ++i) {
			// convert, in case this is necessary
			if(node->getRefinements()[i].isDummy) {
				SettingsProvider<State>::getInstance().getStrategy().advanceToLevel(node->rGetRefinements()[i].initialSet, i);
				node->rGetRefinements()[i].isDummy = false;
			}
		}

		assert(!node->getRefinements().at(mTask->btInfo.btLevel).entryTimestamp.isEmpty());
		std::shared_ptr<Task<State>> newTask(new Task<State>(node, mTask->btInfo));
		newTask->btInfo.currentBTPosition += 2;
		TRACE("hydra.worker.refinement","Created task from updated child node. Bt-Level: " << newTask->btInfo.btLevel << ", bt-path: " << newTask->btInfo.btPath);
		assert(newTask->treeNode->getRefinements().at(newTask->btInfo.btLevel).fullyComputed == false);
		assert(newTask->treeNode->getRefinements().at(newTask->btInfo.btLevel).isDummy == false);
		DEBUG("hydra.worker.refinement", std::this_thread::get_id() << ": Create new CEX-Task<State> (local) with tree node " << newTask->treeNode);
		if(!SettingsProvider<State>::getInstance().useGlobalQueuesOnly()){
			mLocalCEXQueue->nonLockingEnqueue(std::move(newTask));
		} else {
			mLocalCEXQueue->enqueue(std::move(newTask));
		}
		DEBUG("hydra.worker.refinement", std::this_thread::get_id() << ": Local CEX-Queue size: " << mLocalCEXQueue->nonLockingSize() << "localCEXQueue is now:\n" << mLocalCEXQueue);
	}

	inline void createAndRaiseTask(typename ReachTreeNode<State>::Node_t node, typename ReachTreeNode<State>::Node_t update, bool isRefinement) {
		if(isRefinement) {
			createAndRaiseRefinementTask(node, update);
		} else {
			// set refinements in any case - even though it might not be used for further refinement.
			node->setNewRefinement(mTargetLevel,update->getRefinements().at(mTargetLevel));

			// also set intermediate refinements
			for(std::size_t i = 0; i < node->getRefinements().size(); ++i) {
				// convert, in case this is necessary
				if(node->getRefinements()[i].isDummy) {
					SettingsProvider<State>::getInstance().getStrategy().advanceToLevel(node->rGetRefinements()[i].initialSet, i);
					node->rGetRefinements()[i].isDummy = false;
				}
			}

			#ifdef RESET_REFINEMENTS_ON_CONTINUE_AFTER_BT_RUN
        	if(mTargetLevel > 0 && !node->getRefinements().at(0).fullyComputed) {
        		node->convertRefinement(mTargetLevel, 0, SettingsProvider<State>::getInstance().getStrategy().at(0));
				if(SettingsProvider<State>::getInstance().useLocalTiming()) {
		        	node->rGetRefinements().at(0).entryTimestamp = carl::Interval<tNumber>(0);
		        }
        	}
        	#endif
		}
	}

	inline void operator()(typename ReachTreeNode<State>::NodeList_t& toMatch, typename ReachTreeNode<State>::NodeList_t& reference, Transition<typename State::NumberType>* transition) {
		for(auto nodeIt = toMatch.begin(); nodeIt != toMatch.end(); ) {
			// indicates a successful match to an existing child node (update).
			bool matchedNewChild = false;

			// find matching old child, if available.
			for(auto referenceIt = reference.begin(); referenceIt != reference.end(); ) {
				TRACE("hydra.worker.refinement","Add child.");
				assert((*nodeIt)->getPath().size() > 0);
				assert((*nodeIt)->getPath().back().isDiscreteStep());
				// timestamp matching
				// TODO: Attention! We compare the timestamp of the latest bt level of the old child. It would be better to compare
				// to the smallest timestamp for that transition -> add meta-function for that.
				DEBUG("hydra.worker.refinement","New child timestamp: " << (*nodeIt)->getPath().back().timeInterval << " and old child timestamp: " << (*referenceIt)->getTimestamp((*referenceIt)->getLatestBTLevel()));
				if(set_have_intersection((*nodeIt)->getPath().back().timeInterval, (*referenceIt)->getTimestamp((*referenceIt)->getLatestBTLevel()))) {
					DEBUG("hydra.worker.refinement","Timestamps " << (*referenceIt)->getTimestamp((*referenceIt)->getLatestBTLevel()) << " and " << (*nodeIt)->getPath().back().timeInterval << " intersect.");
					DEBUG("hydra.worker.refinement","Updated child " << *referenceIt << " with information from new child " << *nodeIt);

					// if the path has been fully discovered, make the new task a non-refinement task, otherwise keep btInfo
					// 2nd condition: The timestamps have to match to check that we are solely on the bt path and just add child node refinements otherwise.
					TRACE("hydra.worker.refinement", "Still refinement: " << (mTask->btInfo.currentBTPosition < mTask->btInfo.btPath.size()));
					TRACE("hydra.worker.refinement", "Considered transition: " << *mTask->btInfo.btPath.at(mTask->btInfo.currentBTPosition+1).transition << " and required transition: " << *transition);
					if(mTask->btInfo.currentBTPosition < mTask->btInfo.btPath.size() &&
						mTask->btInfo.btPath.at(mTask->btInfo.currentBTPosition+1).transition == transition &&
						set_have_intersection(mTask->btInfo.btPath.at(mTask->btInfo.currentBTPosition+1).timeInterval, (*referenceIt)->getTimestamp((*referenceIt)->getLatestBTLevel())) ){

						createAndRaiseTask(*referenceIt, *nodeIt, true);

					} else {
						createAndRaiseTask(*referenceIt, *nodeIt, false);
					}

					// if the timestamps matched and we used this old node, do not re-consider it for the next new child
					// -> remove from list.
					referenceIt = reference.erase(referenceIt);
					matchedNewChild = true;
					break;
				} else {
					++referenceIt;
				}
			}
			if(matchedNewChild) {
				nodeIt = toMatch.erase(nodeIt);
			} else {
				++nodeIt;
			}
		}
	}

private:
	const std::shared_ptr<Task<State>>& mTask;
	WorkQueue<std::shared_ptr<Task<State>>>* mLocalQueue;
	WorkQueue<std::shared_ptr<Task<State>>>* mLocalCEXQueue;
	std::size_t mTargetLevel;
};

template<typename State>
inline void insertAndCreateTask(typename ReachTreeNode<State>::NodeList_t& toInsert, const std::shared_ptr<Task<State>>& currentTask, WorkQueue<std::shared_ptr<Task<State>>>* localQueue, WorkQueue<std::shared_ptr<Task<State>>>* localCEXQueue, std::size_t targetLevel) {
	auto nodeIt = toInsert.begin();
	while(nodeIt != toInsert.end()) {
		// tree update
		currentTask->treeNode->addChild(*nodeIt);
		DEBUG("hydra.worker.refinement","Add fresh new child " << *nodeIt);
		// At this point the newly updated child can be reached via a transition on the btPath and its entry-timing
		// is intersecting with the transition-timing of the btPath. Thus it effectively lies on the btPath and
		// we create a refinement Task<State> and enqueue it to the CEXQueue.
		if(currentTask->btInfo.currentBTPosition < currentTask->btInfo.btPath.size()){
			assert(currentTask->btInfo.currentBTPosition % 2 == 0);
			assert(!currentTask->btInfo.btPath.at(currentTask->btInfo.currentBTPosition).isDiscreteStep());
			std::shared_ptr<Task<State>> newTask(new Task<State>(*nodeIt, currentTask->btInfo));
			newTask->btInfo.currentBTPosition += 2;
			TRACE("hydra.worker.refinement","Created task from fresh child node. Bt-Level: " << newTask->btInfo.btLevel << ", bt-path: " << newTask->btInfo.btPath);
			assert(newTask->treeNode->getRefinements().at(newTask->btInfo.btLevel).fullyComputed == false);
			assert(newTask->treeNode->getRefinements().at(newTask->btInfo.btLevel).isDummy == false);
			assert(!newTask->treeNode->getRefinements().at(newTask->btInfo.btLevel).entryTimestamp.isEmpty());
			DEBUG("hydra.worker.refinement", std::this_thread::get_id() << ": Create new CEX-Task<State> (local) with tree node " << newTask->treeNode);
			if(!SettingsProvider<State>::getInstance().useGlobalQueuesOnly()){
				localCEXQueue->nonLockingEnqueue(std::move(newTask));
			} else {
				localCEXQueue->enqueue(std::move(newTask));
			}
			DEBUG("hydra.worker.refinement", std::this_thread::get_id() << ": Local CEX-Queue size: " << localCEXQueue->nonLockingSize() << "localCEXQueue is now:\n" << localCEXQueue);
		} else {
			#ifdef RESET_REFINEMENTS_ON_CONTINUE_AFTER_BT_RUN
        	if(targetLevel > 0) {
        		// move refinement to level 0, convert type in case necessary
        		(*nodeIt)->convertRefinement(targetLevel, 0, SettingsProvider<State>::getInstance().getStrategy().at(0));
	        	// also set intermediate refinements
				for(std::size_t i = 0; i < (*nodeIt)->getRefinements().size(); ++i) {
					// convert, in case this is necessary
					if((*nodeIt)->getRefinements()[i].isDummy) {
						(*nodeIt)->convertRefinement(targetLevel,i,SettingsProvider<State>::getInstance().getStrategy()[i]);
						(*nodeIt)->rGetRefinements()[i].isDummy = false;
					}
				}
        	}
        	#endif

			std::shared_ptr<Task<State>> newTask(new Task<State>(*nodeIt));
			TRACE("hydra.worker.refinement","Created task from fresh child node. Bt-Level: " << newTask->btInfo.btLevel << ", bt-path: " << newTask->btInfo.btPath);
			assert(newTask->btInfo.btPath.size() == 0);
			DEBUG("hydra.worker.refinement", std::this_thread::get_id() << ": Refinement finished, create new Task<State> (local) with tree node " << newTask->treeNode);
			if(!SettingsProvider<State>::getInstance().useGlobalQueuesOnly()){
				localQueue->nonLockingEnqueue(std::move(newTask));
			} else {
				localQueue->enqueue(std::move(newTask));
			}
			DEBUG("hydra.worker.refinement", std::this_thread::get_id() << ": Local queue size: " << localQueue->nonLockingSize() << "localQueue is now:\n" << localQueue);
		}
		++nodeIt;
	}
}

} // namespace hypro
