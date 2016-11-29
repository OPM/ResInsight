#include <opm/json/JsonObject.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>


extern "C" {

    Opm::Deck * parser_parse_file(const Opm::Parser * parser , const char * file , const Opm::ParseContext * parse_mode);
    void *      parser_alloc();
    bool        parser_has_keyword(const Opm::Parser * parser , const char * keyword);
    void        parser_free(Opm::Parser * parser);
    void        parser_add_json_keyword(Opm::Parser * parser, const char * json_string);

    /*-----------------------------------------------------------------*/

    Opm::Deck * parser_parse_file(const Opm::Parser * parser , const char * file , const Opm::ParseContext * parse_mode) {
        return parser->newDeckFromFile( file , *parse_mode );
    }


    void * parser_alloc() {
        Opm::Parser * parser = new Opm::Parser( true );
        return parser;
    }

    bool parser_has_keyword(const Opm::Parser * parser , const char * keyword) {
        return parser->hasKeyword( keyword );
    }


    void parser_free(Opm::Parser * parser) {
        delete parser;
    }


    void parser_add_json_keyword(Opm::Parser * parser, const char * json_string) {
        Json::JsonObject json_object(json_string);
        parser->addParserKeyword( json_object );
    }

}
