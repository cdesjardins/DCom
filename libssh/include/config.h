/* Name of package */
#define PACKAGE "libssh"

/* Version number of package */
#define VERSION "0.4.3"

/* #undef LOCALEDIR */
/* #undef DATADIR */
#define LIBDIR "lib"
#define PLUGINDIR "plugins-4"
/* #undef SYSCONFDIR */

/************************** HEADER FILES *************************/

/* Define to 1 if you have the <argp.h> header file. */
/* #undef HAVE_ARGP_H */

/* Define to 1 if you have the <pty.h> header file. */
/* #undef HAVE_PTY_H */

/* Define to 1 if you have the <termios.h> header file. */
/* #undef HAVE_TERMIOS_H */

/* Define to 1 if you have the <openssl/aes.h> header file. */
#define HAVE_OPENSSL_AES_H 1

/* Define to 1 if you have the <wspiapi.h> header file. */
#define HAVE_WSPIAPI_H 1

/* Define to 1 if you have the <openssl/blowfish.h> header file. */
#define HAVE_OPENSSL_BLOWFISH_H 1

/* Define to 1 if you have the <openssl/des.h> header file. */
#define HAVE_OPENSSL_DES_H 1

/*************************** FUNCTIONS ***************************/

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF

/* Define to 1 if you have the `_vsnprintf' function. */
/* #undef HAVE__VSNPRINTF */

/* Define to 1 if you have the `_vsnprintf_s' function. */
/* #undef HAVE__VSNPRINTF_S */

/* Define to 1 if you have the `snprintf' function. */
/* #undef HAVE_SNPRINTF */

/* Define to 1 if you have the `_snprintf' function. */
#define HAVE__SNPRINTF

/* Define to 1 if you have the `_snprintf_s' function. */
#define HAVE__SNPRINTF_S


/* Define to 1 if you have the `strncpy' function. */
#define HAVE_STRNCPY

/* Define to 1 if you have the `cfmakeraw' function. */
/* #undef HAVE_CFMAKERAW */

/* Define to 1 if you have the `getaddrinfo' function. */
#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the `gethostbyname' function. */
#define HAVE_GETHOSTBYNAME 1

/* Define to 1 if you have the `poll' function. */
/* #undef HAVE_POLL */

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* Define to 1 if you have the `regcomp' function. */
/* #undef HAVE_REGCOMP */

/*************************** LIBRARIES ***************************/

/* Define to 1 if you have the `crypto' library (-lcrypto). */
#define HAVE_LIBCRYPTO 1

/* Define to 1 if you have the `gcrypt' library (-lgcrypt). */
/* #undef HAVE_LIBGCRYPT */

/* Define to 1 if you have the `z' library (-lz). */
/* #undef HAVE_LIBZ */

/**************************** OPTIONS ****************************/

/* Define to 1 if you want to enable ZLIB */
/* #undef WITH_LIBZ */

/* Define to 1 if you want to enable SFTP */
#define WITH_SFTP 1

/* Define to 1 if you want to enable SSH1 */
/* #undef WITH_SSH1 */

/* Define to 1 if you want to enable server support */
#define WITH_SERVER 1

/* Define to 1 if you want to enable debug output for crypto functions */
/* #undef DEBUG_CRYPTO */

/* Define to 1 if you want to enable pcap output support (experimental) */
/* #undef WITH_PCAP */

/* Define to 1 if you want to enable calltrace debug output */
#define DEBUG_CALLTRACE 1

/*************************** ENDIAN *****************************/

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
/* #undef WORDS_BIGENDIAN */

/************************* MS Windows ***************************/

#ifdef _WIN32
# ifdef _MSC_VER
/* On Microsoft compilers define inline to __inline on all others use inline */
#  undef inline
#  define inline __inline

#  undef strdup
#  define strdup _strdup
# endif // _MSC_VER
#endif /* _WIN32 */

