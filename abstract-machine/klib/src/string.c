#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i = 0;
  while (s[i] != '\0')
  {
    i ++;
  }
  return i;
}

char *strcpy(char *dst, const char *src) {
  size_t i = 0;
  for(i=0; src[i] != '\0'; i++){
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i++)
      dst[i] = src[i];
  for ( ; i < n; i++)
      dst[i] = '\0';
  return dst;

}

char *strcat(char *dst, const char *src) {
  size_t i = 0, dst_len = strlen(dst);
  for(;src[i] != '\0'; i++){
    dst[dst_len+i] = src[i];
  }
  dst[dst_len+i] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  size_t i=0;
  while (true)
  {
    if (s1[i] < s2[i]){
      return -1;
    }else if (s1[i] > s2[i])
    {
      return 1;
    }else if (s1[i] == '\0' && s2[i] == '\0')
    {
      return 0;
    }
    
    i++;
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i = 0;
  while (n)
  {
    if (s1[i] < s2[i]){
      return -1;
    }else if (s1[i] > s2[i])
    {
      return 1;
    }else if (s1[i] == '\0' && s2[i] == '\0')
    {
      return 0;
    }
    
    i ++;
    n -= 1;
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  size_t i = 0;
  while (n)
  {
    ((unsigned char *)s)[i] = c;
    i++;
    n--;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  void * tmp = malloc(n);
  memcpy(tmp, src, n);
  memcpy(dst, tmp, n);
  free(tmp);
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  size_t i;
  for (i = 0; i < n; i++)
      ((unsigned char *)out)[i] = ((unsigned char *)in)[i];
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  size_t i = 0;
  while (n)
  {
    if (((unsigned char *)s1)[i] < ((unsigned char *)s2)[i]){
      return -1;
    }else if (((unsigned char *)s1)[i] > ((unsigned char *)s2)[i])
    {
      return 1;
    }
    i ++;
    n -= 1;
  }
  return 0;
}

#endif
