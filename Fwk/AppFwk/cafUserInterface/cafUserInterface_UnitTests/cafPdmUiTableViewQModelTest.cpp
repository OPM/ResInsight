
#include "gtest/gtest.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiTableViewQModel.h"

#include <QModelIndex>

using namespace caf;

class TableRowObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    TableRowObject()
    {
        CAF_PDM_InitObject( "TableRowObject" );
        CAF_PDM_InitField( &m_value, "Value", 1.0, "Value" );
    }

    caf::PdmField<double> m_value;
};
CAF_PDM_SOURCE_INIT( TableRowObject, "TableRowObject" );

class TableContainerObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    TableContainerObject()
    {
        CAF_PDM_InitObject( "TableContainerObject" );
        CAF_PDM_InitFieldNoDefault( &m_rows, "Rows", "Rows" );
    }

    caf::PdmChildArrayField<TableRowObject*> m_rows;
};
CAF_PDM_SOURCE_INIT( TableContainerObject, "TableContainerObject" );

//--------------------------------------------------------------------------------------------------
/// An invalid model index has row and column set to -1, and must not be used to index the child array
//--------------------------------------------------------------------------------------------------
TEST( PdmUiTableViewQModelTest, InvalidModelIndex )
{
    TableContainerObject container;
    container.m_rows.push_back( new TableRowObject );

    PdmUiTableViewQModel model( nullptr );
    model.setArrayFieldAndBuildEditors( &container.m_rows, "" );

    EXPECT_EQ( 1, model.rowCount() );

    QModelIndex invalidIndex;
    EXPECT_FALSE( invalidIndex.isValid() );

    EXPECT_EQ( nullptr, model.getField( invalidIndex ) );
    EXPECT_EQ( nullptr, model.pdmObjectForRow( invalidIndex.row() ) );
}
