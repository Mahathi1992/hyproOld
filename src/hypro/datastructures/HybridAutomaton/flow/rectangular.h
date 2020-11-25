#pragma once
#include <carl/core/Variable.h>
#include <carl/interval/Interval.h>
#include <iosfwd>
#include <map>

namespace hypro {

template <typename Number>
class rectangularFlow {
  private:
	std::map<carl::Variable, carl::Interval<Number>> mFlowIntervals;

  public:
	rectangularFlow() = default;
	rectangularFlow( const rectangularFlow<Number>& in ) = default;
	explicit rectangularFlow( const std::map<carl::Variable, carl::Interval<Number>>& intervals )
		: mFlowIntervals( intervals ) {}
	virtual ~rectangularFlow() {}

	static DynamicType type() { return DynamicType::rectangular; }

	void setFlowIntervals( const std::map<carl::Variable, carl::Interval<Number>>& in ) { mFlowIntervals = in; }
	void setFlowIntervalForDimension( const carl::Interval<Number>& intv, carl::Variable dim ) {
		mFlowIntervals[dim] = intv;
	}

	const std::map<carl::Variable, carl::Interval<Number>>& getFlowIntervals() const { return mFlowIntervals; }
	const carl::Interval<Number>& getFlowIntervalForDimension( carl::Variable dim ) const {
		assert( mFlowIntervals.find( dim ) != mFlowIntervals.end() );
		return mFlowIntervals.at( dim );
	}

	std::size_t dimension() const { return mFlowIntervals.size(); }
	std::size_t size() const { return mFlowIntervals.size(); }
	bool empty() const { return mFlowIntervals.empty(); }

	DynamicType getDynamicsType() const {
		return DynamicType::rectangular;
	}

	bool isTimed() const {
		for ( const auto& keyVal : mFlowIntervals ) {
			if ( keyVal.second != carl::Interval<Number>( 1 ) ) {
				return false;
			}
		}
		return true;
	}

	bool isDiscrete() const {
		for ( const auto& keyVal : mFlowIntervals ) {
			if ( keyVal.second != carl::Interval<Number>( 0 ) ) {
				return false;
			}
		}
		return true;
	}

	friend bool operator==( const rectangularFlow<Number>& lhs, const rectangularFlow<Number>& rhs ) {
		if ( lhs.getFlowIntervals().size() != rhs.getFlowIntervals().size() ) {
			return false;
		}

		for ( const auto keyVal : lhs.getFlowIntervals() ) {
			if ( rhs.getFlowIntervals().find( keyVal.first ) == rhs.getFlowIntervals().end() ) {
				return false;
			} else {
				if ( keyVal.second != rhs.getFlowIntervalForDimension( keyVal.first ) ) {
					return false;
				}
			}
		}
		return true;
	}

	friend bool operator!=( const rectangularFlow<Number>& lhs, const rectangularFlow<Number>& rhs ) {
		return !( lhs == rhs );
	}

	friend std::ostream& operator<<( std::ostream& out, const rectangularFlow<Number>& in ) {
		for ( const auto& i : in.getFlowIntervals() ) {
			out << i.first << ": " << i.second << ", ";
		}
		return out;
	}

	std::vector<Point<Number>> vertices() const {
		PermutationSequenceGenerator<WithReturningNoOrder> sequencer{2,dim};
	std::vector<Point<Number>> flowVertices;
	while(!sequencer.end()) {
		std::vector<std::size_t> selection = sequencer();
		Point<Number> vertex{vector_t<Number>::Zero(dim)};
		for(std::size_t i = 0; i < selection.size(); ++i) {
			assert(selection[i] == 1 || selection[i] == 0)
			if(selection[i] == 0) {
				vertex[i] = flow.getFlowIntervals()[VariablePool::getInstance().carlVarByIndex(i)].lower();
			} else {
				vertex[i] = flow.getFlowIntervals()[VariablePool::getInstance().carlVarByIndex(i)].upper();
			}
		}
		flowVertices.emplace_back(std::move(vertex));
	}
	return flowVertices;
	}
};

}  // namespace hypro

namespace std {

template <typename Number>
struct hash<hypro::rectangularFlow<Number>> {
	std::size_t operator()( const hypro::rectangularFlow<Number>& f ) const {
		std::size_t seed = 0;
		for ( const auto& i : f.getFlowIntervals() ) {
			carl::hash_add( seed, std::hash<carl::Variable>()( i.first ) );
			carl::hash_add( seed, std::hash<carl::Interval<Number>>()( i.second ) );
		}
		return seed;
	}
};

}  // namespace std
