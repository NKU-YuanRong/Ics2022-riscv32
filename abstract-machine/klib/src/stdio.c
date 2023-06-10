#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// static char* get_int(char *p, va_list *ap) {
// 	int d = va_arg(*ap, int);
// 	char str[32];
// 	int len = 0;
// 	if (d == 0) {
// 		*p++ = '0';
// 		return p;
// 	}
// 	if (d < 0) {
// 		*p++ = '-';
// 		d *= -1;
// 	}
// 	while (d) {
// 		str[len++] = d % 10 + '0';
// 		d /= 10;
// 	}
// 	for (int i = len - 1; i >= 0; i--) {
// 		*p++ = str[i];
// 	}
// 	return p;
// }

// static char* get_string(char* p, va_list *ap) {
// 	char *str = va_arg(*ap, char*);
// 	while(*str) {
// 		*p++ = *str++;
// 	}
// 	return p;
// }

// static char* get_char(char* p, va_list *ap) {
// 	char ch = (char)va_arg(*ap, int);
// 	*p++ = ch;
// 	return p;
// }

// static char* get_hex(char* p, va_list *ap) {
// 	unsigned int value = va_arg(*ap, unsigned int);
// 	if(value == 0){
// 		*p++ = '0';
// 		return p;
// 	}
// 	for (; value != 0; value /= 16) {
// 		if (value % 16 < 10) {
// 			*p++ = value % 16 + '0';
// 		}
// 		else {
// 			*p++ = value % 16 - 10 + 'A';
// 		}
// 	}
// 	return p;
// }

// static char* get_point(char* p, va_list *ap) {
// 	uint32_t pointer = va_arg(*ap, uint32_t);
// 	for (; pointer != 0; pointer /= 16) {
// 		if (pointer % 16 < 10) {
// 			*p++ = pointer % 16 + '0';
// 		}
// 		else {
// 			*p++ = pointer % 16 - 10 + 'A';
// 		}
// 	}
// 	return p;
// }

static char HEX_CHARACTERS[] = "0123456789ABCDEF";
#define BIT_WIDE_HEX 8
#define append(x) {out[j++]=x; if (j >= n) {break;}}

int get_result(char *out, size_t n, const char *fmt, va_list ap) {
	// char* p = (char*)out;
	// while (*fmt) {
	// 	if (*fmt == '%') {
	// 		if ((uint32_t)p - (uint32_t)out >= n)
	// 			break;
	// 		fmt++;
	// 		switch (*fmt) {
	// 			case 'd': 
	// 				p = get_int(p, &ap);
	// 				break;
	// 			case 's': 
	// 				p = get_string(p, &ap);
	// 				break;
	// 			case 'c':
	// 				p = get_char(p, &ap);
	// 				break;
	// 			case 'p':
	// 				p = get_point(p, &ap);
	// 				break;
	// 			case 'x':
	// 				p = get_hex(p, &ap);
	// 				break;
	// 		}
	// 		fmt++;
	// 	}
	// 	else {
	// 		if ((uint32_t)p - (uint32_t)out >= n)
	// 			break;
	// 		*p++ = *fmt++;
	// 	}
	// }
	// *p++ = '\0';
	// return (uint32_t)p - (uint32_t)out - 1;
		char buffer[128];
	char *txt, cha;
	int num, len;
	unsigned int unum;
	uint32_t pointer;
	int state = 0, i, j;
	for (i = 0, j = 0; fmt[i] != '\0'; ++i){
		switch (state){
		/*
		* this state is used to copy the contents of the 
		* string to the buffer 
		*/
		case 0:
			if (fmt[i] != '%'){
				append(fmt[i]);
			} 
			else
				state = 1;
			break;
		/*
		* this state is used to deal with the format controler  
		*/
		case 1:
			switch (fmt[i]){
				case 's':
					txt = va_arg(ap, char*);
					for (int k = 0; txt[k] !='\0'; ++k)
						append(txt[k]);
					break;
				
				case 'd':
					num = va_arg(ap, int);
					if(num == 0){
						append('0');
						break;
					}
					if (num < 0){
						append('-');
						num = 0 - num;
					}
					for (len = 0; num ; num /= 10, ++len)
						buffer[len] = HEX_CHARACTERS[num % 10];
					for (int k = len - 1; k >= 0; --k)
						append(buffer[k]);
					break;
				
				case 'c':
					cha = (char)va_arg(ap, int);
					append(cha);
					break;

				case 'p':
					pointer = va_arg(ap, uint32_t);
					for (len = 0; pointer ; pointer /= 16, ++len)
						buffer[len] = HEX_CHARACTERS[pointer % 16];
					for (int k = 0; k < BIT_WIDE_HEX - len; ++k)
						append('0');
					for (int k = len - 1; k >= 0; --k)
						append(buffer[k]);
					break;
				case 'x':
					unum = va_arg(ap, unsigned int);
					if(unum == 0){
						append('0');
						break;
					}
					for (len = 0; unum ; unum >>= 4, ++len)
						buffer[len] = HEX_CHARACTERS[unum & 0xF];
					for (int k = len - 1; k >= 0; --k)
						append(buffer[k]);
					break;  
				default:
					//assert(0);
			}
		state = 0;
		break;
		}
	}
	out[j] = '\0';
	return j;
}

int printf(const char *fmt, ...) {
	char out[1024];
	va_list ap;
	va_start(ap, fmt);
	int len = get_result(out, -1, fmt, ap);
	va_end(ap);
	putstr(out);
	return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  	int len = get_result(out, -1, fmt, ap);
	return len;
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