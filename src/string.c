#include <stddef.h>
#include "headers/string.h"
#include "headers/memory.h"
#include "headers/io.h"

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    // Return difference of first differing characters (unsigned char for correct ordering)
    return (unsigned char)(*s1) - (unsigned char)(*s2);
}

void* memset(void *dest, int value, size_t count)
{
    unsigned char val = (unsigned char)value;

    __asm__ __volatile__ (
        "rep stosb"
        : "+D"(dest), "+c"(count)
        : "a"(val)
        : "memory"
    );

    return dest;
}


void* dw_memset(void *dest, uint32_t value, size_t count)
{
    __asm__ __volatile__ (
        "rep stosl"
        : "+D"(dest), "+c"(count)
        : "a"(value)
        : "memory"
    );

    return dest;
}


void* memcpy(void* dest, const void* src, size_t n) {
    if (n <= 0) return dest;

    uintptr_t d = (uintptr_t)dest;
    uintptr_t s = (uintptr_t)src;
    size_t dw_n = n / 4;
    size_t b_n = n % 4;

    void* ret = dest;

    __asm__ volatile (
        "rep movsl"
        : "+D" (d), "+S" (s), "+c" (dw_n)
        :
        : "memory"
    );

    __asm__ volatile (
        "rep movsb"
        : "+D" (d), "+S" (s), "+c" (b_n)
        :
        : "memory"
    );

    return ret;
}




size_t strlen(const char* str){
    size_t size = 0;
    while (*str)
    {
        size++;
        str++;
    }
    return size;
}


bool Starts_With(const char* string, const char* prefix) {
    while (*prefix) {
        if (*string == '\0' || *string != *prefix) {
            return false;
        }
        string++;  
        prefix++; 
    }
    return true;
}


char** Split(const char* string, char separator, int max_tokens, int* out_count) {
    if (!string) {
        if (out_count) *out_count = 0;
        return NULL;
    }

    int capacity = (max_tokens > 0) ? max_tokens : 8; 
    char** tokens = malloc(sizeof(char*) * capacity);
    if (!tokens) return NULL;

    int token_index = 0;
    const char* start = string;
    const char* ptr = string;

    while (*ptr) {
        if (*ptr == separator) {
            int len = ptr - start;
            char* token = malloc(len + 1);
            if (token) {
                strncpy(token, start, len);
                token[len] = '\0';
                tokens[token_index++] = token;
            }

            start = ptr + 1;

            // Resize if needed (only if max_tokens == 0)
            if (max_tokens <= 0 && token_index >= capacity) {
                capacity *= 2;
                char** temp = realloc(tokens, sizeof(char*) * capacity);
                if (!temp) {
                    // On failure, cleanup and return NULL
                    for (int i = 0; i < token_index; i++) free(tokens[i]);
                    free(tokens);
                    if (out_count) *out_count = 0;
                    return NULL;
                }
                tokens = temp;
            } else if (max_tokens > 0 && token_index >= max_tokens - 1) {
                break;
            }
        }
        ptr++;
    }

    // Final token
    if (*start != '\0') {
        char* token = strdup(start);
        if (token) {
            tokens[token_index++] = token;
        }
    }

    if (out_count) *out_count = token_index;
    return tokens;
}

void EndSplit(char** tokens, int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}



char* Concat(char** list, size_t size, char linking_char) {
    if (size == 0) {
        return NULL;
    }

    size_t totalLen = 0;
    for (size_t i = 0; i < size; i++) {
        totalLen += strlen(list[i]);
    }

    size_t separatorSize = (linking_char && size > 1) ? (size - 1) : 0;

    char* result = (char*) malloc(totalLen + separatorSize + 1); 
    if (!result) return NULL;

    size_t offset = 0;
    for (size_t i = 0; i < size; i++) {
        size_t len = strlen(list[i]);
        memcpy(result + offset, list[i], len);
        offset += len;

        if (linking_char && i < size - 1) {
            result[offset] = linking_char;
            offset += 1;
        }
    }

    result[offset] = '\0';
    return result;
}



bool InsertChar(char* str, uint32_t pos, char c) {
    uint32_t length = strlen(str);
    
    if (pos > length ) return false;

    for (uint32_t i = length; i > pos; i--) {
        str[i] = str[i - 1];
    }

    str[pos] = c;
    length++;

    str[length] = '\0';

    return true;
}

void RemoveChar(char* str, uint32_t index) {
    uint32_t length = strlen(str); 
    if (index >= length) return;    

    for (uint32_t i = index; i < length - 1; i++) {
        str[i] = str[i + 1];
    }

    str[length - 1] = '\0'; 
}


int memcmp(const void *s1, const void *s2, size_t n) {
    unsigned char *p1 = (unsigned char *)s1;
    unsigned char *p2 = (unsigned char *)s2;

    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

char *strchr(const char *str, int c) {
    while (*str != '\0') {
        if (*str == c) {
            return (char *)str;
        }
        str++;
    }
    return NULL;
}

char *strdup(const char *s){
    size_t len = strlen(s);
    char* pter = malloc(len+1);
    if(!pter)return NULL;
    memcpy(pter,s,len+1);
    return pter;
}

char *strndup( const char *str, size_t size ){
    size_t stringlen = strlen(str);
    size_t len = size < stringlen? size : stringlen; 
    char* pter = malloc(len+1);
    if(!pter)return NULL;
    memcpy(pter,str,len);
    pter[len] = '\0';
    return pter;
}

char *strcpy(char *dest, const char *src) {
    char *ret = dest;
    while ((*dest++ = *src++));
    return ret;
}


char *strncpy(char *dest, const char *src, size_t n) {
    size_t i = 0;
    while (i < n && src[i]) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i++] = '\0';
    }
    return dest;
}


void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    if (d == s || n == 0)
        return dest;

    size_t dwords = n / 4;
    size_t bytes = n % 4;

    if (d < s) {
        // Forward copy: rep movsl (4 bytes), then rep movsb (bytes)
        asm volatile (
            "rep movsl\n\t"
            "rep movsb"
            : "+S"(s), "+D"(d), "+c"(dwords)
            :
            : "memory"
        );

        if (bytes) {
            asm volatile (
                "rep movsb"
                : "+S"(s), "+D"(d), "+c"(bytes)
                :
                : "memory"
            );
        }
    } else {
        // Backward copy: adjust pointers to end, set DF for backward copy
        s += n;
        d += n;

        asm volatile (
            "std\n\t"          // set direction flag to decrement pointers
            "rep movsl\n\t"
            "rep movsb\n\t"
            "cld"              // clear direction flag
            : "+S"(s), "+D"(d), "+c"(dwords)
            :
            : "memory"
        );

        if (bytes) {
            asm volatile (
                "std\n\t"
                "rep movsb\n\t"
                "cld"
                : "+S"(s), "+D"(d), "+c"(bytes)
                :
                : "memory"
            );
        }
    }

    return dest;
}

//conversion functions
int atoi(const char* str) {
    if (!str) return -1;

    int res = 0;
    bool negative = false;
    int i = 0;

    
    while (str[i] == ' ') i++;

    
    if (str[i] == '-') {
        negative = true;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    // Convert digits
    for (; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') return -1;
        res = res * 10 + (str[i] - '0');
    }

    return negative ? -res : res;
}


void itoa(int value, char* str, int base) {
    char buffer[33]; // Enough for 32-bit int in binary + '\0'
    int i = 0;
    bool is_negative = false;

    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    if (base == 10 && value < 0) {
        is_negative = true;
        value = -value;
    }

    while (value != 0) {
        int digit = value % base;
        buffer[i++] = (digit > 9) ? (digit - 10) + 'A' : digit + '0';
        value /= base;
    }

    if (is_negative) {
        buffer[i++] = '-';
    }

    // Reverse buffer into str
    int j = 0;
    while (i > 0) {
        str[j++] = buffer[--i];
    }
    str[j] = '\0';
}

char* strlow(const char* str) {
    if (str == NULL) return NULL;

    size_t len = strlen(str);
    char* newstr = malloc(len + 1);
    if (!newstr) return NULL;

    for (int i = (int)len - 1; i >= 0; i--) {
        unsigned char chr = str[i];
        if (chr >= 'A' && chr <= 'Z') {
            chr += 32;
        }
        newstr[i] = chr;
    }

    newstr[len] = '\0';
    return newstr;
}

const char* byte_nb_simplify(uint32_t size_bytes, char* buf) {
    
    if (size_bytes >= 1024 * 1024 * 1024) { 
        sprintf(buf, "%uGB", size_bytes / (1024*1024*1024));
    } else if (size_bytes >= 1024 * 1024) { 
        sprintf(buf, "%uMB", size_bytes / (1024*1024));
    } else if (size_bytes >= 1024) { 
        sprintf(buf, "%uKB", size_bytes / 1024);
    } else { 
        sprintf(buf, "%uB", size_bytes);
    }
    return buf;
}
