#include "headers/string.h"
#include "headers/memory.h"

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    // Return difference of first differing characters (unsigned char for correct ordering)
    return (unsigned char)(*s1) - (unsigned char)(*s2);
}

void* memset(void *dest, int value, size_t count) {
    unsigned char *ptr = (unsigned char*)dest;
    while (count--) {
        *ptr++ = (unsigned char)value;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t n) {
    
    char* d = (char*)dest;
    const char* s = (const char*)src;

    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
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

    int capacity = (max_tokens > 0) ? max_tokens : 8; // Start small and grow
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
            if (max_tokens == 0 && token_index >= capacity) {
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

#include <stdlib.h>
#include <string.h>

#include <stdlib.h>
#include <string.h>

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
    
    if (pos > length || length >= STRING_MAX_LEN - 1) return false;

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
