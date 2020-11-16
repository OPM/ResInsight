#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/EndpointScaling.hpp>
#include <opm/common/utility/String.hpp>

namespace Opm {

EndpointScaling EndpointScaling::serializeObject()
{
    EndpointScaling result;
    result.options = std::bitset<4>{13};

    return result;
}

EndpointScaling::operator bool() const noexcept {
    return this->options[ static_cast< ue >( option::any ) ];
}

bool EndpointScaling::directional() const noexcept {
    return this->options[ static_cast< ue >( option::directional ) ];
}

bool EndpointScaling::nondirectional() const noexcept {
    return bool( *this ) && !this->directional();
}

bool EndpointScaling::reversible() const noexcept {
    return this->options[ static_cast< ue >( option::reversible ) ];
}

bool EndpointScaling::irreversible() const noexcept {
    return bool( *this ) && !this->reversible();
}

bool EndpointScaling::twopoint() const noexcept {
    return bool( *this ) && !this->threepoint();
}

bool EndpointScaling::threepoint() const noexcept {
    return this->options[ static_cast< ue >( option::threepoint ) ];
}

bool EndpointScaling::operator==(const EndpointScaling& data) const {
    return options == data.options;
}



namespace {

bool threepoint_scaling( const Deck& deck ) {
    if( !deck.hasKeyword( "SCALECRS" ) ) return false;

    /*
    * the manual says that Y and N are acceptable values for "YES" and "NO", so
    * it's *VERY* likely that only the first character is checked. We preserve
    * this behaviour
    */
    const auto value = std::toupper(
                        deck.getKeyword( "SCALECRS" )
                            .getRecord( 0 )
                            .getItem( "VALUE" )
                            .get< std::string >( 0 ).front() );

    if( value != 'Y' && value != 'N' )
        throw std::invalid_argument( "SCALECRS takes 'YES' or 'NO'" );

    return value == 'Y';
}

bool endscale_nodir( const DeckKeyword& kw ) {
    if( kw.getRecord( 0 ).getItem( 0 ).defaultApplied( 0 ) )
        return true;

    const auto& value = uppercase( kw.getRecord( 0 )
                                     .getItem( 0 )
                                     .get< std::string >( 0 ) );

    if( value != "DIRECT" && value != "NODIR" )
        throw std::invalid_argument(
            "ENDSCALE argument 1 must be defaulted, 'DIRECT' or 'NODIR', was "
            + value
        );

    return value == "NODIR";
}

bool endscale_revers( const DeckKeyword& kw ) {
    if( kw.getRecord( 0 ).getItem( 1 ).defaultApplied( 0 ) )
        return true;

    const auto& value = uppercase( kw.getRecord( 0 )
                                     .getItem( 1 )
                                     .get< std::string >( 0 ) );

    if( value != "IRREVERS" && value != "REVERS" )
        throw std::invalid_argument(
            "ENDSCALE argument 2 must be defaulted, 'REVERS' or 'IRREVERS', was "
            + value
        );

    const auto& item0 = kw.getRecord( 0 ).getItem( 0 );
    if( value == "IRREVERS"
     && uppercase( item0.get< std::string >( 0 ) ) != "DIRECT" )
        throw std::invalid_argument( "'IRREVERS' requires 'DIRECT'" );

    return value == "REVERS";
}

}

EndpointScaling::EndpointScaling( const Deck& deck ) {
    if( deck.hasKeyword( "ENDSCALE" ) || deck.hasKeyword("SWATINIT")) {
        const bool threep_ = threepoint_scaling( deck );
        bool direct_ = false;
        bool reversible_ = true;

        if (deck.hasKeyword("ENDSCALE")) {
            const auto& endscale = deck.getKeyword( "ENDSCALE" );
            direct_ = !endscale_nodir( endscale );
            reversible_ = endscale_revers( endscale );
        }

        this->options.set( static_cast< ue >( option::any ), true );
        this->options.set( static_cast< ue >( option::directional ), direct_ );
        this->options.set( static_cast< ue >( option::reversible ), reversible_ );
        this->options.set( static_cast< ue >( option::threepoint ), threep_ );
    }
}

}
