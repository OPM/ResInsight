#ifndef OPM_ICD_CONVERT_HPP
#define OPM_ICD_CONVERT_HPP

namespace Opm {

template<typename T>
T from_int(int int_status);

template<typename T>
int to_int(T status);

}

#endif
