#pragma once
#include "../../representations/GeometricObject.h"
#include "../../representations/types.h"
#include <iostream>
#include <cstdlib>

namespace hypro {

template<typename Number>
class Condition {
private:
	std::vector<ConstraintSet<Number>> mConstraints;
public:
	Condition() = default;
	Condition(const matrix_t<Number>& mat, const vector_t<Number>& vec) : mConstraints( {ConstraintSet<Number>(mat,vec)} ) {}
	Condition(const std::vector<boost::variant<ConstraintSet<Number>>>& sets);
	Condition(const Condition& orig) = default;
	Condition(Condition&& orig) = default;
	Condition& operator=(const Condition& orig) = default;
	Condition& operator=(Condition&& orig) = default;
	~Condition() {}

	// Access
	std::size_t size() const { return mConstraints.size(); }
	bool empty() const { return mConstraints.empty(); }

	const matrix_t<Number>& getMatrix(std::size_t I = 0) const { assert(mConstraints.size()>I); return mConstraints.at(I).matrix(); }
	const vector_t<Number>& getVector(std::size_t I = 0) const { assert(mConstraints.size()>I); return mConstraints.at(I).vector(); }

	void setMatrix(const matrix_t<Number>& m, std::size_t I = 0);
	void setVector(const vector_t<Number>& v, std::size_t I = 0);

	const std::vector<ConstraintSet<Number>>& constraints() const { return mConstraints; }

	std::string getDotRepresentation(const std::vector<std::string>& vars) const;

	// helper methods
	//template<typename Representation, typename ...Rargs>
	//std::pair<bool,State<Number,Representation, Rargs...>> isSatisfiedBy(const State<Number,Representation, Rargs...>& inState) const;

	friend bool operator==(const Condition& lhs, const Condition& rhs) {
		if (lhs.size() != rhs.size()) {
			return false;
		}

		for(std::size_t i = 0; i < lhs.size(); ++i) {
			if ((lhs.getVector(i) != rhs.getVector(i)) || (lhs.getMatrix(i) != rhs.getMatrix(i))) {
				return false;
			}
		}
		return true;
	}

	friend bool operator!=(const Condition& lhs, const Condition& rhs) {
		return !(lhs == rhs);
	}


#ifdef HYPRO_LOGGING
	friend std::ostream& operator<<(std::ostream& out, const Condition& in) {
		std::size_t i = 0;
		for(const auto& pair : in.constraints()) {
			out << "Constraint " << i << ": " << pair.matrix() << " constants: " << pair.vector() << std::endl;
			++i;
		}
#else
	friend std::ostream& operator<<(std::ostream& out, const Condition& ) {
#endif
		return out;
	}
};

template<typename Number>
Condition<Number> combine(
	const Condition<Number>& lhs, const Condition<Number>& rhs,
    const std::vector<std::string> haVar, const std::vector<std::string> lhsVar, const std::vector<std::string> rhsVar);

} // namespace

#include "Condition.tpp"
