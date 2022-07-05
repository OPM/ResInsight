/*
  Copyright 2021 Statoil ASA.

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

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <getopt.h>
#include <fmt/format.h>
#include <unordered_set>
#include <utility>

#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/I.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/G.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/InputErrorAction.hpp>
#include <opm/input/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/FileDeck.hpp>

namespace fs = std::filesystem;

const std::unordered_set<std::string> remove_from_solution = {"EQUIL", "PRESSURE", "SWAT", "SGAS"};

void print_help_and_exit(const std::optional<std::string> error_msg = {}) {

    if (error_msg.has_value()) {
        std::cerr << "Error:" << std::endl;
        std::cerr << error_msg.value() << std::endl;
        std::cerr << "------------------------------------------------------" << std::endl;
    }

    std::string keep_keywords;
    for (const auto& kw : Opm::FileDeck::rst_keep_in_solution)
        keep_keywords += kw + " ";

    const std::string help_text = fmt::format(R"(

The rst_deck program will load a simulation deck and parameters for a restart
and reformat the deck to become a restart deck. Before the updated deck is
output the program will update the SOLUTION and SCHEDULE sections. All keywords
from the SOLUTION section will be cleared out(1) and a RESTART keyword will be
inserted. In the SCHEDULE section the program can either remove all keywords up
until the restart date, or alternatively insert SKIPREST immediately following
the SCHEDULE keyword.

When creating the updated restart deck the program can either link to unmodified
include files with INCLUDE statements, create a copy of deck structure in an
alternative location or create one large file with all keywords in the same
file. Apart from the alterations to support restart the output deck will be
equivalent to the input deck, but formatting is not retained and comments have
been stripped away.

Arguments:

1. The data file we are starting with.

2. The restart source; this can either be a basename with an optional path
   prefix and a :N to restart from step N; alternatively you can point to an
   existing restart file. If you point to an existing restart file the input
   will be validated in several ways:

   a) Unified/multiple files will be checked against the UNIFIN setting of
      the deck.

   b) Formatted/unformatted will be checked against the FMTIn setting of the
      deck.

   c) If a single file like /path/to/case/HISTORY.X0067 is given as argument the
      :N notation to denote report step should not be used.

   If the restart argument is given as the path to an existing file the content
   of the RESTART keyword will be updated to contain the correct path from the
   location of the restart deck to the location of the restart file. This path
   awareness will be fooled if the restart deck is redirected from stdout to a
   path different from cwd. If the restart argument is given as an absolute
   filename the RESTART keyword will have an absolute path, if the restart
   argument is a relative path the RESTART keyword will get a relative path -
   although an absolute path will be used if the restart file and the output
   deck have different roots. If the restart argument is given as a string not
   pointing to an existing file it will be inserted verbatim in the restart
   deck.

   A restart step value of 0 is interpreted as a dry run - a deck which has not
   been set up for restart will be written out.


3. Basename of the restart deck we create, can optionally contain a path prefix;
   the path will be created if it does not already exist. This argument is
   optional, if it is not provided the program will dump a restart deck on
   stdout. If the argument corresponds to an existing directory the restart case
   will get the same name as the base case.

Options:

-s: Manipulate the SCHEDULE section by inserting a SKIPREST keyword immediately
    following the SCHEDULE keyword. If the -s option is not used the SCHEDULE
    section will be modified by removing all keywords until we reach the restart
    date.

-m: [share|inline|copy] The restart deck can reuse the unmodified include files
    from the base case, this is mode 'share' and is the default. With mode
    'inline' the restart deck will be one long file and with mode 'copy' the
    file structure of the base case will be retained. The default if no -m
    option is given is the 'share' mode.

    In the case of 'share' and 'copy' the correct path to include files will be
    negotiated based on the path given to the output case in the third argument.
    If the restart deck is passed to stdout the include files will be resolved
    based on output in cwd.

Example:

   rst_deck /path/to/history/HISTORY.DATA rst/HISTORY:30 /path/to/rst/RESTART -s

1: The program has a compiled list of keywords which will be retained in the
   SOLUTION section. The current value of that list is: {}

)", keep_keywords);

    std::cerr << help_text << std::endl;
    if (error_msg.has_value())
        std::exit(EXIT_FAILURE);

    std::exit(EXIT_SUCCESS);
}



struct Options {
    std::string input_deck;
    std::string restart_base;
    int restart_step;
    std::optional<std::string> target_path;
    std::optional<std::string> target_fname;

    Opm::FileDeck::OutputMode mode{Opm::FileDeck::OutputMode::SHARE};
    bool skiprest{false};
};


Opm::Deck load_deck(const Options& opt) {
    Opm::ParseContext parseContext(Opm::InputError::WARN);
    Opm::ErrorGuard errors;
    Opm::Parser parser;

    /* Use the same default ParseContext as flow. */
    parseContext.update(Opm::ParseContext::PARSE_RANDOM_SLASH, Opm::InputError::IGNORE);
    parseContext.update(Opm::ParseContext::PARSE_MISSING_DIMS_KEYWORD, Opm::InputError::WARN);
    parseContext.update(Opm::ParseContext::SUMMARY_UNKNOWN_WELL, Opm::InputError::WARN);
    parseContext.update(Opm::ParseContext::SUMMARY_UNKNOWN_GROUP, Opm::InputError::WARN);
    return parser.parseFile(opt.input_deck, parseContext, errors);
}


Opm::FileDeck::OutputMode mode(const std::string& mode_arg) {
    if (mode_arg == "inline")
        return Opm::FileDeck::OutputMode::INLINE;

    if (mode_arg == "share")
        return Opm::FileDeck::OutputMode::SHARE;

    if (mode_arg == "copy")
        return Opm::FileDeck::OutputMode::COPY;

    print_help_and_exit(fmt::format("Mode argument: \'{}\' not recognized. Valid options are inline|share|copy", mode_arg));
    return Opm::FileDeck::OutputMode::INLINE;
}

std::optional<std::size_t> verify_extension(const std::string& extension, bool unified, bool formatted) {
    if (unified) {
        if (formatted) {
            if (extension == ".FUNRST")
                return std::nullopt;
            print_help_and_exit("Deck has specified formatted unified input - expected restart extension: .FUNRST");

        }
        if (extension == ".UNRST")
            return std::nullopt;
        print_help_and_exit("Deck has expected unformatted unified input - expected restart extension: .UNRST");
    }

    std::size_t report_step;
    if ((formatted && (extension[1] == 'F')) || (!formatted && (extension[1] == 'X'))) {
        try {
            report_step = std::stoi(extension.substr(2));
            return report_step;
        }
        catch (...) {}
    }
    print_help_and_exit("Deck has specified multiple input files - expected restart extension: .Xnnnn / .Fnnnn");
    return std::nullopt;
}


bool same_mount(const fs::path& p1, const fs::path& p2) {
    auto abs1 = fs::absolute(p1);
    auto abs2 = fs::absolute(p2);

    auto iter1 = abs1.begin(); iter1++;
    auto iter2 = abs2.begin(); iter2++;

    auto mnt1 = *iter1;
    auto mnt2 = *iter2;
    return (mnt1 == mnt2);
}


void update_restart_path(Options& opt, const std::string& restart_arg, const Opm::IOConfig& io_config) {
    std::string base;
    std::optional<std::size_t> rst_step;
    auto sep_pos = restart_arg.rfind(':');

    auto base_arg = restart_arg.substr(0, sep_pos);
    if (fs::exists(base_arg)) {
        auto unif = io_config.getUNIFIN();
        auto fmt = io_config.getFMTIN();
        auto path = fs::path(base_arg);
        auto extension = path.extension();
        rst_step = verify_extension(extension, unif, fmt);
        if (path.is_absolute()) {
            path.replace_extension();
            base = path;
        } else {
            auto target_path = fs::current_path();
            if (opt.target_path.has_value())
                target_path = fs::path(opt.target_path.value());

            if (same_mount(path, target_path))
                base = fs::relative(path, target_path).replace_extension();
            else
                base = fs::canonical(fs::absolute(path)).replace_extension();
        }
    } else
        base = base_arg;

    if (!rst_step.has_value()) {
        if (sep_pos == std::string::npos)
            print_help_and_exit(fmt::format("Expected restart argument on the form: BASE:NUMBER - e.g. HISTORY:60"));
        rst_step = std::stoi(restart_arg.substr(sep_pos + 1));
    }

    opt.restart_step = rst_step.value();
    opt.restart_base = base;
}


std::pair<Options, std::string> load_options(int argc, char **argv) {
    Options opt;
    while (true) {
        int c;
        c = getopt(argc, argv, "hm:s");
        if (c == -1)
            break;

        switch(c) {
        case 'm':
            opt.mode = mode(optarg);
            break;
        case 's':
            opt.skiprest = true;
            break;
        case 'h':
            print_help_and_exit();
            break;
        }
    }

    auto arg_offset = optind;
    if (arg_offset >= argc)
        print_help_and_exit();

    opt.input_deck = argv[arg_offset];
    std::string restart_arg = argv[arg_offset + 1];
    if ((argc - arg_offset) >= 3) {
        auto target_arg = argv[arg_offset + 2];

        if (fs::is_directory(target_arg)) {
            opt.target_path = target_arg;
            opt.target_fname = fs::path(opt.input_deck).filename();
        } else {
            auto target_path = fs::path( fs::absolute(target_arg) );
            opt.target_path = fs::absolute(target_path.parent_path());
            opt.target_fname = target_path.filename();
        }

        if (opt.mode == Opm::FileDeck::OutputMode::COPY) {
            auto target = fs::path(target_arg).parent_path();
            if (fs::exists(target)) {
                auto input = fs::path(opt.input_deck).parent_path();
                if (fs::equivalent(target, input))
                    opt.mode = Opm::FileDeck::OutputMode::SHARE;
            }
        }
    } else {
        if (opt.mode == Opm::FileDeck::OutputMode::COPY)
            print_help_and_exit("When writing output to stdout you must use inline|share mode");
    }

    return {opt, restart_arg};
}


void update_solution(const Options& opt, Opm::FileDeck& file_deck)
{
    if (opt.restart_step == 0)
        return;

    const auto solution = file_deck.find("SOLUTION");
    if (!solution.has_value())
        print_help_and_exit(fmt::format("Could not find SOLUTION section in input deck: {}", opt.input_deck));

    auto summary = file_deck.find("SUMMARY");
    if (!summary.has_value())
        print_help_and_exit(fmt::format("Could not find SUMMARY section in input deck: {}", opt.input_deck));

    file_deck.rst_solution(opt.restart_base, opt.restart_step);
}


void update_schedule(const Options& opt, Opm::FileDeck& file_deck)
{
    if (opt.restart_step == 0)
        return;

    if (opt.skiprest)
        file_deck.insert_skiprest();
    else
        file_deck.skip(opt.restart_step);
}


int main(int argc, char** argv) {
    auto [options, restart_arg] = load_options(argc, argv);
    auto deck = load_deck(options);
    Opm::FileDeck file_deck(deck);

    update_restart_path(options, restart_arg, Opm::IOConfig(deck));
    update_solution(options, file_deck);
    update_schedule(options, file_deck);
    if (!options.target_path.has_value())
        file_deck.dump_stdout(fs::current_path(), options.mode);
    else
        file_deck.dump( options.target_path.value(), options.target_fname.value(), options.mode);
}

