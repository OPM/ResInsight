#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/InputErrorAction.hpp>

extern "C" {

    Opm::ParseContext * parse_context_alloc();
    void                parse_context_free( Opm::ParseContext * parse_context ) ;
    void                parse_context_update( Opm::ParseContext * parse_context , const char * var , Opm::InputError::Action action);

    /*-----------------------------------------------------------------*/

    Opm::ParseContext * parse_context_alloc() {
        Opm::ParseContext * parse_context = new Opm::ParseContext( );
        return parse_context;
    }


    void parse_context_free( Opm::ParseContext * parse_context ) {
        delete parse_context;
    }


    void parse_context_update( Opm::ParseContext * parse_context , const char * var , Opm::InputError::Action action) {
        parse_context->update( var , action );
    }

}
