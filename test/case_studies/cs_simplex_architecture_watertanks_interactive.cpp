/*
 * Created by stefan on 04.08.21.
 */

#include <filesystem>
#include <hypro/algorithms/reachability/Reach.h>
#include <hypro/parser/antlr4-flowstar/ParserWrapper.h>
#include <hypro/representations/GeometricObjectBase.h>
#include <hypro/util/plotting/InteractivePlotter.h>
#include <string>

void Simplex_Watertanks_Reachability( std::size_t maxJumps = 5 ) {
	using Number = double;
	using Representation = hypro::Box<Number>;

	auto base_path = std::filesystem::current_path().parent_path().parent_path().append( "examples/input/" );
	std::string filename{ "21_simplex_watertanks_deterministic_monitor_small_init.model" };

	auto [automaton, reachSettings] = hypro::parseFlowstarFile<Number>( base_path.string() + filename );

	std::cout << "automaton \n" << automaton << std::endl;

	auto settings = hypro::convert( reachSettings );
	settings.rStrategy().front().detectJumpFixedPoints = true;
	settings.rStrategy().front().detectContinuousFixedPointsLocally = false;
	// settings.rFixedParameters().localTimeHorizon = 1;
	settings.rFixedParameters().jumpDepth = maxJumps;
	settings.rStrategy().begin()->aggregation = hypro::AGG_SETTING::AGG;

	auto roots = hypro::makeRoots<Representation>( automaton );
	auto reacher = hypro::reachability::Reach<Representation>( automaton, settings.fixedParameters(),
															   settings.strategy().front(), roots );
	auto result = reacher.computeForwardReachability();
	std::cout << "System is safe: " << result << std::endl;

	// invoke interactive plotter
	hypro::plotting::InteractivePlotter<Representation> ipl{ roots, settings.plotting() };
	ipl.run();
}

int main() {
	Simplex_Watertanks_Reachability( 12 );
	return 0;
}
