/*
  Copyright 2019 Equinor ASA.

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
#include <getopt.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <fmt/format.h>

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>


struct keyword {
    keyword(const std::string& name_arg, const std::string& filename_arg,
            std::size_t line_number_arg, std::size_t content_hash_arg) :
        name(name_arg),
        filename(filename_arg),
        line_number(line_number_arg),
        content_hash(content_hash_arg)
    {}


    std::string name;
    std::string filename;
    std::size_t line_number;
    std::size_t content_hash;
};


std::vector<keyword> load_deck(const std::string& deck_file) {
    Opm::ParseContext parseContext;
    Opm::ErrorGuard errors;
    Opm::Parser parser;
    std::vector<keyword> keywords;

    /* Use the same default ParseContext as flow. */
    parseContext.update(Opm::ParseContext::PARSE_RANDOM_SLASH, Opm::InputError::IGNORE);
    parseContext.update(Opm::ParseContext::PARSE_MISSING_DIMS_KEYWORD, Opm::InputError::WARN);
    parseContext.update(Opm::ParseContext::SUMMARY_UNKNOWN_WELL, Opm::InputError::WARN);
    parseContext.update(Opm::ParseContext::SUMMARY_UNKNOWN_GROUP, Opm::InputError::WARN);

    auto deck = parser.parseFile(deck_file, parseContext, errors);
    for (const auto& kw : deck) {
        std::stringstream ss;
        const auto& location = kw.location();
        ss << kw;

        keywords.emplace_back(kw.name(), location.filename, location.lineno, std::hash<std::string>{}(ss.str()));
    }
    return keywords;
}


std::size_t make_deck_hash(const std::vector<keyword>& keywords) {
    std::stringstream ss;
    for (const auto& kw : keywords)
        ss << kw.content_hash;

    return std::hash<std::string>{}(ss.str());
}


void print_keywords(const std::vector<keyword>& keywords, std::size_t deck_hash, bool location_info) {
    for (const auto& kw : keywords) {
        if (location_info)
            fmt::print("{:8s} : {}:{} {} \n", kw.name, kw.filename, kw.line_number, kw.content_hash);
        else
            fmt::print("{:8s} : {} \n", kw.name, kw.content_hash);
    }
    fmt::print("\n{:8s} : {}\n", "Total", deck_hash);
}


void print_help_and_exit() {
    const char * help_text = R"(The purpose of the opmhash program is to load a deck and create a summary, by
diffing two such summaries it is simple to determine if two decks are similar.
For each keyword a hash of the normalized content is calculated. The output of
the program will look like this:

  RUNSPEC  : 13167205945009276792
  TITLE    : 16047371705964514902
  DIMENS   : 1264233216877515756
  NONNC    : 10052807539267647959
  OIL      : 6013609912232720008
  WATER    : 14106203893673265964

  Total    : 7362809723723482303

Where the 'random' integer following each keyword is the hash of the content of
that keyword. The hashing is insensitive to changes in white-space and comments
and file location. At the bottom comes a total hash of the complete content. The
hash of each keyword is insensitive to shuffling of keywords, but the total hash
depends on the keyword order.

Options:

 -l : Add filename and linenumber information to each keyword.
 -s : Short form - only print the hash of the complete deck.
 -S : Silent form - will not print any deck output.

It is possible to add multiple deck arguments, they are then scanned repeatedly,
and the decks are compared. In the case of multiple deck arguments the exit
status of the program will be zero if all are equal and nonzero in case of
differences.

)";
    std::cerr << help_text << std::endl;
    exit(1);
}


int main(int argc, char** argv) {
    int arg_offset = 1;
    bool location_info = false;
    bool short_form = false;
    bool silent = false;

    while (true) {
        int c;
        c = getopt(argc, argv, "lsS");
        if (c == -1)
            break;

        switch(c) {
        case 'l':
            location_info = true;
            break;
        case 's':
            short_form = true;
            break;
        case 'S':
            silent = true;
            break;
        }
    }
    arg_offset = optind;
    if (arg_offset >= argc)
        print_help_and_exit();


    std::vector<std::pair<std::string, std::size_t>> deck_hash_table;
    for (int iarg = arg_offset; iarg < argc; iarg++) {
        const std::string deck_file = argv[iarg];
        auto keywords = load_deck(deck_file);
        auto deck_hash = make_deck_hash(keywords);
        deck_hash_table.emplace_back(deck_file, deck_hash);
        if (silent)
            continue;

        if (short_form)
            std::cout << deck_hash << std::endl;
        else
            print_keywords(keywords, deck_hash, location_info);
    }

    if (deck_hash_table.size() > 1) {
        bool equal = true;
        const auto& [first_deck, first_hash] = deck_hash_table[0];
        for (std::size_t index = 1; index < deck_hash_table.size(); index++) {
            const auto& [deck, hash] = deck_hash_table[index];
            if (first_hash != hash)
                equal = false;

            if (silent)
                continue;

            fmt::print("{} {} {}\n",
                       first_deck,
                       (first_hash == hash) ? "==" : "!=",
                       deck);
        }

        if (equal)
            std::exit(EXIT_SUCCESS);
        else
            std::exit(EXIT_FAILURE);
    }

}

