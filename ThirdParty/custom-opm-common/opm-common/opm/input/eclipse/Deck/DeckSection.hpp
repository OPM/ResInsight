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

#include <opm/input/eclipse/Deck/Deck.hpp>

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


        // ---------------------------------------------------------------------------------
        // Highly deprecated shims
        const DeckKeyword& getKeyword(const std::string& keyword, std::size_t index) const {
            auto view = this->operator[](keyword);
            return view[index];
        }

        const DeckKeyword& getKeyword(const std::string& keyword) const {
            auto view = this->operator[](keyword);
            return view.back();
        }


        std::vector<const DeckKeyword*> getKeywordList(const std::string& keyword) const {
            std::vector<const DeckKeyword*> kw_list;
            auto view = this->operator[](keyword);
            for (const auto& kw : view)
                kw_list.push_back(&kw);
            return kw_list;
        }

        template <class Keyword>
        std::vector<const DeckKeyword*> getKeywordList() const {
            return this->getKeywordList(Keyword::keywordName);
        }


        bool hasKeyword(const std::string& keyword) const {
            return this->has_keyword(keyword);
        }

        template <class Keyword>
        bool hasKeyword() const {
            return this->has_keyword(Keyword::keywordName);
        }

        // ---------------------------------------------------------------------------------


    private:
        std::string section_name;
        const UnitSystem& units;

    };

    class RUNSPECSection : public DeckSection {
    public:
        explicit RUNSPECSection(const Deck& deck) : DeckSection(deck, "RUNSPEC") {}
    };

    class GRIDSection : public DeckSection {
    public:
        explicit GRIDSection(const Deck& deck) : DeckSection(deck, "GRID") {}
    };

    class EDITSection : public DeckSection {
    public:
        explicit EDITSection(const Deck& deck) : DeckSection(deck, "EDIT") {}
    };

    class PROPSSection : public DeckSection {
    public:
        explicit PROPSSection(const Deck& deck) : DeckSection(deck, "PROPS") {}
    };

    class REGIONSSection : public DeckSection {
    public:
        explicit REGIONSSection(const Deck& deck) : DeckSection(deck, "REGIONS") {}
    };

    class SOLUTIONSection : public DeckSection {
    public:
        explicit SOLUTIONSection(const Deck& deck) : DeckSection(deck, "SOLUTION") {}
    };

    class SUMMARYSection : public DeckSection {
    public:
        explicit SUMMARYSection(const Deck& deck) : DeckSection(deck, "SUMMARY") {}
    };

    class SCHEDULESection : public DeckSection {
    public:
        explicit SCHEDULESection(const Deck& deck) : DeckSection(deck, "SCHEDULE") {}
    };
}

#endif // SECTION_HPP
