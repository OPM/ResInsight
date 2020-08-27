#define BOOST_TEST_MODULE UDQ_Data

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Well.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>

#include <opm/output/eclipse/AggregateUDQData.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/output/eclipse/InteHEAD.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/DoubHEAD.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQInput.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQActive.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQParams.hpp>

//#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
//#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>

#include <opm/io/eclipse/OutputStream.hpp>

#include <stdexcept>
#include <utility>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace {
    
    Opm::Deck first_sim(std::string fname) {
        return Opm::Parser{}.parseFile(fname);        
    }
    /*
     Opm::UDQActive udq_active() {
      int update_count = 0;
      // construct record data for udq_active
      Opm::UDQParams params;
      Opm::UDQConfig conf(params);
      Opm::UDQActive udq_act;
      Opm::UDAValue uda1("WUOPRL");
      update_count += udq_act.update(conf, uda1, "PROD1", Opm::UDAControl::WCONPROD_ORAT);

      Opm::UDAValue uda2("WULPRL");
      update_count += udq_act.update(conf, uda2, "PROD1", Opm::UDAControl::WCONPROD_LRAT);
      Opm::UDAValue uda3("WUOPRU");
      update_count += udq_act.update(conf, uda3, "PROD2", Opm::UDAControl::WCONPROD_ORAT);
      Opm::UDAValue uda4("WULPRU");
      update_count += udq_act.update(conf, uda4, "PROD2", Opm::UDAControl::WCONPROD_LRAT);

      for (std::size_t index=0; index < udq_act.IUAD_size(); index++)
      {
          const auto & record = udq_act[index];
          auto ind = record.input_index;
          auto udq_key = record.udq;
          auto name = record.wgname;
          auto ctrl_type = record.control;
       }
      return udq_act;
    } 
    */
}

    Opm::SummaryState sum_state()
    {
        auto state = Opm::SummaryState{std::chrono::system_clock::now()};
        state.update_well_var("PROD1", "WUOPRL", 210.);
        state.update_well_var("PROD2", "WUOPRL", 211.);
        state.update_well_var("WINJ1", "WUOPRL", 212.);
        state.update_well_var("WINJ2", "WUOPRL", 213.);

        state.update_well_var("PROD1", "WULPRL", 230.);
        state.update_well_var("PROD2", "WULPRL", 231.);
        state.update_well_var("WINJ1", "WULPRL", 232.);
        state.update_well_var("WINJ2", "WULPRL", 233.);

        state.update_well_var("PROD1", "WUOPRU", 220.);
        state.update_well_var("PROD2", "WUOPRU", 221.);
        state.update_well_var("WINJ1", "WUOPRU", 222.);
        state.update_well_var("WINJ2", "WUOPRU", 223.);

        state.update_group_var("WGRP1", "GUOPRU", 360.);
        state.update_group_var("WGRP2", "GUOPRU", 361.);
        state.update_group_var("GRP1",  "GUOPRU", 362.);
        
        state.update_well_var("PROD1", "WULPRU", 160.);
        state.update_well_var("PROD2", "WULPRU", 161.);
        state.update_well_var("WINJ1", "WULPRU", 162.);
        state.update_well_var("WINJ2", "WULPRU", 163.);
        
        state.update("FULPR", 460.);

        return state;
    }


//int main(int argc, char* argv[])
struct SimulationCase
{
    explicit SimulationCase(const Opm::Deck& deck)
        : es   { deck }
        , grid { deck }
        , python { std::make_shared<Opm::Python>()}
        , sched{ deck, es, python }
    {}

    // Order requirement: 'es' must be declared/initialised before 'sched'.
    Opm::EclipseState es;
    Opm::EclipseGrid  grid;
    std::shared_ptr<Opm::Python> python;
    Opm::Schedule     sched;

};
    
BOOST_AUTO_TEST_SUITE(Aggregate_UDQ)



// test constructed UDQ restart data
BOOST_AUTO_TEST_CASE (Declared_UDQ_data)
{
    const auto simCase = SimulationCase{first_sim("UDQ_TEST_WCONPROD_IUAD-2.DATA")};
        
    Opm::EclipseState es = simCase.es;
    Opm::SummaryState st = sum_state();
    Opm::Schedule     sched = simCase.sched;
    Opm::EclipseGrid  grid = simCase.grid;
    const auto& ioConfig = es.getIOConfig();
    //const auto& restart = es.cfg().restart();

    // Report Step 1: 2008-10-10 --> 2011-01-20
    const auto rptStep = std::size_t{1};    
    
    std::string outputDir = "./";
    std::string baseName = "TEST_UDQRST";
    Opm::EclIO::OutputStream::Restart rstFile {
    Opm::EclIO::OutputStream::ResultSet { outputDir, baseName },
    rptStep,
    Opm::EclIO::OutputStream::Formatted { ioConfig.getFMTOUT() },
      Opm::EclIO::OutputStream::Unified   { ioConfig.getUNIFOUT() }
        };
    
    double secs_elapsed = 3.1536E07;
    const auto ih = Opm::RestartIO::Helpers::
        createInteHead(es, grid, sched, secs_elapsed,
                       rptStep, rptStep, rptStep);

    //set dummy value for next_step_size 
    const double next_step_size= 0.1;
    const auto dh = Opm::RestartIO::Helpers::createDoubHead(es, sched, rptStep, 
                                                            secs_elapsed, next_step_size); 
       
    const auto udqDims = Opm::RestartIO::Helpers::createUdqDims(sched, rptStep, ih);
    auto  udqData = Opm::RestartIO::Helpers::AggregateUDQData(udqDims);
    udqData.captureDeclaredUDQData(sched, rptStep, st, ih);
     
        rstFile.write("ZUDN", udqData.getZUDN());
        rstFile.write("ZUDL", udqData.getZUDL());
        rstFile.write("IUDQ", udqData.getIUDQ());
        rstFile.write("DUDF", udqData.getDUDF());
        rstFile.write("DUDG", udqData.getDUDG());
        rstFile.write("DUDW", udqData.getDUDW());
        rstFile.write("IUAD", udqData.getIUAD());
        rstFile.write("IUAP", udqData.getIUAP());
        rstFile.write("IGPH", udqData.getIGPH());
    
    {
        /*
        Check of InteHEAD and DoubHEAD data for UDQ variables
        
                INTEHEAD 

                UDQPARAM (1)  = - InteHead [267 ]

                ---------------------------------------------------------------------------------------------------------------------

                DOUBHEAD

                UDQPARAM (2)  =  Doubhead [212]
                UDQPARAM (3)  =  Doubhead [213]
                UDQPARAM (4)  =  Doubhead [214]

        */
        
        BOOST_CHECK_EQUAL(ih[267] ,       -1); 
        BOOST_CHECK_EQUAL(dh[212] ,  1.0E+20); 
        BOOST_CHECK_EQUAL(dh[213] ,      0.0); 
        BOOST_CHECK_EQUAL(dh[214] ,   1.0E-4); 

    }

    
    {
        /*
        IUDQ
        3- integers pr UDQ (line/quantity)

        Integer no 1 = type of UDQ (       0 - ASSIGN, UPDATE-OFF
                                           1-update+NEXT, 
                                           2 - DEFINE,  2- UPDATE-ON
                                           3 - units)

        Integer no 2 = -4    : used for  ASSIGN - numerical value
                       -4   : used for DEFINE
                       -1  : used for DEFINE MIN() function, SUM()  function, AVEA() function 
                       -4  : used for DEFINE MAX() - function - also used for SUM() function - must check on (-1 - value)
                        1  : used for UPDATE quantity

        Integer no 3 = sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)
                         (1 - based)

        NOTE: UPDATE - does not define a new quantity, only updates an alredy defined quantity! 
        */
        
        
        const auto& iUdq = udqData.getIUDQ();
        
        auto start = 0*udqDims[1];
        BOOST_CHECK_EQUAL(iUdq[start + 0] ,  2); // udq NO. 1 - ( 0 - ASSIGN, 2 - DEFINE)
        BOOST_CHECK_EQUAL(iUdq[start + 1] , -4); // udq NO. 1 - (-4 - DEFINE / ASSIGN
        BOOST_CHECK_EQUAL(iUdq[start + 2] ,  1); // udq NO. 1 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)
        
        start = 1*udqDims[1];
        BOOST_CHECK_EQUAL(iUdq[start + 0] ,  0); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
        BOOST_CHECK_EQUAL(iUdq[start + 1] , -4); // udq NO. 2 - (-4 - DEFINE / ASSIGN
        BOOST_CHECK_EQUAL(iUdq[start + 2] ,  2); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)
        
        start = 2*udqDims[1];
        BOOST_CHECK_EQUAL(iUdq[start + 0] ,  2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
        BOOST_CHECK_EQUAL(iUdq[start + 1] , -4); // udq NO. 2 - (-4 - DEFINE / ASSIGN
        BOOST_CHECK_EQUAL(iUdq[start + 2] ,  3); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

        start = 3*udqDims[1];
        BOOST_CHECK_EQUAL(iUdq[start + 0] ,  2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
        BOOST_CHECK_EQUAL(iUdq[start + 1] , -4); // udq NO. 2 - (-4 - DEFINE / ASSIGN
        BOOST_CHECK_EQUAL(iUdq[start + 2] ,  1); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

        start = 4*udqDims[1];
        BOOST_CHECK_EQUAL(iUdq[start + 0] ,  2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
        BOOST_CHECK_EQUAL(iUdq[start + 1] , -4); // udq NO. 2 - (-4 - DEFINE / ASSIGN
        BOOST_CHECK_EQUAL(iUdq[start + 2] ,  4); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

        start = 5*udqDims[1];
        BOOST_CHECK_EQUAL(iUdq[start + 0] ,  2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
        BOOST_CHECK_EQUAL(iUdq[start + 1] , -4); // udq NO. 2 - (-4 - DEFINE / ASSIGN
        BOOST_CHECK_EQUAL(iUdq[start + 2] ,  1); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

    }

    {
        /*
        IUAD:
        Sequences of 5 items pr UDQ that is used for various well and group controls,
        i.e. sorted on the various active controls, see list for item (1).This means that
        one udq can occur several times, one for each control it is used for
        Only the active controls are output - and the sequence is according to when 
        they are defined

        dimension 5*no_of_udq-constraint-used in well and group controls

        item (1) : =    200000 + 19 for GCONPROD  and  ORAT
                        300000 + 19 for GCONPROD  and  WRAT
                        400000 + 19 for GCONPROD  and  GRAT
                        500000 + 19 for GCONPROD  and   LRAT
                        300000 + 4   for WCONPROD + oil rate target or upper limit
                        400000 + 4   for WCONPROD + water rate target or upper limit
                        500000 + 4   for WCONPROD + gas rate target or upper limit
                        600000 + 4   for WCONPROD + liquid rate target or upper limit
                        ? 300000 + 3   for WCONINJE   + oil rate target or upper limit
                        400000 + 3   for WCONINJE   + surface rate target or upper limit
                        500000 + 3   for WCONINJE   + reservoir volume  rate target or upper limit
                        1000000 + 27 for CECON  + minimum oil rate

        item (2)  - sequence number of UDQ used (from input sequence) for the actual constraint/target
            
        item (3)  - do not know yet  (value: 1)
        item (4)  - number of times the UDQ variable is used (e.g. for several different wells)
        item (5)  - the sequence number for the first use of the actual UDQ (index  i+1) = 1+sum over <the first i udq's in use >(no_use_udq(i))
        */
        
        const auto& iUad = udqData.getIUAD();
        
        auto start = 0*udqDims[3];
        BOOST_CHECK_EQUAL(iUad[start + 0] ,  300004); // iuad NO. 1  
        BOOST_CHECK_EQUAL(iUad[start + 1] ,       3); // iuad NO. 1  
        BOOST_CHECK_EQUAL(iUad[start + 2] ,       1); // iuad NO. 1  
        BOOST_CHECK_EQUAL(iUad[start + 3] ,       2); // iuad NO. 1  
        BOOST_CHECK_EQUAL(iUad[start + 4] ,       1); // iuad NO. 1  
               
        start = 1*udqDims[3];
        BOOST_CHECK_EQUAL(iUad[start + 0] ,  600004); // iuad NO. 2  
        BOOST_CHECK_EQUAL(iUad[start + 1] ,       5); // iuad NO. 2  
        BOOST_CHECK_EQUAL(iUad[start + 2] ,       1); // iuad NO. 2  
        BOOST_CHECK_EQUAL(iUad[start + 3] ,       2); // iuad NO. 2  
        BOOST_CHECK_EQUAL(iUad[start + 4] ,       3); // iuad NO. 2  
  
    }
    
    {
        /*
        ZUDN:
        contains  UDQ keyword data:
        Pairs of:
            quantity name (item2): e.g. 'WUOPRL  '  and
            units: e.g.: 'SM3/DAY ' 

        Length is  dependent on number of UDQ quantities =  2*no of UDQ's
        */
        
        const auto& zUdn = udqData.getZUDN();
        
        auto start = 0*udqDims[4];
        BOOST_CHECK_EQUAL(zUdn[start + 0].c_str() ,   "WUOPRL  "); // udq NO. 1  
        BOOST_CHECK_EQUAL(zUdn[start + 1].c_str() ,   "SM3/DAY "); // udq NO. 1  
                 
        start = 1*udqDims[4];
        BOOST_CHECK_EQUAL(zUdn[start + 0].c_str() ,   "WULPRL  "); // udq NO. 2  
        BOOST_CHECK_EQUAL(zUdn[start + 1].c_str() ,   "SM3/DAY "); // udq NO. 2  
        
        start = 2*udqDims[4];
        BOOST_CHECK_EQUAL(zUdn[start + 0].c_str() ,   "WUOPRU  "); // udq NO. 3  
        BOOST_CHECK_EQUAL(zUdn[start + 1].c_str() ,   "SM3/DAY "); // udq NO. 3  
        
        start = 3*udqDims[4];
        BOOST_CHECK_EQUAL(zUdn[start + 0].c_str() ,   "GUOPRU  "); // udq NO. 4  
        BOOST_CHECK_EQUAL(zUdn[start + 1].c_str() ,   "SM3/DAY "); // udq NO. 4  
        
        start = 4*udqDims[4];
        BOOST_CHECK_EQUAL(zUdn[start + 0].c_str() ,   "WULPRU  "); // udq NO. 5  
        BOOST_CHECK_EQUAL(zUdn[start + 1].c_str() ,   "SM3/DAY "); // udq NO. 5  
        
        start = 5*udqDims[4];
        BOOST_CHECK_EQUAL(zUdn[start + 0].c_str() ,   "FULPR   "); // udq NO. 6  
        BOOST_CHECK_EQUAL(zUdn[start + 1].c_str() ,   "SM3/DAY "); // udq NO. 6  
        
    }

    
    {
        /*
        ZUDL:
        contains string that define the "Data for operation" for the defined quantity

        e.g. 
        '(WOPR OP' 'L01 - 15' '0) * 0.9' '0       ' '        ' '        ' '        '

        The appropriate data are split into strings of 8 characters each. 

        Length: No of UDQ's * 16
        */
        
        const auto& zUdl = udqData.getZUDL();
        
        auto start = 0*udqDims[5];
        BOOST_CHECK_EQUAL(zUdl[start + 0].c_str() ,   "(WOPR PR"); // udq NO. 1  
        BOOST_CHECK_EQUAL(zUdl[start + 1].c_str() ,   "OD1 - 17"); // udq NO. 1
        BOOST_CHECK_EQUAL(zUdl[start + 2].c_str() ,   "0) * 0.6"); // udq NO. 1  
        BOOST_CHECK_EQUAL(zUdl[start + 3].c_str() ,   "0       "); // udq NO. 1  
        
        start = 3*udqDims[5];
        BOOST_CHECK_EQUAL(zUdl[start + 0].c_str() ,   "(GOPR GR"); // udq NO. 1  
        BOOST_CHECK_EQUAL(zUdl[start + 1].c_str() ,   "P1 - 449"); // udq NO. 1
        BOOST_CHECK_EQUAL(zUdl[start + 2].c_str() ,   ") * 0.77"); // udq NO. 1  
        BOOST_CHECK_EQUAL(zUdl[start + 3].c_str() ,   "        "); // udq NO. 1  
               
        start = 4*udqDims[5];
        BOOST_CHECK_EQUAL(zUdl[start + 0].c_str() ,   "(WLPR PR"); // udq NO. 1 
        BOOST_CHECK_EQUAL(zUdl[start + 1].c_str() ,   "OD2 - 30"); // udq NO. 1
        BOOST_CHECK_EQUAL(zUdl[start + 2].c_str() ,   "0) * 0.8"); // udq NO. 1 
        BOOST_CHECK_EQUAL(zUdl[start + 3].c_str() ,   "0       "); // udq NO. 1 
                
        start = 5*udqDims[5];
        BOOST_CHECK_EQUAL(zUdl[start + 0].c_str() ,   "(FLPR - "); // udq NO. 1  
        BOOST_CHECK_EQUAL(zUdl[start + 1].c_str() ,   "543) * 0"); // udq NO. 1
        BOOST_CHECK_EQUAL(zUdl[start + 2].c_str() ,   ".65     "); // udq NO. 1  
        BOOST_CHECK_EQUAL(zUdl[start + 3].c_str() ,   "        "); // udq NO. 1  
        
    }

#if 0
    {
        /*
        'DUDW    '          24 'DOUB'
           
        Dimension = max no wells * no of UDQ's
        Value = value of UDQ for the different wells 
        */
        
        const auto& dUdw = udqData.getDUDW();
        
        auto start = 0*udqDims[8];
        BOOST_CHECK_EQUAL(dUdw[start + 0] ,       210); // duDw NO. 1  
        BOOST_CHECK_EQUAL(dUdw[start + 1] ,       211); // duDw NO. 1  
        BOOST_CHECK_EQUAL(dUdw[start + 2] ,       212); // duDw NO. 1  
        BOOST_CHECK_EQUAL(dUdw[start + 3] ,       213); // duDw NO. 1  
        BOOST_CHECK_EQUAL(dUdw[start + 4] ,  -0.3E+21); // duDw NO. 1  
               
        start = 1*udqDims[8];
        BOOST_CHECK_EQUAL(dUdw[start + 0] ,       230); // duDw NO. 1  
        BOOST_CHECK_EQUAL(dUdw[start + 1] ,       231); // duDw NO. 1  
        BOOST_CHECK_EQUAL(dUdw[start + 2] ,       232); // duDw NO. 1  
        BOOST_CHECK_EQUAL(dUdw[start + 3] ,       233); // duDw NO. 1  
        BOOST_CHECK_EQUAL(dUdw[start + 4] ,  -0.3E+21); // duDw NO. 1  

        
    } 
#endif
    {
        /*
        'DUDG    '          5 'DOUB'
           
        Dimension = (max no groups+1) * no of group UDQ's
        Value = value of UDQ for the different groups 
        */
        
        const auto& dUdg = udqData.getDUDG();
        
        auto start = 0*udqDims[11];
        BOOST_CHECK_EQUAL(dUdg[start + 0] ,       362); // duDg NO. 1  
        BOOST_CHECK_EQUAL(dUdg[start + 1] ,       360); // duDg NO. 1  
        BOOST_CHECK_EQUAL(dUdg[start + 2] ,       361); // duDg NO. 1  
        BOOST_CHECK_EQUAL(dUdg[start + 3] ,  -0.3E+21); // duDg NO. 1  
        BOOST_CHECK_EQUAL(dUdg[start + 4] ,  -0.3E+21); // duDg NO. 1          

        
    } 
    
    
    {
        /*
        'DUDG    '          1 'DOUB'
           
        Dimension = 1 * no of Field UDQ's
        Value = value of UDQ for the field
        */
        
        const auto& dUdf = udqData.getDUDF();
        
        auto start = 0*udqDims[12];
        BOOST_CHECK_EQUAL(dUdf[start + 0] ,       460); // duDf NO. 1  
        
    } 
}
    
BOOST_AUTO_TEST_SUITE_END()
