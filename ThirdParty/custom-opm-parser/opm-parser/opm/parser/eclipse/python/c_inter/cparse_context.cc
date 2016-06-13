#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/InputErrorAction.hpp>

extern "C" {


    Opm::ParseContext * parse_mode_alloc() {
        Opm::ParseContext * parse_mode = new Opm::ParseContext( );
        return parse_mode;
    }


    void parse_mode_free( Opm::ParseContext * parse_mode ) {
        delete parse_mode;
    }


    void parse_mode_update( Opm::ParseContext * parse_mode , const char * var , Opm::InputError::Action action) {
        parse_mode->update( var , action );
    }

}
