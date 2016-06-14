#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SimpleTable.hpp>

extern "C" {

    bool table_has_column( const Opm::SimpleTable * table , const char * column ) {
        return table->hasColumn( column );
    }


    int table_get_num_rows( const Opm::SimpleTable * table ) {
        return table->numRows();
    }


    double table_get_value( const Opm::SimpleTable * table , const char * column , int row_index) {
        return table->get( column , row_index );
    }


    double table_evaluate( const Opm::SimpleTable * table , const char * column , int row_index) {
        return table->evaluate( column , row_index );
    }


    double table_evaluate_from_index( const Opm::SimpleTable * table , const char * column , const Opm::TableIndex * eval_index) {
        const auto& valueColumn = table->getColumn( column );
        return valueColumn.eval( *eval_index );
    }


    Opm::TableIndex * table_lookup( const Opm::SimpleTable * table , double arg_value ) {
        const auto& argColumn = table->getColumn( 0 );
        Opm::TableIndex index = argColumn.lookup( arg_value );
        return new Opm::TableIndex( index );
    }
}
