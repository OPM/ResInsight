#ifndef ECL_FORTIO_H
#define ECL_FORTIO_H

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * As per the gnu fortran manual, int32 is sufficient for the record byte
 * marker. optionally we could support 8-byte markers with either compile-time
 * configuration or a run-time switch
 *
 * http://gcc.gnu.org/onlinedocs/gfortran/File-format-of-unformatted-sequential-files.html
 *
 * By default, all functions assume strict fortran compatibility (i.e. with
 * trailing record size) and network (big-endian) byte order.
 *
 *
 * A Fortran program writes unformatted data to file in a statemente like:
 *
 *    integer array(100)
 *    write(unit) array
 *
 * it actually writes a head and tail in addition to the actual
 * data. The header and tail is a 4 byte integer, which value is the
 * number of bytes in the immediately following record. I.e. what is
 * actually found on disk after the Fortran code above is:
 *
 *   | 400 | array ...... | 400 |
 *
 */

/*
 * The ecl_fio functions are exception safe, that is, if a function fails, the
 * file pointer is rewinded to before the function was called, and output
 * parameters are not modified, as if the function was never called.
 *
 * This comes with a few exceptions:
 * 1. if ECL_ERR_SEEK is returned, the roll-back of the file pointer itself
 *    failed and NOTHING IS GUARANTEED. The file stream is left in an unspecified
 *    state, and must be recovered accordingly.
 * 2. in eclfio_get, the output record buffer must always be considered dirty
 *    and incomplete unless the function suceeds, or ECL_EINVAL is returned.
 *
 *
 * ECL_ERR_SEEK should be rather rare, but to provide strong guarantees, this
 * error must be handled carefully.
 */

/*
 * every function takes a const char* opts parameter. This is a tiny
 * configuration language inspired by printf and fopen. every character not in
 * the set of keys is ignored. the opts parameter must be null terminated.
 *
 * if two options setting the same parameter (e.g. i and f, or e and E), the
 * last one in the option string takes effect.
 *
 * options
 * -------
 *  record data types:
 *      c - characters, sizeof(char)
 *      i - (signed)integers, sizeof(int32_t), default
 *      f - single-precision float, sizeof(float)
 *      d - double-precision float, sizeof(double)
 *
 * behaviour:
 *      E - assume big-endian record data (default)
 *      e - assume little-endian record data
 *      t - transform/byteswap data according to data type (default)
 *      T - don't transform/byteswap data (does not affect heads/tails)
 *
 * endianness parameter applies to both head, tail, and data, but head/tail can
 * be interepreted with endianness byteswapping data by disabling transform
 *
 * fault tolerance:
 *      # - ignore size hint
 *      ~ - force no-tail (assume only head)
 *      $ - allow no-tail (don't fail on missing tail)
 */

/*
 * Get the size (number of elements) of the current record. The file position
 * is approperiately rewinded afterwards, as if the function was never called.
 *
 * If this function fails, out is not modified.
 *
 * If the read fails, ECL_ERR_READ is returned.
 *
 * This function is largely intended for peeking the size of the next record,
 * to approperiately allocate a large enough buffer, which is useful when
 * dealing with unknown files. If it is know in advance how large the records
 * are, it is not necessary to call this function before reading a record.
 */
int eclfio_sizeof( FILE*, const char* opts, int32_t* out );

/*
 * Advance the file position n records. The file position is reset if the
 * function fails, as if the function was never called.
 *
 * Returns ECL_OK if all records were skipped. If it fails, either
 * ECL_INVALID_RECORD or ECL_ERR_READ is returned, depending on the source of
 * the error, same rules as that of eclfio_get.
 *
 * This function does not distinguish seek errors for any n not +-1, so to
 * figure out which record fails, one record at a time must be skipped.
 */
int eclfio_skip( FILE*, const char* opts, int n );

/*
 * Get the next record, and its number of elements.
 *
 * The record buffer is generally assumed to be of approperiate size, which can
 * be queried with eclfio_sizeof.
 *
 * On success, the value of recordsize denotes the number of elements read,
 * whose size is determined by the "cifd" options. It is generally assumed that
 * recordsize upon calling this function contains the size of the record
 * buffer, as a failsafe mechanism - if a record is larger than this value, the
 * read will be aborted and the file position rolled back. To opt out of this
 * check, add # to opts.
 *
 * Both recordsize and record can be NULL, in which case the number of elements
 * read is not returned, and no data is returned respectively. This allows
 * precise reporting on how many elements each skipped records contains.
 *
 * If the elementsize is larger than 1, and transformation has not been
 * explicitly disabled, endianness will be converted appropriately.
 *
 * It is assumed that all record has an appropriate head and tail. If it is
 * know that no record has a tail, force this by passing ~ in opts. However, if
 * it is uncertain if all records has tails, or it's alternating between tail
 * and no-tail, the $ option tries to recover from missing tails by assuming
 * the current position is the start of the next record.
 *
 * The contents of record* is unspecified in case of read failures, and may not
 * be relied upon. If the function returns ECL_EINVAL, the output record is
 * untouched.
 *
 * This function returns ECL_OK upon success, ECL_ERR_READ in case of read- or
 * seek errors, ECL_INVALID_RECORD if either the record tail is broken and
 * options is set accordingly. The list of error codes is not exhaustive, and
 * robust code should have fallthrough error handling cases.
 */
int eclfio_get( FILE*, const char* opts, int32_t* recordsize, void* record );

/*
 * Put a record of nmemb elements
 *
 * This function will write both head and tail, unless tail writing is
 * explicitly disabled with ~. If (nmemb * elemsize) overflows int32, the write
 * is aborted and ECL_EINVAL is returned.
 *
 * put largely follows the same rules as get, including those of endianness.
 * The file pointer is rolled back if any part of the function should fail, as
 * if the function was never called.
 *
 * If a write fails after partial writes, no attempts are made to roll back
 * written changes.
 *
 * Returns ECL_OK on success, or ECL_ERR_WRITE on failure. If ECL_ERR_SEEK is
 * returned, the integrity of the file stream can not be guaranteed, and its
 * state is considered unspecified.
 */
int eclfio_put( FILE*, const char* opts, int nmemb, const void* );

enum ecl_errno {
    ECL_OK = 0,
    ECL_ERR_UNKNOWN,
    ECL_ERR_SEEK,
    ECL_ERR_READ,
    ECL_ERR_WRITE,
    ECL_INVALID_RECORD,
    ECL_EINVAL,
};

#ifdef __cplusplus
}
#endif

#endif //ECL_FORTIO_H
