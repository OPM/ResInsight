#include <opm/parser/eclipse/Deck/DeckRecord.hpp>

extern "C" {
    int             deck_record_get_size( const Opm::DeckRecord * record );
    bool            deck_record_has_item(const Opm::DeckRecord * record , const char * item);
    Opm::DeckItem * deck_record_iget_item( Opm::DeckRecord * record , int index);
    Opm::DeckItem * deck_record_get_item( Opm::DeckRecord * record , const char * name);

    /*-----------------------------------------------------------------*/

    int deck_record_get_size( const Opm::DeckRecord * record ) {
        return static_cast<int>(record->size());
    }

    bool deck_record_has_item(const Opm::DeckRecord * record , const char * item) {
        return record->hasItem( item );
    }

    Opm::DeckItem * deck_record_iget_item( Opm::DeckRecord * record , int index) {
        return &record->getItem( static_cast<size_t>(index));
    }

    Opm::DeckItem * deck_record_get_item( Opm::DeckRecord * record , const char * name) {
        return &record->getItem( name );
    }

}


