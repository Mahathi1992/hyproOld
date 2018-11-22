#include "carlPolytope.h"

namespace hypro {

    template<typename Number, typename Converter, typename Settings>
    CarlPolytopeT<Number,Converter,Settings>::CarlPolytopeT(const matrix_t<Number>& constraints, const vector_t<Number>& constants) {
        TRACE("hypro.representations.carlPolytope","Construct P from " << constraints << " and " << constants);
        FormulasT<Number> newConstraints;

        for(Eigen::Index row = 0; row < constraints.rows(); ++row) {
            PolyT<Number> p;
            for(Eigen::Index col = 0; col < constraints.cols(); ++col) {
                p += constraints(row,col)*PolyT<Number>(VariablePool::getInstance().carlVarByIndex(col));
            }
            p -= constants(row);
            newConstraints.emplace_back(FormulaT<Number>(ConstraintT<Number>(p, carl::Relation::LEQ)));
        }

        mFormula = FormulaT<Number>{carl::FormulaType::AND, newConstraints};
        TRACE("hypro.representations.carlPolytope","Result formula: " << mFormula);
        detectDimension();
    }

    template<typename Number, typename Converter, typename Settings>
    CarlPolytopeT<Number,Converter,Settings>::CarlPolytopeT(const std::vector<carl::Interval<Number>>& intervals) {
        TRACE("hypro.representations.carlPolytope","Construct P from interval vector.");
        FormulasT<Number> newConstraints;

        for(std::size_t i = 0; i < intervals.size(); ++i) {
            auto tmp = intervalToFormulas(intervals[i], i);
            newConstraints.insert(newConstraints.end(),tmp.begin(),tmp.end());
        }
        mFormula = FormulaT<Number>{carl::FormulaType::AND, newConstraints};
        TRACE("hypro.representations.carlPolytope","Result formula: " << mFormula);
        detectDimension();
    }

    template<typename Number, typename Converter, typename Settings>
    CarlPolytopeT<Number,Converter,Settings> CarlPolytopeT<Number,Converter,Settings>::intersect(const CarlPolytopeT<Number,Converter,Settings>& rhs) const {
        TRACE("hypro.representations.carlPolytope","Intersect " << *this << " and " << rhs);
        assert(mFormula.isConstraintConjunction());
        assert(rhs.getFormula().isConstraintConjunction());
        // collect constraints
        std::vector<ConstraintT<Number>> newConstraints;
        mFormula.getConstraints(newConstraints);
        rhs.getFormula().getConstraints(newConstraints);

        return CarlPolytopeT<Number,Converter,Settings>{FormulaT<Number>(carl::FormulaType::AND, constraintsToFormulas(newConstraints))};
    }

    template<typename Number, typename Converter, typename Settings>
    const std::vector<Halfspace<Number>>& CarlPolytopeT<Number,Converter,Settings>::getHalfspaces() const {
        if(mHalfspaces.empty()) {
            mHalfspaces = computeHalfspaces(mFormula, this->dimension());
        }
        TRACE("hypro.representations.carlPolytope","Computed halfspaces..");
        return mHalfspaces;
    }

    template<typename Number, typename Converter, typename Settings>
    void CarlPolytopeT<Number,Converter,Settings>::addConstraint(const ConstraintT<Number>& constraint) {
        // reset Half-space cache
        mHalfspaces.clear();
        // add constraint to formula
        std::vector<ConstraintT<Number>> constraints;
        mFormula.getConstraints(constraints);
        constraints.push_back(constraint);
        mFormula = FormulaT<Number>(carl::FormulaType::AND, constraintsToFormulas(constraints));
        detectDimension();
    }

    template<typename Number, typename Converter, typename Settings>
    void CarlPolytopeT<Number,Converter,Settings>::addConstraints(const std::vector<ConstraintT<Number>>& constraints) {
        // reset Half-space cache
        mHalfspaces.clear();
        // add constraints to formula
        auto cCopy = constraints;
        mFormula.getConstraints(cCopy);
        mFormula = FormulaT<Number>(carl::FormulaType::AND, cCopy);
        detectDimension();
    }

    template<typename Number, typename Converter, typename Settings>
    std::vector<Point<Number>> CarlPolytopeT<Number,Converter,Settings>::vertices() const {
        auto hpoly = Converter::toHPolytope(*this);
        return hpoly.vertices();
    }

    template<typename Number, typename Converter, typename Settings>
    void CarlPolytopeT<Number,Converter,Settings>::detectDimension() {
        std::size_t d = 0;
        // get maximal state space dimension based on the variables used in mFormula.
        std::for_each(mFormula.variables().begin(),mFormula.variables().end(), [&](const carl::Variable& v){ d = std::max(d,VariablePool::getInstance().id(v)); std::cout << "Var: " << v << " dimension " << VariablePool::getInstance().id(v) << std::endl;});
        mDimension = d+1; // add one as we start counting from zero.
        TRACE("hypro.representations.carlPolytope","Set dimension to " << mDimension );
    }

} // hypro