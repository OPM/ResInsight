#include <ert/ecl/EclKW.hpp>


namespace ERT {
    template <>
    EclKW<int>::EclKW(const std::string& kw, int size_) {
        ecl_kw_type * c_ptr = ecl_kw_alloc( kw.c_str() , size_ , ECL_INT_TYPE );
        reset( c_ptr );
    }

    template <>
    EclKW<float>::EclKW(const std::string& kw, int size_) {
        ecl_kw_type * c_ptr = ecl_kw_alloc( kw.c_str() , size_ , ECL_FLOAT_TYPE );
        reset( c_ptr );
    }

    template <>
    EclKW<double>::EclKW(const std::string& kw, int size_) {
        ecl_kw_type * c_ptr = ecl_kw_alloc( kw.c_str() , size_ , ECL_DOUBLE_TYPE );
        reset( c_ptr );
    }



    template <>
    EclKW<double> EclKW<double>::load(FortIO& fortio) {
        return checkedLoad(fortio , ECL_DOUBLE_TYPE);
    }

    template <>
    EclKW<int> EclKW<int>::load(FortIO& fortio) {
        return checkedLoad(fortio , ECL_INT_TYPE);
    }

    template <>
    EclKW<float> EclKW<float>::load(FortIO& fortio) {
        return checkedLoad(fortio , ECL_FLOAT_TYPE);
    }
}
