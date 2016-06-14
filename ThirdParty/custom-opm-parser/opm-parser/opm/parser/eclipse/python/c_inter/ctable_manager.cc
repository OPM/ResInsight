#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableContainer.hpp>

extern "C" {

    Opm::TableManager * table_manager_alloc( const Opm::Deck * deck ) {
        return new Opm::TableManager( *deck );
    }


    void table_manager_free( Opm::TableManager * table_manager ) {
        delete table_manager;
    }


    bool table_manager_has_table( const Opm::TableManager * table_manager , const char * table_name) {
        return table_manager->hasTables( table_name );
    }

    int table_manager_num_tables( const Opm::TableManager * table_manager, const char * table_name) {
        if (table_manager->hasTables( table_name )) {
            const auto& container = table_manager->getTables( table_name );
            return container.size();
        } else
            return 0;
    }


    const Opm::SimpleTable* table_manager_get_table( const Opm::TableManager * table_manager, const char * table_name, int table_num) {
        const auto& container = table_manager->getTables( table_name );
        const auto& table = container.getTable( table_num );

        return &table;
    }

}
