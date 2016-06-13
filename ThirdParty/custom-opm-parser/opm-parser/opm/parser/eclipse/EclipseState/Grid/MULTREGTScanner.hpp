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


#ifndef MULTREGTSCANNER_HPP
#define MULTREGTSCANNER_HPP

#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FaceDir.hpp>
#include <opm/parser/eclipse/EclipseState/Util/Value.hpp>


namespace Opm {

    template< typename > class GridProperties;

    class DeckRecord;
    class DeckKeyword;

    namespace MULTREGT {


        enum NNCBehaviourEnum {
            NNC = 1,
            NONNC = 2,
            ALL = 3,
            NOAQUNNC = 4
        };

        std::string RegionNameFromDeckValue(const std::string& stringValue);
        NNCBehaviourEnum NNCBehaviourFromString(const std::string& stringValue);
    }




    class MULTREGTRecord {
    public:
        MULTREGTRecord(const DeckRecord& deckRecord , const std::string& defaultRegion);

        Value<int> m_srcRegion;
        Value<int> m_targetRegion;
        double m_transMultiplier;
        int m_directions;
        MULTREGT::NNCBehaviourEnum m_nncBehaviour;
        Value<std::string>  m_region;
    };

    typedef std::map< std::pair<int , int> , const MULTREGTRecord * >  MULTREGTSearchMap;
    typedef std::tuple<size_t , FaceDir::DirEnum , double> MULTREGTConnection;



    class MULTREGTScanner {

    public:
        MULTREGTScanner(const Eclipse3DProperties& cellRegionNumbers,
        		        const std::vector< const DeckKeyword* >& keywords,
						const std::string& defaultRegion);
        double getRegionMultiplier(size_t globalCellIdx1, size_t globalCellIdx2, FaceDir::DirEnum faceDir) const;

    private:
        void addKeyword( const DeckKeyword& deckKeyword, const std::string& defaultRegion);
        void assertKeywordSupported(const DeckKeyword& deckKeyword, const std::string& defaultRegion);
        std::vector< MULTREGTRecord > m_records;
        std::map<std::string , MULTREGTSearchMap> m_searchMap;
        const Eclipse3DProperties& m_cellRegionNumbers;
    };

}

#endif
