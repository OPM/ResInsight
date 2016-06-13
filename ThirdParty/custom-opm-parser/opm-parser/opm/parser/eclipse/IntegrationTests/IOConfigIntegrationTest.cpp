/*
  Copyright 2015 Statoil ASA.

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

#define BOOST_TEST_MODULE IOCONFIG_INTEGRATION_TEST
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <tuple>
#include <vector>
#include <boost/date_time.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>

using namespace Opm;


void verifyRestartConfig(IOConfigConstPtr ioconfig, std::vector<std::tuple<int , bool, boost::gregorian::date>>& rptConfig) {

    for (auto rptrst : rptConfig) {
        int report_step                    = std::get<0>(rptrst);
        bool save                          = std::get<1>(rptrst);
        boost::gregorian::date report_date = std::get<2>(rptrst);

        BOOST_CHECK_EQUAL( save , ioconfig->getWriteRestartFile( report_step ));
        if (save) {
            BOOST_CHECK_EQUAL( report_date, ioconfig->getTimestepDate( report_step ));
        }
    }
}



BOOST_AUTO_TEST_CASE( NorneResttartConfig ) {
    std::vector<std::tuple<int , bool, boost::gregorian::date> > rptConfig;
    rptConfig.push_back( std::make_tuple(0 , true , boost::gregorian::date( 1997,11,6)) );
    rptConfig.push_back( std::make_tuple(1 , true , boost::gregorian::date( 1997,11,14)) );
    rptConfig.push_back( std::make_tuple(2 , true , boost::gregorian::date( 1997,12,1)) );
    rptConfig.push_back( std::make_tuple(3 , true , boost::gregorian::date( 1997,12,17)) );
    rptConfig.push_back( std::make_tuple(4 , true , boost::gregorian::date( 1998,1,1)) );
    rptConfig.push_back( std::make_tuple(5 , true , boost::gregorian::date( 1998,2,1)) );
    rptConfig.push_back( std::make_tuple(6 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(7 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(8 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(9 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(10 , true , boost::gregorian::date( 1998,4,23)) );
    rptConfig.push_back( std::make_tuple(11 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(12 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(13 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(14 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(15 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(16 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(17 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(18 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(19 , true , boost::gregorian::date( 1998,7,16)) );
    rptConfig.push_back( std::make_tuple(20 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(21 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(22 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(23 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(24 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(25 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(26 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(27 , true , boost::gregorian::date( 1998,10,13)) );
    rptConfig.push_back( std::make_tuple(28 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(29 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(30 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(31 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(32 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(33 , true , boost::gregorian::date( 1999,1,4)) );
    rptConfig.push_back( std::make_tuple(34 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(35 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(36 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(37 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(38 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(39 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(40 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(41 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(42 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(43 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(44 , true , boost::gregorian::date( 1999,5,1)) );
    rptConfig.push_back( std::make_tuple(45 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(46 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(47 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(48 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(49 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(50 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(51 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(52 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(53 , true , boost::gregorian::date( 1999,7,15)) );
    rptConfig.push_back( std::make_tuple(54 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(55 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(56 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(57 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(58 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(59 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(60 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(61 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(62 , true , boost::gregorian::date( 1999,10,3)) );
    rptConfig.push_back( std::make_tuple(63 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(64 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(65 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(66 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(67 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(68 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(69 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(70 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(71 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(72 , true , boost::gregorian::date( 2000,2,1)) );
    rptConfig.push_back( std::make_tuple(73 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(74 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(75 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(76 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(77 , true , boost::gregorian::date( 2000,5,1)) );
    rptConfig.push_back( std::make_tuple(78 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(79 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(80 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(81 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(82 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(83 , true , boost::gregorian::date( 2000,8,1)) );
    rptConfig.push_back( std::make_tuple(84 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(85 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(86 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(87 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(88 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(89 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(90 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(91 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(92 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(93 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(94 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(95 , true , boost::gregorian::date( 2000,11,1)) );
    rptConfig.push_back( std::make_tuple(96 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(97 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(98 , true , boost::gregorian::date( 2001,2,1)) );
    rptConfig.push_back( std::make_tuple(99 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(100 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(101 , true , boost::gregorian::date( 2001,5,1)) );
    rptConfig.push_back( std::make_tuple(102 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(103 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(104 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(105 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(106 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(107 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(108 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(109 , true , boost::gregorian::date( 2001,7,2)) );
    rptConfig.push_back( std::make_tuple(110 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(111 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(112 , true , boost::gregorian::date( 2001,7,16)) );
    rptConfig.push_back( std::make_tuple(113 , true , boost::gregorian::date( 2001,7,30)) );
    rptConfig.push_back( std::make_tuple(114 , true , boost::gregorian::date( 2001,8,1)) );
    rptConfig.push_back( std::make_tuple(115 , true , boost::gregorian::date( 2001,8,10)) );
    rptConfig.push_back( std::make_tuple(116 , true , boost::gregorian::date( 2001,8,16)) );
    rptConfig.push_back( std::make_tuple(117 , true , boost::gregorian::date( 2001,9,1)) );
    rptConfig.push_back( std::make_tuple(118 , true , boost::gregorian::date( 2001,9,10)) );
    rptConfig.push_back( std::make_tuple(119 , true , boost::gregorian::date( 2001,10,1)) );
    rptConfig.push_back( std::make_tuple(120 , true , boost::gregorian::date( 2001,11,1)) );
    rptConfig.push_back( std::make_tuple(121 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(122 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(123 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(124 , true , boost::gregorian::date( 2002,2,1)) );
    rptConfig.push_back( std::make_tuple(125 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(126 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(127 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(128 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(129 , true , boost::gregorian::date( 2002,5,1)) );
    rptConfig.push_back( std::make_tuple(130 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(131 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(132 , true , boost::gregorian::date( 2002,7,8)) );
    rptConfig.push_back( std::make_tuple(133 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(134 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(135 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(136 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(137 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(138 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(139 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(140 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(141 , true , boost::gregorian::date( 2002,10,7)) );
    rptConfig.push_back( std::make_tuple(142 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(143 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(144 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(145 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(146 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(147 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(148 , true , boost::gregorian::date( 2003,1,2)) );
    rptConfig.push_back( std::make_tuple(149 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(150 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(151 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(152 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(153 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(154 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(155 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(156 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(157 , true , boost::gregorian::date( 2003,5,1)) );
    rptConfig.push_back( std::make_tuple(158 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(159 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(160 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(161 , true , boost::gregorian::date( 2003,7,10)) );
    rptConfig.push_back( std::make_tuple(162 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(163 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(164 , true , boost::gregorian::date( 2003,8,12)) );
    rptConfig.push_back( std::make_tuple(165 , true , boost::gregorian::date( 2003,9,1)) );
    rptConfig.push_back( std::make_tuple(166 , true , boost::gregorian::date( 2003,9,2)) );
    rptConfig.push_back( std::make_tuple(167 , true , boost::gregorian::date( 2003,9,10)) );
    rptConfig.push_back( std::make_tuple(168 , true , boost::gregorian::date( 2003,9,12)) );
    rptConfig.push_back( std::make_tuple(169 , true , boost::gregorian::date( 2003,9,13)) );
    rptConfig.push_back( std::make_tuple(170 , true , boost::gregorian::date( 2003,9,16)) );
    rptConfig.push_back( std::make_tuple(171 , true , boost::gregorian::date( 2003,10,1)) );
    rptConfig.push_back( std::make_tuple(172 , true , boost::gregorian::date( 2003,10,23)) );
    rptConfig.push_back( std::make_tuple(173 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(174 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(175 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(176 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(177 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(178 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(179 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(180 , true , boost::gregorian::date( 2004,1,19)) );
    rptConfig.push_back( std::make_tuple(181 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(182 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(183 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(184 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(185 , true , boost::gregorian::date( 2004,5,1)) );
    rptConfig.push_back( std::make_tuple(186 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(187 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(188 , true , boost::gregorian::date( 2004,7,3)) );
    rptConfig.push_back( std::make_tuple(189 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(190 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(191 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(192 , true , boost::gregorian::date( 2004,8,16)) );
    rptConfig.push_back( std::make_tuple(193 , true , boost::gregorian::date( 2004,9,1)) );
    rptConfig.push_back( std::make_tuple(194 , true , boost::gregorian::date( 2004,9,20)) );
    rptConfig.push_back( std::make_tuple(195 , true , boost::gregorian::date( 2004,10,1)) );
    rptConfig.push_back( std::make_tuple(196 , true , boost::gregorian::date( 2004,11,1)) );
    rptConfig.push_back( std::make_tuple(197 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(198 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(199 , true , boost::gregorian::date( 2005,1,12)) );
    rptConfig.push_back( std::make_tuple(200 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(201 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(202 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(203 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(204 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(205 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(206 , true , boost::gregorian::date( 2005,4,24)) );
    rptConfig.push_back( std::make_tuple(207 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(208 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(209 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(210 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(211 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(212 , true , boost::gregorian::date( 2005,7,10)) );
    rptConfig.push_back( std::make_tuple(213 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(214 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(215 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(216 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(217 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(218 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(219 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(220 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(221 , true , boost::gregorian::date( 2005,11,1)) );
    rptConfig.push_back( std::make_tuple(222 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(223 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(224 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(225 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(226 , true , boost::gregorian::date( 2006,1,18)) );
    rptConfig.push_back( std::make_tuple(227 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(228 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(229 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(230 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(231 , true , boost::gregorian::date( 2006,4,25)) );
    rptConfig.push_back( std::make_tuple(232 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(233 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(234 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(235 , true , boost::gregorian::date( 2006,8,1)) );
    rptConfig.push_back( std::make_tuple(236 , false , boost::gregorian::date(1970,1,1)) );
    rptConfig.push_back( std::make_tuple(237 , true , boost::gregorian::date( 2006,8,16)) );
    rptConfig.push_back( std::make_tuple(238 , true , boost::gregorian::date( 2006,9,1)) );
    rptConfig.push_back( std::make_tuple(239 , true , boost::gregorian::date( 2006,9,14)) );
    rptConfig.push_back( std::make_tuple(240 , true , boost::gregorian::date( 2006,10,1)) );
    rptConfig.push_back( std::make_tuple(241 , true , boost::gregorian::date( 2006,10,10)) );


    auto state = Parser::parse("testdata/integration_tests/IOConfig/RPTRST_DECK.DATA");
    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}




BOOST_AUTO_TEST_CASE( RestartConfig2 ) {
    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;

    for (size_t report_step = 0; report_step <= 251; ++report_step) {
        if (0 == report_step)        rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2000,1,1)));
        else if (8   == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2000,7,1)));
        else if (27  == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2001,1,1)));
        else if (45  == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2001,7,1)));
        else if (61  == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2002,1,1)));
        else if (79  == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2002,7,1)));
        else if (89  == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2003,1,1)));
        else if (99  == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2003,7,1)));
        else if (109 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2004,1,1)));
        else if (128 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2004,7,1)));
        else if (136 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2005,1,1)));
        else if (146 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2005,7,1)));
        else if (158 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2006,1,1)));
        else if (164 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2006,7,1)));
        else if (170 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2007,1,1)));
        else if (178 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2007,7,1)));
        else if (184 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2008,1,1)));
        else if (192 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2008,7,1)));
        else if (198 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2009,1,1)));
        else if (204 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2009,7,1)));
        else if (210 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2010,1,1)));
        else if (216 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2010,7,1)));
        else if (222 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2011,1,1)));
        else if (228 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2011,7,1)));
        else if (234 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2012,1,1)));
        else if (240 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2012,7,1)));
        else if (246 == report_step) rptConfig.push_back( std::make_tuple(report_step, true, boost::gregorian::date(2013,1,1)));
        else    rptConfig.push_back( std::make_tuple(report_step, false, boost::gregorian::date(2000,1,1)));
    }

    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/RPT_TEST2.DATA", parseContext);
    EclipseState state( deck , parseContext );
    std::shared_ptr<const IOConfig> ioConfig = state.getIOConfigConst();
    verifyRestartConfig(ioConfig, rptConfig);

    BOOST_CHECK_EQUAL( ioConfig->getFirstRestartStep() , 0 );
}




//----------------- TESTS on SPE1CASE2 ---------------------------------------------------------------------

void fillRptConfig(std::vector<std::tuple<int, bool, boost::gregorian::date>>& rptConfig) {

    rptConfig.push_back( std::make_tuple(0,  false,  boost::gregorian::date(2015,1,1)));

    rptConfig.push_back( std::make_tuple(1,  true,   boost::gregorian::date(2015,2,1)));
    rptConfig.push_back( std::make_tuple(2,  true,   boost::gregorian::date(2015,3,1)));
    rptConfig.push_back( std::make_tuple(3,  true,   boost::gregorian::date(2015,4,1)));
    rptConfig.push_back( std::make_tuple(4,  true,   boost::gregorian::date(2015,5,1)));
    rptConfig.push_back( std::make_tuple(5,  true,   boost::gregorian::date(2015,6,1)));
    rptConfig.push_back( std::make_tuple(6,  true,   boost::gregorian::date(2015,7,1)));
    rptConfig.push_back( std::make_tuple(7,  true,   boost::gregorian::date(2015,8,1)));
    rptConfig.push_back( std::make_tuple(8,  true,   boost::gregorian::date(2015,9,1)));
    rptConfig.push_back( std::make_tuple(9,  true,   boost::gregorian::date(2015,10,1)));
    rptConfig.push_back( std::make_tuple(10, true,   boost::gregorian::date(2015,11,1)));
    rptConfig.push_back( std::make_tuple(11, true,   boost::gregorian::date(2015,12,1)));
    rptConfig.push_back( std::make_tuple(12, true,   boost::gregorian::date(2016,1,1)));
    rptConfig.push_back( std::make_tuple(13, true,   boost::gregorian::date(2016,2,1)));

    rptConfig.push_back( std::make_tuple(14, false,  boost::gregorian::date(2016,2,29)));

    rptConfig.push_back( std::make_tuple(15, true,   boost::gregorian::date(2016,3,31)));
    rptConfig.push_back( std::make_tuple(16, true,   boost::gregorian::date(2016,4,30)));
    rptConfig.push_back( std::make_tuple(17, true,   boost::gregorian::date(2016,5,31)));
    rptConfig.push_back( std::make_tuple(18, true,   boost::gregorian::date(2016,6,30)));
    rptConfig.push_back( std::make_tuple(19, true,   boost::gregorian::date(2016,7,31)));
    rptConfig.push_back( std::make_tuple(20, true,   boost::gregorian::date(2016,8,31)));
    rptConfig.push_back( std::make_tuple(21, true,   boost::gregorian::date(2016,9,30)));
    rptConfig.push_back( std::make_tuple(22, true,   boost::gregorian::date(2016,10,31)));
    rptConfig.push_back( std::make_tuple(23, true,   boost::gregorian::date(2016,11,30)));
    rptConfig.push_back( std::make_tuple(24, true,   boost::gregorian::date(2016,12,31)));

    rptConfig.push_back( std::make_tuple(25, true,   boost::gregorian::date(2017,1,31)));
    rptConfig.push_back( std::make_tuple(26, true,   boost::gregorian::date(2017,2,28)));
    rptConfig.push_back( std::make_tuple(27, true,   boost::gregorian::date(2017,3,31)));
    rptConfig.push_back( std::make_tuple(28, true,   boost::gregorian::date(2017,4,30)));
    rptConfig.push_back( std::make_tuple(29, true,   boost::gregorian::date(2017,5,31)));
    rptConfig.push_back( std::make_tuple(30, true,   boost::gregorian::date(2017,6,30)));
    rptConfig.push_back( std::make_tuple(31, true,   boost::gregorian::date(2017,7,31)));
    rptConfig.push_back( std::make_tuple(32, true,   boost::gregorian::date(2017,8,31)));
    rptConfig.push_back( std::make_tuple(33, true,   boost::gregorian::date(2017,9,30)));
    rptConfig.push_back( std::make_tuple(34, true,   boost::gregorian::date(2017,10,31)));
    rptConfig.push_back( std::make_tuple(35, true,   boost::gregorian::date(2017,11,30)));
    rptConfig.push_back( std::make_tuple(36, true,   boost::gregorian::date(2017,12,31)));

    rptConfig.push_back( std::make_tuple(37, true,   boost::gregorian::date(2018,1,31)));
    rptConfig.push_back( std::make_tuple(38, true,   boost::gregorian::date(2018,2,28)));
    rptConfig.push_back( std::make_tuple(39, true,   boost::gregorian::date(2018,3,31)));
    rptConfig.push_back( std::make_tuple(40, true,   boost::gregorian::date(2018,4,30)));
    rptConfig.push_back( std::make_tuple(41, true,   boost::gregorian::date(2018,5,31)));
    rptConfig.push_back( std::make_tuple(42, true,   boost::gregorian::date(2018,6,30)));
    rptConfig.push_back( std::make_tuple(43, true,   boost::gregorian::date(2018,7,31)));
    rptConfig.push_back( std::make_tuple(44, true,   boost::gregorian::date(2018,8,31)));
    rptConfig.push_back( std::make_tuple(45, true,   boost::gregorian::date(2018,9,30)));
    rptConfig.push_back( std::make_tuple(46, true,   boost::gregorian::date(2018,10,31)));
    rptConfig.push_back( std::make_tuple(47, true,   boost::gregorian::date(2018,11,30)));
    rptConfig.push_back( std::make_tuple(48, true,   boost::gregorian::date(2018,12,31)));

    rptConfig.push_back( std::make_tuple(49, true,   boost::gregorian::date(2019,1,31)));
    rptConfig.push_back( std::make_tuple(50, true,   boost::gregorian::date(2019,2,28)));
    rptConfig.push_back( std::make_tuple(51, true,   boost::gregorian::date(2019,3,31)));
    rptConfig.push_back( std::make_tuple(52, true,   boost::gregorian::date(2019,4,30)));
    rptConfig.push_back( std::make_tuple(53, true,   boost::gregorian::date(2019,5,31)));
    rptConfig.push_back( std::make_tuple(54, true,   boost::gregorian::date(2019,6,30)));
    rptConfig.push_back( std::make_tuple(55, true,   boost::gregorian::date(2019,7,31)));
    rptConfig.push_back( std::make_tuple(56, true,   boost::gregorian::date(2019,8,31)));
    rptConfig.push_back( std::make_tuple(57, true,   boost::gregorian::date(2019,9,30)));
    rptConfig.push_back( std::make_tuple(58, true,   boost::gregorian::date(2019,10,31)));
    rptConfig.push_back( std::make_tuple(59, true,   boost::gregorian::date(2019,11,30)));
    rptConfig.push_back( std::make_tuple(60, true,   boost::gregorian::date(2019,12,31)));

    rptConfig.push_back( std::make_tuple(61, true,   boost::gregorian::date(2020,1,31)));
    rptConfig.push_back( std::make_tuple(62, true,   boost::gregorian::date(2020,2,28)));
    rptConfig.push_back( std::make_tuple(63, true,   boost::gregorian::date(2020,3,30)));
    rptConfig.push_back( std::make_tuple(64, true,   boost::gregorian::date(2020,4,29)));
    rptConfig.push_back( std::make_tuple(65, true,   boost::gregorian::date(2020,5,30)));
    rptConfig.push_back( std::make_tuple(66, true,   boost::gregorian::date(2020,6,29)));
    rptConfig.push_back( std::make_tuple(67, true,   boost::gregorian::date(2020,7,30)));
    rptConfig.push_back( std::make_tuple(68, true,   boost::gregorian::date(2020,8,30)));
    rptConfig.push_back( std::make_tuple(69, true,   boost::gregorian::date(2020,9,29)));
    rptConfig.push_back( std::make_tuple(70, true,   boost::gregorian::date(2020,10,30)));
    rptConfig.push_back( std::make_tuple(71, true,   boost::gregorian::date(2020,11,29)));
    rptConfig.push_back( std::make_tuple(72, true,   boost::gregorian::date(2020,12,30)));

    rptConfig.push_back( std::make_tuple(73, true,   boost::gregorian::date(2021,1,30)));
    rptConfig.push_back( std::make_tuple(74, true,   boost::gregorian::date(2021,2,27)));
    rptConfig.push_back( std::make_tuple(75, true,   boost::gregorian::date(2021,3,30)));
    rptConfig.push_back( std::make_tuple(76, true,   boost::gregorian::date(2021,4,29)));
    rptConfig.push_back( std::make_tuple(77, true,   boost::gregorian::date(2021,5,30)));
    rptConfig.push_back( std::make_tuple(78, true,   boost::gregorian::date(2021,6,29)));
    rptConfig.push_back( std::make_tuple(79, true,   boost::gregorian::date(2021,7,30)));
    rptConfig.push_back( std::make_tuple(80, true,   boost::gregorian::date(2021,8,30)));
    rptConfig.push_back( std::make_tuple(81, true,   boost::gregorian::date(2021,9,29)));
    rptConfig.push_back( std::make_tuple(82, true,   boost::gregorian::date(2021,10,30)));
    rptConfig.push_back( std::make_tuple(83, true,   boost::gregorian::date(2021,11,29)));
    rptConfig.push_back( std::make_tuple(84, true,   boost::gregorian::date(2021,12,30)));

    rptConfig.push_back( std::make_tuple(85, true,   boost::gregorian::date(2022,1,30)));
    rptConfig.push_back( std::make_tuple(86, true,   boost::gregorian::date(2022,2,27)));
    rptConfig.push_back( std::make_tuple(87, true,   boost::gregorian::date(2022,3,30)));
    rptConfig.push_back( std::make_tuple(88, true,   boost::gregorian::date(2022,4,29)));
    rptConfig.push_back( std::make_tuple(89, true,   boost::gregorian::date(2022,5,30)));
    rptConfig.push_back( std::make_tuple(90, true,   boost::gregorian::date(2022,6,29)));
    rptConfig.push_back( std::make_tuple(91, true,   boost::gregorian::date(2022,7,30)));
    rptConfig.push_back( std::make_tuple(92, true,   boost::gregorian::date(2022,8,30)));
    rptConfig.push_back( std::make_tuple(93, true,   boost::gregorian::date(2022,9,29)));
    rptConfig.push_back( std::make_tuple(94, true,   boost::gregorian::date(2022,10,30)));
    rptConfig.push_back( std::make_tuple(95, true,   boost::gregorian::date(2022,11,29)));
    rptConfig.push_back( std::make_tuple(96, true,   boost::gregorian::date(2022,12,30)));

    rptConfig.push_back( std::make_tuple(97,  true,   boost::gregorian::date(2023,1,30)));
    rptConfig.push_back( std::make_tuple(98,  true,   boost::gregorian::date(2023,2,27)));
    rptConfig.push_back( std::make_tuple(99,  true,   boost::gregorian::date(2023,3,30)));
    rptConfig.push_back( std::make_tuple(100, true,   boost::gregorian::date(2023,4,29)));
    rptConfig.push_back( std::make_tuple(101, true,   boost::gregorian::date(2023,5,30)));
    rptConfig.push_back( std::make_tuple(102, true,   boost::gregorian::date(2023,6,29)));
    rptConfig.push_back( std::make_tuple(103, true,   boost::gregorian::date(2023,7,30)));
    rptConfig.push_back( std::make_tuple(104, true,   boost::gregorian::date(2023,8,30)));
    rptConfig.push_back( std::make_tuple(105, true,   boost::gregorian::date(2023,9,29)));
    rptConfig.push_back( std::make_tuple(106, true,   boost::gregorian::date(2023,10,30)));
    rptConfig.push_back( std::make_tuple(107, true,   boost::gregorian::date(2023,11,29)));
    rptConfig.push_back( std::make_tuple(108, true,   boost::gregorian::date(2023,12,30)));

    rptConfig.push_back( std::make_tuple(109, true,   boost::gregorian::date(2024,1,30)));
    rptConfig.push_back( std::make_tuple(110, true,   boost::gregorian::date(2024,2,27)));
    rptConfig.push_back( std::make_tuple(111, true,   boost::gregorian::date(2024,3,29)));
    rptConfig.push_back( std::make_tuple(112, true,   boost::gregorian::date(2024,4,28)));
    rptConfig.push_back( std::make_tuple(113, true,   boost::gregorian::date(2024,5,29)));
    rptConfig.push_back( std::make_tuple(114, true,   boost::gregorian::date(2024,6,28)));
    rptConfig.push_back( std::make_tuple(115, true,   boost::gregorian::date(2024,7,29)));
    rptConfig.push_back( std::make_tuple(116, true,   boost::gregorian::date(2024,8,29)));
    rptConfig.push_back( std::make_tuple(117, true,   boost::gregorian::date(2024,9,28)));
    rptConfig.push_back( std::make_tuple(118, true,   boost::gregorian::date(2024,10,29)));
    rptConfig.push_back( std::make_tuple(119, true,   boost::gregorian::date(2024,11,28)));
    rptConfig.push_back( std::make_tuple(120, true,   boost::gregorian::date(2024,12,29)));
}



BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC5_SET_ON_TIMESTEP_1 ) {

    //Write reportfile every first day of a month
    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);
    EclipseState state( deck , parseContext );

    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();

    int basic = 5;
    TimeMapConstPtr timemap = state.getSchedule()->getTimeMap();
    ioconfig->handleRPTRSTBasic(timemap,
                                1,
                                basic);


    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;
    fillRptConfig(rptConfig);

    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}



BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC5_SET_ON_TIMESTEP_13 ) {

    //Write reportfile every first day of a month
    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);
    EclipseState state( deck , parseContext );
    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();

    int basic = 5;
    TimeMapConstPtr timemap = state.getSchedule()->getTimeMap();
    ioconfig->handleRPTRSTBasic(timemap,
                                13,
                                basic);

    // Expecting same results as Eclipse:
    // Report steps 13 (1 Feb 2016) - 120 (29 Dec 2024) should be written,
    // except for step 14!

    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;
    fillRptConfig(rptConfig);
    for (size_t reportstep = 1; reportstep <= 12; ++reportstep) {
        std::get<1>(rptConfig[reportstep]) = false;
    }

    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}




BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC5_SET_ON_TIMESTEP_14 ) {

    //Write reportfile every first day of a month
    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);
    EclipseState state( deck , parseContext );
    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();

    int basic = 5;
    TimeMapConstPtr timemap = state.getSchedule()->getTimeMap();
    ioconfig->handleRPTRSTBasic(timemap,
                                14,
                                basic);

    // Expecting same results as Eclipse:
    // Report steps 15 (31 Mars 2016) - 120 (29 Dec 2024) should be written

    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;
    fillRptConfig(rptConfig);
    for (size_t reportstep = 1; reportstep <= 14; ++reportstep) {
        std::get<1>(rptConfig[reportstep]) = false;
    }

    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}


BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC5_SET_ON_TIMESTEP_37 ) {

    //Write reportfile every first day of a month
    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);
    EclipseState state( deck , parseContext );
    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();

    int basic = 5;
    TimeMapConstPtr timemap = state.getSchedule()->getTimeMap();
    ioconfig->handleRPTRSTBasic(timemap,
                                37,
                                basic);

    // Expecting same results as Eclipse:
    // Report steps 37 (31 Jan 2018) - 120 (29 Dec 2024) should be written

    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;
    fillRptConfig(rptConfig);
    for (size_t reportstep = 1; reportstep <= 36; ++reportstep) {
        std::get<1>(rptConfig[reportstep]) = false;
    }

    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}


BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC5_FREQ10_SET_ON_TIMESTEP_10 ) {
    //Write reportfile every first day of each month, with frequency 10

    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;

    for (size_t reportstep = 0; reportstep <= 120; ++reportstep) {
        if (20 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2016,8,31)));
        } else if (30 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2017,6,30)));
        } else if (40 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2018,4,30)));
        } else if (50 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2019,2,28)));
        } else if (60 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2019,12,31)));
        } else if (70 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2020,10,30)));
        } else if (80 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2021,8,30)));
        } else if (90 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2022,6,29)));
        } else if (100 == reportstep) {
          rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2023,4,29)));
        } else if (110 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2024,2,27)));
        } else if (120 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2024,12,29)));
        } else {
            rptConfig.push_back( std::make_tuple(reportstep, false, boost::gregorian::date(2015,1,1)));
        }
    }

    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);

    EclipseState state( deck , parseContext );
    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();

    int basic = 5;
    int freq  = 10;
    ioconfig->handleRPTRSTBasic(state.getSchedule()->getTimeMap(),
                                10,
                                basic,
                                freq);


    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}


BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC5_FREQ40_SET_ON_TIMESTEP_1 ) {
    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;

    for (size_t reportstep = 0; reportstep <= 120; ++reportstep) {
        if (0 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, false, boost::gregorian::date(2015,1,1)));
        } else if (41 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2018,5,31)));
        } else if (81 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2021,9,29)));
        } else {
            rptConfig.push_back( std::make_tuple(reportstep, false, boost::gregorian::date(2015,1,1)));
        }
    }

    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);

    EclipseState state( deck , parseContext );
    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();

    int basic = 5;
    int freq  = 40;
    ioconfig->handleRPTRSTBasic(state.getSchedule()->getTimeMap(),
                                1,
                                basic,
                                freq);



    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}



BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC5_FREQ40_SET_ON_TIMESTEP_14 ) {

    //Write reportfile every first day of a month, with frequency 40
    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);
    EclipseState state( deck , parseContext );
    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();

    int basic = 5;
    int freq  = 40;
    TimeMapConstPtr timemap = state.getSchedule()->getTimeMap();
    ioconfig->handleRPTRSTBasic(timemap,
                                14,
                                basic,
                                freq);

    // Expecting same results as Eclipse:
    // Report steps 54 and 94 should be written
    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;

    for (size_t reportstep = 0; reportstep <= 120; ++reportstep) {
        if (54 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2019,6,30)));
        } else if (94 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2022,10,30)));
        } else {
            rptConfig.push_back( std::make_tuple(reportstep, false, boost::gregorian::date(2015,1,1)));
        }
    }


    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}





BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC4_SET_ON_TIMESTEP_1 ) {
    //Write reportfile every first day of each year

    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;

    for (size_t reportstep = 0; reportstep <= 120; ++reportstep) {
        if (12 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2016,1,1)));
        } else if (25 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2017,1,31)));
        } else if (37 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2018,1,31)));
        } else if (49 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2019,1,31)));
        } else if (61 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2020,1,31)));
        } else if (73 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2021,1,30)));
        } else if (85 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2022,1,30)));
        } else if (97 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2023,1,30)));
        } else if (109 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2024,1,30)));
        } else {
            rptConfig.push_back( std::make_tuple(reportstep, false, boost::gregorian::date(2015,1,1)));
        }
    }

    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);

    EclipseState state( deck , parseContext );
    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();

    int basic = 4;
    ioconfig->handleRPTRSTBasic(state.getSchedule()->getTimeMap(),
                                1,
                                basic);


    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}


BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC4_SET_ON_TIMESTEP_12 ) {

    //Write reportfile every first day of each year

    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;

    for (size_t reportstep = 0; reportstep <= 120; ++reportstep) {
        if (12 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2016,1,1)));
        } else if (25 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2017,1,31)));
        } else if (37 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2018,1,31)));
        } else if (49 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2019,1,31)));
        } else if (61 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2020,1,31)));
        } else if (73 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2021,1,30)));
        } else if (85 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2022,1,30)));
        } else if (97 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2023,1,30)));
        } else if (109 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2024,1,30)));
        } else {
            rptConfig.push_back( std::make_tuple(reportstep, false, boost::gregorian::date(2015,1,1)));
        }
    }


    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);

    EclipseState state( deck , parseContext );
    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();


    int basic = 4;
    ioconfig->handleRPTRSTBasic(state.getSchedule()->getTimeMap(),
                                12,
                                basic);

    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}


BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC4_SET_ON_TIMESTEP_13 ) {

    //Write reportfile every first day of each year

    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;

    for (size_t reportstep = 0; reportstep <= 120; ++reportstep) {
        if (25 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2017,1,31)));
        } else if (37 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2018,1,31)));
        } else if (49 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2019,1,31)));
        } else if (61 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2020,1,31)));
        } else if (73 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2021,1,30)));
        } else if (85 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2022,1,30)));
        } else if (97 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2023,1,30)));
        } else if (109 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2024,1,30)));
        } else {
            rptConfig.push_back( std::make_tuple(reportstep, false, boost::gregorian::date(2015,1,1)));
        }
    }


    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);

    EclipseState state( deck , parseContext );
    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();


    int basic = 4;
    ioconfig->handleRPTRSTBasic(state.getSchedule()->getTimeMap(),
                                13,
                                basic);

    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}




BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC4_FREQ3_SET_ON_TIMESTEP_13 ) {

    //Write reportfile every first day of each year

    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;

    for (size_t reportstep = 0; reportstep <= 120; ++reportstep) {
        if (49 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2019,1,31)));
        } else if (85 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2022,1,30)));
        } else {
            rptConfig.push_back( std::make_tuple(reportstep, false, boost::gregorian::date(2015,1,1)));
        }
    }


    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);

    EclipseState state( deck , parseContext );
    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();


    int basic = 4;
    int freq  = 3;
    ioconfig->handleRPTRSTBasic(state.getSchedule()->getTimeMap(),
                                13,
                                basic,
                                freq);



    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}




BOOST_AUTO_TEST_CASE( SPE1CASE2_BASIC3_FREQ5_SET_ON_TIMESTEP_11 ) {

    //Write reportfile every first day of each year
    std::vector<std::tuple<int, bool, boost::gregorian::date>> rptConfig;

    for (size_t reportstep = 0; reportstep <= 120; ++reportstep) {
        if (15 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2016,3,31)));
        } else if (20 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2016,8,31)));
        } else if (25 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2017,1,31)));
        } else if (30 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2017,6,30)));
        } else if (35 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2017,11,30)));
        } else if (40 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2018,4,30)));
        } else if (45 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2018,9,30)));
        } else if (50 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2019,2,28)));
        } else if (55 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2019,7,31)));
        } else if (60 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2019,12,31)));
        } else if (65 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2020,5,30)));
        } else if (70 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2020,10,30)));
        } else if (75 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2021,3,30)));
        } else if (80 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2021,8,30)));
        } else if (85 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2022,1,30)));
        } else if (90 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2022,6,29)));
        } else if (95 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2022,11,29)));
        } else if (100 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2023,4,29)));
        } else if (105 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2023,9,29)));
        } else if (110 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2024,2,27)));
        } else if (115 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2024,7,29)));
        } else if (120 == reportstep) {
            rptConfig.push_back( std::make_tuple(reportstep, true, boost::gregorian::date(2024,12,29)));
        } else {
            rptConfig.push_back( std::make_tuple(reportstep, false, boost::gregorian::date(2015,1,1)));
        }
    }


    ParseContext parseContext;
    ParserPtr parser(new Parser());
    DeckConstPtr deck = parser->parseFile("testdata/integration_tests/IOConfig/SPE1CASE2.DATA", parseContext);

    EclipseState state( deck , parseContext );
    std::shared_ptr<IOConfig> ioconfig = state.getIOConfig();


    int basic = 3;
    int freq  = 5;
    ioconfig->handleRPTRSTBasic(state.getSchedule()->getTimeMap(),
                                11,
                                basic,
                                freq);

    verifyRestartConfig(state.getIOConfigConst(), rptConfig);
}










