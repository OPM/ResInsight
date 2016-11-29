#ifndef OPM_ERT_SMSPEC_HPP
#define OPM_ERT_SMSPEC_HPP

#include <memory>
#include <string>

#include <ert/ecl/smspec_node.h>
#include <ert/util/ert_unique_ptr.hpp>

namespace ERT {

    class smspec_node {
        public:
            smspec_node( const smspec_node& );
            smspec_node( smspec_node&& );

            smspec_node& operator=( const smspec_node& );
            smspec_node& operator=( smspec_node&& );

            smspec_node(
                    ecl_smspec_var_type,
                    const std::string& wgname,
                    const std::string& keyword
            );

            smspec_node( const std::string& keyword );

            smspec_node( const std::string& keyword,
                    const int dims[ 3 ],
                    const int ijk[ 3 ] );

            smspec_node( const std::string& keyword,
                    const std::string& wellname,
                    const int dims[ 3 ],
                    const int ijk[ 3 ] );

            smspec_node( const std::string& keyword,
                    const int dims[ 3 ],
                    int region );

            int type() const;
            const char* keyword() const;
            const char* wgname() const;
            const char* key1() const;
            int num() const;
            smspec_node_type* get();
            const smspec_node_type* get() const;

        private:
            smspec_node(
                ecl_smspec_var_type,
                const char*, const char*, const char*, const char*,
                const int[3], int, int = 0, float = 0 );

            ert_unique_ptr< smspec_node_type, smspec_node_free > node;
    };

}

#endif //OPM_ERT_SMSPEC_HPP
