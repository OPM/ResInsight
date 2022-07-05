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

#define BOOST_TEST_MODULE TableSchemaTests

#include <boost/test/unit_test.hpp>


#include <opm/input/eclipse/EclipseState/Tables/TableSchema.hpp>
#include <opm/input/eclipse/EclipseState/Tables/ColumnSchema.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableEnums.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE( CreateTest ) {
    TableSchema schema;
    ColumnSchema col1("Name1" , Table::INCREASING , Table::DEFAULT_NONE);
    ColumnSchema col2("Name2" , Table::INCREASING , Table::DEFAULT_NONE);
    BOOST_CHECK_EQUAL( 0U , schema.size( ) );

    schema.addColumn( col1 );
    BOOST_CHECK_EQUAL( 1U , schema.size( ) );

    schema.addColumn( col2 );
    BOOST_CHECK_EQUAL( 2U , schema.size( ) );

    BOOST_CHECK_THROW( schema.getColumn( "NO/NOT/THIS/COLUMN" ) , std::invalid_argument );
    BOOST_CHECK_THROW( schema.getColumn( 5 ) , std::invalid_argument );
}

