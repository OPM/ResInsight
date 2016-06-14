#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/InitConfig/Equil.hpp>

namespace Opm {

    EquilRecord::EquilRecord( const DeckRecord& record ) :
            datum_depth( record.getItem( 0 ).getSIDouble( 0 ) ),
            datum_depth_ps( record.getItem( 1 ).getSIDouble( 0 ) ),
            water_oil_contact_depth( record.getItem( 2 ).getSIDouble( 0 ) ),
            water_oil_contact_capillary_pressure( record.getItem( 3 ).getSIDouble( 0 ) ),
            gas_oil_contact_depth( record.getItem( 4 ).getSIDouble( 0 ) ),
            gas_oil_contact_capillary_pressure( record.getItem( 5 ).getSIDouble( 0 ) ),
            live_oil_init_proc( record.getItem( 6 ).get< int >( 0 ) <= 0 ),
            wet_gas_init_proc( record.getItem( 7 ).get< int >( 0 ) <= 0 ),
            init_target_accuracy( record.getItem( 8 ).get< int >( 0 ) )
    {}

    double EquilRecord::datumDepth() const {
        return this->datum_depth;
    }

    double EquilRecord::datumDepthPressure() const {
        return this->datum_depth_ps;
    }

    double EquilRecord::waterOilContactDepth() const {
        return this->water_oil_contact_depth;
    }

    double EquilRecord::waterOilContactCapillaryPressure() const {
        return this->water_oil_contact_capillary_pressure;
    }

    double EquilRecord::gasOilContactDepth() const {
        return this->gas_oil_contact_depth;
    }

    double EquilRecord::gasOilContactCapillaryPressure() const {
        return this->gas_oil_contact_capillary_pressure;
    }

    bool EquilRecord::liveOilInitConstantRs() const {
        return this->live_oil_init_proc;
    }

    bool EquilRecord::wetGasInitConstantRv() const {
        return this->wet_gas_init_proc;
    }

    int EquilRecord::initializationTargetAccuracy() const {
        return this->init_target_accuracy;
    }

    /* */

    Equil::Equil( const DeckKeyword& keyword ) :
        records( keyword.begin(), keyword.end() )
    {}

    const EquilRecord& Equil::getRecord( size_t id ) const {
        return this->records.at( id );
    }

    size_t Equil::size() const {
        return this->records.size();
    }

    bool Equil::empty() const {
        return this->records.empty();
    }

    Equil::const_iterator Equil::begin() const {
        return this->records.begin();
    }

    Equil::const_iterator Equil::end() const {
        return this->records.end();
    }
}
