#ifndef ERT_UNIQUE_PTR
#define ERT_UNIQUE_PTR

#include <memory>

namespace ERT {

    template <typename T , void (*F)(T*)>
    struct deleter
    {
        void operator() (T * arg) const {
            F( arg );
        }
    };

    template <typename T , void (*F)(T*)>
    using ert_unique_ptr = std::unique_ptr<T, deleter<T,F> >;

}

#endif


