#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <cassert>
#include "RootGrowTree.h"

//Specialized subclass for transformations as example of a unary operator
class TrafoOp : public RootGrowNode {
private:
	////// General Interface

	SF_TYPE type = SF_TYPE::TRAFO;
	bool isUnary = true;
	unsigned originCount = 1;

	////// Members for this class

	int factor = 5;
	int translation = -10;

public:
	TrafoOp() {}
	~TrafoOp(){}
	SF_TYPE getType() const override { return type; }
	bool isItUnary() const override { return isUnary; }
	unsigned getOriginCount() const { return originCount; }

	//Return transformed result of its children
	std::vector<EvalResult> evaluate(Matrix m) { 
		auto tmp = mChildren.at(0)->evaluate(m);
		for(unsigned i=0; i < tmp.size(); i++){
			tmp.at(i) = EvalResult(factor*tmp.at(i) + translation);
		}
		return tmp; 
	}

	//Given the results, return vector of evaluation results (here only first place needed, since unary op), here, we also modify
	std::vector<EvalResult> accumulate(std::vector<std::vector<EvalResult>>& resultStackBack){ 
		//Make sure only one vector of parameters here
		assert(resultStackBack.size() == 1); 
		for(unsigned i = 0; i < resultStackBack.at(0).size(); i++){
			resultStackBack.at(0).at(i) = EvalResult(factor*resultStackBack.at(0).at(i).res + translation);	
		}
		return resultStackBack.at(0);
	}

	//Push needed children onto callStack, push transformed given parameter onto paramStack, push callingFrame/results pair onto resultStack
	void pushToStacks(std::vector<RootGrowNode*>& callStack, 
					 std::vector<Matrix>& paramStack,
					 std::vector<std::pair<int,std::vector<std::vector<EvalResult>>>>& resultStack,
					 Matrix param,
					 std::size_t callingFrame) override {
		std::cout << "in TrafoOp::pushToStack, callStack size before is: " << callStack.size() << std::endl;
		callStack.push_back(mChildren.at(0));
		paramStack.push_back(factor*param.entry + translation);
		resultStack.push_back(std::make_pair(callingFrame, std::vector<std::vector<EvalResult>>()));
	}
};


//Specialized subclass for sums as example of a binary operator
class SumOp : public RootGrowNode {
private:
	////// General Interface

	SF_TYPE type = SF_TYPE::SUM;
	bool isUnary = false;
	unsigned originCount = 2;

	////// Members for this class: uses the children of the Node

public:
	SumOp(){}
	~SumOp(){}
	bool isItUnary() const override { return isUnary; }
	SF_TYPE getType() const override { return type; }
	unsigned getOriginCount() const { return originCount; }

	//Do the sum op
	std::vector<EvalResult> evaluate(Matrix m) { 
		auto tmp0 = mChildren.at(0)->evaluate(m);
		auto tmp1 = mChildren.at(1)->evaluate(m);
		std::vector<EvalResult> result;
		for(unsigned i=0; i < tmp0.size(); i++){
			result.at(i) = tmp0.at(i) + tmp1.at(i);
		}
		return result;
	}

	//Given two result vecs, sum them coefficientwise
	std::vector<EvalResult> accumulate(std::vector<std::vector<EvalResult>>& resultStackBack) {
		assert(resultStackBack.size() == 2);
		assert(resultStackBack.at(0).size() == resultStackBack.at(1).size());
		std::vector<EvalResult> r;
		for(unsigned i=0; i < resultStackBack.at(0).size(); i++){
			EvalResult e;
			e.res = resultStackBack.at(0).at(i).res + resultStackBack.at(1).at(i).res;
			r.emplace_back(e);
		}
		return r;
	}

	//push onto stacks
	void pushToStacks(std::vector<RootGrowNode*>& callStack, 
		 			 std::vector<Matrix>& paramStack,
					 std::vector<std::pair<int,std::vector<std::vector<EvalResult>>>>& resultStack,
					 Matrix param,
					 std::size_t callingFrame) {
		std::cout << "in SumOp::pushToStack, callStack size before is: " << callStack.size() << std::endl;
		callStack.push_back(mChildren.at(0));
		callStack.push_back(mChildren.at(1));
		std::cout << "in SumOp::pushToStack, callStack size after is: " << callStack.size() << std::endl;
		paramStack.push_back(param);
		paramStack.push_back(param);
		resultStack.emplace_back(std::make_pair(callingFrame,std::vector<std::vector<EvalResult>>()));
		resultStack.emplace_back(std::make_pair(callingFrame,std::vector<std::vector<EvalResult>>()));
	}
};

//Another still templated subclass of RootGrowNode
template<typename Rep> 
class Leaf : public RootGrowNode {
private:
	Rep member = Rep();
	SF_TYPE type = SF_TYPE::LEAF;
	bool isUnary = true;
	unsigned originCount = 0;
	int mem = 2;
public:
	Leaf(){}
	Leaf(Rep r) : member(r) {}
	~Leaf(){}
	bool isItUnary() const override { return isUnary; }
	SF_TYPE getType() const override { return type; }
	unsigned getOriginCount() const { return originCount; }
	Rep say(){ return member; }

	//Evaluate leaf. Just return a vector of length m, filled with mem
	std::vector<EvalResult> evaluate(Matrix m){ 
		return std::vector<EvalResult>(m.entry, mem); 
	}

	//Leaves cannot accumulate
	std::vector<EvalResult> accumulate(std::vector<std::vector<EvalResult>>& resultStackBack){
		assert(false);
		return std::vector<EvalResult>();
	}

	//Leaves cannot pushToStack as they cannot be an intermediate node
	void pushToStacks(	std::vector<RootGrowNode*>& callStack, 
								std::vector<Matrix>& paramStack,
								std::vector<std::pair<int,std::vector<std::vector<EvalResult>>>>& resultStack,
								Matrix param,
								std::size_t callingFrame) {
		std::cout << "in Leaf::pushToStack, callStack size before is: " << callStack.size() << std::endl;
		assert(false);
	}
};

//Support Function Content
class SFC : public RootGrowTree {
//private:
	//Inherited:
	//RootGrowNode* root
public:
	SFC() {}
	SFC(RootGrowNode* root) : RootGrowTree(root) {}

	std::vector<RootGrowNode*> getChildren(){
		return this->getRoot()->getChildren();
	}

	friend std::ostream& operator<< (std::ostream& ostr, const SFC& rhs){
		ostr << *(rhs.getRoot()) << std::endl;
		return ostr;
	}

	//Here: function evaluate, that calls traverse on the root node to start the traversing process
	std::vector<EvalResult> evaluate(Matrix directions){

		//Usings
		using Node = RootGrowNode*;
		using Param = Matrix;
		using Res = std::vector<EvalResult>;
		
		//Prepare Stacks
		std::vector<Node> callStack;
		std::vector<Param> paramStack;
		std::vector<std::pair<int,std::vector<Res>>> resultStack;  
		callStack.push_back(mRoot);
		paramStack.push_back(directions);
		resultStack.push_back(std::make_pair(-1, std::vector<Res>()));

		while(!callStack.empty()){

			Node cur = callStack.back();
			Param currentParam = paramStack.back();

			std::cout << "callStack size: " << callStack.size() << std::endl;
			std::cout << "cur type is: " << cur->getType() << std::endl;

			std::cout << "START EVALUATION\n";

			if(cur->getOriginCount() == 0){

				std::pair<int,std::vector<Res>> currentResult = resultStack.back();

				//If leaf and end of stack is reached
				if(currentResult.first == -1){

					std::cout << "Case 1\n";
					return cur->evaluate(currentParam);

				//If leaf and not end of stack is reached	
				} else {

					std::cout << "Case 2\n";
					auto tmp = cur->evaluate(currentParam);
					resultStack.at(currentResult.first).second.emplace_back(tmp);

				}

				// delete result frame and close recursive call
				callStack.pop_back();
				paramStack.pop_back();
				resultStack.pop_back();

			} else {

				//If enough arguments for operation of node and #arguments != 0
				if(resultStack.back().second.size() == cur->getOriginCount()) {

					std::cout << "Case 3\n";

					Res accumulatedResult = cur->accumulate(resultStack.back().second);

					// we reached the top, exit
					if(resultStack.back().first == -1) {
						return accumulatedResult;
					}

					// forward result.
					resultStack.at(resultStack.back().first).second.emplace_back(accumulatedResult);

					// delete result frame and close recursive call
					callStack.pop_back();
					paramStack.pop_back();
					resultStack.pop_back();

				//Every other case (recursive call)
				} else {

					std::cout << "Case 4\n";

					// here we create the new stack levels.
					std::size_t callingFrame = callStack.size() - 1;

					cur->pushToStacks(callStack, paramStack, resultStack, currentParam, callingFrame);
					std::cout << "In SFC::evaluate, callStack size is: " << callStack.size() << std::endl;
				}
			}
		}

		std::cout << "THIS SHOULD NOT HAPPEN." << std::endl;
		return std::vector<EvalResult>();
	}
	

	//NEXT: Implement evaluate in all node types and implement stack operations

};

//Support Function
class SF {
private:
	SFC* content;
public:
	SF(){}
	SF(SFC* c) : content(c) {}

	SFC* getContent() const { return content; }

	void listAllDirectChildren(){
		for(auto ch : content->getRoot()->getChildren()){
			std::cout << ch->getType() << "," << std::endl;
		}
	}

	void listUnderlyingTree(){
		std::cout << *(content->getRoot()) << std::endl;
	}

	friend std::ostream& operator<< (std::ostream& ostr, const SF& rhs){
		ostr << *(rhs.getContent()) << std::endl;
		return ostr;
	}
};
