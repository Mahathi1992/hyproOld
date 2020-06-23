#include "analysis.h"

namespace hydra {
namespace reachability {

using namespace hypro;

template <typename Analyzer>
void concrete_analyze( Analyzer& analyzer ) {
	START_BENCHMARK_OPERATION( Verification );
	auto result = analyzer.run();

	if ( result == REACHABILITY_RESULT::UNKNOWN ) {
		std::cout << "Could not verify safety." << std::endl;
		// Call bad state handling (e.g., return path)
	} else {
		std::cout << "The model is safe." << std::endl;
	}
	EVALUATE_BENCHMARK_RESULT( Verification );

	// call to plotting.
	START_BENCHMARK_OPERATION( Plotting );
	std::size_t amount = 0;
	for ( const auto& fp : analyzer.getFlowpipes() ) {
		amount += fp.size();
	}

	auto& plt = Plotter<typename State::NumberType>::getInstance();
	for ( std::size_t pic = 0; pic < setting.plotDimensions.size(); ++pic ) {
		std::cout << "Prepare plot " << pic + 1 << "/" << setting.plotDimensions.size() << "." << std::endl;
		plt.setFilename( setting.plotFileNames[pic] );
		std::size_t segmentCount = 0;
		for ( const auto& fp : analyzer.getFlowpipes() ) {
			for ( const auto& segment : fp ) {
				std::cout << "\r" << segmentCount++ << "/" << amount << "..." << std::flush;
				plt.addObject( segment.project( setting.plotDimensions[pic] ).vertices() );
			}
		}
		plt.plot2d( setting.plottingFileType );	 // writes to .plt file for pdf creation
	}
	EVALUATE_BENCHMARK_RESULT( Plotting );
}

struct Dispatcher {
	template <typename Rep>
	void operator()( HybridAutomaton<Number>& automaton, Settings setting ) {
		LTIAnalyzer<hypro::State<hydra::Number>, Rep> > analyzer{ automaton, setting };
		concrete_analyze( analyzer );
	}
};

void analyze( HybridAutomaton<Number>& automaton, Settings setting ) {
	if ( setting.strategy.size() == 1 ) {
		dispatch<hydra::Number, Converter<hydra::Number>>( setting.strategy.front().representation_type, setting.strategy.front().representation_setting, Dispatcher{}, automaton, setting );
	} else {
		CEGARAnalyzer<Number> analyzer{ automaton, setting.strategy };
	}
}

}  // namespace reachability
}  // namespace hydra
