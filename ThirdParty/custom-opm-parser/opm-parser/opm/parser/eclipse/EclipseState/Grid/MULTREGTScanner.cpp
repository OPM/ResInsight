/*
  Copyright 2014 Statoil ASA.

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

#include <stdexcept>
#include <map>
#include <set>

#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaceDir.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/MULTREGTScanner.hpp>

namespace Opm {

    namespace MULTREGT {

        std::string RegionNameFromDeckValue(const std::string& stringValue) {
            if (stringValue == "O")
                return "OPERNUM";

            if (stringValue == "F")
                return "FLUXNUM";

            if (stringValue == "M")
                return "MULTNUM";

            throw std::invalid_argument("The input string: " + stringValue + " was invalid. Expected: O/F/M");
        }



        NNCBehaviourEnum NNCBehaviourFromString(const std::string& stringValue) {
            if (stringValue == "ALL")
                return ALL;

            if (stringValue == "NNC")
                return NNC;

            if (stringValue == "NONNC")
                return NONNC;

            if (stringValue == "NOAQUNNC")
                return NOAQUNNC;

            throw std::invalid_argument("The input string: " + stringValue + " was invalid. Expected: ALL/NNC/NONNC/NOAQUNNC");
        }


    }




    MULTREGTRecord::MULTREGTRecord( const DeckRecord& deckRecord, const std::string& defaultRegion) :
        m_srcRegion("SRC_REGION"),
        m_targetRegion("TARGET_REGION"),
        m_region("REGION" , defaultRegion)
    {
        const auto& srcItem = deckRecord.getItem("SRC_REGION");
        const auto& targetItem = deckRecord.getItem("TARGET_REGION");
        const auto& tranItem = deckRecord.getItem("TRAN_MULT");
        const auto& dirItem = deckRecord.getItem("DIRECTIONS");
        const auto& nncItem = deckRecord.getItem("NNC_MULT");
        const auto& defItem = deckRecord.getItem("REGION_DEF");


        if (!srcItem.defaultApplied(0))
            m_srcRegion.setValue( srcItem.get< int >(0) );

        if (!targetItem.defaultApplied(0))
            m_targetRegion.setValue( targetItem.get< int >(0) );

        m_transMultiplier = tranItem.get< double >(0);
        m_directions = FaceDir::FromMULTREGTString( dirItem.get< std::string >(0) );
        m_nncBehaviour = MULTREGT::NNCBehaviourFromString( nncItem.get< std::string >(0));

        if (!defItem.defaultApplied(0))
            m_region.setValue( MULTREGT::RegionNameFromDeckValue ( defItem.get< std::string >(0) ));
    }


    /*****************************************************************/
    /*
      Observe that the (REGION1 -> REGION2) pairs behave like keys;
      i.e. for the MULTREGT keyword

        MULTREGT
          2  4   0.75    Z   ALL    M /
          2  4   2.50   XY   ALL    F /
        /

      The first record is completely overweritten by the second
      record, this is because the both have the (2 -> 4) region
      identifiers. This behaviourt is ensured by using a map with
      std::pair<region1,region2> as key.

      This function starts with some initial preprocessing to create a
      map which looks like this:


         searchMap = {"MULTNUM" : {std::pair(1,2) : std::tuple(TransFactor , Face , Region),
                                   std::pair(4,7) : std::tuple(TransFactor , Face , Region),
                                   ...},
                      "FLUXNUM" : {std::pair(4,8) : std::tuple(TransFactor , Face , Region),
                                   std::pair(1,4) : std::tuple(TransFactor , Face , Region),
                                   ...}}

      Then it will go through the different regions and looking for
      interface with the wanted region values.
    */
    MULTREGTScanner::MULTREGTScanner(const Eclipse3DProperties& cellRegionNumbers,
    		                         const std::vector< const DeckKeyword* >& keywords,
									 const std::string& defaultRegion ) :
        m_cellRegionNumbers(cellRegionNumbers) {

        for (size_t idx = 0; idx < keywords.size(); idx++)
            this->addKeyword(*keywords[idx] , defaultRegion);

        MULTREGTSearchMap searchPairs;
        for (std::vector<MULTREGTRecord>::const_iterator record = m_records.begin(); record != m_records.end(); ++record) {
            if (cellRegionNumbers.hasDeckIntGridProperty( record->m_region.getValue())) {
                if (record->m_srcRegion.hasValue() && record->m_targetRegion.hasValue()) {
                    int srcRegion    = record->m_srcRegion.getValue();
                    int targetRegion = record->m_targetRegion.getValue();
                    if (srcRegion != targetRegion) {
                        std::pair<int,int> pair{ srcRegion, targetRegion };
                        searchPairs[pair] = &(*record);
                    }
                }
            }
            else
                throw std::logic_error("MULTREGT record is based on region: " + record->m_region.getValue() + " which is not in the deck");
        }


        for (auto iter = searchPairs.begin(); iter != searchPairs.end(); ++iter) {
            const MULTREGTRecord * record = (*iter).second;
            std::pair<int,int> pair = (*iter).first;
            const std::string& keyword = record->m_region.getValue();
            if (m_searchMap.count(keyword) == 0)
                m_searchMap[keyword] = MULTREGTSearchMap();

            m_searchMap[keyword][pair] = record;
        }


    }


    void MULTREGTScanner::assertKeywordSupported( const DeckKeyword& deckKeyword, const std::string& defaultRegion) {
        for (auto iter = deckKeyword.begin(); iter != deckKeyword.end(); ++iter) {
            MULTREGTRecord record( *iter , defaultRegion);

            if (record.m_nncBehaviour == MULTREGT::NOAQUNNC)
                throw std::invalid_argument("Sorry - currently we do not support \'NOAQUNNC\' for MULTREGT.");

            if (!record.m_srcRegion.hasValue())
                throw std::invalid_argument("Sorry - currently it is not supported with a defaulted source region value.");

            if (!record.m_targetRegion.hasValue())
                throw std::invalid_argument("Sorry - currently it is not supported with a defaulted target region value.");

            if (record.m_srcRegion.getValue() == record.m_targetRegion.getValue())
                throw std::invalid_argument("Sorry - multregt applied internally to a region is not yet supported");

        }
    }



    void MULTREGTScanner::addKeyword( const DeckKeyword& deckKeyword , const std::string& defaultRegion) {
        assertKeywordSupported( deckKeyword , defaultRegion );

        for (auto iter = deckKeyword.begin(); iter != deckKeyword.end(); ++iter) {
            MULTREGTRecord record( *iter , defaultRegion );
            /*
              The default value for the region item is to use the
              region item on the previous record, or alternatively
              'MULTNUM' for the first record.
            */
            if (!record.m_region.hasValue()) {
                if (m_records.size() > 0) {
                    auto previousRecord = m_records.back();
                    record.m_region.setValue( previousRecord.m_region.getValue() );
                } else
                    record.m_region.setValue( "MULTNUM" );
            }
            m_records.push_back( record );
        }

    }


    /*
      This function will check the region values in globalIndex1 and
      globalIndex and see if they match the regionvalues specified in
      the deck. The function checks both directions:

      Assume the relevant MULTREGT record looks like:

         1  2   0.10  XYZ  ALL M /

      I.e. we are checking for the boundary between regions 1 and
      2. We assign the transmissibility multiplier to the correct face
      of the cell with value 1:

         -----------
         | 1  | 2  |   =>  MultTrans( i,j,k,FaceDir::XPlus ) *= 0.50
         -----------

         -----------
         | 2  | 1  |   =>  MultTrans( i+1,j,k,FaceDir::XMinus ) *= 0.50
         -----------

    */
    double MULTREGTScanner::getRegionMultiplier(size_t globalIndex1 , size_t globalIndex2, FaceDir::DirEnum faceDir) const {

        for (auto iter = m_searchMap.begin(); iter != m_searchMap.end(); iter++) {
            const Opm::GridProperty<int>& region = m_cellRegionNumbers.getIntGridProperty( (*iter).first );
            MULTREGTSearchMap map = (*iter).second;

            int regionId1 = region.iget(globalIndex1);
            int regionId2 = region.iget(globalIndex2);


            std::pair<int,int> pair{regionId1 , regionId2};
            if (map.count(pair) != 1 || !(map.at(pair)->m_directions & faceDir)) {
                pair = std::pair<int,int>{regionId2 , regionId1};
                if (map.count(pair) != 1 || !(map.at(pair)->m_directions & faceDir))
                    continue;
            }
            const MULTREGTRecord * record = map[pair];

            bool applyMultiplier = true;
            int i1 = globalIndex1 % region.getNX();
            int i2 = globalIndex2 % region.getNX();
            int j1 = globalIndex1 / region.getNX() % region.getNY();
            int j2 = globalIndex2 / region.getNX() % region.getNY();

            if (record->m_nncBehaviour == MULTREGT::NNC){
                applyMultiplier = true;
                if ((std::abs(i1-i2) == 0 && std::abs(j1-j2) == 1) || (std::abs(i1-i2) == 1 && std::abs(j1-j2) == 0))
                    applyMultiplier = false;
            }
            else if (record->m_nncBehaviour == MULTREGT::NONNC){
                applyMultiplier = false;
                if ((std::abs(i1-i2) == 0 && std::abs(j1-j2) == 1) || (std::abs(i1-i2) == 1 && std::abs(j1-j2) == 0))
                    applyMultiplier = true;
            }

            if (applyMultiplier) {
                return record->m_transMultiplier;
            }

        }
        return 1;
    }
}
