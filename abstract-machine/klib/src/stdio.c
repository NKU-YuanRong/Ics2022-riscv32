#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char* get_int(char *p, va_list *ap) {
	int d = va_arg(*ap, int);
	char str[32];
	int len = 0;
	if (d == 0) {
		*p++ = '0';
		return p;
	}
	if (d < 0) {
		*p++ = '-';
		d *= -1;
	}
	while (d) {
		str[len++] = d % 10 + '0';
		d /= 10;
	}
	for (int i = len - 1; i >= 0; i--) {
		*p++ = str[i];
	}
	return p;
}

static char* get_string(char* p, va_list *ap) {
	char *str = va_arg(*ap, char*);
	while(*str) {
		*p++ = *str++;
	}
	return p;
}

static char* get_char(char* p, va_list *ap) {
	char ch = (char)va_arg(*ap, int);
	*p++ = ch;
	return p;
}

static char* get_hex(char* p, va_list *ap) {
	unsigned int value = va_arg(*ap, unsigned int);
	if(value == 0){
		*p++ = '0';
		return p;
	}
	for (; value != 0; value /= 16) {
		if (value % 16 < 10) {
			*p++ = value % 16 + '0';
		}
		else {
			*p++ = value % 16 - 10 + 'A';
		}
	}
	return p;
}

static char* get_point(char* p, va_list *ap) {
	uint32_t pointer = va_arg(*ap, uint32_t);
	for (; pointer != 0; pointer /= 16) {
		if (pointer % 16 < 10) {
			*p++ = pointer % 16 + '0';
		}
		else {
			*p++ = pointer % 16 - 10 + 'A';
		}
	}
	return p;
}

int get_result(char *out, size_t n, const char *fmt, va_list ap) {
	char* p = (char*)out;
	while (*fmt) {
		if (*fmt == '%') {
			if ((uint32_t)p - (uint32_t)out >= n)
				break;
			fmt++;
			switch (*fmt) {
				case 'd': 
					p = get_int(p, &ap);
					break;
				case 's': 
					p = get_string(p, &ap);
					break;
				case 'c':
					p = get_char(p, &ap);
					break;
				case 'p':
					p = get_point(p, &ap);
					break;
				case 'x':
					p = get_hex(p, &ap);
					break;
			}
			fmt++;
		}
		else {
			if ((uint32_t)p - (uint32_t)out >= n)
				break;
			*p++ = *fmt++;
		}
	}
	*p++ = '\0';
	return (uint32_t)p - (uint32_t)out - 1;
}

int printf(const char *fmt, ...) {
	char out[4096];
	va_list ap;
	va_start(ap, fmt);
	int len = get_result(out, -1, fmt, ap);
	va_end(ap);
	putstr(out);
	return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  	return get_result(out, -1, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int len = get_result(out, -1, fmt, ap);
	va_end(ap);
	return len;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int len = get_result(out, n, fmt, ap);
	va_end(ap);
	return len;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  	int len = get_result(out, n, fmt, ap);
	return len;
}

#endif