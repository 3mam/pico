#ifndef MACRO_H
#define MACRO_H

#include <stdint.h>
#include <stdlib.h>
#define NEW(type) (type*)calloc(1, sizeof(type))
#define MAKE(type, size) (type*)calloc(size, sizeof(type))
#define LEN(x) sizeof(x) / sizeof(x[0])

typedef uint8_t ui8;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;
typedef float f32;
typedef double f64;

typedef ui8 bool;
#define TRUE 1
#define FALSE 0

#endif  // MACRO_H
