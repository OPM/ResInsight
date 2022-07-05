/*
  Copyright 2016 Statoil ASA.

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

#ifndef OPM_FUNCTIONAL_HPP
#define OPM_FUNCTIONAL_HPP

#include <algorithm>
#include <iterator>
#include <vector>
#include <numeric>
#include <functional>

namespace Opm {

namespace fun {

    /*
     * The Utility/Functional library provides convenient high level
     * functionality and higher order functions inspiried by functional
     * languages (in particular Haskell) and modern C++. The goal is to provide
     * lightweight features that reduce boilerplate and make code more
     * declarative.
     */

    /*
     * map :: (a -> b) -> [a] -> [b]
     *
     * maps the elements [a] of the passed container C to [b], by using the
     * passed function f :: a -> b. Works like map in haskell, lisp, python etc.
     *
     * C can be any foreach-compatible container (that supports .begin,
     * .end), but will always return a vector.
     *
     * F can be any Callable, that is both function pointer,
     * operator()-providing class or std::function, including lambdas. F is
     * typically passed by reference. F must be unary of type A (which must
     * match what C::const_iterator::operator* returns) and have return
     * type B (by value).
     *
     * In short, this function deal with vector allocation, resizing and
     * population based on some function f.
     *
     * fun::map( f, vec ) is equivalent to:
     *  vector dst;
     *  for( auto& x : vec ) dst.push_back( f( x ) );
     *  return dst;
     *
     * The behaviour is undefined if F has any side effects.
     *
     * --
     *
     * int plus1( int x ) { return x + 1; }
     * base_vec = { 0, 1, 2, 3, 4 };
     * vec = fun::map( &plus1, base_vec );
     *
     * vec => { 1, 2, 3, 4, 5 }
     *
     * --
     *
     * int mul2 = []( int x ) { return x * 2; };
     * base_vec = { 0, 1, 2, 3, 4 };
     * vec = fun::map( mul2, base_vec );
     *
     * vec => { 0, 2, 4, 6, 8 };
     *
     */
    template <typename F, typename C>
    auto map(F&& f, C&& c)
    {
        using Val = std::remove_cv_t<std::remove_reference_t<
            decltype(std::invoke(std::forward<F>(f), *std::begin(std::forward<C>(c))))>>;

        std::vector<Val> ret{};
        ret.reserve(std::size(std::forward<C>(c)));

        std::transform(std::begin(c), std::end(c), std::back_inserter(ret), std::forward<F>(f));
        return ret;
    }
    /*
     * concat :: [[a]] -> [a]
     *
     * A primitive concat taking a vector of vectors, flattened into a
     * single 1 dimensional vector. Moves all the elements so no unecessary
     * copies are done.
     *
     * vec = { { 1 }, { 2, 2 }, { 3, 3, 3 } }
     * cvec = concat( vec ) => { 1, 2, 2, 3, 3, 3 }
     */
    template< typename A >
    std::vector< A > concat( std::vector< std::vector< A > >&& src ) {
        const auto size = std::accumulate( src.begin(), src.end(), 0,
                []( std::size_t acc, const std::vector< A >& x ) {
                    return acc + x.size();
                    }
                );

        std::vector< A > dst;
        dst.reserve( size );

        for( auto& x : src )
            std::move( x.begin(), x.end(), std::back_inserter( dst ) );

        return dst;
    }


    /*
     * iota :: int -> [int]
     * iota :: (int,int) -> [int]
     *
     * iota (ι) is borrowed from the APL programming language. This particular
     * implementation behaves as a generator-like constant-space consecutive
     * sequence of integers [m,n). Written to feel similar to std::iota, but as
     * a producer instead of straight-up writer. This is similar to python2.7s
     * xrange(), python3s range() and haskell's [0..(n-1)]. Some examples
     * follow.
     *
     * Notes:
     * * iota defaults to [0,n)
     * * iota uses 0 indexing to feel more familiar to C++'s zero indexing.
     * * iota can start at negative indices, but will always count upwards.
     * * iota::const_iterator does not support operator-- (which would allow
     *   support for reverse iterators). This can be implemented if need arises.
     * * iota is meant to play nice with the rest of fun and to be able to
     *   replace mundane for loops when the loops only purpose is to create the
     *   sequence of elements. iota can feel more declarative and work better
     *   with functions.
     * * iota adds value semantics to things that in C++ normally relies on
     *   variable mutations. iota is meant to make it less painful to write
     *   immutable and declarative code.
     * * as with all iterators, iota( n, m ) behaviour is undefined if m < n
     * * unlike python's range, iota doesn't support steps (only increments).
     *   this is by design to keep this simple and minimal, as well as the name
     *   iota being somewhat unsuitable for stepping ranges. If the need for
     *   this arises it will be a separate function.
     *
     * fun::iota( 5 ) => [ 0, 1, 2, 3, 4 ]
     * fun::iota( 3 ) => [ 0, 1, 2 ]
     * fun::iota( 1, 6 ) => [ 1, 2, 3, 4, 5 ]
     *
     * --
     *
     * std::vector< int > vec ( 5, 0 );
     * std::iota( vec.begin(), vec.end(), 0 );
     * vec => [ 0, 1, 2, 3, 4 ]
     *
     * fun::iota i( 5 );
     * std::vector vec( i.begin(), i.end() );
     * vec => [ 0, 1, 2, 3, 4 ]
     *
     * --
     *
     * int plus( int x ) { return x + 1; }
     * auto vec = fun::map( &plus, fun::iota( 5 ) );
     * vec => [ 1, 2, 3, 4, 5 ]
     *
     * is equivalent to
     *
     * int plus( int x ) { return x + 1; }
     * std::vector< int > vec;
     * for( int i = 0; i < 5; ++i )
     *     vec.push_back( plus( i ) );
     * vec => [ 1, 2, 3, 4, 5 ]
     *
     * --
     *
     * While not the primary intended use case, this enables foreach loop
     * syntax over intervals:
     *
     * for( auto i : fun::iota( 5 ) )
     *    std::cout << i << " ";
     *
     * => 0 1 2 3 4
     *
     * for( auto i : fun::iota( 1, 6 ) )
     *    std::cout << i << " ";
     *
     * => 1 2 3 4 5
     *
     */
    class iota {
        public:
            explicit iota( int end );
            iota( int begin, int end );

            class const_iterator {
                public:
                    using difference_type = int;
                    using value_type = int;
                    using pointer = int*;
                    using reference = int&;
                    using iterator_category = std::forward_iterator_tag;

                    const_iterator() = default;

                    int operator*() const;

                    const_iterator& operator++();
                    const_iterator operator++( int );

                    bool operator==( const const_iterator& rhs ) const;
                    bool operator!=( const const_iterator& rhs ) const;

                private:
                    explicit const_iterator( int );
                    int value;

                    friend class iota;
            };

            size_t size() const;

            const_iterator begin() const;
            const_iterator end() const;

        private:
            int first;
            int last;
    };

}
}

#endif //OPM_FUNCTIONAL_HPP
