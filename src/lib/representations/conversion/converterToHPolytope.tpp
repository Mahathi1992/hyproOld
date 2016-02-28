/**
 * Specialization for a conversion to an H-polytope.
 * @file converterToHPolytope.tpp
 * @author Simon Froitzheim
 * @author Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *
 * @since	2015-12-17
 * @version	2015-12-17
 */

#include "Converter.h"

//conversion from H-Polytope to H-Polytope (no differentiation between conversion modes - always EXACT)
template<typename Number>
typename Converter<Number>::HPolytope Converter<Number>::toHPolytope( const HPolytope& _source, const CONV_MODE mode ){
    return _source;
}

//conversion from V-Polytope to H-Polytope (no differentiation between conversion modes - always EXACT)
template<typename Number>
typename Converter<Number>::HPolytope Converter<Number>::toHPolytope( const VPolytope& _source, const CONV_MODE mode ){
	HPolytope target;
    if ( !_source.empty() ) {
		// degenerate cases
		unsigned size = _source.size();
		unsigned dimension = _source.dimension();
		if ( size == 1 ) {
			// TODO: Return Box constraints.
		} else if ( size < dimension ) {
			std::vector<Point<Number>> vertices = _source.vertices();

			// ATTENTION: Assumption here: _source is reduced, such that the d points in _source span a d-1 dimensional object.

			// find all hyperplanar descriptions by reducing to d dimensions (get the plane)
			std::size_t dim = vertices.size();
			polytope::dPermutator permutator(dimension, dim);

			std::vector<unsigned> permutation;
			while(!permutator.end()) {
				permutation = permutator();

				// project to chosen dimensions
				std::vector<Point<Number>> reducedVertices;
				reducedVertices.reserve(dim);
				for(const auto& vertex : vertices) {
					vector_t<Number> reductor = vector_t<Number>(dim);
					for(unsigned d = 0; d < dim; ++d)
						reductor(d) = vertex.at(d);

					reducedVertices.push_back(Point<Number>(std::move(reductor)));
				}

				std::vector<std::shared_ptr<Facet<Number>>> facets = convexHull( reducedVertices ).first;
				//std::cout << "Conv Hull end" << std::endl;
				for ( auto &facet : facets ) {
					target.insert( facet->hyperplane() );
				}

			}

			assert( false );
		} else {
			//std::cout << "Conv Hull" << std::endl;
			// TODO: Chose suitable convex hull algorithm
			typename std::vector<Point<Number>> points = _source.vertices();
			std::vector<std::shared_ptr<Facet<Number>>> facets = convexHull( points ).first;
			//std::cout << "Conv Hull end" << std::endl;
			for ( auto &facet : facets ) {
				target.insert( facet->hyperplane() );
			}
			facets.clear();
		}
	}                                                  //Converter<Number>::toHPolytopes the source object into an H-polytope via constructor
    return target;
}

//conversion from Box to H-Polytope (no differentiation between conversion modes - always EXACT)
template<typename Number>
typename Converter<Number>::HPolytope Converter<Number>::toHPolytope( const Box& _source, const CONV_MODE mode ){
     //gets dimension of box
     unsigned dim = _source.dimension();  
     //only continue if dimension is at least 1
     assert( dim >= 1);                                                                      

     //initialize normal matrix as zero matrix with 2*dim rows and dim columns
     matrix_t<Number> directions = matrix_t<Number>::Zero( 2 * dim, dim );       
     //for every dimension:
     for ( unsigned i = 0; i < dim; ++i ) { 
         //write fixed entries (because of box) into the normal matrix (2 each column)
           directions( 2 * i, i ) = -1;
           directions( 2 * i + 1, i ) = 1;                                                   
     }

     //initialize distance vector with 2*dim rows
     vector_t<Number> distances = vector_t<Number>( 2 * dim );                               

      //gets intervals of box
     std::vector<carl::Interval<Number>> intervals = _source.boundaries();
      //for every dimension:
     for ( unsigned i = 0; i < dim; ++i ) {  
         //write inverted lower bound values and upper bound values into the distance vector
           distances( 2 * i ) = -intervals[i].lower();
           distances( 2 * i + 1 ) = intervals[i].upper();                                    
     }

     return std::move(HPolytope(directions, distances));
}

//conversion from zonotope to H-Polytope (no differentiation between conversion modes - always EXACT)
template<typename Number>
typename Converter<Number>::HPolytope Converter<Number>::toHPolytope( const Zonotope& _source, const CONV_MODE mode ){
    //computes vertices from source object
    typename std::vector<vector_t<Number>> vertices = _source.vertices();                   
    //only continue if any actual vertices were received at all
    assert( !vertices.empty() );                                                                            
    std::vector<Point<Number>> points;
    
    for(const auto& vertex : vertices){
        points.emplace_back(vertex);
    }
    HPolytope target = HPolytope(std::move(points)); 
    return target;
}

//TODO conversion from support function to H-Polytope (no differentiation between conversion modes - always OVER)
template<typename Number>
typename Converter<Number>::HPolytope Converter<Number>::toHPolytope( const SupportFunction& _source, const CONV_MODE mode){
    return std::move(HPolytope());
}