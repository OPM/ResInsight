#include <iostream>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>

extern "C" {

    Opm::Schedule * schedule_alloc(Opm::ParseContext * context, Opm::EclipseGrid * grid, Opm::Deck * deck);
    void            schedule_free( Opm::Schedule * schedule );
    time_t          schedule_end( const Opm::Schedule * schedule );
    time_t          schedule_start( const Opm::Schedule * schedule );

    /*-----------------------------------------------------------------*/

    time_t schedule_end( const Opm::Schedule * schedule ) {
        return schedule->posixEndTime();
    }

    time_t schedule_start( const Opm::Schedule * schedule ) {
        return schedule->posixStartTime();
    }


    Opm::Schedule * schedule_alloc(Opm::ParseContext * context, Opm::EclipseGrid * grid, Opm::Deck * deck) {
        return new Opm::Schedule( *context , *grid, *deck );
    }

    void schedule_free( Opm::Schedule * schedule  ) {
        delete schedule;
    }

}


