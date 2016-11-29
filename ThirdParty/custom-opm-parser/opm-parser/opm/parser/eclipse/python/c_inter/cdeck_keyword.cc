#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

extern "C" {

    Opm::DeckKeyword *     deck_keyword_alloc( const char * name );
    void                   deck_keyword_free( Opm::DeckKeyword * keyword );
    const char *           deck_keyword_get_name( const Opm::DeckKeyword * keyword );
    int                    deck_keyword_get_size( const Opm::DeckKeyword * keyword );
    const Opm::DeckRecord* deck_keyword_iget_record( const Opm::DeckKeyword * keyword , int index);

    /*-----------------------------------------------------------------*/

    Opm::DeckKeyword * deck_keyword_alloc( const char * name ) {
        auto keyword = new Opm::DeckKeyword( name );
        return keyword;
    }


    void deck_keyword_free( Opm::DeckKeyword * keyword ) {
        delete keyword;
    }


    const char * deck_keyword_get_name( const Opm::DeckKeyword * keyword ) {
        const std::string& std_string = keyword->name();
        return std_string.c_str();
    }


    int deck_keyword_get_size( const Opm::DeckKeyword * keyword ) {
        return static_cast<int>(keyword->size());
    }


    const Opm::DeckRecord* deck_keyword_iget_record( const Opm::DeckKeyword * keyword , int index) {
        return &keyword->getRecord( static_cast<int>(index) );
    }

}


