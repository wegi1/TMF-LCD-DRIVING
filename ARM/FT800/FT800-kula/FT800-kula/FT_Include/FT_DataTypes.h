#ifndef _FT_DATATYPES_H_
#define _FT_DATATYPES_H_

#include <stdint.h>

#define FT_FALSE           (0)
#define FT_TRUE            (1)

typedef uint8_t ft_uint8_t;
typedef char ft_char8_t;
typedef signed char ft_schar8_t;
typedef unsigned char ft_uchar8_t;
typedef int16_t  ft_int16_t;
typedef uint16_t ft_uint16_t;
typedef uint32_t ft_uint32_t;
typedef int32_t ft_int32_t;
typedef void ft_void_t;

typedef _Bool ft_bool_t;

typedef const unsigned char  ft_prog_uchar8_t;
typedef const char   ft_prog_char8_t;
typedef const uint16_t ft_prog_uint16_t;


#define FT_PROGMEM 
#define ft_pgm_read_byte_near 
#define ft_pgm_read_byte 
#define ft_pgm_read_word 
#define ft_random(x)		(rand() %(x))

#define TRUE     (1)

#endif /*_FT_DATATYPES_H_*/


/* Nothing beyond this*/




