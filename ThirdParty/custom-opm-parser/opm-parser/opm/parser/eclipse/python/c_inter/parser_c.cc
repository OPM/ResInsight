#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>


extern "C" {

Opm::Deck * parser_parse_file(const Opm::Parser * parser , const char * file) {
    return parser->newDeckFromFile( file , true );
}


void * parser_alloc() {
    Opm::Parser * parser = new Opm::Parser( true );
    return parser;
}


void parser_free(Opm::Parser * parser) {
    delete parser;
}

}
