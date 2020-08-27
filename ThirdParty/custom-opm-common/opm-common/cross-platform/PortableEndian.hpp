#ifndef PORTABLE_ENDIAN_H__
#define PORTABLE_ENDIAN_H__

#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64)) && !defined(__WINDOWS__)

#	define __WINDOWS__

#endif

#if defined(__WINDOWS__)
#define __builtin_bswap16(x) _byteswap_ushort((x))
#define __builtin_bswap32(x) _byteswap_ulong((x))
#define __builtin_bswap64(x) _byteswap_uint64((x))
#endif

#endif // PORTABLE_ENDIAN_H__