#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  // panic("Not implemented");
  size_t len = 0;
	char *t = (char *)s;
	while (*t != '\0') {
		len += 1;
    t++;
	}
	return len;
}

char *strcpy(char *dst, const char *src) {
  // panic("Not implemented");
  size_t i = 0;
  while (src[i] != '\0') {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  // panic("Not implemented");
  size_t i = 0;
  while (src[i] != '\0' && n--) {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  // panic("Not implemented");
  size_t len = strlen(dst), i = 0;
  for (; src[i] != '\0'; i++) {
    dst[len + i] = src[i];
  }
  dst[len + i] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  // panic("Not implemented");
  char *t1 = (char *)s1, *t2 = (char *)s2;
  while (*t1 == *t2) {
    if (*t1 == '\0') {
      return 0;
    }
    t1++;
    t2++;
  }
  return *t1 > *t2 ? 1 : -1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  char *t1 = (char *)s1, *t2 = (char *)s2;
  while (*t1 == *t2) {
    if (*t1 == '\0') {
      return 0;
    }
    if (!n--) {
      break;
    }
    t1++;
    t2++;
  }
  return *t1 - *t2;
}

void *memset(void *s, int c, size_t n) {
  // panic("Not implemented");
  size_t i = 0;
  char *t = (char *)s;
  for (; i < n; i++) {
    t[i] = (char)c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  // panic("Not implemented");
  char *t1 = (char *)out, *t2 = (char *)in;
  while (n--) {
    *t1 = *t2;
    t1++;
    t2++;
  }
  return out;
}
/*
int memcmp(const void *s1, const void *s2, size_t n) {
  // panic("Not implemented");
  char *t1 = (char *)s1, *t2 = (char *)s2;
  while (*t1 == *t2) {
    if (!n--) {
      break;
    }
    t1++;
    t2++;
  }
  return *t1 - *t2;
}*/

int memcmp(const void *s1, const void *s2, size_t n) {
  char *p = (char*)s1;
	char *q = (char*)s2;
	int flag = 0;
	while (n--) {
		flag = *p - *q;
		if (!flag) break;
		++p;
		++q;
	}
	return flag;
}

#endif
