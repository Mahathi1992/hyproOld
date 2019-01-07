/**
 * @file sampling.h
 *
 */

#pragma once

#include <random>
#include "../logging/Logger.h"
// #include "../../representations/GeometricObject.h"

namespace hypro {


// TODO: Put function: "sampleSet" here, use in conversions.
enum sampling_strategy_set { CENTER, VERTICES, RANDOM_SET_K };
enum sampling_strategy_interval { UNIFORM_K, BORDERS, RANDOM_INTV_K };

// state sets sampling
template <typename SetType, typename Number, sampling_strategy_set SMPL_STRAT>
std::vector<SimulationSample<Number>> sample (const SetType& in, unsigned k=0) {
	// CENTER sampling heuristic
	if (SMPL_STRAT == CENTER) {
		// calculate center of gravity of all representation variants by summing up all vertices and dividing them by how many they are
		std::vector<Point<Number>> vertices = in.vertices();
		std::size_t numberVertices = vertices.size();
		// create an object with the right dimensions to store the sum of all vertices
		Point<Number> sumOfVertices = Point<Number>( vector_t<Number>::Zero( vertices[1].dimension() ));
		for ( std::size_t i=0; i<numberVertices; i++ ) {
			assert(sumOfVertices.dimension() == vertices[i].dimension());
			sumOfVertices += vertices[i];
		}
		// create an object to store the value of the center of gravity
		SimulationSample<Number> center;
		// for each dimension of the object, divide the element by the number of vertices
		for ( std::size_t i=0; i<sumOfVertices.dimension(); i++ ) {
			center[i] = sumOfVertices[i]/numberVertices;
		}
		// create a simulation sample object out of a point
		assert(sumOfVertices.dimension() == center.dimension());
		return std::vector<SimulationSample<Number>>({center});

	// VERTICES sampling heuristic
	} else if (SMPL_STRAT == VERTICES) {
		std::vector<Point<Number>> vertices = in.vertices();
		std::vector<SimulationSample<Number>> vectorOfSamples;

		// store each vertex as an element of list of samples
		for( std::size_t i=0; i<vertices.size(); i++ ) {
			SimulationSample<Number> tmp = SimulationSample<Number>(vertices[i].rawCoordinates());
			vectorOfSamples.emplace_back(tmp);
		}
		assert(vectorOfSamples.size() == vertices.size());
		return vectorOfSamples;

	// RANDOM_SET_K sampling heuristic
/*	} else if (SMPL_STRAT == RANDOM_SET_K) {
		if (k>0 && in.type() == representation_name::box){
			std::random_device rd;  //Will be used to obtain a seed for the random number engine
			std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
			std::vector<SimulationSample<Number>> vectorOfSamples;

			for(unsigned i=0; i<k; i++) {
				std::vector<Number> pointOnInterval = std::vector<Number>(in.dimension());
				for(unsigned d=0; d<in.dimension(); d++){
					//Use dis to transform the random double generated by gen into an double in the given interval
					std::uniform_real_distribution<> dis( carl::toDouble(in.interval(d).lower()), carl::toDouble(in.interval(d).upper()) );
					pointOnInterval[d] = dis(gen);
				}
				SimulationSample<Number> samplesPerDimension = SimulationSample<Number>((Point<Number>(pointOnInterval)).rawCoordinates());
				vectorOfSamples.emplace_back(samplesPerDimension);
			}
			assert(vectorOfSamples.size() == k);
			return vectorOfSamples;
		}
*/	}
	else return std::vector<SimulationSample<Number>>({SimulationSample<Number>()});
}


// time intervals sampling
// for one dimensional interval.
template < typename tNumber, sampling_strategy_interval SMPL_STRAT>
std::vector<tNumber> sample (const carl::Interval<tNumber> guardIntersectInterval, unsigned k=0) {
	// create a vector to store all samples on an interval
	std::vector<tNumber> timings;

	if (SMPL_STRAT == UNIFORM_K) {
		// divide the initial interval into k+1 smaller intervals of equal size
		if (!guardIntersectInterval.isEmpty()){
			std::list<carl::Interval<tNumber>> intervalChunks = guardIntersectInterval.split(k+1);
			// take the upper bound of each interval as a sample
			tNumber upperBound;
			unsigned dropLast = 0;
			for (auto it = intervalChunks.begin(); it != intervalChunks.end(); it++)
			{
				// do not add the last sample, it is the upper bound of the whole interval and is redundant
				if(dropLast<k) {
					upperBound = it->upper();
					timings.emplace_back(upperBound);
				}

				dropLast++;
			}
		}

	} else if (SMPL_STRAT == BORDERS){
		tNumber lowerBound;
		tNumber upperBound;
		if (!guardIntersectInterval.isEmpty()){
			// set lower bound
			lowerBound = guardIntersectInterval.lower();
			timings.emplace_back(lowerBound);

			// set upper bound
			upperBound = guardIntersectInterval.upper();
			timings.emplace_back(upperBound);
		}

	} else if (SMPL_STRAT == RANDOM_INTV_K){
		if(k>0){
			std::random_device rd;  //Will be used to obtain a seed for the random number engine
			std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
			std::uniform_int_distribution<> dis(guardIntersectInterval.lower().get_num().get_ui(), guardIntersectInterval.upper().get_num().get_ui());
		
			for (unsigned i=0; i<k; i++) {
				//Use dis to transform the random unsigned int generated by gen into an int in the given interval
				unsigned long timeNum = dis(gen);
				tNumber q( timeNum, guardIntersectInterval.lower().get_den().get_ui());
				timings.emplace_back(q);
			}
			assert(timings.size() == k);
		}
		
	}
	return timings;
}

} // namespace hypro

