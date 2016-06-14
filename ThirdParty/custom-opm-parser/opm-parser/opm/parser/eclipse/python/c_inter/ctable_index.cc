#include <opm/parser/eclipse/EclipseState/Tables/TableIndex.hpp>

extern "C" {

    void table_index_free( Opm::TableIndex * table_index ) {
        delete table_index;
    }


}
