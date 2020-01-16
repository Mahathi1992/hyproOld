//
// Created by leander.behr on 11/11/19.
//

#include "Facet.h"

namespace hypro {
    //modifiers
    template<typename Number, bool Euclidian>
    void FloatQuickhull<Number, Euclidian>::Facet::invert() {
        mNormal *= -1;
        mOffset *= -1;
    }

    template<typename Number, bool Euclidian>
    void FloatQuickhull<Number, Euclidian>::Facet::setOrientation(point_t const& containedPoint, Facet const& adjacentFacet) {
        //Set orientation of normal
        Number distance = this->distance(containedPoint);
        
        if(distance > 0) {
            invert();
        } else if(distance == 0) {
            mNormal = adjacentFacet.mNormal;
            mOffset = adjacentFacet.mOffset;
        }
    }

    //queries
    template<typename Number, bool Euclidian>
    template<int RoundingMode>
    Number FloatQuickhull<Number, Euclidian>::Facet::distance(point_t const& point) const {
        ScopedRoundingMode rounding{RoundingMode};

        if constexpr(Euclidian) { 
            return point.dot(mNormal) - mOffset;
        } else {
            Number dotProd = point.head(point.rows() - 1).dot(mNormal) + point[point.rows() - 1] * -mOffset;
            return dotProd;
        }
    }

    template<typename Number, bool Euclidian>
    bool FloatQuickhull<Number, Euclidian>::Facet::visible(point_t const& vertex) const {
        return distance(vertex) > Number(0);
    }

    template<typename Number, bool Euclidian>
    size_t FloatQuickhull<Number, Euclidian>::Facet::findNeighborIndex(facet_ind_t facet_i) {
        for(size_t neighbor_pos = 0; neighbor_pos < mNeighbors.size(); ++neighbor_pos) {
            if(mNeighbors[neighbor_pos] == facet_i) return neighbor_pos;
        }
        assert(false);
    }

}
