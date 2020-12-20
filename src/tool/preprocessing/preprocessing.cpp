#include "preprocessing.h"

namespace hydra {
namespace preprocessing {
hypro::PreprocessingInformation preprocess( const hypro::HybridAutomaton<hydra::Number>& automaton,
											const hypro::ReachabilitySettings&,
                                            bool decompose ) {
	auto dynamicsType = getDynamicType( automaton );
    hypro::PreprocessingInformation information{ dynamicsType };

    if ( decompose ) {
        auto [decomposedHa, decomposition] = decomposeAutomaton( automaton );
        for ( std::size_t subspace = 0; subspace < decomposition.subspaceTypes.size(); ++subspace ) {
            if ( decomposition.subspaceTypes[ subspace ] != hypro::DynamicType::linear && decomposition.subspaceTypes[ subspace ] != hypro::DynamicType::affine ) {
                addClockToAutomaton( decomposedHa, subspace );
            }
        }
        information.decomposition = decomposition;
        information.decomposedHa = decomposedHa;
    }
	return information;
}

}  // namespace preprocessing
}  // namespace hydra
