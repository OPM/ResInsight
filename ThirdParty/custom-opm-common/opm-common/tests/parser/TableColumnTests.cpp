/*
  Copyright (C) 2015 by Statoil ASA

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

#define BOOST_TEST_MODULE TableColumnTests

#include <boost/test/unit_test.hpp>


#include <opm/input/eclipse/EclipseState/Tables/TableIndex.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableColumn.hpp>
#include <opm/input/eclipse/EclipseState/Tables/ColumnSchema.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE( CreateTest ) {
    ColumnSchema schema("COLUMN" , Table::STRICTLY_INCREASING , Table::DEFAULT_LINEAR );
    TableColumn column( schema );
    BOOST_CHECK_EQUAL( column.size() , 0U );

    column.addValue( 0 );
    column.addValue( 1 );
    column.addValue( 2 );

    BOOST_CHECK_EQUAL( column.size() , 3U );

    BOOST_CHECK_EQUAL( column[0] , 0 );
    BOOST_CHECK_EQUAL( column[1] , 1 );
    BOOST_CHECK_EQUAL( column[2] , 2 );

    BOOST_CHECK_THROW( column[3] , std::invalid_argument );

    {
        std::vector<double> cp(column.size());
        std::copy( column.begin() , column.end() , cp.begin());

        for (size_t i = 0; i < column.size(); i++)
            BOOST_CHECK_EQUAL( column[i] , cp[i] );
    }
}



BOOST_AUTO_TEST_CASE( TestDefault ) {
    ColumnSchema schema("COLUMN" , Table::STRICTLY_INCREASING , Table::DEFAULT_LINEAR );
    TableColumn column( schema );


    column.addDefault( );
    column.addDefault( );
    column.addDefault( );
    BOOST_CHECK_EQUAL( column.size() , 3U );
    BOOST_CHECK_THROW( column[0] , std::invalid_argument );

    column.updateValue(0 , 10);
    BOOST_CHECK_EQUAL( column[0] , 10 );
    BOOST_CHECK( column.hasDefault( ) );
}


BOOST_AUTO_TEST_CASE( TestAscending ) {
    ColumnSchema schema("COLUMN" , Table::STRICTLY_INCREASING , Table::DEFAULT_LINEAR);
    TableColumn column( schema );

    BOOST_CHECK_EQUAL( column.size() , 0U );

    column.addValue( 10 );
    BOOST_CHECK_THROW( column.addValue( 9 ) , std::invalid_argument );
    column.addDefault( );
    BOOST_CHECK( column.hasDefault( ) );
    BOOST_CHECK_THROW( column.updateValue( 1, 9 ) , std::invalid_argument );
    column.addValue( 12 );
    BOOST_CHECK_THROW( column.updateValue( 1, 13 ) , std::invalid_argument );
    column.updateValue( 1, 11 );

    column.addDefault( );
    column.addDefault( );
    column.addDefault( );
    column.addValue(16);

    column.updateValue( 3,13 );
    column.updateValue( 4,14 );
    column.updateValue( 5,15 );
    BOOST_CHECK( !column.hasDefault( ) );
}


BOOST_AUTO_TEST_CASE( TestWeaklyAscending ) {
    ColumnSchema schema("COLUMN" , Table::INCREASING  , Table::DEFAULT_LINEAR);
    TableColumn column( schema );

    column.addValue(1);
    column.addValue(1);

    BOOST_CHECK( !column.hasDefault( ) );
}


BOOST_AUTO_TEST_CASE( TestDescending ) {
    ColumnSchema schema("COLUMN" , Table::STRICTLY_DECREASING , Table::DEFAULT_LINEAR);
    TableColumn column( schema );

    BOOST_CHECK_EQUAL( column.size() , 0U );

    column.addValue( -10 );
    BOOST_CHECK_THROW( column.addValue( -9 ) , std::invalid_argument );
    column.addDefault( );
    BOOST_CHECK_THROW( column.updateValue( 1, -9 ) , std::invalid_argument );
    column.addValue( -12 );
    BOOST_CHECK_THROW( column.updateValue( 1, -13 ) , std::invalid_argument );
    column.updateValue( 1, -11 );

    column.addDefault( );
    column.addDefault( );
    column.addDefault( );
    column.addValue(-16);

    column.updateValue( 3,-13 );
    column.updateValue( 4,-14 );
    column.updateValue( 5,-15 );
}


BOOST_AUTO_TEST_CASE( TestDEFAULT_NONE) {
    ColumnSchema schema("COLUMN" , Table::STRICTLY_DECREASING , Table::DEFAULT_NONE);
    TableColumn column( schema );

    BOOST_CHECK_THROW( column.addDefault(  ) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE( Test_MIN_MAX) {
    ColumnSchema schema("COLUMN" , Table::RANDOM , Table::DEFAULT_LINEAR);
    TableColumn column( schema );

    BOOST_CHECK_THROW( column.max( ) , std::invalid_argument );
    BOOST_CHECK_THROW( column.min( ) , std::invalid_argument );

    column.addValue( 1 );
    BOOST_CHECK_EQUAL( 1 , column.min() );
    BOOST_CHECK_EQUAL( 1 , column.max() );

    column.addValue( 100 );
    BOOST_CHECK_EQUAL( 1 , column.min() );
    BOOST_CHECK_EQUAL( 100 , column.max() );

    column.addValue( 50 );
    BOOST_CHECK_EQUAL( 1 , column.min() );
    BOOST_CHECK_EQUAL( 100 , column.max() );

    column.addDefault( );
    BOOST_CHECK_THROW( column.max( ) , std::invalid_argument );
    BOOST_CHECK_THROW( column.min( ) , std::invalid_argument );

    column.updateValue( 3 , 67 );
    BOOST_CHECK_EQUAL( 1 , column.min() );
    BOOST_CHECK_EQUAL( 100 , column.max() );
}

BOOST_AUTO_TEST_CASE( Test_IN_RANGE) {
    ColumnSchema schema("COLUMN" , Table::RANDOM , Table::DEFAULT_LINEAR);
    TableColumn column( schema );

    column.addValue(10);
    column.addValue(20);
    BOOST_CHECK_THROW( column.inRange( 15 ) , std::invalid_argument );


    ColumnSchema schema2("COLUMN" , Table::INCREASING, Table::DEFAULT_LINEAR);
    TableColumn column2( schema2 );


    BOOST_CHECK_THROW( column2.inRange( 15 ) , std::invalid_argument );
    column2.addValue(10);
    BOOST_CHECK_THROW( column2.inRange( 15 ) , std::invalid_argument );
    column2.addValue(20);
    BOOST_CHECK( column2.inRange( 15 ));
    BOOST_CHECK( column2.inRange( 10 ));
    BOOST_CHECK( column2.inRange( 20 ));

    BOOST_CHECK( !column2.inRange( 9 ));
    BOOST_CHECK( !column2.inRange( 21 ));

    column2.addDefault( );
    BOOST_CHECK_THROW( column2.inRange( 15 ) , std::invalid_argument );
}



BOOST_AUTO_TEST_CASE( Test_Table_Index ) {
    {
        ColumnSchema schema("COLUMN" , Table::RANDOM , Table::DEFAULT_NONE);
        TableColumn column( schema );

        /* Can not look up with random ordering */
        BOOST_CHECK_THROW( column.lookup( 0.67 ) , std::invalid_argument );
    }

    {
        ColumnSchema schema("COLUMN" , Table::INCREASING , Table::DEFAULT_LINEAR);
        TableColumn column( schema );

        /* Can not look up in empty column */
        BOOST_CHECK_THROW( column.lookup( 0.67 ) , std::invalid_argument );

        column.addValue( 10 );

        column.addDefault( );
        /* Can not look up in column with defaults */
        BOOST_CHECK_THROW( column.lookup( 0.67 ) , std::invalid_argument );


        column.updateValue(1 , 20 );
    }
}


BOOST_AUTO_TEST_CASE( Test_EVAL_INCREASING ) {
    ColumnSchema schema("COLUMN" , Table::INCREASING , Table::DEFAULT_LINEAR);
    TableColumn column( schema );

    column.addValue(0);

    /* Out of range - constant end-point extrapolation , size = 1*/
    BOOST_CHECK_EQUAL( column.eval( column.lookup( -1 )) , 0 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup(  1 )) , 0 );


    column.addValue(1);
    column.addValue(2);
    column.addValue(3);

    BOOST_CHECK_EQUAL( column.eval( column.lookup( 0 )) , 0 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup( 1 )) , 1 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup( 2 )) , 2 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup( 3 )) , 3 );

    BOOST_CHECK_EQUAL( column.eval( column.lookup( 0.25 )) , 0.25 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup( 1.75 )) , 1.75 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup( 2.5 )) , 2.5 );

    /* Out of range - constant end-point extrapolation */
    BOOST_CHECK_EQUAL( column.eval( column.lookup( -1 )) , 0 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup(  4 )) , 3 );
}


BOOST_AUTO_TEST_CASE( Test_EVAL_DECREASING ) {
    ColumnSchema schema("COLUMN" , Table::DECREASING , Table::DEFAULT_LINEAR);
    TableColumn column( schema );

    column.addValue(3);
    column.addValue(2);
    column.addValue(1);
    column.addValue(0);

    BOOST_CHECK_EQUAL( column.eval( column.lookup( 0 )) , 0 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup( 1 )) , 1 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup( 2 )) , 2 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup( 3 )) , 3 );

    BOOST_CHECK_EQUAL( column.eval( column.lookup( 0.25 )) , 0.25 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup( 1.75 )) , 1.75 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup( 2.5 )) , 2.5 );

    /* Out of range - constant end-point extrapolation */
    BOOST_CHECK_EQUAL( column.eval( column.lookup( -1 )) , 0 );
    BOOST_CHECK_EQUAL( column.eval( column.lookup(  4 )) , 3 );
}




BOOST_AUTO_TEST_CASE( Test_CONST_DEFAULT ) {
    ColumnSchema schema("COLUMN" , Table::DECREASING , 1.0);
    TableColumn column( schema );
    column.addDefault( );
    column.addDefault( );
    BOOST_CHECK( !column.hasDefault( ) );

    BOOST_CHECK_EQUAL( column[0] , 1.0 );
    BOOST_CHECK_EQUAL( column[1] , 1.0 );
}


BOOST_AUTO_TEST_CASE( Test_LINEAR_DEFAULT ) {
    ColumnSchema argSchema("COLUMN" , Table::INCREASING , Table::DEFAULT_NONE);
    ColumnSchema valueSchema("COLUMN" , Table::RANDOM , Table::DEFAULT_LINEAR);
    TableColumn argColumn( argSchema );
    TableColumn valueColumn( valueSchema );

    argColumn.addValue( 0 );    valueColumn.addValue( 0 );
    argColumn.addValue( 0.05 ); valueColumn.addDefault( );
    argColumn.addValue( 0.10 ); valueColumn.addValue(1.0);
    argColumn.addValue( 0.50 ); valueColumn.addDefault( );
    argColumn.addValue( 0.80 ); valueColumn.addValue(1.0);
    argColumn.addValue( 0.95 ); valueColumn.addDefault( );
    argColumn.addValue( 1.00 );

    BOOST_CHECK_THROW( valueColumn.applyDefaults( argColumn ) , std::invalid_argument );
    valueColumn.addValue(0.0);
    valueColumn.applyDefaults( argColumn );

    BOOST_CHECK( !valueColumn.hasDefault( ) );
    BOOST_CHECK_CLOSE( valueColumn[1] , 0.50 , 1e-6);
    BOOST_CHECK_CLOSE( valueColumn[3] , 1.00 , 1e-6);
    BOOST_CHECK_CLOSE( valueColumn[5] , 0.25 , 1e-6);
}
