#ifndef LAYER_CXX_HPP
#define LAYER_CXX_HPP

#include <vector>
#include <ert/ecl/layer.hpp>

bool     layer_trace_block_edge( const layer_type * layer , int i , int j , int value , std::vector<int_point2d_type>& corner_list, int_vector_type * cell_list);

#endif
