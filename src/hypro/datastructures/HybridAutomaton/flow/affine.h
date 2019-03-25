#pragma once
#include "linear.h"
#include "../../../types.h"

namespace hypro {

template<typename Number>
class affineFlow : public linearFlow<Number> {
public:
protected:
    vector_t<Number> mTranslation;
    mutable TRIBOOL mIsNoTranslation = TRIBOOL::NSET;
public:
    affineFlow() = default;
    affineFlow(const affineFlow<Number>& in) = default;
    affineFlow(const matrix_t<Number>& A, const vector_t<Number>& b)
        : linearFlow<Number>(A)
        , mTranslation(b)
    {}

    virtual ~affineFlow() {}

    static DynamicType type() { return DynamicType::affine; }
    std::size_t dimension() const { return linearFlow<Number>::dimension(); }

    void setTranslation(const vector_t<Number>& newB) { mTranslation = newB; mIsNoTranslation = TRIBOOL::NSET; }
    const vector_t<Number>& getTranslation() const { return mTranslation; }

    bool hasTranslation() const {
        if(mIsNoTranslation == TRIBOOL::NSET) {
            if(mTranslation.rows() == 0) {
                mIsNoTranslation = TRIBOOL::FALSE;
                return false;
            } else {
                mIsNoTranslation = mTranslation == vector_t<Number>::Zero(mTranslation.rows()) ? TRIBOOL::TRUE : TRIBOOL::FALSE;
            }
        }
        return mIsNoTranslation == TRIBOOL::FALSE;
    }

    bool isTimed() const {
        TRACE("hypro.decisionEntity","Flowmatrix: " << linearFlow<Number>::getFlowMatrix());
        if(linearFlow<Number>::isTimed()){
            // TODO: we need to remove distinguishing of linear and affine flow.
            return true;
        }
        return linearFlow<Number>::hasNoFlow() && mTranslation == vector_t<Number>::Ones(mTranslation.rows());
    }

    bool isDiscrete() const {
        return linearFlow<Number>::hasNoFlow() && !hasTranslation();
    }

    friend bool operator==(const affineFlow<Number>& lhs, const affineFlow<Number>& rhs) {
        return (lhs.getFlowMatrix() == rhs.getFlowMatrix()
                && lhs.getTranslation() == rhs.getTranslation());
    }

    friend bool operator!=(const affineFlow<Number>& lhs, const affineFlow<Number>& rhs) {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& out, const affineFlow<Number>& in) {
        return out << in.getFlowMatrix() << ", " << in.getTranslation();
    }
};

} // hypro

namespace std {

    template<typename Number>
    struct hash<hypro::affineFlow<Number>> {
        std::size_t operator()(const hypro::affineFlow<Number>& f) const
        {
            std::size_t seed = 0;
            carl::hash_add(seed, std::hash<hypro::linearFlow<Number>>()(hypro::linearFlow<Number>(f)));
            carl::hash_add(seed, std::hash<hypro::vector_t<Number>>()(f.getTranslation()));
            return seed;
        }
    };

}  // namespace std
