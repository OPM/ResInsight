#include <memory>

int main( int argc , char ** argv) {
    std::shared_ptr<int> ptr = std::make_shared<int>( 10 );
}
