#include <iostream>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>

extern "C" {

    Opm::EclipseGrid * eclipse_grid_alloc(Opm::Deck * deck);
    void               eclipse_grid_free( Opm::EclipseGrid * grid );

    /*-----------------------------------------------------------------*/

    Opm::EclipseGrid * eclipse_grid_alloc(Opm::Deck * deck) {
        return new Opm::EclipseGrid( *deck, nullptr );
    }

    void eclipse_grid_free( Opm::EclipseGrid * grid ) {
        delete grid;
    }

}


