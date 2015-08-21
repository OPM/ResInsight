/*
  Copyright 2015 Statoil ASA.

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

#ifndef OPM_ERT_ECL_KW
#define OPM_ERT_ECL_KW

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <iostream>


#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/FortIO.hpp>



namespace ERT {
    template <typename T>
    class EclKW
    {
    public:
        EclKW(const std::string& kw, int size_);
        EclKW() { ; }

        static EclKW load(FortIO& fortio);

        size_t size() const {
            return static_cast<size_t>( ecl_kw_get_size( m_kw.get() ));
        }

        T& operator[](size_t index) {
            return *( static_cast<T *>( ecl_kw_iget_ptr( m_kw.get() , index) ));
        }

        
        void fwrite(FortIO& fortio) const {
            ecl_kw_fwrite( m_kw.get() , fortio.getPointer() );
        }

        
        void assignVector(const std::vector<T>& data) {
            if (data.size() == size())
                ecl_kw_set_memcpy_data( m_kw.get() , data.data() );
            else
                throw std::invalid_argument("Size error");
        }

        ecl_kw_type * getPointer() const {
	    return m_kw.get();
	}
        
    private:
        EclKW(ecl_kw_type * c_ptr) {
            reset(c_ptr);
        }
        
        void reset(ecl_kw_type * c_ptr) {
            m_kw.reset( c_ptr , ecl_kw_free);
        }

        
        static EclKW checkedLoad(FortIO& fortio, ecl_type_enum expectedType) {
            ecl_kw_type * c_ptr = ecl_kw_fread_alloc( fortio.getPointer() );
            if (c_ptr) {
                if (ecl_kw_get_type( c_ptr ) == expectedType) 
                    return EclKW( c_ptr );
                else
                    throw std::invalid_argument("Type error");
            } else
                throw std::invalid_argument("fread kw failed - EOF?");
        }

        
        std::shared_ptr<ecl_kw_type> m_kw;
    };
}

#endif
