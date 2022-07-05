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

#ifndef FILE_DECK_HPP
#define FILE_DECK_HPP

#include <optional>
#include <string>
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <fmt/format.h>

#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/FileDeck.hpp>

namespace fs = std::filesystem;

namespace Opm {
class Deck;

class FileDeck {
public:
    static const std::unordered_set<std::string> rst_keep_in_solution;
    static const std::unordered_set<std::string> rst_keep_in_schedule;

enum class OutputMode {
    INLINE = 1,
    SHARE = 2,
    COPY = 3
};



struct Index {
    std::size_t file_index;
    std::size_t keyword_index;

    Index(std::size_t file_index_arg, std::size_t keyword_index_arg, const FileDeck* deck_arg)
        : file_index(file_index_arg)
        , keyword_index(keyword_index_arg)
        , deck(deck_arg)
    {}

    Index& operator--();
    Index  operator--(int);
    Index& operator++();
    Index  operator++(int);
    bool   operator==(const Index& other) const;
    bool   operator!=(const Index& other) const;
    bool   operator<(const Index& other) const;
    Index  operator+(std::size_t shift) const;

private:
    const FileDeck * deck;
};



class Block {
public:
    explicit Block(const std::string& filename);
    std::size_t size() const;
    void load(const Deck& deck, std::size_t deck_index);
    std::optional<std::size_t> find(const std::string& keyword, std::size_t keyword_index) const;
    bool empty() const;
    void erase(const FileDeck::Index& index);
    void insert(std::size_t keyword_index, const DeckKeyword& keyword);
    void dump(DeckOutput& out) const;

private:
    std::string fname;
    std::vector<DeckKeyword> keywords;

friend FileDeck;
};


    explicit FileDeck(const Deck& deck);
    std::optional<Index> find(const std::string& keyword, const Index& offset) const;
    std::optional<Index> find(const std::string& keyword) const;
    std::size_t count(const std::string& keyword) const;
    void erase(const Index& index);
    void erase(const Index& begin, const Index& end);
    void insert(const Index& index, const DeckKeyword& keyword);

    void dump_stdout(const std::string& output_dir, OutputMode mode) const;
    void dump(const std::string& dir, const std::string& fname, OutputMode mode) const;
    const DeckKeyword& operator[](const Index& index) const;
    const Index start() const;
    const Index stop() const;

    void rst_solution(const std::string& rst_base, int report_step);
    void insert_skiprest();
    void skip(int report_step);

private:
    std::vector<Block> blocks;
    std::string input_directory;
    std::unordered_set<std::string> modified_files;
    DeckTree deck_tree;

    struct DumpContext {
        std::unordered_map<std::string, std::ofstream> stream_map;
        std::unordered_map<std::string, std::string> file_map;

        bool has_file(const std::string& fname) const {
            return this->file_map.count(fname) > 0;
        }

        std::optional<std::ofstream *> get_stream(const std::string& deck_name) {
            auto name_iter = this->file_map.find(deck_name);
            if (name_iter == this->file_map.end())
                return {};

            return &this->stream_map.at(name_iter->second);
        }


        std::ofstream& open_file(const std::string& deck_name, const fs::path& output_file)
        {
            if (this->stream_map.count(output_file.string()) == 0) {
                this->file_map.insert(std::make_pair( deck_name, output_file.string() ));

                if (!fs::is_directory(output_file.parent_path()))
                    fs::create_directories(output_file.parent_path());

                std::ofstream stream{output_file};
                if (!stream)
                    throw std::logic_error(fmt::format("Opening {} for writing failed", output_file.string()));
                this->stream_map.insert(std::make_pair(output_file.string(), std::move(stream)));
            }
            return this->stream_map.at(output_file.string());
        }

    };

    void dump(std::ostream& os) const;
    void dump_shared(std::ostream& stream, const std::string& output_dir) const;
    void dump_inline() const;
    std::string dump_block(const Block& block, const std::string& dir, const std::optional<std::string>& fname, DumpContext& context) const;
    void include_block(const std::string& source_file, const std::string& target_file, const std::string& dir, DumpContext& context) const;
};

}

#endif
