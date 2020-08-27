/*
   Copyright (C) 2013 by Andreas Lauser

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

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/ColumnSchema.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableSchema.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/C.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/V.hpp>
#include <opm/parser/eclipse/Units/Dimension.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include <opm/parser/eclipse/EclipseState/Tables/FlatTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/EnkrvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/EnptvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/GasvisctTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/ImkrvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/ImptvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/MiscTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/MsfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/OilvisctTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlyadsTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlydhflfTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlymaxTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlyrockTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlyshlogTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlyviscTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/FoamadsTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/FoammobTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PmiscTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TlpmixpaTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvdgTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvdoTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvdsTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvtgTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvtgwTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvtgwoTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvtoTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RocktabTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RockwnodTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/OverburdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RsvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RtempvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RvvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PbvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PdvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PermfactTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SaltvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SaltpvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgcwmisTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgwfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SlgofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/Sof2Table.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/Sof3Table.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SorwmisTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SpecheatTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SpecrockTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SsfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SwfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SwofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableContainer.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TracerVdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/WatvisctTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/AqutabTable.hpp>

namespace Opm {

PvtgTable::PvtgTable( const DeckKeyword& keyword, size_t tableIdx ) :
    PvtxTable("P") {

        m_underSaturatedSchema.addColumn( ColumnSchema( "RV"  , Table::STRICTLY_DECREASING , Table::DEFAULT_NONE ));
        m_underSaturatedSchema.addColumn( ColumnSchema( "BG"  , Table::RANDOM , Table::DEFAULT_LINEAR ));
        m_underSaturatedSchema.addColumn( ColumnSchema( "MUG" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        m_saturatedSchema.addColumn( ColumnSchema( "PG"  , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
        m_saturatedSchema.addColumn( ColumnSchema( "RV"  , Table::RANDOM , Table::DEFAULT_NONE ));
        m_saturatedSchema.addColumn( ColumnSchema( "BG"  , Table::RANDOM , Table::DEFAULT_LINEAR ));
        m_saturatedSchema.addColumn( ColumnSchema( "MUG" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        PvtxTable::init(keyword, tableIdx);
    }

PvtgTable PvtgTable::serializeObject() {
    PvtgTable result;
    static_cast<PvtxTable&>(result) = PvtxTable::serializeObject();

    return result;
}

bool PvtgTable::operator==(const PvtgTable& data) const {
    return static_cast<const PvtxTable&>(*this) == static_cast<const PvtxTable&>(data);
}

PvtgwTable::PvtgwTable( const DeckKeyword& keyword, size_t tableIdx ) :
    PvtxTable("P") {

        m_underSaturatedSchema.addColumn( ColumnSchema( "RW"  , Table::STRICTLY_DECREASING , Table::DEFAULT_NONE ));
        m_underSaturatedSchema.addColumn( ColumnSchema( "BG"  , Table::RANDOM , Table::DEFAULT_LINEAR ));
        m_underSaturatedSchema.addColumn( ColumnSchema( "MUG" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        m_saturatedSchema.addColumn( ColumnSchema( "PG"  , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
        m_saturatedSchema.addColumn( ColumnSchema( "RW"  , Table::RANDOM , Table::DEFAULT_NONE ));
        m_saturatedSchema.addColumn( ColumnSchema( "BG"  , Table::RANDOM , Table::DEFAULT_LINEAR ));
        m_saturatedSchema.addColumn( ColumnSchema( "MUG" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        PvtxTable::init(keyword, tableIdx);
    }

PvtgwTable PvtgwTable::serializeObject() {
    PvtgwTable result;
    static_cast<PvtxTable&>(result) = PvtxTable::serializeObject();

    return result;
}

bool PvtgwTable::operator==(const PvtgwTable& data) const {
    return static_cast<const PvtxTable&>(*this) == static_cast<const PvtxTable&>(data);
}

PvtgwoTable::PvtgwoTable( const DeckKeyword& keyword, size_t tableIdx ) :
    PvtxTable("P") {

        m_underSaturatedSchema.addColumn( ColumnSchema( "RV"  , Table::STRICTLY_DECREASING , Table::DEFAULT_NONE ));
        m_underSaturatedSchema.addColumn( ColumnSchema( "RW"  , Table::STRICTLY_DECREASING , Table::DEFAULT_NONE ));
        m_underSaturatedSchema.addColumn( ColumnSchema( "BG"  , Table::RANDOM , Table::DEFAULT_LINEAR ));
        m_underSaturatedSchema.addColumn( ColumnSchema( "MUG" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        m_saturatedSchema.addColumn( ColumnSchema( "PG"  , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
        m_saturatedSchema.addColumn( ColumnSchema( "RV"  , Table::RANDOM , Table::DEFAULT_NONE ));
        m_saturatedSchema.addColumn( ColumnSchema( "RW"  , Table::RANDOM , Table::DEFAULT_NONE ));
        m_saturatedSchema.addColumn( ColumnSchema( "BG"  , Table::RANDOM , Table::DEFAULT_LINEAR ));
        m_saturatedSchema.addColumn( ColumnSchema( "MUG" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        PvtxTable::init(keyword, tableIdx);
    }

PvtgwoTable PvtgwoTable::serializeObject() {
    PvtgwoTable result;
    static_cast<PvtxTable&>(result) = PvtxTable::serializeObject();

    return result;
}

bool PvtgwoTable::operator==(const PvtgwoTable& data) const {
    return static_cast<const PvtxTable&>(*this) == static_cast<const PvtxTable&>(data);
}

PvtoTable::PvtoTable( const DeckKeyword& keyword, size_t tableIdx) :
    PvtxTable("RS") {
        m_underSaturatedSchema.addColumn( ColumnSchema( "P"  , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
        m_underSaturatedSchema.addColumn( ColumnSchema( "BO" , Table::STRICTLY_DECREASING, Table::DEFAULT_LINEAR ));
        m_underSaturatedSchema.addColumn( ColumnSchema( "MU" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        m_saturatedSchema.addColumn( ColumnSchema( "RS" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
        m_saturatedSchema.addColumn( ColumnSchema( "P"  , Table::RANDOM , Table::DEFAULT_NONE ));
        m_saturatedSchema.addColumn( ColumnSchema( "BO" , Table::STRICTLY_INCREASING, Table::DEFAULT_LINEAR ));
        m_saturatedSchema.addColumn( ColumnSchema( "MU" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        PvtxTable::init(keyword , tableIdx);
    }

PvtoTable PvtoTable::serializeObject() {
    PvtoTable result;
    static_cast<PvtxTable&>(result) = PvtxTable::serializeObject();

    return result;
}

bool PvtoTable::operator==(const PvtoTable& data) const {
    return static_cast<const PvtxTable&>(*this) == static_cast<const PvtxTable&>(data);
}


SpecheatTable::SpecheatTable(const DeckItem& item)
{
    m_schema.addColumn(ColumnSchema("TEMPERATURE", Table::STRICTLY_INCREASING, Table::DEFAULT_NONE));
    m_schema.addColumn(ColumnSchema("CV_OIL", Table::RANDOM, Table::DEFAULT_LINEAR));
    m_schema.addColumn(ColumnSchema("CV_WATER", Table::RANDOM, Table::DEFAULT_LINEAR));
    m_schema.addColumn(ColumnSchema("CV_GAS", Table::RANDOM, Table::DEFAULT_LINEAR));

    SimpleTable::init(item);
}

const TableColumn& SpecheatTable::getTemperatureColumn() const
{ return SimpleTable::getColumn(0); }

const TableColumn& SpecheatTable::getCvOilColumn() const
{ return SimpleTable::getColumn(1); }

const TableColumn& SpecheatTable::getCvWaterColumn() const
{ return SimpleTable::getColumn(2); }

const TableColumn& SpecheatTable::getCvGasColumn() const
{ return SimpleTable::getColumn(3); }

SpecrockTable::SpecrockTable(const DeckItem& item)
{
    m_schema.addColumn(ColumnSchema("TEMPERATURE", Table::STRICTLY_INCREASING, Table::DEFAULT_NONE));
    m_schema.addColumn(ColumnSchema("CV_ROCK", Table::RANDOM, Table::DEFAULT_LINEAR));

    SimpleTable::init(item);
}

const TableColumn& SpecrockTable::getTemperatureColumn() const
{ return SimpleTable::getColumn(0); }

const TableColumn& SpecrockTable::getCvRockColumn() const
{ return SimpleTable::getColumn(1); }

SwofTable::SwofTable( const DeckItem& item , const bool jfunc) {

    m_schema.addColumn( ColumnSchema( "SW"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema( "KRW"  , Table::RANDOM              , Table::DEFAULT_LINEAR) );
    m_schema.addColumn( ColumnSchema( "KROW" , Table::RANDOM              , Table::DEFAULT_LINEAR) );
    m_schema.addColumn( ColumnSchema( "PCOW" , Table::RANDOM              , Table::DEFAULT_LINEAR) );

    m_jfunc = jfunc;
    SimpleTable::init( item );
}

const TableColumn& SwofTable::getSwColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& SwofTable::getKrwColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& SwofTable::getKrowColumn() const {
    return SimpleTable::getColumn(2);
}

const TableColumn& SwofTable::getPcowColumn() const {
    SimpleTable::assertJFuncPressure(false);
    return SimpleTable::getColumn(3);
}

const TableColumn& SwofTable::getJFuncColumn() const {
    SimpleTable::assertJFuncPressure(true);
    return SimpleTable::getColumn(3);
}

SgwfnTable::SgwfnTable( const DeckItem& item ) {

    m_schema.addColumn( ColumnSchema( "SG"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema.addColumn( ColumnSchema( "KRG"  , Table::RANDOM ,              Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KRGW" , Table::RANDOM ,              Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "PCGW" , Table::RANDOM ,              Table::DEFAULT_LINEAR ) );

    SimpleTable::init( item );
}

const TableColumn& SgwfnTable::getSgColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& SgwfnTable::getKrgColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& SgwfnTable::getKrgwColumn() const {
    return SimpleTable::getColumn(2);
}

const TableColumn& SgwfnTable::getPcgwColumn() const {
    return SimpleTable::getColumn(3); 
}

SgofTable::SgofTable( const DeckItem& item , const bool jfunc) {
    m_schema.addColumn( ColumnSchema("SG"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema.addColumn( ColumnSchema("KRG"  , Table::RANDOM              , Table::DEFAULT_LINEAR ));
    m_schema.addColumn( ColumnSchema("KROG" , Table::RANDOM              , Table::DEFAULT_LINEAR ));
    m_schema.addColumn( ColumnSchema("PCOG" , Table::RANDOM              , Table::DEFAULT_LINEAR ));

    m_jfunc = jfunc;
    SimpleTable::init( item );
}

const TableColumn& SgofTable::getSgColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& SgofTable::getKrgColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& SgofTable::getKrogColumn() const {
    return SimpleTable::getColumn(2);
}

const TableColumn& SgofTable::getPcogColumn() const {
    SimpleTable::assertJFuncPressure(false);
    return SimpleTable::getColumn(3);
}

const TableColumn& SgofTable::getJFuncColumn() const {
    SimpleTable::assertJFuncPressure(true);
    return SimpleTable::getColumn(3);
}

SlgofTable::SlgofTable( const DeckItem& item, const bool jfunc ) {
    m_schema.addColumn( ColumnSchema("SL"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema("KRG"  , Table::DECREASING , Table::DEFAULT_LINEAR ));
    m_schema.addColumn( ColumnSchema("KROG" , Table::INCREASING , Table::DEFAULT_LINEAR ));
    m_schema.addColumn( ColumnSchema("PCOG" , Table::DECREASING , Table::DEFAULT_LINEAR ));

    m_jfunc = jfunc;
    SimpleTable::init( item );

    if (getSlColumn().back() != 1.0) {
        throw std::invalid_argument("The last saturation of the SLGOF keyword must be 1!");
    }
}

const TableColumn& SlgofTable::getSlColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& SlgofTable::getKrgColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& SlgofTable::getKrogColumn() const {
    return SimpleTable::getColumn(2);
}

const TableColumn& SlgofTable::getPcogColumn() const {
    SimpleTable::assertJFuncPressure(false);
    return SimpleTable::getColumn(3);
}

const TableColumn& SlgofTable::getJFuncColumn() const {
    SimpleTable::assertJFuncPressure(true);
    return SimpleTable::getColumn(3);
}

Sof2Table::Sof2Table( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "SO"  , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema( "KRO" , Table::INCREASING , Table::DEFAULT_LINEAR));

    SimpleTable::init(item);
}

const TableColumn& Sof2Table::getSoColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& Sof2Table::getKroColumn() const {
    return SimpleTable::getColumn(1);
}

Sof3Table::Sof3Table( const DeckItem& item ) {

    m_schema.addColumn( ColumnSchema("SO" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema("KROW" , Table::INCREASING , Table::DEFAULT_LINEAR ));
    m_schema.addColumn( ColumnSchema("KROG" , Table::INCREASING , Table::DEFAULT_LINEAR ));

    SimpleTable::init( item );
}


const TableColumn& Sof3Table::getSoColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& Sof3Table::getKrowColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& Sof3Table::getKrogColumn() const {
    return SimpleTable::getColumn(2);
}

PvdgTable::PvdgTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "P"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema( "BG"  , Table::STRICTLY_DECREASING , Table::DEFAULT_LINEAR));
    m_schema.addColumn( ColumnSchema( "MUG" , Table::INCREASING          , Table::DEFAULT_LINEAR ));

    SimpleTable::init( item );
}


const TableColumn& PvdgTable::getPressureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PvdgTable::getFormationFactorColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& PvdgTable::getViscosityColumn() const {
    return SimpleTable::getColumn(2);
}

PvdoTable::PvdoTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "P"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema( "BO"  , Table::STRICTLY_DECREASING , Table::DEFAULT_LINEAR));
    m_schema.addColumn( ColumnSchema( "MUO" , Table::INCREASING          , Table::DEFAULT_LINEAR ));

    SimpleTable::init( item );
}


const TableColumn& PvdoTable::getPressureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PvdoTable::getFormationFactorColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& PvdoTable::getViscosityColumn() const {
    return SimpleTable::getColumn(2);
}

SwfnTable::SwfnTable( const DeckItem& item, const bool jfunc ) {
    m_schema.addColumn( ColumnSchema("SW"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema("KRW"  , Table::INCREASING , Table::DEFAULT_LINEAR ));
    m_schema.addColumn( ColumnSchema("PCOW" , Table::DECREASING , Table::DEFAULT_LINEAR ));

    m_jfunc = jfunc;
    SimpleTable::init( item );
}

const TableColumn& SwfnTable::getSwColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& SwfnTable::getKrwColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& SwfnTable::getPcowColumn() const {
    SimpleTable::assertJFuncPressure(false);
    return SimpleTable::getColumn(2);
}

const TableColumn& SwfnTable::getJFuncColumn() const {
    SimpleTable::assertJFuncPressure(true);
    return SimpleTable::getColumn(2);
}


SgfnTable::SgfnTable( const DeckItem& item, const bool jfunc ) {
    m_schema.addColumn( ColumnSchema("SG"  , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema.addColumn( ColumnSchema("KRG" , Table::INCREASING , Table::DEFAULT_LINEAR));
    m_schema.addColumn( ColumnSchema("PCOG" , Table::INCREASING , Table::DEFAULT_LINEAR));

    m_jfunc = jfunc;
    SimpleTable::init( item );
}


const TableColumn& SgfnTable::getSgColumn() const { 
    return SimpleTable::getColumn(0);
}

const TableColumn& SgfnTable::getKrgColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& SgfnTable::getPcogColumn() const {
    SimpleTable::assertJFuncPressure(false);
    return SimpleTable::getColumn(2);
}

const TableColumn& SgfnTable::getJFuncColumn() const {
    SimpleTable::assertJFuncPressure(true);
    return SimpleTable::getColumn(2);
}

SsfnTable::SsfnTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema("SolventFraction" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema.addColumn( ColumnSchema("GasRelPermMultiplier" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema.addColumn( ColumnSchema("SolventRelPermMultiplier" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));

    SimpleTable::init(item);
}

const TableColumn& SsfnTable::getSolventFractionColumn() const { 
    return SimpleTable::getColumn(0);
}

const TableColumn& SsfnTable::getGasRelPermMultiplierColumn() const { 
    return SimpleTable::getColumn(1);
}

const TableColumn& SsfnTable::getSolventRelPermMultiplierColumn() const { 
    return SimpleTable::getColumn(2);

}

PvdsTable::PvdsTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "P"   , Table::STRICTLY_INCREASING  , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema( "BG"  , Table::STRICTLY_DECREASING , Table::DEFAULT_LINEAR ));
    m_schema.addColumn( ColumnSchema( "MUG" , Table::INCREASING  , Table::DEFAULT_LINEAR ));

    SimpleTable::init(item);
}

const TableColumn& PvdsTable::getPressureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PvdsTable::getFormationFactorColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& PvdsTable::getViscosityColumn() const {
    return SimpleTable::getColumn(2);
}

PlyadsTable::PlyadsTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema("PolymerConcentration" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema("AdsorbedPolymer" , Table::INCREASING , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}


const TableColumn& PlyadsTable::getPolymerConcentrationColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PlyadsTable::getAdsorbedPolymerColumn() const {
    return SimpleTable::getColumn(1);
}

FoamadsTable::FoamadsTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema("FoamConcentration" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema("AdsorbedFoam" , Table::INCREASING , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}

const TableColumn& FoamadsTable::getFoamConcentrationColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& FoamadsTable::getAdsorbedFoamColumn() const {
    return SimpleTable::getColumn(1);
}

FoammobTable::FoammobTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema("FoamConcentration" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema("MobilityMultiplier" , Table::DECREASING , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}

const TableColumn& FoammobTable::getFoamConcentrationColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& FoammobTable::getMobilityMultiplierColumn() const {
    return SimpleTable::getColumn(1);
}

PlymaxTable::PlymaxTable( const DeckRecord& record ) {
    m_schema.addColumn( ColumnSchema("C_POLYMER",     Table::RANDOM , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema("C_POLYMER_MAX", Table::RANDOM , Table::DEFAULT_NONE) );

    addColumns();
    for (size_t colIdx = 0; colIdx < record.size(); colIdx++) {
        auto& column = getColumn( colIdx );

        column.addValue( record.getItem( colIdx ).getSIDouble(0) );
    }
}

const TableColumn& PlymaxTable::getPolymerConcentrationColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PlymaxTable::getMaxPolymerConcentrationColumn() const {
    return SimpleTable::getColumn(1);
}

PlyrockTable::PlyrockTable( const DeckRecord& record ) {
    m_schema.addColumn( ColumnSchema("DeadPoreVolume",            Table::RANDOM , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema("ResidualResistanceFactor",  Table::RANDOM , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema("RockDensityFactor",         Table::RANDOM , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema("AdsorbtionIndex",           Table::RANDOM , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema("MaxAdsorbtion",             Table::RANDOM , Table::DEFAULT_NONE) );

    addColumns();
    for (size_t colIdx = 0; colIdx < record.size(); colIdx++) {
        auto& column = getColumn( colIdx );

        column.addValue( record.getItem( colIdx ).getSIDouble(0) );
    }
}

const TableColumn& PlyrockTable::getDeadPoreVolumeColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PlyrockTable::getResidualResistanceFactorColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& PlyrockTable::getRockDensityFactorColumn() const {
    return SimpleTable::getColumn(2);
}

const TableColumn& PlyrockTable::getAdsorbtionIndexColumn() const {
    return SimpleTable::getColumn(3);
}

const TableColumn& PlyrockTable::getMaxAdsorbtionColumn() const {
    return SimpleTable::getColumn(4);
}

PlyviscTable::PlyviscTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "PolymerConcentration" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema.addColumn( ColumnSchema( "ViscosityMultiplier" , Table::INCREASING , Table::DEFAULT_NONE));
    SimpleTable::init(item);
}

const TableColumn& PlyviscTable::getPolymerConcentrationColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PlyviscTable::getViscosityMultiplierColumn() const {
    return SimpleTable::getColumn(1);
}

PlydhflfTable::PlydhflfTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema("Temperature" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema.addColumn( ColumnSchema("PolymerHalflife" , Table::STRICTLY_DECREASING , Table::DEFAULT_NONE ) );

    SimpleTable::init(item);
}

const TableColumn& PlydhflfTable::getTemperatureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PlydhflfTable::getPolymerHalflifeColumn() const {
    return SimpleTable::getColumn(1);
}

PlyshlogTable::PlyshlogTable(
        const DeckRecord& indexRecord,
        const DeckRecord& dataRecord ) {

    {
        const auto& item = indexRecord.getItem<ParserKeywords::PLYSHLOG::REF_POLYMER_CONCENTRATION>();
        setRefPolymerConcentration(item.get< double >(0));
    }

    {
        const auto& item = indexRecord.getItem<ParserKeywords::PLYSHLOG::REF_SALINITY>();
        if (item.hasValue(0)) {
            setHasRefSalinity(true);
            setRefSalinity(item.get< double >(0));
        } else
            setHasRefSalinity(false);
    }

    {
        const auto& item = indexRecord.getItem<ParserKeywords::PLYSHLOG::REF_TEMPERATURE>();
        if (item.hasValue(0)) {
            setHasRefTemperature(true);
            setRefTemperature(item.get< double >(0));
        } else
            setHasRefTemperature(false);
    }

    m_schema.addColumn( ColumnSchema("WaterVelocity"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema.addColumn( ColumnSchema("ShearMultiplier" , Table::RANDOM , Table::DEFAULT_NONE));

    SimpleTable::init( dataRecord.getItem<ParserKeywords::PLYSHLOG::DATA>() );
}

PlyshlogTable PlyshlogTable::serializeObject()
{
    PlyshlogTable result;
    static_cast<SimpleTable&>(result) = SimpleTable::serializeObject();
    result.m_refPolymerConcentration = 1.0;
    result.m_refSalinity = 2.0;
    result.m_refTemperature = 3.0;
    result.m_hasRefSalinity = true;
    result.m_hasRefTemperature = true;

    return result;
}

double PlyshlogTable::getRefPolymerConcentration() const {
    return m_refPolymerConcentration;
}
double PlyshlogTable::getRefSalinity() const {
    return m_refSalinity;
}

double PlyshlogTable::getRefTemperature() const{
    return m_refTemperature;
}

void PlyshlogTable::setRefPolymerConcentration(const double refPlymerConcentration) {
    m_refPolymerConcentration = refPlymerConcentration;
}

void PlyshlogTable::setRefSalinity(const double refSalinity) {
    m_refSalinity = refSalinity;
}

void PlyshlogTable::setRefTemperature(const double refTemperature) {
    m_refTemperature = refTemperature;
}

bool PlyshlogTable::hasRefSalinity() const {
    return m_hasRefSalinity;
}

bool PlyshlogTable::hasRefTemperature() const {
    return m_hasRefTemperature;
}

void PlyshlogTable::setHasRefSalinity(const bool has) {
    m_hasRefSalinity = has;
}

void PlyshlogTable::setHasRefTemperature(const bool has) {
    m_refTemperature = has;
}

const TableColumn& PlyshlogTable::getWaterVelocityColumn() const {
    return getColumn(0);
}

const TableColumn& PlyshlogTable::getShearMultiplierColumn() const {
    return getColumn(1);
}

bool PlyshlogTable::operator==(const PlyshlogTable& data) const {
    return this->SimpleTable::operator==(data) &&
           m_refPolymerConcentration == data.m_refPolymerConcentration &&
           m_refSalinity == data.m_refSalinity &&
           m_refTemperature == data.m_refTemperature &&
           m_hasRefSalinity == data.m_hasRefSalinity &&
           m_hasRefTemperature == data.m_hasRefTemperature;
}

OilvisctTable::OilvisctTable( const DeckItem& item ) {

    m_schema.addColumn( ColumnSchema("Temperature" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema("Viscosity" , Table::DECREASING , Table::DEFAULT_NONE) );
    SimpleTable::init(item);
}

const TableColumn& OilvisctTable::getTemperatureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& OilvisctTable::getOilViscosityColumn() const {
    return SimpleTable::getColumn(1);
}

WatvisctTable::WatvisctTable( const DeckItem& item ) {
    m_schema.addColumn(ColumnSchema("Temperature" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema.addColumn(ColumnSchema("Viscosity"   , Table::DECREASING , Table::DEFAULT_NONE));

    SimpleTable::init(item);
}

const TableColumn& WatvisctTable::getTemperatureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& WatvisctTable::getWaterViscosityColumn() const {
    return SimpleTable::getColumn(1);
} 

GasvisctTable::GasvisctTable( const Deck& deck, const DeckItem& deckItem ) {
    int numComponents = deck.getKeyword<ParserKeywords::COMPS>().getRecord(0).getItem(0).get< int >(0);

    auto temperatureDimension = deck.getActiveUnitSystem().getDimension("Temperature");
    auto viscosityDimension = deck.getActiveUnitSystem().getDimension("Viscosity");

    m_schema.addColumn( ColumnSchema( "Temperature" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    for (int compIdx = 0; compIdx < numComponents; ++ compIdx) {
        std::string columnName = "Viscosity" + std::to_string(compIdx);
        m_schema.addColumn( ColumnSchema( columnName, Table::INCREASING , Table::DEFAULT_NONE ) );
    }

    SimpleTable::addColumns();

    if ( deckItem.data_size() % numColumns() != 0)
        throw std::runtime_error("Number of columns in the data file is inconsistent "
                                 "with the expected number for keyword GASVISCT");

    size_t rows = deckItem.data_size() / m_schema.size();
    for (size_t columnIndex=0; columnIndex < m_schema.size(); columnIndex++) {
        auto& column = getColumn( columnIndex );
        for (size_t rowIdx = 0; rowIdx < rows; rowIdx++) {
            size_t deckIndex = rowIdx * m_schema.size() + columnIndex;

            if (deckItem.defaultApplied( deckIndex ))
                column.addDefault();
            else {
                double rawValue = deckItem.get< double >( deckIndex );
                double SIValue;

                if (columnIndex == 0)
                    SIValue = temperatureDimension.convertRawToSi(rawValue);
                else
                    SIValue = viscosityDimension.convertRawToSi(rawValue);

                column.addValue( SIValue );
            }
        }
    }
}

const TableColumn& GasvisctTable::getTemperatureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& GasvisctTable::getGasViscosityColumn(size_t compIdx) const {
    return SimpleTable::getColumn(1 + compIdx);
}

RtempvdTable::RtempvdTable(const DeckItem& item) {
    m_schema.addColumn( ColumnSchema( "Depth"       , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema( "Temperature" , Table::RANDOM , Table::DEFAULT_NONE) );

    SimpleTable::init( item );
}

const TableColumn& RtempvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0);
}


const TableColumn& RtempvdTable::getTemperatureColumn() const {
    return SimpleTable::getColumn(1);
}

RocktabTable::RocktabTable(
        const DeckItem& item,
        bool isDirectional,
        bool hasStressOption ) :
    m_isDirectional( isDirectional )
{

    Table::ColumnOrderEnum POOrder;

    if (hasStressOption)
        POOrder = Table::STRICTLY_INCREASING;
    else
        POOrder = Table::STRICTLY_DECREASING;

    m_schema.addColumn( ColumnSchema("PO"      , POOrder      , Table::DEFAULT_NONE ) );
    m_schema.addColumn( ColumnSchema("PV_MULT" , Table::RANDOM , Table::DEFAULT_LINEAR ) );

    if (isDirectional) {
        m_schema.addColumn( ColumnSchema("PV_MULT_TRANX" , Table::RANDOM , Table::DEFAULT_LINEAR ) );
        m_schema.addColumn( ColumnSchema("PV_MULT_TRANY" , Table::RANDOM , Table::DEFAULT_LINEAR ) );
        m_schema.addColumn( ColumnSchema("PV_MULT_TRANZ" , Table::RANDOM , Table::DEFAULT_LINEAR ) );
    } else
        m_schema.addColumn( ColumnSchema("PV_MULT_TRAN" , Table::RANDOM , Table::DEFAULT_LINEAR ) );

    SimpleTable::init(item);
}

RocktabTable RocktabTable::serializeObject()
{
    RocktabTable result;
    static_cast<SimpleTable&>(result) = Opm::SimpleTable::serializeObject();
    result.m_isDirectional = true;

    return result;
}

const TableColumn& RocktabTable::getPressureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& RocktabTable::getPoreVolumeMultiplierColumn() const {
    return SimpleTable::getColumn(1); 
}

const TableColumn& RocktabTable::getTransmissibilityMultiplierColumn() const {
    return SimpleTable::getColumn(2); 
}

const TableColumn& RocktabTable::getTransmissibilityMultiplierXColumn() const {
    return SimpleTable::getColumn(2); 
}

const TableColumn& RocktabTable::getTransmissibilityMultiplierYColumn() const {
    if (!m_isDirectional)
        return SimpleTable::getColumn(2);
    return SimpleTable::getColumn(3);
}

const TableColumn& RocktabTable::getTransmissibilityMultiplierZColumn() const {
    if (!m_isDirectional)
        return SimpleTable::getColumn(2);
    return SimpleTable::getColumn(4);
}

bool RocktabTable::operator==(const RocktabTable& data) const {
    return this->SimpleTable::operator==(data) &&
           m_isDirectional == data.m_isDirectional;
}

RsvdTable::RsvdTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "DEPTH" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema( "RS" , Table::RANDOM , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}

const TableColumn& RsvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& RsvdTable::getRsColumn() const {
    return SimpleTable::getColumn(1); 
}

RvvdTable::RvvdTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "DEPTH" ,  Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema.addColumn( ColumnSchema( "RV"    ,  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    SimpleTable::init(item);
}

const TableColumn& RvvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0); 
}

const TableColumn& RvvdTable::getRvColumn() const {
    return SimpleTable::getColumn(1);
}

PbvdTable::PbvdTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "DEPTH" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema( "PBUB" , Table::RANDOM , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}

const TableColumn& PbvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PbvdTable::getPbubColumn() const {
    return SimpleTable::getColumn(1);
}

PdvdTable::PdvdTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "DEPTH" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema( "PDEW" , Table::RANDOM , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}

const TableColumn& PdvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PdvdTable::getPdewColumn() const {
    return SimpleTable::getColumn(1);
}

SaltvdTable::SaltvdTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "DEPTH" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema( "SALT" , Table::RANDOM , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}

const TableColumn& SaltvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& SaltvdTable::getSaltColumn() const {
    return SimpleTable::getColumn(1); 
}

SaltpvdTable::SaltpvdTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "DEPTH" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema( "SALTP" , Table::RANDOM , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}

const TableColumn& SaltpvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& SaltpvdTable::getSaltpColumn() const {
    return SimpleTable::getColumn(1); 
}

PermfactTable::PermfactTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "POROSITYCHANGE" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema.addColumn( ColumnSchema( "PERMEABILIYMULTIPLIER" , Table::RANDOM , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}

const TableColumn& PermfactTable::getPorosityChangeColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PermfactTable::getPermeabilityMultiplierColumn() const {
    return SimpleTable::getColumn(1); 
}


AqutabTable::AqutabTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "TD" ,  Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema.addColumn( ColumnSchema( "PD"    ,  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    SimpleTable::init(item);
}

const TableColumn& AqutabTable::getTimeColumn() const {
    return SimpleTable::getColumn(0); 
}

const TableColumn& AqutabTable::getPressureColumn() const {
    return SimpleTable::getColumn(1);
}

EnkrvdTable::EnkrvdTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "DEPTH" ,  Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema.addColumn( ColumnSchema( "KRWMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KRGMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KROMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KRWCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KRGCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KROCRITG",Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KROCRITW",Table::RANDOM , Table::DEFAULT_LINEAR ) );

    SimpleTable::init(item);
}

const TableColumn& EnkrvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& EnkrvdTable::getKrwmaxColumn() const {
    return SimpleTable::getColumn(1); 
}

const TableColumn& EnkrvdTable::getKrgmaxColumn() const {
    return SimpleTable::getColumn(2);
}

const TableColumn& EnkrvdTable::getKromaxColumn() const {
    return SimpleTable::getColumn(3);
}

const TableColumn& EnkrvdTable::getKrwcritColumn() const {
    return SimpleTable::getColumn(4); 
}

const TableColumn& EnkrvdTable::getKrgcritColumn() const {
    return SimpleTable::getColumn(5);
}

const TableColumn& EnkrvdTable::getKrocritgColumn() const {
    return SimpleTable::getColumn(6);
}

const TableColumn& EnkrvdTable::getKrocritwColumn() const {
    return SimpleTable::getColumn(7);
}

EnptvdTable::EnptvdTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "DEPTH" ,  Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema.addColumn( ColumnSchema( "SWCO",    Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SWCRIT",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SWMAX",   Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SGCO",    Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SGCRIT",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SGMAX",   Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SOWCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SOGCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );

    SimpleTable::init(item);
}

const TableColumn& EnptvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0); 
}

const TableColumn& EnptvdTable::getSwcoColumn() const {
    return SimpleTable::getColumn(1); 
}

const TableColumn& EnptvdTable::getSwcritColumn() const {
    return SimpleTable::getColumn(2); 
}

const TableColumn& EnptvdTable::getSwmaxColumn() const {
    return SimpleTable::getColumn(3); 
}

const TableColumn& EnptvdTable::getSgcoColumn() const {
    return SimpleTable::getColumn(4); 
}

const TableColumn& EnptvdTable::getSgcritColumn() const {
    return SimpleTable::getColumn(5); 
}

const TableColumn& EnptvdTable::getSgmaxColumn() const {
    return SimpleTable::getColumn(6); 
}

const TableColumn& EnptvdTable::getSowcritColumn() const {
    return SimpleTable::getColumn(7); 
}

const TableColumn& EnptvdTable::getSogcritColumn() const {
    return SimpleTable::getColumn(8); 
}

ImkrvdTable::ImkrvdTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "DEPTH" ,  Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema.addColumn( ColumnSchema( "KRWMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KRGMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KROMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KRWCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KRGCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KROCRITG",Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "KROCRITW", Table::RANDOM , Table::DEFAULT_LINEAR ) );

    SimpleTable::init(item);
}

const TableColumn& ImkrvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0); 
}

const TableColumn& ImkrvdTable::getKrwmaxColumn() const {
    return SimpleTable::getColumn(1); 
}

const TableColumn& ImkrvdTable::getKrgmaxColumn() const {
    return SimpleTable::getColumn(2); 
}

const TableColumn& ImkrvdTable::getKromaxColumn() const {
    return SimpleTable::getColumn(3); 
}

const TableColumn& ImkrvdTable::getKrwcritColumn() const {
    return SimpleTable::getColumn(4); 
}

const TableColumn& ImkrvdTable::getKrgcritColumn() const {
    return SimpleTable::getColumn(5); 
}

const TableColumn& ImkrvdTable::getKrocritgColumn() const {
    return SimpleTable::getColumn(6); 
}

const TableColumn& ImkrvdTable::getKrocritwColumn() const {
    return SimpleTable::getColumn(7); 
}


ImptvdTable::ImptvdTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "DEPTH" ,  Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema.addColumn( ColumnSchema( "SWCO",    Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SWCRIT",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SWMAX",   Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SGCO",    Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SGCRIT",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SGMAX",   Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SOWCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema.addColumn( ColumnSchema( "SOGCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );

    SimpleTable::init(item);
}

const TableColumn& ImptvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0); 
}

const TableColumn& ImptvdTable::getSwcoColumn() const {
    return SimpleTable::getColumn(1); 
}

const TableColumn& ImptvdTable::getSwcritColumn() const {
    return SimpleTable::getColumn(2); 
}

const TableColumn& ImptvdTable::getSwmaxColumn() const {
    return SimpleTable::getColumn(3); 
}

const TableColumn& ImptvdTable::getSgcoColumn() const {
    return SimpleTable::getColumn(4); 
}

const TableColumn& ImptvdTable::getSgcritColumn() const {
    return SimpleTable::getColumn(5); 
}

const TableColumn& ImptvdTable::getSgmaxColumn() const {
    return SimpleTable::getColumn(6); 
}

const TableColumn& ImptvdTable::getSowcritColumn() const {
    return SimpleTable::getColumn(7); 
}

const TableColumn& ImptvdTable::getSogcritColumn() const {
    return SimpleTable::getColumn(8); 
}

SorwmisTable::SorwmisTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "WaterSaturation" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema( "MiscibleResidualOilSaturation" , Table::INCREASING , Table::DEFAULT_NONE) );

    SimpleTable::init(item);
}

const TableColumn& SorwmisTable::getWaterSaturationColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& SorwmisTable::getMiscibleResidualOilColumn() const {
    return SimpleTable::getColumn(1);
}

SgcwmisTable::SgcwmisTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "WaterSaturation" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema( "MiscibleResidualGasSaturation" , Table::INCREASING , Table::DEFAULT_NONE) );
    SimpleTable::init(item);
}

const TableColumn& SgcwmisTable::getWaterSaturationColumn() const {
    return SimpleTable::getColumn(0); 
}

const TableColumn& SgcwmisTable::getMiscibleResidualGasColumn() const {
    return SimpleTable::getColumn(1);
}

MiscTable::MiscTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "SolventFraction" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema( "Miscibility" , Table::INCREASING , Table::DEFAULT_NONE) );
    SimpleTable::init( item );
}

const TableColumn& MiscTable::getSolventFractionColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& MiscTable::getMiscibilityColumn() const {
    return SimpleTable::getColumn(1); 
}


PmiscTable::PmiscTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "OilPhasePressure" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema( "Miscibility" , Table::INCREASING , Table::DEFAULT_NONE) );
    SimpleTable::init( item );
}

const TableColumn& PmiscTable::getOilPhasePressureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PmiscTable::getMiscibilityColumn() const {
    return SimpleTable::getColumn(1);
}

TlpmixpaTable::TlpmixpaTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "OilPhasePressure" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema( "Miscibility" , Table::INCREASING , Table::DEFAULT_NONE) );
    SimpleTable::init( item );
}

const TableColumn& TlpmixpaTable::getOilPhasePressureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& TlpmixpaTable::getMiscibilityColumn() const {
    return SimpleTable::getColumn(1);
}

MsfnTable::MsfnTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "GasPhaseFraction", Table::STRICTLY_INCREASING  , Table::DEFAULT_NONE));
    m_schema.addColumn( ColumnSchema( "GasSolventRelpermMultiplier", Table::INCREASING  , Table::DEFAULT_NONE));
    m_schema.addColumn( ColumnSchema( "OilRelpermMultiplier", Table::DECREASING  , Table::DEFAULT_NONE));

    SimpleTable::init( item );

    getColumn("GasPhaseFraction").assertUnitRange();
}


const TableColumn& MsfnTable::getGasPhaseFractionColumn() const {
    return SimpleTable::getColumn(0); 
}

const TableColumn& MsfnTable::getGasSolventRelpermMultiplierColumn() const {
    return SimpleTable::getColumn(1); 
}

const TableColumn& MsfnTable::getOilRelpermMultiplierColumn() const {
    return SimpleTable::getColumn(2); 
}

RockwnodTable::RockwnodTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "Saturation" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    SimpleTable::init( item );
}

const TableColumn& RockwnodTable::getSaturationColumn() const {
    return SimpleTable::getColumn(0);
}

OverburdTable::OverburdTable( const DeckItem& item ) {
    m_schema.addColumn( ColumnSchema( "Depth" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema.addColumn( ColumnSchema( "OverburdenPressure" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    SimpleTable::init( item );
}

const TableColumn& OverburdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& OverburdTable::getOverburdenPressureColumn() const {
    return SimpleTable::getColumn(1);
}

TracerVdTable::TracerVdTable(const Opm::DeckItem& item, double inv_volume) {
    m_schema.addColumn(Opm::ColumnSchema("DEPTH", Table::STRICTLY_INCREASING, Table::DEFAULT_NONE));
    m_schema.addColumn(Opm::ColumnSchema("TRACER_CONCENTRATION", Table::RANDOM, Table::DEFAULT_NONE));
    SimpleTable::init(item, inv_volume);
}

const TableColumn& TracerVdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& TracerVdTable::getTracerConcentration() const {
    return SimpleTable::getColumn(1);
}

namespace {

/*
 * Create a compile-time sequence of integers [0,N). In C++14 this can be
 * replaced by std::index_sequence.
 */
template< std::size_t... > struct seq { using type = seq; };
template< std::size_t N, std::size_t... Is >
struct mkseq : mkseq< N - 1, N - 1, Is... > {};
template< std::size_t... Is >
struct mkseq< 0u, Is... > : seq< Is... >{ using type = seq< Is... >; };

/*
 * Convenince function for creating a 'flat table', e.g. PVTW and DENSITY.
 * Assumes the following:
 *
 * 1. The table has vector semantics with no other to enforce
 * 2. That the following struct is implemented:
 * struct record {
 *  static constexpr std::size_t size = [number-of-members]
 *  double members ...;
 * };
 * 3. The table is declared as
 * struct table : public FlatTable< table > {
 *  using FlatTable< table >::FlatTable;
 * }
 *
 * If some field can *not* be defaulted, e.g. 0, specialise the flat_props
 * struct (in this namespace) as such:
 * template<> struct< record, 0 > {
 *  static constexpr bool can_default() { return false; }
 *  static constexpr const char* errmsg() { "error message"; }
 * };
 * and the parser will throw std::invalid_argument if the field is defaulted in
 * the input.
 *
 */

template< typename T, std::size_t N >
struct flat_props {
    static constexpr bool can_default() { return true; }
    static constexpr const char* errmsg() { return ""; }
};

template< typename T, std::size_t N >
double flat_get( const DeckRecord& rec ) {
    if( !flat_props< T, N >::can_default()
     && rec.getItem( N ).defaultApplied( 0 ) ) {
        throw std::invalid_argument( flat_props< T, N >::errmsg() );
    }

    return rec.getItem( N ).getSIDouble( 0 );
}

template< typename T, std::size_t... Is >
std::vector< T > flat_records( const DeckKeyword& kw, seq< Is... > ) {
    std::vector< T > xs;
    for( const auto& record : kw )
        xs.emplace_back( T { flat_get< T, Is >( record )... } );

    return xs;
}

template<>
struct flat_props< PVTWRecord, 0 > {
    static constexpr bool can_default() { return false; }
    static constexpr const char* errmsg() {
        return "PVTW reference pressure cannot be defaulted";
    }
};

constexpr const char* pvcdo_err[] = {
    "PVCDO reference pressure cannot be defaulted",
    "PVCDO oil volume factor cannot be defaulted",
    "PVCDO compressibility cannot be defaulted",
    "PVCDO viscosity cannot be defaulted",
    "PVCDO viscosibility cannot be defaulted",
};

template< std::size_t N >
struct flat_props< PVCDORecord, N > {
    static constexpr bool can_default() { return false; }
    static constexpr const char* errmsg() {
        return pvcdo_err[ N ];
    }
};

}

template< typename T >
FlatTable< T >::FlatTable( const DeckKeyword& kw ) :
    std::vector< T >( flat_records< T >( kw, mkseq< T::size >{} ) )
{}

template FlatTable< DENSITYRecord >::FlatTable( const DeckKeyword& );
template FlatTable< PVTWRecord >::FlatTable( const DeckKeyword& );
template FlatTable< PVCDORecord >::FlatTable( const DeckKeyword& );
template FlatTable< ROCKRecord >::FlatTable( const DeckKeyword& );
template FlatTable< PlyvmhRecord >::FlatTable( const DeckKeyword& );
template FlatTable< VISCREFRecord >::FlatTable( const DeckKeyword& );
template FlatTable< PlmixparRecord>::FlatTable( const DeckKeyword& );
template FlatTable< ShrateRecord >::FlatTable( const DeckKeyword& );
template FlatTable< Stone1exRecord >::FlatTable( const DeckKeyword& );
template FlatTable< TlmixparRecord>::FlatTable( const DeckKeyword& );
template FlatTable< WATDENTRecord >::FlatTable( const DeckKeyword& );

} // namespace Opm
