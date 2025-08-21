#ifndef PSCHSL_CRC_H
#define PSCHSL_CRC_H

#include <stdint.h>
#include <stddef.h>

uint32_t PSCHSL__crc32(const void*, size_t);
uint64_t PSCHSL__crc64(const void*, size_t);
uint32_t PSCHSL__strcrc32(const char*);
uint64_t PSCHSL__strcrc64(const char*);
uint32_t PSCHSL__strcasecrc32(const char*);
uint64_t PSCHSL__strcasecrc64(const char*);
uint32_t PSCHSL__ccrc32(uint32_t, const void*, size_t);
uint64_t PSCHSL__ccrc64(uint64_t, const void*, size_t);
uint32_t PSCHSL__cstrcrc32(uint32_t, const char*);
uint64_t PSCHSL__cstrcrc64(uint64_t, const char*);
uint32_t PSCHSL__cstrcasecrc32(uint32_t, const char*);
uint64_t PSCHSL__cstrcasecrc64(uint64_t, const char*);

#endif
