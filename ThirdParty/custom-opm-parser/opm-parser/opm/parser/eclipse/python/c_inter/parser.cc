#include <opm/parser/eclipse/Deck/Deck.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>

/*
#include <c_inter.h>
#ifdef __cplusplus
extern "C" {
#endif

void * parser_parse_file(const void * parser , const char * file);
void * parser_alloc();

#ifdef __cplusplus
}
#endif
*/

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
