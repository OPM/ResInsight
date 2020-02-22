#ifndef ECL_TEST_MATCHERS_HPP
#define ECL_TEST_MATCHERS_HPP

#include <vector>
#include <string>

#include <catch/catch.hpp>

class FstreamCount : public Catch::MatcherBase< int > {
    public:
        FstreamCount( int ex ) : expected( ex ) {}

        virtual bool match( const int& count ) const override {
            return count == this->expected;
        }

    protected:
        int expected;
};

struct FreadCount : public FstreamCount {
    using FstreamCount::FstreamCount;
    virtual std::string describe() const override {
        return "elems read, expected " + std::to_string( this->expected );
    }
};

struct FwriteCount : public FstreamCount {
    using FstreamCount::FstreamCount;

    virtual std::string describe() const override {
        return "elems written, expected " + std::to_string( this->expected );
    }
};

struct Err {
    Err( int ex ) : expected( ex ) {}

    bool operator == ( Err other ) const {
        return this->expected == other.expected;
    }
    bool operator != ( Err other ) const {
        return !(*this == other);
    }

    static Err ok()                { return ECL_OK; }
    static Err invalid_record()    { return ECL_INVALID_RECORD; }
    static Err read()              { return ECL_ERR_READ; }

    int expected;
};

namespace Catch {
template<>
struct StringMaker< Err > {
    static std::string convert( const Err& err ) {
        switch( err.expected ) {
            case ECL_OK:                return "OK";
            case ECL_ERR_SEEK:          return "SEEK";
            case ECL_ERR_READ:          return "READ";
            case ECL_ERR_WRITE:         return "WRITE";
            case ECL_INVALID_RECORD:    return "INVALID RECORD";
            case ECL_EINVAL:            return "INVALID ARGUMENT";
            case ECL_ERR_UNKNOWN:       return "UNKNOWN";
        }

        return "Unknown error";
    }
};

}

#endif //ECL_TEST_MATCHERS_HPP
