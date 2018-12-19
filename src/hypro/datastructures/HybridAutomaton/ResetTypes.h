#pragma once
#include "../../types.h"
#include <carl/interval/Interval.h>
#include <vector>

namespace hypro {

    enum class ResetType{ affine=0, interval, none };

    template<typename Number>
    struct AffineTransformation {
        ConstraintSetT<Number> mTransformation;

        static ResetType type() { return ResetType::affine; }

        std::size_t size() {return mTransformation.size();}

        bool isIdentity() const {
            if( mTransformation.matrix() != matrix_t<Number>::Identity(mTransformation.matrix().rows(),mTransformation.matrix().rows())
				|| mTransformation.vector() != vector_t<Number>::Zero(mTransformation.vector().rows()) ) {
					return false;
            }
            return true;
        }

        friend std::ostream& operator<<(std::ostream& out, const AffineTransformation<Number>& in) {
            out << in.mTransformation;
            return out;
        }

        friend bool operator==(const AffineTransformation<Number>& lhs, const AffineTransformation<Number>& rhs) {
            return lhs.mTransformation == rhs.mTransformation;
        }
    };

    template<typename Number>
    struct IntervalAssignment {
        std::vector<carl::Interval<Number>> mIntervals;

        static ResetType type() { return ResetType::interval; }

        std::size_t size() {return mIntervals.size();}

        bool isIdentity() const {
            return mIntervals.empty();
        }

        friend std::ostream& operator<<(std::ostream& out, const IntervalAssignment<Number>& in) {
            for(const auto& i : in.mIntervals) {
                out << i << ", ";
            }
            return out;
        }

        friend bool operator==(const IntervalAssignment<Number>& lhs, const IntervalAssignment<Number>& rhs) {
            bool res = true;
            if(lhs.mIntervals.size() != rhs.mIntervals.size()) return false;

            for(std::size_t i = 0; i < lhs.mIntervals.size(); ++i) {
                if(lhs.mIntervals[i] != rhs.mIntervals[i]) return false;
            }
            return true;
        }
    };

    struct NoneAssignment {

        std::size_t mDimensions = 1;

        static ResetType type() { return ResetType::none; }

        std::size_t size() const {return mDimensions;}

        bool isIdentity() const {
            return false;
        }

        friend std::ostream& operator<<(std::ostream& out, const NoneAssignment& in) {
            out << "NONE " << in.mDimensions;
            return out;
        }

        friend bool operator==(const NoneAssignment& lhs, const NoneAssignment& rhs) {
            return lhs.size() == rhs.size();
        }
    };

    namespace detail {

        template<typename T>
        auto getType(const T&) {
            return T::type();
        }

    } // detail

} // hypro

namespace std {
    template<typename Number>
    struct hash<hypro::AffineTransformation<Number>>{
        std::size_t operator()(const hypro::AffineTransformation<Number>& reset) const {
            std::size_t seed = 0;
            carl::hash_add(seed, reset.mTransformation.matrix());
            carl::hash_add(seed, reset.mTransformation.vector());
            return seed;
        }
    };

    template<typename Number>
    struct hash<hypro::IntervalAssignment<Number>>{
        std::size_t operator()(const hypro::IntervalAssignment<Number>& reset) const {
            std::size_t seed = 0;
            carl::hash_add(seed, reset.mIntervals);
            return seed;
        }
    };

    template<>
    struct hash<hypro::NoneAssignment>{
        std::size_t operator()(const hypro::NoneAssignment& reset) const {
            std::size_t seed = 0;
            carl::hash_add(seed, reset.mDimensions);
            return seed;
        }
    };
} // std
