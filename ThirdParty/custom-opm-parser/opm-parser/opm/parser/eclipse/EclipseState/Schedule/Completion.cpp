/*
  Copyright 2013 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <cassert>
#include <vector>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Completion.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <opm/parser/eclipse/EclipseState/Util/Value.hpp>

namespace Opm {

    Completion::Completion(int i, int j , int k ,
                           double depth,
                           WellCompletion::StateEnum state ,
                           const Value<double>& connectionTransmissibilityFactor,
                           const Value<double>& diameter,
                           const Value<double>& skinFactor,
                           const WellCompletion::DirectionEnum direction)
        : m_i(i), m_j(j), m_k(k),
          m_diameter(diameter),
          m_connectionTransmissibilityFactor(connectionTransmissibilityFactor),
          m_wellPi(1.0),
          m_skinFactor(skinFactor),
          m_state(state),
          m_direction(direction),
          m_segment_number(-1),
          m_center_depth( depth )
    {}

    Completion::Completion(std::shared_ptr<const Completion> oldCompletion, WellCompletion::StateEnum newStatus)
    :
        m_i(oldCompletion->getI()),
        m_j(oldCompletion->getJ()),
        m_k(oldCompletion->getK()),
        m_diameter(oldCompletion->getDiameterAsValueObject()),
        m_connectionTransmissibilityFactor(oldCompletion->getConnectionTransmissibilityFactorAsValueObject()),
        m_wellPi(oldCompletion->getWellPi()),
        m_skinFactor(oldCompletion->getSkinFactorAsValueObject()),
        m_state(newStatus),
        m_direction(oldCompletion->getDirection()),
        m_center_depth(oldCompletion->getCenterDepth())
    {
        if (oldCompletion->attachedToSegment()) {
            m_segment_number = oldCompletion->getSegmentNumber();
        } else {
            m_segment_number = -1;
        }
    }

    Completion::Completion(std::shared_ptr<const Completion> oldCompletion, double wellPi)
            :
            m_i(oldCompletion->getI()),
            m_j(oldCompletion->getJ()),
            m_k(oldCompletion->getK()),
            m_diameter(oldCompletion->getDiameterAsValueObject()),
            m_connectionTransmissibilityFactor(oldCompletion->getConnectionTransmissibilityFactorAsValueObject()),
            m_wellPi(oldCompletion->getWellPi()),
            m_skinFactor(oldCompletion->getSkinFactorAsValueObject()),
            m_state(oldCompletion->getState()),
            m_direction(oldCompletion->getDirection()),
            m_center_depth(oldCompletion->getCenterDepth())
    {
        if (oldCompletion->attachedToSegment()) {
            m_segment_number = oldCompletion->getSegmentNumber();
        } else {
            m_segment_number = -1;
        }

        if(m_wellPi!=0){
            m_wellPi*=wellPi;
        }else{
            m_wellPi=wellPi;
        }
    }

    Completion::Completion(std::shared_ptr<const Completion> oldCompletion)
            :
            m_i(oldCompletion->getI()),
            m_j(oldCompletion->getJ()),
            m_k(oldCompletion->getK()),
            m_diameter(oldCompletion->getDiameterAsValueObject()),
            m_connectionTransmissibilityFactor(oldCompletion->getConnectionTransmissibilityFactorAsValueObject()),
            m_wellPi(oldCompletion->getWellPi()),
            m_skinFactor(oldCompletion->getSkinFactorAsValueObject()),
            m_state(oldCompletion->getState()),
            m_direction(oldCompletion->getDirection()),
            m_center_depth(oldCompletion->getCenterDepth())
    {
        if (oldCompletion->attachedToSegment()) {
            m_segment_number = oldCompletion->getSegmentNumber();
        } else {
            m_segment_number = -1;
        }
    }

    bool Completion::sameCoordinate(const Completion& other) const {
        if ((m_i == other.m_i) &&
            (m_j == other.m_j) &&
            (m_k == other.m_k))
            return true;
        else
            return false;
    }

    bool Completion::sameCoordinate(const int i, const int j, const int k) const {
        if ((m_i == i) && (m_j == j) && (m_k == k)) {
            return true;
        } else {
            return false;
        }
    }


    /**
       This will break up one record and return a pair: <name ,
       [Completion1, Completion2, ... , CompletionN]>. The reason it
       will return a list is that the 'K1 K2' structure is
       disentangled, and each completion is returned separately.
    */

    inline std::vector< CompletionPtr >
    fromCOMPDAT( const EclipseGrid& grid, const DeckRecord& compdatRecord, const Well& well ) {

        std::vector<CompletionPtr> completions;

        // We change from eclipse's 1 - n, to a 0 - n-1 solution
        // I and J can be defaulted with 0 or *, in which case they are fetched
        // from the well head
        const auto& itemI = compdatRecord.getItem( "I" );
        const auto defaulted_I = itemI.defaultApplied( 0 ) || itemI.get< int >( 0 ) == 0;
        const int I = !defaulted_I ? itemI.get< int >( 0 ) - 1 : well.getHeadI();

        const auto& itemJ = compdatRecord.getItem( "J" );
        const auto defaulted_J = itemJ.defaultApplied( 0 ) || itemJ.get< int >( 0 ) == 0;
        const int J = !defaulted_J ? itemJ.get< int >( 0 ) - 1 : well.getHeadJ();

        int K1 = compdatRecord.getItem("K1").get< int >(0) - 1;
        int K2 = compdatRecord.getItem("K2").get< int >(0) - 1;
        WellCompletion::StateEnum state = WellCompletion::StateEnumFromString( compdatRecord.getItem("STATE").getTrimmedString(0) );
        Value<double> connectionTransmissibilityFactor("ConnectionTransmissibilityFactor");
        Value<double> diameter("Diameter");
        Value<double> skinFactor("SkinFactor");

        {
            const auto& connectionTransmissibilityFactorItem = compdatRecord.getItem("CONNECTION_TRANSMISSIBILITY_FACTOR");
            const auto& diameterItem = compdatRecord.getItem("DIAMETER");
            const auto& skinFactorItem = compdatRecord.getItem("SKIN");

            if (connectionTransmissibilityFactorItem.hasValue(0) && connectionTransmissibilityFactorItem.getSIDouble(0) > 0)
                connectionTransmissibilityFactor.setValue(connectionTransmissibilityFactorItem.getSIDouble(0));

            if (diameterItem.hasValue(0))
                diameter.setValue( diameterItem.getSIDouble(0));

            if (skinFactorItem.hasValue(0))
                skinFactor.setValue( skinFactorItem.get< double >(0));
        }

        const WellCompletion::DirectionEnum direction = WellCompletion::DirectionEnumFromString(compdatRecord.getItem("DIR").getTrimmedString(0));

        for (int k = K1; k <= K2; k++) {
            double depth = grid.getCellDepth( I,J,k );
            CompletionPtr completion(new Completion(I , J , k , depth , state , connectionTransmissibilityFactor, diameter, skinFactor, direction ));
            completions.push_back( completion );
        }

        return completions;
    }

    /*
      Will return a map:

      {
         "WELL1" : [ Completion1 , Completion2 , ... , CompletionN ],
         "WELL2" : [ Completion1 , Completion2 , ... , CompletionN ],
         ...
      }
    */

    std::map< std::string, std::vector< CompletionPtr > >
    Completion::fromCOMPDAT( const EclipseGrid& grid ,
                             const DeckKeyword& compdatKeyword,
                             const std::vector< const Well* >& wells ) {

        std::map< std::string, std::vector< CompletionPtr > > res;

        for( const auto& record : compdatKeyword ) {

            const auto wellname = record.getItem( "WELL" ).getTrimmedString( 0 );
            const auto name_eq = [&]( const Well* w ) {
                return w->name() == wellname;
            };

            auto well = std::find_if( wells.begin(), wells.end(), name_eq );

            if( well == wells.end() ) continue;

            auto completions = Opm::fromCOMPDAT( grid, record, **well );

            res[ wellname ].insert( res[ wellname ].end(),
                                    completions.begin(),
                                    completions.end() );
        }

        return res;
    }

    void Completion::fixDefaultIJ(int wellHeadI , int wellHeadJ) {
        if (m_i < 0)
            m_i = wellHeadI;

        if (m_j < 0)
            m_j = wellHeadJ;
    }


    int Completion::getI() const {
        return m_i;
    }

    int Completion::getJ() const {
        return m_j;
    }

    int Completion::getK() const {
        return m_k;
    }

    WellCompletion::StateEnum Completion::getState() const {
        return m_state;
    }

    double Completion::getConnectionTransmissibilityFactor() const {
        return m_connectionTransmissibilityFactor.getValue();
    }

    double Completion::getDiameter() const {
        return m_diameter.getValue();
    }

    double Completion::getSkinFactor() const {
        return m_skinFactor.getValue();
    }

    const Value<double>& Completion::getConnectionTransmissibilityFactorAsValueObject() const {
        return m_connectionTransmissibilityFactor;
    }

    Value<double> Completion::getDiameterAsValueObject() const {
        return m_diameter;
    }

    Value<double> Completion::getSkinFactorAsValueObject() const {
        return m_skinFactor;
    }

    WellCompletion::DirectionEnum Completion::getDirection() const {
        return m_direction;
    }

    double Completion::getWellPi() const {
        return m_wellPi;
    }

    int Completion::getSegmentNumber() const {
        if (!attachedToSegment()) {
            throw std::runtime_error(" the completion is not attached to a segment!\n ");
        }
        return m_segment_number;
    }

    double Completion::getCenterDepth() const {
        return m_center_depth;
    }

    void Completion::attachSegment(int segmentNumber , double centerDepth) {
        assert(segmentNumber > 0);

        m_segment_number = segmentNumber;
        m_center_depth = centerDepth;
    }

    bool Completion::attachedToSegment() const {
        return (m_segment_number > 0);
    }


}


