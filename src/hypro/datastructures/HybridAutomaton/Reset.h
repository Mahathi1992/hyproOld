#pragma once
#include "State.h"
#include "Visitors.h"
#include "../../types.h"

namespace hypro {

template<typename Number>
class Reset {
protected:
	std::vector<ConstraintSet<Number>> mResets;

public:
	Reset() = default;
	Reset(const Reset& orig) = default;
	Reset(Reset&& orig) = default;
	Reset& operator=(const Reset<Number>& orig) = default;
	Reset& operator=(Reset<Number>&& orig) = default;
	~Reset() {}

	bool empty() const { return mResets.empty(); }
	std::size_t size() const { return mResets.size(); }

	vector_t<Number> getVector(std::size_t I = 0) const { return mResets.at(I).vector(); }
	matrix_t<Number> getMatrix(std::size_t I = 0) const { return mResets.at(I).matrix(); }
	matrix_t<Number>& rGetMatrix(std::size_t I = 0) const { return mResets[I].rMatrix(); }
	vector_t<Number>& rGetVector(std::size_t I = 0) const { return mResets[I].rVector(); }

	ConstraintSet<Number> getReset(std::size_t I = 0) const { return mResets.at(I); }
	ConstraintSet<Number>& rGetReset(std::size_t I = 0) const { return mResets[I]; }
	const std::vector<ConstraintSet<Number>>& getResetTransformations() const { return mResets; }

	void setVector(const vector_t<Number>& in, std::size_t I = 0);
	void setMatrix(const matrix_t<Number>& in, std::size_t I = 0);

	template<typename Representation, typename ...Rargs>
	State<Number,Representation,Rargs...> applyReset(const State<Number,Representation,Rargs...>& inState) const;

     /**
    * decomposes reset
    */
    void decompose(std::vector<std::vector<size_t>> decomposition);

#ifdef HYPRO_LOGGING
    friend std::ostream& operator<<(std::ostream& ostr, const Reset<Number>& a)
#else
    friend std::ostream& operator<<(std::ostream& ostr, const Reset<Number>&)
#endif
    {
#ifdef HYPRO_LOGGING
    	ostr << "Resets: " << std::endl;
    	for(const auto& r : a.getResetTransformations()) {
    		ostr << r;
    	}
#endif
        return ostr;
    }

    friend bool operator==(const Reset<Number>& lhs, const Reset<Number>& rhs) {
    	if(lhs.size() != rhs.size()) {
    		return false;
    	}

    	for(std::size_t i = 0; i < lhs.size(); ++i) {
    		if(lhs.getReset(i) != rhs.getReset(i)) {
    			return false;
    		}
    	}
    	return true;
    }

    friend bool operator!=(const Reset<Number>& lhs, const Reset<Number>& rhs) {
    	return !(lhs == rhs);
    }
};

} // namespace

#include "Reset.tpp"
