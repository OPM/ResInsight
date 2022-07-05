/*
  Copyright 2018 Statoil ASA.

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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <getopt.h>

#include <opm/input/eclipse/Parser/ParserKeywords/I.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/G.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/InputErrorAction.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>

namespace fs = std::filesystem;

Opm::Deck pack_deck( const char * deck_file, std::ostream& os) {
    Opm::ParseContext parseContext(Opm::InputError::WARN);
    Opm::ErrorGuard errors;
    Opm::Parser parser;

    auto deck = parser.parseFile(deck_file, parseContext, errors);
    os << deck;

    return deck;
}


void print_help_and_exit() {
    const char * help_text = R"(
The opmpack program will load a deck, resolve all include
files and then print it out again on stdout. All comments
will be stripped and the value types will be validated.

By passing the option -o you can redirect the output to a file
or a directory.

Print on stdout:

   opmpack  /path/to/case/CASE.DATA


Print MY_CASE.DATA in /tmp:

    opmpack -o /tmp /path/to/MY_CASE.DATA


Print NEW_CASE in cwd:

    opmpack -o NEW_CASE.DATA path/to/MY_CASE.DATA

As an alternative to the -o option you can use -c; that is equivalent to -o -
but restart and import files referred to in the deck are also copied. The -o and
-c options are mutually exclusive. )";
    std::cerr << help_text << std::endl;
    exit(1);
}


void copy_file(const fs::path& source_dir, fs::path fname, const fs::path& target_dir) {
    if (fname.is_absolute()) {
        // change when moving to gcc8+
        // fname = fs::relative(fname, source_dir);
        auto prefix_len = fs::canonical(source_dir).string().size();
        fname = fs::canonical(fname);
        fname = fs::path( fname.string().substr(prefix_len + 1) );
    }

    auto source_file = source_dir / fname;
    auto target_file = target_dir / fname;
    {
        const auto& parent_path = target_file.parent_path();
        if (!parent_path.empty() && !fs::is_directory(parent_path))
            fs::create_directories(parent_path);
    }

    fs::copy_file(source_file, target_file, fs::copy_options::overwrite_existing);
    std::cerr << "Copying file " << source_file.string() << " -> " << target_file.string() << std::endl;
}



int main(int argc, char** argv) {
    int arg_offset = 1;
    bool stdout_output = true;
    bool copy_binary = false;
    const char * coutput_arg;

    while (true) {
        int c;
        c = getopt(argc, argv, "c:o:");
        if (c == -1)
            break;

        switch(c) {
        case 'o':
            stdout_output = false;
            coutput_arg = optarg;
            break;
        case 'c':
            stdout_output = false;
            copy_binary = true;
            coutput_arg = optarg;
            break;
        }
    }
    arg_offset = optind;
    if (arg_offset >= argc)
        print_help_and_exit();

    if (stdout_output)
        pack_deck(argv[arg_offset], std::cout);
    else {
        std::ofstream os;
        fs::path input_arg(argv[arg_offset]);
        fs::path output_arg(coutput_arg);
        fs::path output_dir(coutput_arg);

        if (fs::is_directory(output_arg)) {
            fs::path output_path = output_arg / input_arg.filename();
            os.open(output_path.string());
        } else {
            os.open(output_arg.string());
            output_dir = output_arg.parent_path();
        }


        const auto& deck = pack_deck(argv[arg_offset], os);
        if (copy_binary) {
            Opm::InitConfig init_config(deck);
            if (init_config.restartRequested()) {
                Opm::IOConfig io_config(deck);
                fs::path restart_file(io_config.getRestartFileName( init_config.getRestartRootName(), init_config.getRestartStep(), false ));
                copy_file(input_arg.parent_path(), restart_file, output_dir);
            }

            using IMPORT = Opm::ParserKeywords::IMPORT;
            for (const auto& import_keyword : deck.get<IMPORT>()) {
                const auto& fname = import_keyword.getRecord(0).getItem<IMPORT::FILE>().get<std::string>(0);
                copy_file(input_arg.parent_path(), fname, output_dir);
            }


            using PYACTION = Opm::ParserKeywords::PYACTION;
            for (const auto& pyaction_keyword : deck.get<PYACTION>()) {
                const auto& fname = pyaction_keyword.getRecord(1).getItem<PYACTION::FILENAME>().get<std::string>(0);
                copy_file(input_arg.parent_path(), fname, output_dir);
            }


            using GDFILE = Opm::ParserKeywords::GDFILE;
            if (deck.hasKeyword<GDFILE>()) {
                const auto& gdfile_keyword = deck.get<GDFILE>().back();
                const auto& fname = gdfile_keyword.getRecord(0).getItem<GDFILE::filename>().get<std::string>(0);
                copy_file(input_arg.parent_path(), fname, output_dir);
            }
        }
    }
}

