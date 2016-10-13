/*
   The functions buffer_fread_string() and buffer_fwrite_string()
   should not be used; the embedded integer just creates chaos and
   should the sole responsability of the calling scope.
*/

/**
   Storing strings:
   ----------------

   When storing a string (\0 terminated char pointer) what is actually
   written to the buffer is

     1. The length of the string - as returned from strlen().
     2. The string content INCLUDING the terminating \0.


*/


/**
   This function will return a pointer to the current position in the
   buffer, and advance the buffer position forward until a \0
   terminater is found. If \0 is not found the thing will abort().

   Observe that the return value will point straight into the buffer,
   this is highly volatile memory, and in general it will be safer to
   use buffer_fread_alloc_string() to get a copy of the string.
*/

const char * buffer_fread_string(buffer_type * buffer) {
  int    string_length = buffer_fread_int( buffer );
  char * string_ptr    = &buffer->data[buffer->pos];
  char   c;
  buffer_fskip( buffer , string_length );
  c = buffer_fread_char( buffer );
  if (c != '\0')
    util_abort("%s: internal error - malformed string representation in buffer \n",__func__);
  return string_ptr;
}



char * buffer_fread_alloc_string(buffer_type * buffer) {
  return util_alloc_string_copy( buffer_fread_string( buffer ));
}



/**
   Observe that this function writes a leading integer string length.
*/
void buffer_fwrite_string(buffer_type * buffer , const char * string) {
  buffer_fwrite_int( buffer , strlen( string ));               /* Writing the length of the string */
  buffer_fwrite(buffer , string , 1 , strlen( string ) + 1);   /* Writing the string content ** WITH ** the terminating \0 */
}

