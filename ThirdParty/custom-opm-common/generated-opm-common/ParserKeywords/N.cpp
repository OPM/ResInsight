#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/N.hpp>
namespace Opm {
namespace ParserKeywords {
NARROW::NARROW( ) : ParserKeyword("NARROW")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("NARROW");
}
const std::string NARROW::keywordName = "NARROW";


NCONSUMP::NCONSUMP( ) : ParserKeyword("NCONSUMP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NCONSUMP");
  {
     ParserRecord record;
     {
        ParserItem item("NODE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GAS_CONSUMPTION_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("REMOVAL_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NCONSUMP::keywordName = "NCONSUMP";
const std::string NCONSUMP::NODE::itemName = "NODE";
const std::string NCONSUMP::GAS_CONSUMPTION_RATE::itemName = "GAS_CONSUMPTION_RATE";
const double NCONSUMP::GAS_CONSUMPTION_RATE::defaultValue = 0.000000;
const std::string NCONSUMP::REMOVAL_GROUP::itemName = "REMOVAL_GROUP";


NEFAC::NEFAC( ) : ParserKeyword("NEFAC")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NEFAC");
  {
     ParserRecord record;
     {
        ParserItem item("NODE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("EFF_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NEFAC::keywordName = "NEFAC";
const std::string NEFAC::NODE::itemName = "NODE";
const std::string NEFAC::EFF_FACTOR::itemName = "EFF_FACTOR";
const double NEFAC::EFF_FACTOR::defaultValue = 1.000000;


NETBALAN::NETBALAN( ) : ParserKeyword("NETBALAN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("NETBALAN");
  {
     ParserRecord record;
     {
        ParserItem item("TIME_INTERVAL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_CONVERGENCE_LIMT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-05) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ITER", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     {
        ParserItem item("THP_CONVERGENCE_LIMIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.010000) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ITER_THP", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     {
        ParserItem item("TARGET_BALANCE_ERROR", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_BALANCE_ERROR", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_TIME_STEP", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NETBALAN::keywordName = "NETBALAN";
const std::string NETBALAN::TIME_INTERVAL::itemName = "TIME_INTERVAL";
const double NETBALAN::TIME_INTERVAL::defaultValue = 0.000000;
const std::string NETBALAN::PRESSURE_CONVERGENCE_LIMT::itemName = "PRESSURE_CONVERGENCE_LIMT";
const double NETBALAN::PRESSURE_CONVERGENCE_LIMT::defaultValue = 0.000010;
const std::string NETBALAN::MAX_ITER::itemName = "MAX_ITER";
const int NETBALAN::MAX_ITER::defaultValue = 10;
const std::string NETBALAN::THP_CONVERGENCE_LIMIT::itemName = "THP_CONVERGENCE_LIMIT";
const double NETBALAN::THP_CONVERGENCE_LIMIT::defaultValue = 0.010000;
const std::string NETBALAN::MAX_ITER_THP::itemName = "MAX_ITER_THP";
const int NETBALAN::MAX_ITER_THP::defaultValue = 10;
const std::string NETBALAN::TARGET_BALANCE_ERROR::itemName = "TARGET_BALANCE_ERROR";
const double NETBALAN::TARGET_BALANCE_ERROR::defaultValue = 100000000000000000000.000000;
const std::string NETBALAN::MAX_BALANCE_ERROR::itemName = "MAX_BALANCE_ERROR";
const double NETBALAN::MAX_BALANCE_ERROR::defaultValue = 100000000000000000000.000000;
const std::string NETBALAN::MIN_TIME_STEP::itemName = "MIN_TIME_STEP";


NETCOMPA::NETCOMPA( ) : ParserKeyword("NETCOMPA")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NETCOMPA");
  {
     ParserRecord record;
     {
        ParserItem item("INLET", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OUTLET", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        item.setDefault( std::string("") );
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        item.setDefault( std::string("GAS") );
        record.addItem(item);
     }
     {
        ParserItem item("VFT_TABLE_NUM", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ALQ", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_CONSUMPTION_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("EXTRACTION_GROUP", ParserItem::itype::STRING);
        item.setDefault( std::string("") );
        record.addItem(item);
     }
     {
        ParserItem item("COMPRESSOR_TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NUM_COMPRESSION_LEVELS", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ALQ_LEVEL1", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("COMP_SWITCH_SEQ_NUM", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NETCOMPA::keywordName = "NETCOMPA";
const std::string NETCOMPA::INLET::itemName = "INLET";
const std::string NETCOMPA::OUTLET::itemName = "OUTLET";
const std::string NETCOMPA::GROUP::itemName = "GROUP";
const std::string NETCOMPA::GROUP::defaultValue = "";
const std::string NETCOMPA::PHASE::itemName = "PHASE";
const std::string NETCOMPA::PHASE::defaultValue = "GAS";
const std::string NETCOMPA::VFT_TABLE_NUM::itemName = "VFT_TABLE_NUM";
const int NETCOMPA::VFT_TABLE_NUM::defaultValue = 0;
const std::string NETCOMPA::ALQ::itemName = "ALQ";
const double NETCOMPA::ALQ::defaultValue = 0.000000;
const std::string NETCOMPA::GAS_CONSUMPTION_RATE::itemName = "GAS_CONSUMPTION_RATE";
const double NETCOMPA::GAS_CONSUMPTION_RATE::defaultValue = 0.000000;
const std::string NETCOMPA::EXTRACTION_GROUP::itemName = "EXTRACTION_GROUP";
const std::string NETCOMPA::EXTRACTION_GROUP::defaultValue = "";
const std::string NETCOMPA::COMPRESSOR_TYPE::itemName = "COMPRESSOR_TYPE";
const std::string NETCOMPA::NUM_COMPRESSION_LEVELS::itemName = "NUM_COMPRESSION_LEVELS";
const std::string NETCOMPA::ALQ_LEVEL1::itemName = "ALQ_LEVEL1";
const std::string NETCOMPA::COMP_SWITCH_SEQ_NUM::itemName = "COMP_SWITCH_SEQ_NUM";


NETWORK::NETWORK( ) : ParserKeyword("NETWORK")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NETWORK");
  {
     ParserRecord record;
     {
        ParserItem item("NODMAX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NBRMAX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NBCMAX", ParserItem::itype::INT);
        item.setDefault( 20 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NETWORK::keywordName = "NETWORK";
const std::string NETWORK::NODMAX::itemName = "NODMAX";
const std::string NETWORK::NBRMAX::itemName = "NBRMAX";
const std::string NETWORK::NBCMAX::itemName = "NBCMAX";
const int NETWORK::NBCMAX::defaultValue = 20;


NEWTRAN::NEWTRAN( ) : ParserKeyword("NEWTRAN")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NEWTRAN");
}
const std::string NEWTRAN::keywordName = "NEWTRAN";


NEXT::NEXT( ) : ParserKeyword("NEXT")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NEXT");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_STEP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("APPLY_TO_ALL", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NEXT::keywordName = "NEXT";
const std::string NEXT::MAX_STEP::itemName = "MAX_STEP";
const std::string NEXT::APPLY_TO_ALL::itemName = "APPLY_TO_ALL";
const std::string NEXT::APPLY_TO_ALL::defaultValue = "NO";


NEXTSTEP::NEXTSTEP( ) : ParserKeyword("NEXTSTEP")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NEXTSTEP");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_STEP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("APPLY_TO_ALL", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NEXTSTEP::keywordName = "NEXTSTEP";
const std::string NEXTSTEP::MAX_STEP::itemName = "MAX_STEP";
const std::string NEXTSTEP::APPLY_TO_ALL::itemName = "APPLY_TO_ALL";
const std::string NEXTSTEP::APPLY_TO_ALL::defaultValue = "NO";


NEXTSTPL::NEXTSTPL( ) : ParserKeyword("NEXTSTPL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NEXTSTPL");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_LENGTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("APPLY_TO_ALL", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NEXTSTPL::keywordName = "NEXTSTPL";
const std::string NEXTSTPL::MAX_LENGTH::itemName = "MAX_LENGTH";
const std::string NEXTSTPL::APPLY_TO_ALL::itemName = "APPLY_TO_ALL";
const std::string NEXTSTPL::APPLY_TO_ALL::defaultValue = "NO";


NINENUM::NINENUM( ) : ParserKeyword("NINENUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NINENUM");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::INT);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string NINENUM::keywordName = "NINENUM";
const std::string NINENUM::data::itemName = "data";


NINEPOIN::NINEPOIN( ) : ParserKeyword("NINEPOIN")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NINEPOIN");
}
const std::string NINEPOIN::keywordName = "NINEPOIN";


NMATOPTS::NMATOPTS( ) : ParserKeyword("NMATOPTS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NMATOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("GEOMETRY", ParserItem::itype::STRING);
        item.setDefault( std::string("LINEAR") );
        record.addItem(item);
     }
     {
        ParserItem item("FRACTION_PORE_VOL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("METHOD", ParserItem::itype::STRING);
        item.setDefault( std::string("FPORV") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NMATOPTS::keywordName = "NMATOPTS";
const std::string NMATOPTS::GEOMETRY::itemName = "GEOMETRY";
const std::string NMATOPTS::GEOMETRY::defaultValue = "LINEAR";
const std::string NMATOPTS::FRACTION_PORE_VOL::itemName = "FRACTION_PORE_VOL";
const double NMATOPTS::FRACTION_PORE_VOL::defaultValue = 0.100000;
const std::string NMATOPTS::METHOD::itemName = "METHOD";
const std::string NMATOPTS::METHOD::defaultValue = "FPORV";


NMATRIX::NMATRIX( ) : ParserKeyword("NMATRIX")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NMATRIX");
  {
     ParserRecord record;
     {
        ParserItem item("NUM_SUB_CELLS", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NMATRIX::keywordName = "NMATRIX";
const std::string NMATRIX::NUM_SUB_CELLS::itemName = "NUM_SUB_CELLS";


NNC::NNC( ) : ParserKeyword("NNC")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NNC");
  {
     ParserRecord record;
     {
        ParserItem item("I1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("I2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TRAN", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Transmissibility");
        record.addItem(item);
     }
     {
        ParserItem item("SIM_DEPENDENT1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("ContextDependent");
        record.addItem(item);
     }
     {
        ParserItem item("SIM_DEPENDENT2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("ContextDependent");
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_TABLE1", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_TABLE2", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("VE_FACE1", ParserItem::itype::STRING);
        item.setDefault( std::string("") );
        record.addItem(item);
     }
     {
        ParserItem item("VE_FACE2", ParserItem::itype::STRING);
        item.setDefault( std::string("") );
        record.addItem(item);
     }
     {
        ParserItem item("DIFFUSIVITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("SIM_DEPENDENT3", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("ContextDependent");
        record.addItem(item);
     }
     {
        ParserItem item("VDFLOW_AREA", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("VDFLOW_PERM", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Permeability");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NNC::keywordName = "NNC";
const std::string NNC::I1::itemName = "I1";
const std::string NNC::J1::itemName = "J1";
const std::string NNC::K1::itemName = "K1";
const std::string NNC::I2::itemName = "I2";
const std::string NNC::J2::itemName = "J2";
const std::string NNC::K2::itemName = "K2";
const std::string NNC::TRAN::itemName = "TRAN";
const double NNC::TRAN::defaultValue = 0.000000;
const std::string NNC::SIM_DEPENDENT1::itemName = "SIM_DEPENDENT1";
const double NNC::SIM_DEPENDENT1::defaultValue = 0.000000;
const std::string NNC::SIM_DEPENDENT2::itemName = "SIM_DEPENDENT2";
const double NNC::SIM_DEPENDENT2::defaultValue = 0.000000;
const std::string NNC::PRESSURE_TABLE1::itemName = "PRESSURE_TABLE1";
const int NNC::PRESSURE_TABLE1::defaultValue = 0;
const std::string NNC::PRESSURE_TABLE2::itemName = "PRESSURE_TABLE2";
const int NNC::PRESSURE_TABLE2::defaultValue = 0;
const std::string NNC::VE_FACE1::itemName = "VE_FACE1";
const std::string NNC::VE_FACE1::defaultValue = "";
const std::string NNC::VE_FACE2::itemName = "VE_FACE2";
const std::string NNC::VE_FACE2::defaultValue = "";
const std::string NNC::DIFFUSIVITY::itemName = "DIFFUSIVITY";
const double NNC::DIFFUSIVITY::defaultValue = 0.000000;
const std::string NNC::SIM_DEPENDENT3::itemName = "SIM_DEPENDENT3";
const double NNC::SIM_DEPENDENT3::defaultValue = 0.000000;
const std::string NNC::VDFLOW_AREA::itemName = "VDFLOW_AREA";
const double NNC::VDFLOW_AREA::defaultValue = 0.000000;
const std::string NNC::VDFLOW_PERM::itemName = "VDFLOW_PERM";
const double NNC::VDFLOW_PERM::defaultValue = 0.000000;


NNEWTF::NNEWTF( ) : ParserKeyword("NNEWTF")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NNEWTF");
  {
     ParserRecord record;
     {
        ParserItem item("NTHRBL", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NLNHBL", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NNEWTF::keywordName = "NNEWTF";
const std::string NNEWTF::NTHRBL::itemName = "NTHRBL";
const std::string NNEWTF::NLNHBL::itemName = "NLNHBL";


NOCASC::NOCASC( ) : ParserKeyword("NOCASC")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NOCASC");
}
const std::string NOCASC::keywordName = "NOCASC";


NODEPROP::NODEPROP( ) : ParserKeyword("NODEPROP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NODEPROP");
  {
     ParserRecord record;
     {
        ParserItem item("NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("AS_CHOKE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("CHOKE_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SOURCE_SINK_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NETWORK_VALUE_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("PROD") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NODEPROP::keywordName = "NODEPROP";
const std::string NODEPROP::NAME::itemName = "NAME";
const std::string NODEPROP::PRESSURE::itemName = "PRESSURE";
const std::string NODEPROP::AS_CHOKE::itemName = "AS_CHOKE";
const std::string NODEPROP::AS_CHOKE::defaultValue = "NO";
const std::string NODEPROP::CHOKE_GROUP::itemName = "CHOKE_GROUP";
const std::string NODEPROP::SOURCE_SINK_GROUP::itemName = "SOURCE_SINK_GROUP";
const std::string NODEPROP::NETWORK_VALUE_TYPE::itemName = "NETWORK_VALUE_TYPE";
const std::string NODEPROP::NETWORK_VALUE_TYPE::defaultValue = "PROD";


NODPPM::NODPPM( ) : ParserKeyword("NODPPM")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("GRID");
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NODPPM");
}
const std::string NODPPM::keywordName = "NODPPM";


NOECHO::NOECHO( ) : ParserKeyword("NOECHO")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("NOECHO");
}
const std::string NOECHO::keywordName = "NOECHO";


NOGGF::NOGGF( ) : ParserKeyword("NOGGF")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NOGGF");
}
const std::string NOGGF::keywordName = "NOGGF";


NOGRAV::NOGRAV( ) : ParserKeyword("NOGRAV")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NOGRAV");
}
const std::string NOGRAV::keywordName = "NOGRAV";


NOHMD::NOHMD( ) : ParserKeyword("NOHMD")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("NOHMD");
  {
     ParserRecord record;
     {
        ParserItem item("GRAD_PARAMS", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NOHMD::keywordName = "NOHMD";
const std::string NOHMD::GRAD_PARAMS::itemName = "GRAD_PARAMS";


NOHMO::NOHMO( ) : ParserKeyword("NOHMO")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("NOHMO");
  {
     ParserRecord record;
     {
        ParserItem item("GRAD_PARAMS", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NOHMO::keywordName = "NOHMO";
const std::string NOHMO::GRAD_PARAMS::itemName = "GRAD_PARAMS";


NOHYST::NOHYST( ) : ParserKeyword("NOHYST")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NOHYST");
}
const std::string NOHYST::keywordName = "NOHYST";


NOINSPEC::NOINSPEC( ) : ParserKeyword("NOINSPEC")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NOINSPEC");
}
const std::string NOINSPEC::keywordName = "NOINSPEC";


NOMONITO::NOMONITO( ) : ParserKeyword("NOMONITO")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("NOMONITO");
}
const std::string NOMONITO::keywordName = "NOMONITO";


NONNC::NONNC( ) : ParserKeyword("NONNC")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NONNC");
}
const std::string NONNC::keywordName = "NONNC";


NORSSPEC::NORSSPEC( ) : ParserKeyword("NORSSPEC")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NORSSPEC");
}
const std::string NORSSPEC::keywordName = "NORSSPEC";


NOSIM::NOSIM( ) : ParserKeyword("NOSIM")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NOSIM");
}
const std::string NOSIM::keywordName = "NOSIM";


NOWARN::NOWARN( ) : ParserKeyword("NOWARN")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("NOWARN");
}
const std::string NOWARN::keywordName = "NOWARN";


NOWARNEP::NOWARNEP( ) : ParserKeyword("NOWARNEP")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("NOWARNEP");
}
const std::string NOWARNEP::keywordName = "NOWARNEP";


NRSOUT::NRSOUT( ) : ParserKeyword("NRSOUT")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NRSOUT");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_NUM", ParserItem::itype::INT);
        item.setDefault( 3600 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NRSOUT::keywordName = "NRSOUT";
const std::string NRSOUT::MAX_NUM::itemName = "MAX_NUM";
const int NRSOUT::MAX_NUM::defaultValue = 3600;


NSTACK::NSTACK( ) : ParserKeyword("NSTACK")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NSTACK");
  {
     ParserRecord record;
     {
        ParserItem item("LINEAR_SOLVER_SIZE", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NSTACK::keywordName = "NSTACK";
const std::string NSTACK::LINEAR_SOLVER_SIZE::itemName = "LINEAR_SOLVER_SIZE";
const int NSTACK::LINEAR_SOLVER_SIZE::defaultValue = 10;


NTG::NTG( ) : ParserKeyword("NTG")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NTG");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string NTG::keywordName = "NTG";
const std::string NTG::data::itemName = "data";


NUMRES::NUMRES( ) : ParserKeyword("NUMRES")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NUMRES");
  {
     ParserRecord record;
     {
        ParserItem item("num", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NUMRES::keywordName = "NUMRES";
const std::string NUMRES::num::itemName = "num";
const int NUMRES::num::defaultValue = 1;


NUPCOL::NUPCOL( ) : ParserKeyword("NUPCOL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NUPCOL");
  {
     ParserRecord record;
     {
        ParserItem item("NUM_ITER", ParserItem::itype::INT);
        item.setDefault( 12 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NUPCOL::keywordName = "NUPCOL";
const std::string NUPCOL::NUM_ITER::itemName = "NUM_ITER";
const int NUPCOL::NUM_ITER::defaultValue = 12;


NWATREM::NWATREM( ) : ParserKeyword("NWATREM")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NWATREM");
  {
     ParserRecord record;
     {
        ParserItem item("NODE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WAX_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(10000000000000000159028911097599180468360808563945281389781327557747838772170381060813469985856815104.000000) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_FRAC_REMOVAL", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string NWATREM::keywordName = "NWATREM";
const std::string NWATREM::NODE::itemName = "NODE";
const std::string NWATREM::WAX_RATE::itemName = "WAX_RATE";
const double NWATREM::WAX_RATE::defaultValue = 10000000000000000159028911097599180468360808563945281389781327557747838772170381060813469985856815104.000000;
const std::string NWATREM::MAX_FRAC_REMOVAL::itemName = "MAX_FRAC_REMOVAL";
const double NWATREM::MAX_FRAC_REMOVAL::defaultValue = 1.000000;


NXFIN::NXFIN( ) : ParserKeyword("NXFIN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NXFIN");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::INT);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string NXFIN::keywordName = "NXFIN";
const std::string NXFIN::data::itemName = "data";


NYFIN::NYFIN( ) : ParserKeyword("NYFIN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NYFIN");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::INT);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string NYFIN::keywordName = "NYFIN";
const std::string NYFIN::data::itemName = "data";


NZFIN::NZFIN( ) : ParserKeyword("NZFIN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NZFIN");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::INT);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string NZFIN::keywordName = "NZFIN";
const std::string NZFIN::data::itemName = "data";


}
}
