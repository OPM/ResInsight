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

#define BOOST_TEST_MODULE SimpleTableTests

#include <boost/test/unit_test.hpp>


#include <opm/input/eclipse/EclipseState/Tables/ColumnSchema.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SimpleTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableColumn.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableIndex.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableSchema.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE( CreateTest ) {
    TableSchema schema;

    {
        ColumnSchema col1("Name1" , Table::INCREASING , Table::DEFAULT_NONE);
        ColumnSchema col2("Name2" , Table::INCREASING , Table::DEFAULT_NONE);
        schema.addColumn( col1 );
        schema.addColumn( col2 );
    }

    SimpleTable table(schema);

    BOOST_CHECK_THROW( table.addRow( {1,2,3} ), std::invalid_argument);
    table.addRow( {1,2} );
    table.addRow( {3,4} );

    {
        const auto& col1 = table.getColumn( 0 );
        const auto& col2 = table.getColumn( 1 );


        BOOST_CHECK_EQUAL( col1[0] , 1 );
        BOOST_CHECK_EQUAL( col2[0] , 2 );
        BOOST_CHECK_EQUAL( col1[1] , 3 );
        BOOST_CHECK_EQUAL( col2[1] , 4 );
    }

    BOOST_CHECK_THROW( table.get("NameX" , 0) , std::invalid_argument);
    BOOST_CHECK_THROW( table.get(3 , 0) , std::invalid_argument);

    BOOST_CHECK_THROW( table.get("Name1" , 3) , std::invalid_argument);
    BOOST_CHECK_THROW( table.get(0 , 3) , std::invalid_argument);


    BOOST_CHECK_EQUAL( table.get("Name1" , 0) , 1 );
    BOOST_CHECK_EQUAL( table.get("Name1" , 1) , 3 );
    BOOST_CHECK_EQUAL( table.get(0 , 0) , 1 );
    BOOST_CHECK_EQUAL( table.get(0 , 1) , 3 );

    BOOST_CHECK_EQUAL( table.get("Name2" , 0) , 2 );
    BOOST_CHECK_EQUAL( table.get("Name2" , 1) , 4 );
    BOOST_CHECK_EQUAL( table.get(1 , 0) , 2 );
    BOOST_CHECK_EQUAL( table.get(1 , 1) , 4 );

    {
        const auto& col = table.getColumn("Name1");
        auto exportCol = col.vectorCopy();

        BOOST_CHECK_EQUAL( col.size() , exportCol.size());
        for (size_t i = 0; i < col.size(); i++)
            BOOST_CHECK_EQUAL( col[i] , exportCol[i]);
    }
}

