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

  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fmt/format.h>
#include <opm/common/utility/FileSystem.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckValue.hpp>
#include <opm/input/eclipse/Deck/DeckOutput.hpp>
#include <opm/input/eclipse/Deck/FileDeck.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/D.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/R.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>

namespace fs = std::filesystem;

namespace Opm {


namespace {

void INCLUDE(std::ostream& stream , const std::string& fname)
{
    auto include_string = fmt::format(R"(
INCLUDE
   '{}' /
)", fname);
    stream << include_string;
}

void touch_file(const fs::path& file) {
    if (!fs::exists(file)) {
        const auto& parent_path = file.parent_path();
        if (!fs::is_directory(parent_path))
            fs::create_directories(parent_path);
        std::ofstream{file};
    }
}
}

FileDeck::Index& FileDeck::Index::operator--() {
    if (this->keyword_index > 0)
        this->keyword_index--;
    else {
        if (this->file_index == 0)
            throw std::logic_error("Going beyond start of container");

        this->file_index--;
        this->keyword_index = this->deck->blocks[this->file_index].size() - 1;
    }
    return *this;
}

FileDeck::Index  FileDeck::Index::operator--(int) {
    auto current = *this;
    this->operator--();
    return current;
}

FileDeck::Index& FileDeck::Index::operator++() {
    if (this->deck->blocks.empty())
        throw std::logic_error("Trying to iterate empty container");

    const auto& block = this->deck->blocks[this->file_index];
    if (this->keyword_index < block.size() - 1)
        this->keyword_index++;
    else {
        this->file_index++;
        this->keyword_index = 0;
    }
    return *this;
}

FileDeck::Index  FileDeck::Index::operator++(int) {
    auto current = *this;
    this->operator++();
    return current;
}

FileDeck::Index  FileDeck::Index::operator+(std::size_t shift) const {
    auto sum = *this;

    for (std::size_t arg = 0; arg < shift; arg++)
        sum++;

    return sum;
}


bool FileDeck::Index::operator==(const Index& other) const {
    return this->file_index == other.file_index &&
           this->keyword_index == other.keyword_index;
}

bool FileDeck::Index::operator!=(const Index& other) const {
    return !(*this == other);
}

bool FileDeck::Index::operator<(const Index& other) const {
    if (this->file_index < other.file_index)
        return true;

    if (this->file_index == other.file_index)
        return (this->keyword_index < other.keyword_index);

    return false;
}

FileDeck::FileDeck(const Deck& deck)
    : input_directory(deck.getInputPath())
    , deck_tree(deck.tree())
{
    if (deck.empty())
        return;

    std::size_t deck_index = 0;
    while (true) {
        const auto& current_file = deck[deck_index].location().filename;

        FileDeck::Block block(current_file);
        block.load(deck, deck_index);
        deck_index += block.size();
        this->blocks.push_back(std::move(block));

        if (deck_index >= deck.size())
            break;
    }

    this->modified_files.insert(this->blocks[0].fname);
}

const DeckKeyword& FileDeck::operator[](const Index& index) const {
    const auto& file_block = this->blocks.at(index.file_index);
    return file_block.keywords.at(index.keyword_index);
}



void FileDeck::FileDeck::Block::load(const Deck& deck, std::size_t deck_index) {
    const auto& current_file = deck[deck_index].location().filename;
    while (true) {
        this->keywords.push_back(deck[deck_index]);
        deck_index += 1;

        if (deck_index >= deck.size())
            break;

        if (deck[deck_index].location().filename != current_file)
            break;
    }
}

std::size_t FileDeck::FileDeck::Block::size() const {
    return this->keywords.size();
}

std::optional<std::size_t> FileDeck::Block::find(const std::string& keyword, std::size_t keyword_index) const {
    auto iter = std::find_if(this->keywords.begin() + keyword_index, this->keywords.end(), [&keyword](const DeckKeyword& kw) { return kw.name() == keyword;});
    if (iter == this->keywords.end())
        return {};

    return std::distance(this->keywords.begin(), iter);
}



void FileDeck::erase(const FileDeck::Index& index) {
    auto& block = this->blocks.at(index.file_index);
    this->modified_files.insert(block.fname);
    block.erase(index);
}

void FileDeck::erase(const Index& begin, const Index& end) {
    auto current = end;
    while (current != begin) {
        current--;
        this->erase(current);
    }
}

bool FileDeck::Block::empty() const {
    return this->keywords.empty();
}

void FileDeck::Block::erase(const FileDeck::Index& index) {
    if (index.keyword_index >= this->keywords.size())
        throw std::logic_error("Invalid keyword index in block");

    this->keywords.erase(this->keywords.begin() + index.keyword_index);
}

void FileDeck::Block::insert(std::size_t keyword_index, const DeckKeyword& keyword) {
    this->keywords.insert(this->keywords.begin() + keyword_index, keyword);
}

void FileDeck::Block::dump(DeckOutput& out) const {
    for (const auto& kw : this->keywords) {
        kw.write( out );
        out.write_string( out.fmt.keyword_sep );
    }
}


FileDeck::FileDeck::Block::Block(const std::string& filename)
    : fname(fs::canonical(filename).string())
{}

std::optional<FileDeck::Index> FileDeck::find(const std::string& keyword, const Index& offset) const {
    std::size_t file_index = offset.file_index;
    std::size_t keyword_index = offset.keyword_index;

    while (true) {
        if (file_index >= this->blocks.size())
            break;

        const auto& file_block = this->blocks[file_index];
        const auto& block_index = file_block.find(keyword, keyword_index);
        if (block_index.has_value())
            return std::make_optional<FileDeck::Index>(file_index, block_index.value(), this);

        file_index++;
        keyword_index = 0;
    }

    return {};
}

std::optional<FileDeck::Index> FileDeck::find(const std::string& keyword) const {
    return this->find(keyword, this->start());
}

std::size_t FileDeck::count(const std::string& keyword) const {
    std::size_t c = 0;
    auto index = this->start();
    while (index != this->stop()) {
        const auto& deck_kw = this->operator[](index);
        if (deck_kw.name() == keyword)
            c += 1;

        index++;
    }
    return c;
}


void FileDeck::insert(const Index& index, const DeckKeyword& keyword)
{
    auto& block = this->blocks.at(index.file_index);
    block.insert(index.keyword_index, keyword);

    this->modified_files.insert(block.fname);
}


const FileDeck::Index FileDeck::start() const {
    return FileDeck::Index{0,0, this};
}

const FileDeck::Index FileDeck::stop() const {
    return FileDeck::Index{this->blocks.size(), 0 , this};
}

void FileDeck::dump(std::ostream& os) const {
    DeckOutput out( os , 10 );
    for (const auto& block : this->blocks)
        block.dump(out);
}


void FileDeck::dump_inline() const {
    this->dump(std::cout);
}


std::string FileDeck::dump_block(const FileDeck::Block& block, const std::string& output_dir, const std::optional<std::string>& data_file , FileDeck::DumpContext& context) const {
    const auto& deck_name = block.fname;
    auto old_stream = context.get_stream(deck_name);
    if (old_stream.has_value()) {
        DeckOutput out(*old_stream.value(), 10);
        block.dump( out );
        return "";
    }


    fs::path output_file;
    if (data_file.has_value())
        output_file = fs::path(output_dir) / data_file.value();
    else {
        // Should ideally use fs::relative()
        auto rel_path = fs::proximate(block.fname, this->input_directory);
        output_file = output_dir / rel_path;
    }

    touch_file(output_file);
    output_file = fs::canonical(output_file);

    auto& stream = context.open_file(deck_name, output_file);
    DeckOutput out(stream, 10);
    block.dump( out );
    return output_file.string();
}


void FileDeck::include_block(const std::string& input_file, const std::string& output_file, const std::string& output_dir, FileDeck::DumpContext& context) const {
    auto current_file = input_file;
    while (true) {
        const auto& parent = this->deck_tree.parent(current_file);
        auto stream = context.get_stream(parent);
        if (stream.has_value()) {
            // Should ideally use fs::relative()
            std::string include_file = fs::proximate(output_file, output_dir).string();
            INCLUDE(*stream.value(), include_file);
            break;
        }
        current_file = parent;
    }
}


void FileDeck::dump(const std::string& output_dir, const std::string& fname, OutputMode mode) const {
    if (!fs::is_directory(output_dir))
        fs::create_directories(output_dir);

    auto output_cwd = fs::path(output_dir);
    if (mode == OutputMode::INLINE) {
        std::ofstream os(output_cwd / fname);
        this->dump(os);
        return;
    }


    if (mode == OutputMode::COPY) {
        DumpContext context;
        this->dump_block(this->blocks[0], output_dir, fname, context);

        for (std::size_t block_index = 1; block_index < this->blocks.size(); block_index++) {
            const auto& block = this->blocks[block_index];
            const auto& include_file = this->dump_block(block, output_dir, {}, context);
            if (block.fname != this->deck_tree.root())
                this->include_block(block.fname, include_file, output_dir, context);
        }
    }


    if (mode == OutputMode::SHARE) {
       std::ofstream stream{output_cwd / fname};
       this->dump_shared(stream, output_dir);
    }
}


void FileDeck::dump_shared(std::ostream& stream, const std::string& output_dir) const {
    for (std::size_t block_index = 0; block_index < this->blocks.size(); block_index++) {
        const auto& block = this->blocks[block_index];
        if (block_index == 0 || this->modified_files.count(block.fname) > 0 || this->deck_tree.has_include(block.fname)) {
            DeckOutput out(stream, 10);
            block.dump( out );
        } else {
            // Should ideally use fs::relative()
            std::string include_file = fs::proximate(block.fname, output_dir).string();
            if (include_file.find(block.fname) == std::string::npos)
                INCLUDE(stream, include_file);
            else
                INCLUDE(stream, block.fname);
        }
    }
}


void FileDeck::dump_stdout(const std::string& output_dir, OutputMode mode) const {
    if (mode == OutputMode::COPY)
        throw std::logic_error("dump to stdout can not be combined outputmode COPY");

    if (mode == OutputMode::INLINE)
        this->dump_inline();
    else if (mode == OutputMode::SHARE)
        this->dump_shared(std::cout, output_dir);
}


void FileDeck::rst_solution(const std::string& rst_base, int report_step) {
    auto index = this->find("SOLUTION").value();
    auto summary_index = this->find("SUMMARY").value();

    index++;
    {
        while (true) {
            const auto& keyword = this->operator[](index);
            if (FileDeck::rst_keep_in_solution.count(keyword.name()) == 0) {
                this->erase(index);
                summary_index--;
            } else
                index++;

            if (index == summary_index)
                break;
        }
    }

    {
        Opm::UnitSystem units;
        std::vector<DeckValue> values{ DeckValue{rst_base}, DeckValue{report_step} };
        DeckKeyword restart(ParserKeywords::RESTART{}, std::vector<std::vector<DeckValue>>{ values }, units, units);

        auto solution = this->find("SOLUTION").value();
        this->insert(++solution, restart);
    }
}

void FileDeck::insert_skiprest() {
    DeckKeyword skiprest( ParserKeywords::SKIPREST{} );
    const auto schedule = this->find("SCHEDULE");
    auto index = schedule.value();
    this->insert(++index, skiprest);
}


void FileDeck::skip(int report_step) {
    int current_report = 0;
    const auto& schedule = this->find("SCHEDULE");
    auto deck_pos = schedule.value();
    while (true) {
        const auto& deck_keyword = this->operator[](deck_pos);

        if (deck_keyword.name() == "DATES")
            current_report += deck_keyword.size();
        else if (deck_keyword.name() == "TSTEP")
            current_report += deck_keyword[0].getItem<ParserKeywords::TSTEP::step_list>().data_size();

        if (current_report >= report_step)
            break;

        deck_pos++;
        if (deck_pos == this->stop())
            throw std::logic_error(fmt::format("Could not find DATES keyword corresponding to report_step {}", report_step));
    }

    auto index = schedule.value() + 1;
    auto end_pos = deck_pos;
    while (index < end_pos) {
        const auto& keyword = this->operator[](index);
        if (FileDeck::rst_keep_in_schedule.count(keyword.name()) == 0) {
            this->erase(index);
            end_pos--;
        } else
            index++;
    }

    if (current_report == report_step)
        this->erase(end_pos);
    else {
        auto deck_keyword = this->operator[](end_pos);
        this->erase(end_pos);
        std::vector<std::vector<DeckValue>> records;

        using D = ParserKeywords::DATES;
        current_report -= deck_keyword.size();
        for (size_t record_index = report_step - current_report; record_index < deck_keyword.size(); record_index++) {
            const auto& record = deck_keyword[record_index];
            records.push_back( {DeckValue{record.getItem<D::DAY>().get<int>(0)},
                                DeckValue{record.getItem<D::MONTH>().get<std::string>(0)},
                                DeckValue{record.getItem<D::YEAR>().get<int>(0)} });
        }

        UnitSystem unit_system;
        this->insert(end_pos, DeckKeyword{ParserKeywords::DATES{}, records, unit_system, unit_system});
    }
}



const std::unordered_set<std::string> FileDeck::rst_keep_in_solution = {"RPTRST"};
const std::unordered_set<std::string> FileDeck::rst_keep_in_schedule = {"VFPPROD", "VFPINJ", "RPTSCHED", "RPTRST", "TUNING", "MESSAGES"};

}
