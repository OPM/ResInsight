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

#ifndef SECTION_HPP
#define SECTION_HPP

#include <string>

#include <opm/parser/eclipse/Deck/Deck.hpp>

namespace Opm {

enum class Section {
    RUNSPEC,
    GRID,
    EDIT,
    PROPS,
    REGIONS,
    SOLUTION,
    SUMMARY,
    SCHEDULE
};

    class UnitSystem;
    class Parser;


class DeckSection : public DeckView {
    public:
        using DeckView::const_iterator;

        DeckSection( const Deck& deck, const std::string& startKeyword );
        const std::string& name() const;
        const UnitSystem& unitSystem() const;

        static bool hasRUNSPEC( const Deck& );
        static bool hasGRID( const Deck& );
        static bool hasEDIT( const Deck& );
        static bool hasPROPS( const Deck& );
        static bool hasREGIONS( const Deck& );
        static bool hasSOLUTION( const Deck& );
        static bool hasSUMMARY( const Deck& );
        static bool hasSCHEDULE( const Deck& );

        // returns whether the deck has all mandatory sections and if all sections are in
        // the right order
        static bool checkSectionTopology(const Deck& deck,
                                         const Parser&,
                                         bool ensureKeywordSectionAffiliation = false);

    private:
        std::string section_name;
        const UnitSystem& units;

    };

    class RUNSPECSection : public DeckSection {
    public:
        using DeckSection::const_iterator;
        explicit RUNSPECSection(const Deck& deck) : DeckSection(deck, "RUNSPEC") {}
    };

    class GRIDSection : public DeckSection {
    public:
        using DeckSection::const_iterator;
        explicit GRIDSection(const Deck& deck) : DeckSection(deck, "GRID") {}
    };

    class EDITSection : public DeckSection {
    public:
        using DeckSection::const_iterator;
        explicit EDITSection(const Deck& deck) : DeckSection(deck, "EDIT") {}
    };

    class PROPSSection : public DeckSection {
    public:
        using DeckSection::const_iterator;
        explicit PROPSSection(const Deck& deck) : DeckSection(deck, "PROPS") {}
    };

    class REGIONSSection : public DeckSection {
    public:
        using DeckSection::const_iterator;
        explicit REGIONSSection(const Deck& deck) : DeckSection(deck, "REGIONS") {}
    };

    class SOLUTIONSection : public DeckSection {
    public:
        using DeckSection::const_iterator;
        explicit SOLUTIONSection(const Deck& deck) : DeckSection(deck, "SOLUTION") {}
    };

    class SUMMARYSection : public DeckSection {
    public:
        using DeckSection::const_iterator;
        explicit SUMMARYSection(const Deck& deck) : DeckSection(deck, "SUMMARY") {}
    };

    class SCHEDULESection : public DeckSection {
    public:
        using DeckSection::const_iterator;
        explicit SCHEDULESection(const Deck& deck) : DeckSection(deck, "SCHEDULE") {}
    };
}

#endif // SECTION_HPP
