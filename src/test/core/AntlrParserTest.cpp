#include "../../hypro/parser/antlr4-flowstar/ParserWrapper.h"
#include "../../hypro/datastructures/HybridAutomaton/HybridAutomaton.h"
#include <iostream>
#include <fstream>
#include "gtest/gtest.h"
#include "../defines.h"

#include <unistd.h>		//getcwd()

using namespace antlr4;
using namespace hypro;

template<typename Number>
class AntlrParserTest : public ::testing::Test {

	protected:

		AntlrParserTest(){}
		~AntlrParserTest(){}
		virtual void setUp(){}
		virtual void tearDown(){}

		void cwd(){
			char cwd[1024];
		   	if (getcwd(cwd, sizeof(cwd)) != NULL)
		       fprintf(stdout, "Current working dir: %s\n", cwd);
		   	else
		       std::cerr << "getcwd() error" << std::endl;
		}

		//Equality of Locations - without looking at transitions or id
		bool equals(const Location<Number>* lhs, const Location<Number>* rhs){
			return (lhs->getFlow() == rhs->getFlow() &&
					lhs->getExternalInput() == rhs->getExternalInput() &&
					lhs->getInvariant() == rhs->getInvariant() &&
					lhs->getName() == rhs->getName() );
		}

		//Equality of Transitions
		bool equals(Transition<Number>* lhs, Transition<Number>* rhs){
			if( (!equals(lhs->getSource(), rhs->getSource())) ||
				(!equals(lhs->getTarget(), rhs->getTarget())) ||
				(lhs->getUrgent() != rhs->getUrgent()) ||
				(lhs->getGuard() != rhs->getGuard()) ||
				(lhs->getReset() != rhs->getReset()) ) {
				return false;
			}
			return true;
		}

		//Equality of States
		bool equals(State_t<Number,Number> lhs, State_t<Number,Number> rhs){
			// quick checks first
    		if (lhs.getNumberSets() != rhs.getNumberSets() || !equals(lhs.getLocation(),rhs.getLocation()) || lhs.getTimestamp() != rhs.getTimestamp()) {
    			return false;
    		}
    		for(std::size_t i = 0; i < lhs.getNumberSets(); ++i) {
    			if( lhs.getSetType(i) != rhs.getSetType(i)) {
    				return false;
    			}
    			if(!boost::apply_visitor(genericCompareVisitor(), lhs.getSet(i), rhs.getSet(i))) {
    				return false;
    			}
    		}
    		return true;
		}

};

TYPED_TEST(AntlrParserTest, JustTesting){

	std::string path("../../../../src/test/core/examples/test_bouncing_ball.txt");
	//std::string path("../../src/test/core/examples/test_bouncing_ball.txt");
	//std::string path("/home/tobias/RWTH/8_WS2017/BA/hypro/src/test/core/examples/test_bouncing_ball.txt");

	this->cwd();
	try{
		boost::tuple<hypro::HybridAutomaton<TypeParam,hypro::State_t<TypeParam,TypeParam>>, hypro::ReachabilitySettings<TypeParam>> h = hypro::parseFlowstarFile<TypeParam>(path);
		SUCCEED();
	}catch (const std::runtime_error& e){
		std::cout << e.what() << std::endl;
		FAIL();
	}

}

TYPED_TEST(AntlrParserTest, EmptyFile){

	std::string path("../../../../src/test/core/examples/test_empty_file.txt");
	//std::string path("/home/tobias/RWTH/8_WS2017/BA/hypro/src/test/core/examples/test_empty_file.txt");

	try{
		boost::tuple<hypro::HybridAutomaton<TypeParam,hypro::State_t<TypeParam,TypeParam>>, hypro::ReachabilitySettings<TypeParam>> h = hypro::parseFlowstarFile<TypeParam>(path);
		FAIL();
	} catch(const std::runtime_error& e){
		std::cout << e.what() << std::endl;
		SUCCEED();
	}
}

TYPED_TEST(AntlrParserTest, MinimalAcceptedFile){

	/*
	 * The simplest hybrid automaton HA that will be accepted by the parser is:
	 * - location l with x' = 1, no invariant, no transitions
	 * - initial state x in [0,0]
	 * - Settings for stepsize, maximum amount of jumps, duration of computation, output name, dimensions to plot
	 */


	std::string path("../../../../src/test/core/examples/test_minimal_accepted_file.txt");
	//std::string path("/home/tobias/RWTH/8_WS2017/BA/hypro/src/test/core/examples/test_minimal_accepted_file.txt");
	boost::tuple<hypro::HybridAutomaton<TypeParam,hypro::State_t<TypeParam,TypeParam>>, hypro::ReachabilitySettings<TypeParam>> h;
	try{
		h = hypro::parseFlowstarFile<TypeParam>(path);
	} catch(const std::runtime_error& e){
		std::cout << e.what() << std::endl;
		FAIL();
	}

	//Test if Settings were right
	hypro::ReachabilitySettings<TypeParam> controlSettings;
	controlSettings.timeBound = TypeParam(3);
	controlSettings.jumpDepth = 1;
	controlSettings.timeStep = TypeParam(0.01);
	controlSettings.fileName = std::string("test_minimal_accepted_file");
	std::vector<std::size_t> dimensions;
	dimensions.push_back(0);
	std::vector<std::vector<std::size_t>> plotDims;
	plotDims.push_back(dimensions);
	controlSettings.plotDimensions = plotDims;
	EXPECT_EQ(boost::get<1>(h), controlSettings);

	//Build hybrid automaton HA
	//The location
	hypro::matrix_t<TypeParam> flow = hypro::matrix_t<TypeParam>::Zero(2,2);
	flow(0,1) = TypeParam(1);
	hypro::LocationManager<TypeParam>& locManager = hypro::LocationManager<TypeParam>::getInstance();
	hypro::Location<TypeParam>* loc = locManager.create(flow);
	std::string name("l");
	loc->setName(name);
	//External Input
	std::vector<carl::Interval<TypeParam>> emptyExtInput;
	emptyExtInput.push_back(carl::Interval<TypeParam>());
	loc->setExtInput(hypro::Box<TypeParam>(emptyExtInput));
	//Put location into locSet
	std::set<hypro::Location<TypeParam>*> locSet;
	locSet.insert(loc);
	//Initial state
	hypro::State_t<TypeParam,TypeParam> initState(loc);
	initState.setTimestamp(carl::Interval<TypeParam>(0));
	typename hypro::HybridAutomaton<TypeParam, hypro::State_t<TypeParam,TypeParam>>::locationStateMap lsMap;
	lsMap.insert(std::make_pair(loc, initState));
	//put everything together
	hypro::HybridAutomaton<TypeParam,hypro::State_t<TypeParam,TypeParam>> controlHA;
	controlHA.setLocations(locSet);
	controlHA.setInitialStates(lsMap);

	//Test if parsed HybridAutomaton has the same content as HA
	//NOTE: We cannot check for equality via operator== yet as locations differ in their id
	//EXPECT_EQ(boost::get<0>(h), controlHA);

	//Check location - this hybrid automaton should only have one location
	HybridAutomaton<TypeParam, hypro::State_t<TypeParam,TypeParam>>& parsedHA = boost::get<0>(h);
	EXPECT_EQ(parsedHA.getLocations().size(), std::size_t(1));
	EXPECT_TRUE(this->equals(parsedHA.getLocation(name), controlHA.getLocation(name)));
	//Check Transitions - this automaton has no transitions
	EXPECT_EQ(parsedHA.getTransitions().size(), std::size_t(0));
	EXPECT_EQ(controlHA.getTransitions().size(), std::size_t(0));
	//Check Initialstates - one init state with one state
	EXPECT_EQ(parsedHA.getInitialStates().size(), std::size_t(1));
	EXPECT_EQ(controlHA.getInitialStates().size(), std::size_t(1));
	EXPECT_NE(parsedHA.getInitialStates().find(parsedHA.getLocation(name)), parsedHA.getInitialStates().end());
	EXPECT_NE(controlHA.getInitialStates().find(controlHA.getLocation(name)), controlHA.getInitialStates().end());
	EXPECT_TRUE(this->equals(parsedHA.getInitialStates().begin()->first, parsedHA.getInitialStates().begin()->first));
	EXPECT_TRUE(this->equals(parsedHA.getInitialStates().begin()->second, parsedHA.getInitialStates().begin()->second));
	//Check local badstates - none
	EXPECT_EQ(parsedHA.getLocalBadStates().size(), std::size_t(0));
	//Check global badstates - none
	EXPECT_EQ(parsedHA.getGlobalBadStates().size(), std::size_t(0));

}

/*
TYPED_TEST(AntlrParserTest, parallelComposition){
	std::string pathA("/home/tobias/RWTH/8_WS2017/BA/examples/A.model");
	std::string pathB("/home/tobias/RWTH/8_WS2017/BA/examples/B.model");

	try{
		std::cout << "Parser A" << std::endl;
		boost::tuple<hypro::HybridAutomaton<TypeParam,hypro::State_t<TypeParam,TypeParam>>, hypro::ReachabilitySettings<TypeParam>> hA = hypro::parseFlowstarFile<TypeParam>(pathA);

		std::cout << "Parser B" << std::endl;
		boost::tuple<hypro::HybridAutomaton<TypeParam,hypro::State_t<TypeParam,TypeParam>>, hypro::ReachabilitySettings<TypeParam>> hB = hypro::parseFlowstarFile<TypeParam>(pathB);

		hypro::HybridAutomaton<TypeParam,hypro::State_t<TypeParam,TypeParam>> haA = boost::get<0>(hA);
		hypro::HybridAutomaton<TypeParam,hypro::State_t<TypeParam,TypeParam>> haB = boost::get<0>(hB);

		haA||haB;

		SUCCEED();
	} catch(const std::runtime_error& e){
		std::cout << e.what() << std::endl;
		FAIL();
	}
}


TYPED_TEST(AntlrParserTest, bouncing_ball_with_label){
	std::string path("/home/tobias/RWTH/8_WS2017/BA/examples/bouncing_ball_with_label.model");

	try{
		boost::tuple<hypro::HybridAutomaton<TypeParam,hypro::State_t<TypeParam,TypeParam>>, hypro::ReachabilitySettings<TypeParam>> h = hypro::parseFlowstarFile<TypeParam>(path);
		SUCCEED();
	} catch(const std::runtime_error& e){
		std::cout << e.what() << std::endl;
		FAIL();
	}
}

TYPED_TEST(AntlrParserTest, railraod_crossing){
	std::string path("/home/tobias/RWTH/8_WS2017/BA/examples/railraod_crossing.model");

	try{
		boost::tuple<hypro::HybridAutomatonComp<TypeParam,hypro::State_t<TypeParam,TypeParam>>, hypro::ReachabilitySettings<TypeParam>> h = hypro::parseFlowstarCompFile<TypeParam>(path);
		SUCCEED();
	} catch(const std::runtime_error& e){
		std::cout << e.what() << std::endl;
		FAIL();
	}
}
*/

//TYPED_TEST(AntlrParserTest, VarNamesPrefixesOfEachOther){ } 	//x, x1, x10, x101 ...
//TYPED_TEST(AntlrParserTest, SettingsTest){}					//smth like maxjumps 0, maxjumps 1e-10...
//TYPED_TEST(AntlrParserTest, LocationsTest){}					//lots of locations, flows, extInputs, invariants
//TYPED_TEST(AntlrParserTest, TransitionsTest){}				//lots of transitions, weird guards, resets, ...
//TYPED_TEST(AntlrParserTest, InitialStatesTest){}
//TYPED_TEST(AntlrParserTest, LocalBadStatesTest){}
//TYPED_TEST(AntlrParserTest, GlobalBadStatesTest){}
//Tests to see if smth throws an exception

//TYPED_TEST(AntlrParserTest, OnlyStart){
//
//	std::string path("../../../../src/test/core/examples/test_only_start.txt");
//	//std::string path("../../src/test/core/examples/test_only_start.txt");
//	try{
//		boost::tuple<hypro::HybridAutomaton<TypeParam,hypro::State_t<TypeParam,TypeParam>>, hypro::ReachabilitySettings<TypeParam>> h = hypro::parseFlowstarFile<TypeParam>(path);
//		FAIL();
//	} catch(const std::runtime_error& e){
//		std::cout << e.what() << std::endl;
//		SUCCEED();
//	}
//
//}

