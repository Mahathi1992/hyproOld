#include "DifferenceBounds.h"

namespace hypro {


    template <typename Number, typename Converter>
    DifferenceBoundsT<Number, Converter>::DifferenceBoundsT(){
        m_dbm = matrix_t<DBMEntry>(1,1);
        m_dbm << DBMEntry(0,DifferenceBoundsT<Number, Converter>::BOUND_TYPE::SMALLER);
        m_timeHorizon = 0.0;
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number, Converter>::DBMEntry::DBMEntry(Number number, BOUND_TYPE boundType) : std::pair<Number, BOUND_TYPE>(number, boundType) {
        //nop
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number, Converter>::DBMEntry::DBMEntry() : std::pair<Number, BOUND_TYPE>() {
        //nop
    }

    template <typename Number, typename Converter>
    matrix_t<typename DifferenceBoundsT<Number, Converter>::DBMEntry> DifferenceBoundsT<Number, Converter>::getDBM() const{
        return m_dbm;
    }
    template <typename Number, typename Converter>
    void DifferenceBoundsT<Number, Converter>::setDBM(matrix_t<DifferenceBoundsT<Number, Converter>::DBMEntry> dbm) {
        m_dbm = dbm;
    }

    template <typename Number, typename Converter>
    Number DifferenceBoundsT<Number, Converter>::getTimeHorizon() const{
        return m_timeHorizon;
    }

    template <typename Number, typename Converter>
    void DifferenceBoundsT<Number, Converter>::setTimeHorizon(Number horizon) {
        m_timeHorizon = horizon;
    }

    template <typename Number, typename Converter>
    std::size_t DifferenceBoundsT<Number, Converter>::dimension() const{
        // number of clocks
        return this->getDBM().cols();
    }

    template <typename Number, typename Converter>
    std::size_t DifferenceBoundsT<Number, Converter>::size() const{
        // number of dbm entries
        return this->getDBM().cols()*this->getDBM().rows();
    }

    template <typename Number, typename Converter>
    bool DifferenceBoundsT<Number, Converter>::empty() const{
        /*
         *  A DBM is empty if the following holds for a position i,j in a dbm matrix
         *
         *  Let i>j, i.e. d_ij defines an upper bound. Then its value must be larger
         *  or equal to the corresponding lower bound d_ji.
         *
         *  Consequently it suffices to traverse the lower half of the dbm matrix
         *  (i.e. where i is larger than j) to check emptiness.
         *
         *  Further, we can abuse the entry d_00 to keep track of an invalid dbm by
         *  assigning it -1.
         *
         *  This will for one avoid double checking emptiness and secondly if we
         *  recognize during transformation that the resulting dbm will be
         *  empty we can directly invalidate the dbm by setting d00 = -1 and thus
         *  save future emptiness checks.
         *
         */
        if(m_dbm(0,0).first == -1.0){
            // d_00 is -1 so we already found out that the dbm is empty
            return true;
        }
        // we assume that the dbm is non empty
        for(int i=0; i< m_dbm.rows();i++){
            // lower half (i > j)
            for(int j = 0; j < i; j++){
                // if the upper bound is lower than the corresponding lower bound we know the dbm is invalid
                if(m_dbm(i,j) <  m_dbm(j,i)){
                     m_dbm(0,0).first = -1.0;
                    return true;
                }

            }
        }
        return false;
    }

    template <typename Number, typename Converter>
    std::vector<Point<Number>> DifferenceBoundsT<Number, Converter>::vertices( const matrix_t<Number>& ) const {
        hypro::HPolytopeT<Number,Converter> poly = Converter::toHPolytope(*this);
        // TODO enhace plotting for inifinite polytopes
        if (getTimeHorizon() != 0.0) {
            // we need an additional timeHorizon constraint for each clock (except 0 clock)
            int numclocks = getDBM().cols()-1;
            //constraints of the polytope
            hypro::matrix_t<Number> HPolyConstraints = poly.matrix();
            hypro::vector_t<Number> HPolyConstants = poly.vector();
            //std::cout << "Old matrix: " << HPolyConstraints <<"\n";
            //std::cout << "Old constants: " << HPolyConstants <<"\n";
            int numconstraints = HPolyConstraints.rows();
            HPolyConstraints.conservativeResize(numconstraints+numclocks, HPolyConstraints.cols());
            HPolyConstants.conservativeResize(numconstraints+numclocks, HPolyConstraints.cols());

            int counter = numconstraints;//start at next row
            for (int i = 0; i < numclocks; i++) {
                // for each clock at a timehorizon constraint so the polytope to plot is finite
                matrix_t<Number> constraintVars = matrix_t<Number>::Zero(1, numclocks);
                constraintVars(0, i) = 1.0;
                //std::cout << "Constraint variable vector: " << constraintVars <<"\n";
                HPolyConstraints.row(counter) = constraintVars;
                //std::cout << "New constraint matrix: " << HPolyConstraints << "\n";
                HPolyConstants(counter, 0) = getTimeHorizon();
                //std::cout << "New constant vector: " << HPolyConstants << "\n";
                counter++;
            }
            hypro::HPolytopeT<Number,Converter> polyNew(HPolyConstraints,HPolyConstants);
            return polyNew.vertices();
        }
        else{
            std::cout << "Warning: TimeHorizon may not be configured correctly. Converting potentially unbounded polytope to vertices.";
        }
        return poly.vertices();
    }

    template <typename Number, typename Converter>
    std::pair<CONTAINMENT, DifferenceBoundsT<Number, Converter>> DifferenceBoundsT<Number, Converter>::satisfiesHalfspace( const Halfspace<Number>& rhs ) const{
        // TODO we need conversion polytopy-> dbm
        return std::make_pair(CONTAINMENT::FULL,DifferenceBoundsT<Number,Converter>());
    }

    template <typename Number, typename Converter>
    std::pair<CONTAINMENT, DifferenceBoundsT<Number, Converter>> DifferenceBoundsT<Number, Converter>::satisfiesHalfspaces( const matrix_t<Number>& _mat, const vector_t<Number>& _vec ) const{
        // TODO we need conversion polytopy-> dbm
        return std::make_pair(CONTAINMENT::FULL,DifferenceBoundsT<Number,Converter>());
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::project(const std::vector<std::size_t>& dimensions) const{
        // TODO i assume that the projection vector does not consider the 0 clock. Hence dimension will never contain 0, given that dimension contains a list of dimension to project to, i.e. {1,3,4}
        hypro::matrix_t<DBMEntry> mat =  hypro::matrix_t<DBMEntry>(m_dbm);
        for(int i=0; i < mat.rows(); i++){
            for(int j=0; j < mat.cols();j++){
                if(std::find(dimensions.begin(), dimensions.end(), i) == dimensions.end() || std::find(dimensions.begin(), dimensions.end(), j) == dimensions.end()){
                    // either i or j is not in the projection dimension, replace the current entry with (0, <=)
                    mat(i,j) = DBMEntry(0.0, BOUND_TYPE::SMALLER_EQ);
                }
            }
        }
        hypro::DifferenceBoundsT<Number,Converter> res = hypro::DifferenceBoundsT<Number,Converter>(*this);
        res.setDBM(mat);
        return res;
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::linearTransformation( const matrix_t<Number>& A ) const{
        // TODO we need backwards conversion hpoly to dbm for this
        return DifferenceBoundsT<Number,Converter>();
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::affineTransformation( const matrix_t<Number>& A, const vector_t<Number>& b ) const{
        // TODO we need backwards conversion hpoly to dbm for this
        return DifferenceBoundsT<Number,Converter>();
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::minkowskiSum( const DifferenceBoundsT<Number,Converter>& _rhs ) const{
        // TODO we need backwards conversion hpoly to dbm for this
        return DifferenceBoundsT<Number,Converter>();
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::intersect( const DifferenceBoundsT<Number,Converter>& _rhs ) const{
        hypro::DifferenceBoundsT<Number,Converter> retDBM = hypro::DifferenceBoundsT<Number,Converter>(*this);
        //std::cout << "Initial DBM:" << retDBM << "\n";
        // for each entry in the other dbm, intersect that entry with this dbm
        for(int i = 0; i < _rhs.getDBM().rows(); i++){
            for(int j = 0; j < _rhs.getDBM().cols();j++){
                if(i!=j) {
                    retDBM = retDBM.intersectConstraint(i,j,_rhs.getDBM()(i,j));
                    //std::cout << "DBM after intersection with ("<<i<<","<<j<<")="<<_rhs.getDBM()(i,j).first<<" yields: \n"<< retDBM << "\n";
                }
            }
        }
        //std::cout << "Final DBM:" << retDBM << "\n";
        return retDBM;
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::intersectHalfspace( const Halfspace<Number>& hs ) const{
        // TODO we need conversion polytopy-> dbm
        return DifferenceBoundsT<Number,Converter>();
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::intersectHalfspaces( const matrix_t<Number>& _mat, const vector_t<Number>& _vec ) const{
        // TODO we need conversion polytopy-> dbm
        return DifferenceBoundsT<Number,Converter>();
    }

    template <typename Number, typename Converter>
    bool DifferenceBoundsT<Number, Converter>::contains( const Point<Number>& _point ) const{
        // describe point as dbm
        vector_t<Number> coordinates = _point.rawCoordinates();
        hypro::matrix_t<DBMEntry> mat =  hypro::matrix_t<DBMEntry>(m_dbm.rows(),m_dbm.cols());
        for(int i=0; i < mat.rows(); i++){
            for(int j=0; j < mat.cols();j++){
                if(i==j){
                    // diagonals are zero
                    mat(i,j)= DBMEntry(0.0, BOUND_TYPE::SMALLER_EQ);
                }
                else if(i == 0){
                    // constraint of the form 0-x_j-1<=b <=> x_j-1 >=-b
                    mat(i,j) = DBMEntry(-1.0*coordinates(j-1), BOUND_TYPE::SMALLER_EQ);
                }
                else if(j == 0){
                    // constraint of the form x_i-1-0 <= b
                    mat(i,j) = DBMEntry(coordinates(i-1), BOUND_TYPE::SMALLER_EQ);
                }
                else{
                    // constaint of the form x_i-1-x_j-1<=b
                    mat(i,j) = DBMEntry(coordinates(i-1)-coordinates(j-1), BOUND_TYPE::SMALLER_EQ);
                }
            }
        }
        hypro::DifferenceBoundsT<Number,Converter> pDBM = hypro::DifferenceBoundsT<Number,Converter>(*this);
        pDBM.setDBM(mat);
        return this->contains(pDBM);
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::unite( const DifferenceBoundsT<Number,Converter>& _rhs ) const{
        hypro::matrix_t<DBMEntry> mat =  hypro::matrix_t<DBMEntry>(m_dbm);
        for(int i=0; i < mat.rows(); i++){
            for(int j=0; j < mat.cols();j++) {
                if(i!=j){
                    // replace the entry with the maximum of lhs(ij) and rhs(ij)
                    mat(i,j) = DBMEntry::max(mat(i,j), _rhs.getDBM()(i,j));
                }
            }
        }
        hypro::DifferenceBoundsT<Number,Converter> res = hypro::DifferenceBoundsT<Number,Converter>(*this);
        res.setDBM(mat);
        return res;
    }


    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::elapse() const{
        hypro::matrix_t<DBMEntry> mat =  hypro::matrix_t<DBMEntry>(m_dbm);
        for(int i = 1; i < m_dbm.rows(); i++){
            mat(i,0) = DBMEntry(0.0, BOUND_TYPE::INFTY);
        }
        hypro::DifferenceBoundsT<Number,Converter> res = hypro::DifferenceBoundsT<Number,Converter>(*this);
        res.setDBM(mat);
        return res;
    }


    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::rewind() const{
        hypro::matrix_t<DBMEntry> mat =  hypro::matrix_t<DBMEntry>(m_dbm);
        for(int i = 1; i < m_dbm.rows(); i++){
            mat(0,i) = DBMEntry(0.0, BOUND_TYPE::SMALLER_EQ);
            for (int j = 1; j < m_dbm.rows(); j++){
                // if the diagonal intersects the clock axis later than 0, it is tighter than the 0 constraint
                if(m_dbm(j,i) < m_dbm(0,i)){
                    mat(0,i) =  DBMEntry(m_dbm(j,i).first,m_dbm(j,i).second);
                }
            }
        }
        hypro::DifferenceBoundsT<Number,Converter> res = hypro::DifferenceBoundsT<Number,Converter>(*this);
        res.setDBM(mat);
        return res;
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::free(int x) const{
        hypro::matrix_t<DBMEntry> mat =  hypro::matrix_t<DBMEntry>(m_dbm);
        for(int i = 0; i < m_dbm.rows(); i++){
            if (i != x){
                // d_xi = inf
                mat(x,i) = DBMEntry(0.0, BOUND_TYPE::INFTY);

                // d_ix = d_i0
                mat(i,x) = DBMEntry(m_dbm(i,0).first, m_dbm(i,0).second);
            }
        }
        hypro::DifferenceBoundsT<Number,Converter> res = hypro::DifferenceBoundsT<Number,Converter>(*this);
        res.setDBM(mat);
        return res;
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::reset(int x, Number value) const{
        hypro::matrix_t<DBMEntry> mat =  hypro::matrix_t<DBMEntry>(m_dbm);
        for (int i = 0; i < m_dbm.rows();i++){
            // d_xi = (value,<=)+d_0i
            mat(x,i) = DBMEntry(value, BOUND_TYPE::SMALLER_EQ)+m_dbm(0,i);

            // d_ix = d_i0 + (-value, <=)
            mat(i,x) = m_dbm(i,0)+ DBMEntry(-value,BOUND_TYPE::SMALLER_EQ);
        }
        hypro::DifferenceBoundsT<Number,Converter> res = hypro::DifferenceBoundsT<Number,Converter>(*this);
        res.setDBM(mat);
        return res;
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::copy(int src, int dest) const{
        hypro::matrix_t<DBMEntry> mat =  hypro::matrix_t<DBMEntry>(m_dbm);
        for(int i = 0; i < m_dbm.rows(); i++){
            if (i != dest){
                // d_desti = d_srci
                mat(dest,i) = DBMEntry(m_dbm(src,i).first,m_dbm(src,i).second);
                // d_idest = d_isrc
                mat(i,dest) = DBMEntry(m_dbm(i,src).first,m_dbm(i,src).second);
            }
        }
        // d_destsrc = (0,<=)
        mat(dest,src)= DBMEntry(0.0,BOUND_TYPE::SMALLER_EQ);
        // d_srcdest = (0,<=)
        mat(src,dest) = DBMEntry(0.0,BOUND_TYPE::SMALLER_EQ);

        hypro::DifferenceBoundsT<Number,Converter> res = hypro::DifferenceBoundsT<Number,Converter>(*this);
        res.setDBM(mat);
        return res;
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::shift(int x, Number offset) const {
        hypro::matrix_t<DBMEntry> mat = hypro::matrix_t<DBMEntry>(m_dbm);

        for (int i = 0; i < m_dbm.rows(); i++) {
            if (i != x) {
                // d_xi = d_xi+ (offset, <=)
                mat(x, i) = m_dbm(x, i) + DBMEntry(offset, BOUND_TYPE::SMALLER_EQ);

                // d_ix = d_ix + (-offset, <=)
                mat(i, x) = m_dbm(i, x) + DBMEntry(-offset, BOUND_TYPE::SMALLER_EQ);
            }
        }
        //d_x0 = max(d_x0, (0, <=))
        mat(x, 0) = DBMEntry::max(mat(x,0), DBMEntry(0, BOUND_TYPE::SMALLER_EQ));
        //d_0x = min(d_0x, (0, <=))
        mat(0, x) = DBMEntry::min(mat(0,x), DBMEntry(0, BOUND_TYPE::SMALLER_EQ));

        hypro::DifferenceBoundsT<Number, Converter> res = hypro::DifferenceBoundsT<Number, Converter>(*this);
        res.setDBM(mat);
        return res;
    }

    template <typename Number, typename Converter>
    bool DifferenceBoundsT<Number, Converter>::contains( const DifferenceBoundsT<Number,Converter>& _rhs ) const{
        // we assert that both DBM constraint the same clocks. Note that a DBM that would describe a projection
        // to some subset of clocks can be given by setting the entries of the other clocks in the DBM to 0.
        // Hence a DBM should always be as large as the number of clocks in the automaton.
        assert(this->getDBM().cols() == _rhs.getDBM().cols());
        bool contains = true;
        for(int i=0; i < this->getDBM().rows();i++){
            for(int j=0; j < this->getDBM().cols();j++){
                if(i != j) {
                    contains = contains && (_rhs.getDBM()(i, j) <= this->getDBM()(i, j));
                }
            }
        }
        return contains;
    }

    template <typename Number, typename Converter>
    DifferenceBoundsT<Number,Converter> DifferenceBoundsT<Number, Converter>::intersectConstraint( const int x, const int y, const DBMEntry& bound ) const{
        hypro::matrix_t<DBMEntry> mat = hypro::matrix_t<DBMEntry>(m_dbm);

        // d_yx+bound < 0
        if(mat(y,x)+bound < DBMEntry(0.0, BOUND_TYPE::SMALLER_EQ)){
            //invalid dbm because upper bound became negative
            mat(0,0).first = -1.0;
        }
        // bound < d_xy
        else if (bound < mat(x,y)){
            mat(x,y) = bound;
            // cutting off parts of the dbm can yield to other constrainst not being tight anymore
            // we correct this because other operations depend on this assumption
            for(int i=0; i < mat.rows();i++){
                for(int j=0; j < mat.cols();j++){
                    if(mat(i,x)+mat(x,j) < mat(i,j)){
                        mat(i,j) = mat(i,x)+mat(x,j);
                    }
                    if(mat(i,y) + mat(y,j)< mat(i,j)){
                        mat(i,j) = mat(i,y)+mat(y,j);
                    }
                }
            }
        }

        hypro::DifferenceBoundsT<Number, Converter> res = hypro::DifferenceBoundsT<Number, Converter>(*this);
        res.setDBM(mat);
        return res;
    }

}

