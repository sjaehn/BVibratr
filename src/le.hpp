#include <cstdint>
#include <cstring>
#include <endian.h>


/**
Class for strict and system-independly storing data in little endian format 
and converting from / to the host unsigned int.
 */
class le16_t
{
private:
    uint16_t __data;
public:
    le16_t() : __data(0){}
    le16_t(const uint16_t uint16_t_value) : __data(htole16(uint16_t_value)) {}
    le16_t(const void* const src) : le16_t() {memcpy(&__data, src, sizeof(__data));}
    operator uint16_t() {return le16toh(__data);}
};


/**
Class for strict and system-independly storing data in little endian format 
and converting from / to the host unsigned int.
 */
class le32_t
{
private:
    uint32_t __data;
public:
    le32_t() : __data(0){}
    le32_t(const uint32_t uint32_t_value) : __data(htole32(uint32_t_value)) {}
    le32_t(const void* const src) : le32_t() {memcpy(&__data, src, sizeof(__data));}
    operator uint32_t() {return le32toh(__data);}
};


/**
Class for strict and system-independly storing data in little endian format 
and converting from / to the host unsigned int.
 */
class le64_t
{
private:
    uint64_t __data;
public:
    le64_t() : __data(0){}
    le64_t(const uint64_t uint64_t_value) : __data(htole64(uint64_t_value)) {}
    le64_t(const void* const src) : le64_t() {memcpy(&__data, src, sizeof(__data));}
    operator uint64_t() {return le64toh(__data);}
};