#include "gtest/gtest.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <memory>
#include <numeric>

#include "Properties.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

class PropertiesClient
{
public:
    PropertiesClient( std::shared_ptr<Channel> channel )
        : m_stub( rips::Properties::NewStub( channel ) )
    {
    }
    Status GetActiveCellProperty( rips::PropertyType   propType,
                                  const std::string&   propertyName,
                                  int                  timeStep,
                                  std::vector<double>* results ) const
    {
        rips::PropertyRequest request;
        rips::CaseRequest*    requestCase = new rips::CaseRequest;
        requestCase->set_id( 0 );
        request.set_allocated_case_request( requestCase );
        request.set_grid_index( 0 );
        request.set_porosity_model( rips::PorosityModelType::MATRIX_MODEL );
        request.set_property_type( propType );
        request.set_property_name( propertyName );
        request.set_time_step( timeStep );
        rips::PropertyChunk resultArray;
        ClientContext       context;

        std::unique_ptr<ClientReader<rips::PropertyChunk>> reader = m_stub->GetActiveCellProperty( &context, request );
        while ( reader->Read( &resultArray ) )
        {
            results->insert( results->end(), resultArray.values().begin(), resultArray.values().end() );
        }
        return reader->Finish();
    }

private:
    std::unique_ptr<rips::Properties::Stub> m_stub;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( DISABLED_RiaGrpcInterface, SoilAverage )
{
    PropertiesClient client( grpc::CreateChannel( "localhost:50051", grpc::InsecureChannelCredentials() ) );

    for ( size_t i = 0; i < 10; ++i )
    {
        std::vector<double> results;
        Status status = client.GetActiveCellProperty( rips::PropertyType::DYNAMIC_NATIVE, "SOIL", i, &results );
        std::cout << "Number of results: " << results.size() << std::endl;
        double sum = std::accumulate( results.begin(), results.end(), 0.0 );
        std::cout << "Avg: " << sum / static_cast<double>( results.size() ) << std::endl;
        EXPECT_EQ( grpc::OK, status.error_code() );
    }
}
