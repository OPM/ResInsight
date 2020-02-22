/*
   Copyright (C) 2013  Equinor ASA, Norway.

   The file 'well_conn_collection.c' is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#include <stdbool.h>

#include <vector>

#include <ert/util/util.h>
#include <ert/util/type_macros.hpp>

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_rsthead.hpp>

#include <ert/ecl_well/well_const.hpp>
#include <ert/ecl_well/well_conn.hpp>
#include <ert/ecl_well/well_conn_collection.hpp>


#define WELL_CONN_COLLECTION_TYPE_ID 67150087

struct well_conn_collection_struct {
  UTIL_TYPE_ID_DECLARATION;
  std::vector<well_conn_type*> connection_list;
  std::vector<bool>            connection_list_owned;
};


UTIL_IS_INSTANCE_FUNCTION( well_conn_collection , WELL_CONN_COLLECTION_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION( well_conn_collection , WELL_CONN_COLLECTION_TYPE_ID )


well_conn_collection_type * well_conn_collection_alloc() {
  well_conn_collection_type * wellcc = new well_conn_collection_type();
  UTIL_TYPE_ID_INIT( wellcc , WELL_CONN_COLLECTION_TYPE_ID );
  return wellcc;
}

/*
  The collection takes ownership of the connection object and frees it
  when the collection is discarded.
*/

void well_conn_collection_add( well_conn_collection_type * wellcc , well_conn_type * conn) {
  wellcc->connection_list.push_back(conn);
  wellcc->connection_list_owned.push_back(true);
}

/*
  The collection only stores a refernce to the object, which will be destroyed by 'someone else'.
*/

void well_conn_collection_add_ref( well_conn_collection_type * wellcc , well_conn_type * conn) {
  wellcc->connection_list.push_back(conn);
  wellcc->connection_list_owned.push_back(false); 
}


void well_conn_collection_free( well_conn_collection_type * wellcc ) {
  for (size_t i = 0; i < wellcc->connection_list.size(); i++)
    if (wellcc->connection_list_owned[i])
      well_conn_free(wellcc->connection_list[i]);
  delete wellcc;
}

void well_conn_collection_free__( void * arg ) {
  well_conn_collection_type * wellcc = well_conn_collection_safe_cast( arg );
  well_conn_collection_free( wellcc );
}


int well_conn_collection_get_size( const well_conn_collection_type * wellcc ) {
  return wellcc->connection_list.size();
}


const well_conn_type * well_conn_collection_iget_const(const well_conn_collection_type * wellcc , int index) {
  int size = well_conn_collection_get_size( wellcc );
  if (index < size)
    return wellcc->connection_list[index];
  else
    return NULL;
}


well_conn_type * well_conn_collection_iget(const well_conn_collection_type * wellcc , int index) {
  int size = well_conn_collection_get_size( wellcc );
  if (index < size)
    return wellcc->connection_list[index];
  else
    return NULL;
}


int well_conn_collection_load_from_kw( well_conn_collection_type * wellcc ,
                                       const ecl_kw_type * iwel_kw ,
                                       const ecl_kw_type * icon_kw ,
                                       const ecl_kw_type * scon_kw ,
                                       const ecl_kw_type * xcon_kw ,
                                       int iwell ,
                                       const ecl_rsthead_type * rst_head) {

  const int iwel_offset = rst_head->niwelz * iwell;
  int num_connections   = ecl_kw_iget_int( iwel_kw , iwel_offset + IWEL_CONNECTIONS_INDEX );
  int iconn;

  for (iconn = 0; iconn < num_connections; iconn++) {
    well_conn_type * conn = well_conn_alloc_from_kw( icon_kw , scon_kw, xcon_kw, rst_head , iwell , iconn );
    if (conn)
      well_conn_collection_add( wellcc , conn );
  }
  return num_connections;

}

