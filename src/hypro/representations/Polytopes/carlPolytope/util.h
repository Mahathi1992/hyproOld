#pragma once
#include "../../../carlTypes.h"
#include "../../../datastructures/Halfspace.h"
#include "../../../util/VariablePool.h"

namespace hypro {

    template<typename N, typename D>
    inline FormulasT<N> halfspacesToConstraints(const matrix_t<D>& constraints, const vector_t<D>& constants) {
        FormulasT<N> newConstraints;

        for(Eigen::Index row = 0; row < constraints.rows(); ++row) {
            PolyT<N> p;
            for(Eigen::Index col = 0; col < constraints.cols(); ++col) {
                p += carl::convert<D,N>(constraints(row,col))*PolyT<N>(VariablePool::getInstance().carlVarByIndex(col));
            }
            p -= carl::convert<D,N>(constants(row));
            newConstraints.emplace_back(FormulaT<N>(ConstraintT<N>(p, carl::Relation::LEQ)));
        }
        return newConstraints;
    }

    template<typename N>
    inline FormulasT<N> constraintsToFormulas(const std::vector<ConstraintT<N>>& in) {
        FormulasT<N> res;
        std::for_each(in.begin(),in.end(), [&](auto in){res.emplace_back(FormulaT<N>(in));});
        return res;
    }

    template<typename N, typename D>
    FormulasT<N> intervalToFormulas(const carl::Interval<D>& interval, std::size_t variableIndex) {
        TRACE("hypro.representations.carlPolytope","Create interval constraints for variable index " << variableIndex);
        FormulasT<N> res;
        res.emplace_back(ConstraintT<N>(PolyT<N>(VariablePool::getInstance().carlVarByIndex(variableIndex)) - PolyT<N>(carl::convert<D,N>(interval.lower())), carl::Relation::GEQ));
        res.emplace_back(ConstraintT<N>(PolyT<N>(VariablePool::getInstance().carlVarByIndex(variableIndex)) - PolyT<N>(carl::convert<D,N>(interval.upper())), carl::Relation::LEQ));
        return res;
    }

    template<typename N, typename D>
    vector_t<D> constraintNormal(const ConstraintT<N>& c, std::size_t dim) {
        vector_t<D> normal = vector_t<D>::Zero(dim);
        for(const auto& var : c.variables()) {
            assert(VariablePool::getInstance().hasDimension(var));
            assert(c.lhs().isLinear());
            assert(c.lhs().coeff(var,1).isNumber());
            if(c.relation() == carl::Relation::LEQ || c.relation() == carl::Relation::LESS || c.relation() == carl::Relation::EQ) {
                normal(VariablePool::getInstance().id(var)) = carl::convert<N,D>(c.lhs().coeff(var,1).constantPart());
            } else {
                normal(VariablePool::getInstance().id(var)) = -carl::convert<N,D>(c.lhs().coeff(var,1).constantPart());
            }
        }
        return normal;
    }

    template<typename N, typename D>
    D normalizedOffset(const ConstraintT<N>& c) {
        if(c.relation() == carl::Relation::LEQ || c.relation() == carl::Relation::LESS || c.relation() == carl::Relation::EQ) {
            return -carl::convert<N,D>(c.lhs().constantPart());
        } else {
            return carl::convert<N,D>(c.lhs().constantPart());
        }
    }

    template<typename N, typename D>
    std::vector<Halfspace<D>> constraintToHalfspace(const ConstraintT<N> constraint, std::size_t dim) {
        TRACE("hypro.representations.carlPolytope","Compute half-spaces from " << constraint << " with dimension " << dim);
        std::vector<Halfspace<D>> res;

        D offset = normalizedOffset<N,D>(constraint);
        vector_t<D> normalV = constraintNormal<N,D>(constraint,dim);

        res.emplace_back(Halfspace<D>{normalV,offset});
        TRACE("hypro.representations.carlPolytope","Hsp: " << res.back());

        if(constraint.relation() == carl::Relation::EQ) {
            res.emplace_back(Halfspace<D>{-normalV, -offset});
            TRACE("hypro.representations.carlPolytope","Hsp: " << res.back());
        }

        return res;
    }

    template<typename N, typename D>
    std::vector<Halfspace<D>> computeHalfspaces(const FormulaT<N>& formula, std::size_t dim) {
        assert(formula.isConstraintConjunction());

        std::vector<Halfspace<D>> res;
        std::vector<ConstraintT<N>> constraints;
        formula.getConstraints(constraints);

        for(const auto& c : constraints) {
            auto tmp = constraintToHalfspace<N,D>(c, dim);
            res.insert(res.end(), tmp.begin(), tmp.end());
        }

        return res;
    }

} // hypro
