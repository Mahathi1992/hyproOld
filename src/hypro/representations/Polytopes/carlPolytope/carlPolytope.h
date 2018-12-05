#pragma once

#include "carlPolytopeSettings.h"
#include "util.h"
#include "../../../datastructures/Halfspace.h"
#include "../../../algorithms/quantifierElimination/qe.h"
#include "../../../carlTypes.h"
#include "../../../util/VariablePool.h"
#include "../../../util/logging/Logger.h"
#include "../../../util/linearOptimization/Optimizer.h"

namespace hypro {

template<typename Number, typename Converter, typename Settings>
class CarlPolytopeT {
public:
    // typedefs

private:
    FormulaT<Number> mFormula; /// The description of the polytope as a conjunction of linear constraints.
    mutable std::vector<Halfspace<Number>> mHalfspaces; /// Caches transformed half-spaces.
    mutable std::size_t mDimension = 0; /// Stores the state space dimension the polytope resides in.
    mutable TRIBOOL mEmpty = TRIBOOL::NSET; /// Caches whether the polytope is empty.
public:

    CarlPolytopeT()
        : mFormula()
        , mHalfspaces()
    {}

    CarlPolytopeT(const FormulaT<Number>& formula, std::size_t dimension = 0)
        : mFormula(formula)
        , mHalfspaces()
        , mDimension(dimension)
    {
        assert(formula.isRealConstraintConjunction());
        if(dimension==0) {
            detectDimension();
        }
        TRACE("hypro.representations.carlPolytope","Constructed P from " << mFormula);
    }

    CarlPolytopeT(const matrix_t<Number>& constraints, const vector_t<Number>& constants);

    CarlPolytopeT(const std::vector<carl::Interval<Number>>& intervals);

    static CarlPolytopeT<Number,Converter,Settings> Empty();

    CarlPolytopeT<Number,Converter,Settings> intersect(const CarlPolytopeT<Number,Converter,Settings>& rhs) const;

    std::size_t dimension() const { return mDimension; }
    const FormulaT<Number>& getFormula() const { return mFormula; }
    const std::vector<Halfspace<Number>>& getHalfspaces() const;

    void setDimension(std::size_t d) { mDimension = d; }
    void addConstraint(const ConstraintT<Number>& constraint);
    void addConstraints(const std::vector<ConstraintT<Number>>& constraints);
    void substituteVariable(carl::Variable oldVar, carl::Variable newVar);

    void eliminateVariable(carl::Variable var);
    void eliminateVariables(const std::vector<carl::Variable>& vars);

    std::vector<Point<Number>> vertices() const;
    matrix_t<Number> matrix() const;
    vector_t<Number> vector() const;
    std::size_t size() const { return mFormula.size(); }
    bool empty() const;

    void reduceRepresentation();

    friend std::ostream& operator<<(std::ostream& out, const CarlPolytopeT<Number,Converter,Settings>& in ) {
        out << in.getFormula();
        return out;
    }

private:
    void clearCache() const;
    void detectDimension() const ;

    /**
     * @brief Should only be used in assertions to detect inconsistency in mDimension.
     */
    bool dimensionWasCorrectlySet() const;
};

} // hypro

#include "carlPolytope.tpp"
