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
#include <opm/parser/eclipse/EclipseState/Tables/PmiscTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TlpmixpaTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvdgTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvdoTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvdsTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvtgTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvtoTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RocktabTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RsvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RtempvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/RvvdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgcwmisTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgwfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SlgofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/Sof2Table.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/Sof3Table.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SorwmisTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SsfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SwfnTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SwofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableContainer.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/WatvisctTable.hpp>

namespace Opm {

PvtgTable::PvtgTable( const DeckKeyword& keyword, size_t tableIdx ) :
    PvtxTable("P") {

        m_saturatedSchema = std::make_shared< TableSchema >();
        m_underSaturatedSchema = std::make_shared< TableSchema >();

        m_underSaturatedSchema->addColumn( ColumnSchema( "RV"  , Table::STRICTLY_DECREASING , Table::DEFAULT_NONE ));
        m_underSaturatedSchema->addColumn( ColumnSchema( "BG"  , Table::RANDOM , Table::DEFAULT_LINEAR ));
        m_underSaturatedSchema->addColumn( ColumnSchema( "MUG" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        m_saturatedSchema = std::make_shared< TableSchema >( );
        m_saturatedSchema->addColumn( ColumnSchema( "PG"  , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
        m_saturatedSchema->addColumn( ColumnSchema( "RV"  , Table::RANDOM , Table::DEFAULT_NONE ));
        m_saturatedSchema->addColumn( ColumnSchema( "BG"  , Table::RANDOM , Table::DEFAULT_LINEAR ));
        m_saturatedSchema->addColumn( ColumnSchema( "MUG" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        PvtxTable::init(keyword, tableIdx);
    }

PvtoTable::PvtoTable( const DeckKeyword& keyword, size_t tableIdx) :
    PvtxTable("P") {

        m_saturatedSchema = std::make_shared< TableSchema >();
        m_underSaturatedSchema = std::make_shared< TableSchema >();

        m_underSaturatedSchema->addColumn( ColumnSchema( "P"  , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
        m_underSaturatedSchema->addColumn( ColumnSchema( "BO" , Table::RANDOM , Table::DEFAULT_LINEAR ));
        m_underSaturatedSchema->addColumn( ColumnSchema( "MU" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        m_saturatedSchema->addColumn( ColumnSchema( "RS" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
        m_saturatedSchema->addColumn( ColumnSchema( "P"  , Table::RANDOM , Table::DEFAULT_NONE ));
        m_saturatedSchema->addColumn( ColumnSchema( "BO" , Table::RANDOM , Table::DEFAULT_LINEAR ));
        m_saturatedSchema->addColumn( ColumnSchema( "MU" , Table::RANDOM , Table::DEFAULT_LINEAR ));

        PvtxTable::init(keyword , tableIdx);
    }

SwofTable::SwofTable( const DeckItem& item ) {

    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "SW"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema( "KRW"  , Table::RANDOM              , Table::DEFAULT_LINEAR) );
    m_schema->addColumn( ColumnSchema( "KROW" , Table::RANDOM              , Table::DEFAULT_LINEAR) );
    m_schema->addColumn( ColumnSchema( "PCOW" , Table::RANDOM              , Table::DEFAULT_LINEAR) );

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
    return SimpleTable::getColumn(3);
}

SgwfnTable::SgwfnTable( const DeckItem& item ) {

    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "SG"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema->addColumn( ColumnSchema( "KRG"  , Table::RANDOM ,              Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KRGW" , Table::RANDOM ,              Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "PCGW" , Table::RANDOM ,              Table::DEFAULT_LINEAR ) );

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

SgofTable::SgofTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();
    m_schema->addColumn( ColumnSchema("SG"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema->addColumn( ColumnSchema("KRG"  , Table::RANDOM              , Table::DEFAULT_LINEAR ));
    m_schema->addColumn( ColumnSchema("KROG" , Table::RANDOM              , Table::DEFAULT_LINEAR ));
    m_schema->addColumn( ColumnSchema("PCOG" , Table::RANDOM              , Table::DEFAULT_LINEAR ));

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
    return SimpleTable::getColumn(3);

}

SlgofTable::SlgofTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema("SL"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema->addColumn( ColumnSchema("KRG"  , Table::DECREASING , Table::DEFAULT_LINEAR ));
    m_schema->addColumn( ColumnSchema("KROG" , Table::INCREASING , Table::DEFAULT_LINEAR ));
    m_schema->addColumn( ColumnSchema("PCOG" , Table::DECREASING , Table::DEFAULT_LINEAR ));

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
    return SimpleTable::getColumn(3);
}

Sof2Table::Sof2Table( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "SO"  , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema->addColumn( ColumnSchema( "KRO" , Table::INCREASING , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}

const TableColumn& Sof2Table::getSoColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& Sof2Table::getKroColumn() const {
    return SimpleTable::getColumn(1);
}

Sof3Table::Sof3Table( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema("SO" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema->addColumn( ColumnSchema("KROW" , Table::INCREASING , Table::DEFAULT_LINEAR ));
    m_schema->addColumn( ColumnSchema("KROG" , Table::INCREASING , Table::DEFAULT_LINEAR ));

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
    m_schema = std::make_shared< TableSchema >();
    m_schema->addColumn( ColumnSchema( "P"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema->addColumn( ColumnSchema( "BG"  , Table::STRICTLY_DECREASING , Table::DEFAULT_LINEAR));
    m_schema->addColumn( ColumnSchema( "MUG" , Table::INCREASING          , Table::DEFAULT_LINEAR ));

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
    m_schema = std::make_shared< TableSchema >();
    m_schema->addColumn( ColumnSchema( "P"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema->addColumn( ColumnSchema( "BO"  , Table::STRICTLY_DECREASING , Table::DEFAULT_LINEAR));
    m_schema->addColumn( ColumnSchema( "MUO" , Table::INCREASING          , Table::DEFAULT_LINEAR ));

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

SwfnTable::SwfnTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema("SW"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema->addColumn( ColumnSchema("KRW"  , Table::INCREASING , Table::DEFAULT_LINEAR ));
    m_schema->addColumn( ColumnSchema("PCOW" , Table::DECREASING , Table::DEFAULT_LINEAR ));

    SimpleTable::init(item);
}

const TableColumn& SwfnTable::getSwColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& SwfnTable::getKrwColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& SwfnTable::getPcowColumn() const {
    return SimpleTable::getColumn(2);
}

SgfnTable::SgfnTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema("SG"  , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema->addColumn( ColumnSchema("KRG" , Table::INCREASING , Table::DEFAULT_LINEAR));
    m_schema->addColumn( ColumnSchema("PCOG" , Table::INCREASING , Table::DEFAULT_LINEAR));

    SimpleTable::init(item);
}


const TableColumn& SgfnTable::getSgColumn() const { 
    return SimpleTable::getColumn(0);
}

const TableColumn& SgfnTable::getKrgColumn() const {
    return SimpleTable::getColumn(1);
}

const TableColumn& SgfnTable::getPcogColumn() const {
    return SimpleTable::getColumn(2);
}

SsfnTable::SsfnTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema("SolventFraction" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema->addColumn( ColumnSchema("GasRelPermMultiplier" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema->addColumn( ColumnSchema("SolventRelPermMultiplier" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));

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
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "P"   , Table::STRICTLY_INCREASING  , Table::DEFAULT_NONE ));
    m_schema->addColumn( ColumnSchema( "BG"  , Table::STRICTLY_DECREASING , Table::DEFAULT_LINEAR ));
    m_schema->addColumn( ColumnSchema( "MUG" , Table::INCREASING  , Table::DEFAULT_LINEAR ));

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
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema("PolymerConcentration" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema->addColumn( ColumnSchema("AdsorbedPolymer" , Table::INCREASING , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}


const TableColumn& PlyadsTable::getPolymerConcentrationColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PlyadsTable::getAdsorbedPolymerColumn() const {
    return SimpleTable::getColumn(1);
}

PlymaxTable::PlymaxTable( const DeckRecord& record ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema("C_POLYMER",     Table::RANDOM , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema("C_POLYMER_MAX", Table::RANDOM , Table::DEFAULT_NONE) );

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
    m_schema = std::make_shared< TableSchema >();
    m_schema->addColumn( ColumnSchema("DeadPoreVolume",            Table::RANDOM , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema("ResidualResistanceFactor",  Table::RANDOM , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema("RockDensityFactor",         Table::RANDOM , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema("AdsorbtionIndex",           Table::RANDOM , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema("MaxAdsorbtion",             Table::RANDOM , Table::DEFAULT_NONE) );

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
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "PolymerConcentration" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema->addColumn( ColumnSchema( "ViscosityMultiplier" , Table::INCREASING , Table::DEFAULT_NONE));
    SimpleTable::init(item);
}

const TableColumn& PlyviscTable::getPolymerConcentrationColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PlyviscTable::getViscosityMultiplierColumn() const {
    return SimpleTable::getColumn(1);
}

PlydhflfTable::PlydhflfTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema("Temperature" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema->addColumn( ColumnSchema("PolymerHalflife" , Table::STRICTLY_DECREASING , Table::DEFAULT_NONE ) );

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
    m_schema = std::make_shared< TableSchema >();

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

    m_schema->addColumn( ColumnSchema("WaterVelocity"   , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema->addColumn( ColumnSchema("ShearMultiplier" , Table::RANDOM , Table::DEFAULT_NONE));

    SimpleTable::init( dataRecord.getItem<ParserKeywords::PLYSHLOG::DATA>() );
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

OilvisctTable::OilvisctTable( const DeckItem& item ) {

    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema("Temperature" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema("Viscosity" , Table::DECREASING , Table::DEFAULT_NONE) );
    SimpleTable::init(item);
}

const TableColumn& OilvisctTable::getTemperatureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& OilvisctTable::getOilViscosityColumn() const {
    return SimpleTable::getColumn(1);
}

WatvisctTable::WatvisctTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn(ColumnSchema("Temperature" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    m_schema->addColumn(ColumnSchema("Viscosity"   , Table::DECREASING , Table::DEFAULT_NONE));

    SimpleTable::init(item);
}

const TableColumn& WatvisctTable::getTemperatureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& WatvisctTable::getWaterViscosityColumn() const {
    return SimpleTable::getColumn(1);
} 

GasvisctTable::GasvisctTable( const Deck& deck, const DeckItem& deckItem ) {
    m_schema = std::make_shared< TableSchema >();

    int numComponents = deck.getKeyword<ParserKeywords::COMPS>().getRecord(0).getItem(0).get< int >(0);

    auto temperatureDimension = deck.getActiveUnitSystem().getDimension("Temperature");
    auto viscosityDimension = deck.getActiveUnitSystem().getDimension("Viscosity");

    m_schema->addColumn( ColumnSchema( "Temperature" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE));
    for (int compIdx = 0; compIdx < numComponents; ++ compIdx) {
        std::string columnName = "Viscosity" + std::to_string(compIdx);
        m_schema->addColumn( ColumnSchema( columnName, Table::INCREASING , Table::DEFAULT_NONE ) );
    }

    SimpleTable::addColumns();

    if ( deckItem.size() % numColumns() != 0)
        throw std::runtime_error("Number of columns in the data file is inconsistent "
                "with the expected number for keyword GASVISCT");

        size_t rows = deckItem.size() / m_schema->size();
        for (size_t columnIndex=0; columnIndex < m_schema->size(); columnIndex++) {
            auto& column = getColumn( columnIndex );
            for (size_t rowIdx = 0; rowIdx < rows; rowIdx++) {
                size_t deckIndex = rowIdx * m_schema->size() + columnIndex;

                if (deckItem.defaultApplied( deckIndex ))
                    column.addDefault();
                else {
                    double rawValue = deckItem.get< double >( deckIndex );
                    double SIValue;

                    if (columnIndex == 0)
                        SIValue = temperatureDimension->convertRawToSi(rawValue);
                    else
                        SIValue = viscosityDimension->convertRawToSi(rawValue);

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
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "Depth"       , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema( "Temperature" , Table::RANDOM , Table::DEFAULT_NONE) );

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

    m_schema = std::make_shared< TableSchema >();

    Table::ColumnOrderEnum POOrder;

    if (hasStressOption)
        POOrder = Table::STRICTLY_INCREASING;
    else
        POOrder = Table::STRICTLY_DECREASING;

    m_schema->addColumn( ColumnSchema("PO"      , POOrder      , Table::DEFAULT_NONE ) );
    m_schema->addColumn( ColumnSchema("PV_MULT" , Table::RANDOM , Table::DEFAULT_LINEAR ) );

    if (isDirectional) {
        m_schema->addColumn( ColumnSchema("PV_MULT_TRANX" , Table::RANDOM , Table::DEFAULT_LINEAR ) );
        m_schema->addColumn( ColumnSchema("PV_MULT_TRANY" , Table::RANDOM , Table::DEFAULT_LINEAR ) );
        m_schema->addColumn( ColumnSchema("PV_MULT_TRANZ" , Table::RANDOM , Table::DEFAULT_LINEAR ) );
    } else
        m_schema->addColumn( ColumnSchema("PV_MULT_TRAN" , Table::RANDOM , Table::DEFAULT_LINEAR ) );

    SimpleTable::init(item);
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

RsvdTable::RsvdTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "DEPTH" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ));
    m_schema->addColumn( ColumnSchema( "RS" , Table::RANDOM , Table::DEFAULT_NONE ));

    SimpleTable::init(item);
}

const TableColumn& RsvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& RsvdTable::getRsColumn() const {
    return SimpleTable::getColumn(1); 
}

RvvdTable::RvvdTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "DEPTH" ,  Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema->addColumn( ColumnSchema( "RV"    ,  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    SimpleTable::init(item);
}

const TableColumn& RvvdTable::getDepthColumn() const {
    return SimpleTable::getColumn(0); 
}

const TableColumn& RvvdTable::getRvColumn() const {
    return SimpleTable::getColumn(1);
}

EnkrvdTable::EnkrvdTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "DEPTH" ,  Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema->addColumn( ColumnSchema( "KRWMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KRGMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KROMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KRWCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KRGCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KROCRITG",Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KROCRITW",Table::RANDOM , Table::DEFAULT_LINEAR ) );

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
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "DEPTH" ,  Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema->addColumn( ColumnSchema( "SWCO",    Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SWCRIT",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SWMAX",   Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SGCO",    Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SGCRIT",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SGMAX",   Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SOWCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SOGCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );

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
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "DEPTH" ,  Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema->addColumn( ColumnSchema( "KRWMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KRGMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KROMAX",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KRWCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KRGCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KROCRITG",Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "KROCRITW", Table::RANDOM , Table::DEFAULT_LINEAR ) );

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
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "DEPTH" ,  Table::STRICTLY_INCREASING , Table::DEFAULT_NONE ) );
    m_schema->addColumn( ColumnSchema( "SWCO",    Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SWCRIT",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SWMAX",   Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SGCO",    Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SGCRIT",  Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SGMAX",   Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SOWCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );
    m_schema->addColumn( ColumnSchema( "SOGCRIT", Table::RANDOM , Table::DEFAULT_LINEAR ) );

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
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "WaterSaturation" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema( "MiscibleResidualOilSaturation" , Table::INCREASING , Table::DEFAULT_NONE) );

    SimpleTable::init(item);
}

const TableColumn& SorwmisTable::getWaterSaturationColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& SorwmisTable::getMiscibleResidualOilColumn() const {
    return SimpleTable::getColumn(1);
}

SgcwmisTable::SgcwmisTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "WaterSaturation" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema( "MiscibleResidualGasSaturation" , Table::INCREASING , Table::DEFAULT_NONE) );
    SimpleTable::init(item);
}

const TableColumn& SgcwmisTable::getWaterSaturationColumn() const {
    return SimpleTable::getColumn(0); 
}

const TableColumn& SgcwmisTable::getMiscibleResidualGasColumn() const {
    return SimpleTable::getColumn(1);
}

MiscTable::MiscTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "SolventFraction" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema( "Miscibility" , Table::INCREASING , Table::DEFAULT_NONE) );
    SimpleTable::init( item );
}

const TableColumn& MiscTable::getSolventFractionColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& MiscTable::getMiscibilityColumn() const {
    return SimpleTable::getColumn(1); 
}


PmiscTable::PmiscTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "OilPhasePressure" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema( "Miscibility" , Table::INCREASING , Table::DEFAULT_NONE) );
    SimpleTable::init( item );
}

const TableColumn& PmiscTable::getOilPhasePressureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& PmiscTable::getMiscibilityColumn() const {
    return SimpleTable::getColumn(1);
}

TlpmixpaTable::TlpmixpaTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "OilPhasePressure" , Table::STRICTLY_INCREASING , Table::DEFAULT_NONE) );
    m_schema->addColumn( ColumnSchema( "Miscibility" , Table::INCREASING , Table::DEFAULT_NONE) );
    SimpleTable::init( item );
}

const TableColumn& TlpmixpaTable::getOilPhasePressureColumn() const {
    return SimpleTable::getColumn(0);
}

const TableColumn& TlpmixpaTable::getMiscibilityColumn() const {
    return SimpleTable::getColumn(1);
}

MsfnTable::MsfnTable( const DeckItem& item ) {
    m_schema = std::make_shared< TableSchema >();

    m_schema->addColumn( ColumnSchema( "GasPhaseFraction", Table::STRICTLY_INCREASING  , Table::DEFAULT_NONE));
    m_schema->addColumn( ColumnSchema( "GasSolventRelpermMultiplier", Table::INCREASING  , Table::DEFAULT_NONE));
    m_schema->addColumn( ColumnSchema( "OilRelpermMultiplier", Table::DECREASING  , Table::DEFAULT_NONE));

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

} // namespace Opm
