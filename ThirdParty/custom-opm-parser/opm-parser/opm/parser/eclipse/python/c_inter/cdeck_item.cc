#include <opm/parser/eclipse/Deck/DeckItem.hpp>

extern "C" {

    int deck_item_get_size( const Opm::DeckItem * item ) {
        return static_cast<int>(item->size());
    }

    /*
      These types must be *manually* syncronized with the values in
      the Python module: opm/deck/item_type_enum.py
    */

    int deck_item_get_type( const Opm::DeckItem * item ) {
        return item->typeof();
    }

    int deck_item_iget_int( const Opm::DeckItem * item , int index) {
        return item->get< int >(static_cast<size_t>(index));
    }

    double deck_item_iget_double( const Opm::DeckItem * item , int index) {
        return item->get< double >(static_cast<size_t>(index));
    }

    const char * deck_item_iget_string( const Opm::DeckItem * item , int index) {
        const std::string& string = item->get< std::string >(static_cast<size_t>(index));
        return string.c_str();
    }
}


