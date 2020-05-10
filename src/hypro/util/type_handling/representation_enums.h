#pragma once
#include "better_enums/enum.h"

namespace hypro {
/**
 * @brief      Enum encapsulating all provided state set representations to be able to determine a type.
 */
BETTER_ENUM( representation_name, int,
			 box,
			 carl_polytope,
			 constraint_set,
			 polytope_h,
			 polytope_v,
			 ppl_polytope,
			 support_function,
			 taylor_model,
			 zonotope,
			 difference_bounds,
			 SFN,
			 UNDEF,
			 genStar )
}  // namespace hypro