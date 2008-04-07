#include <iostream>

#define GLOBAL extern "C"

typedef struct zeb_str{
	unsigned int len;
	char * data;
} zeb_str;

GLOBAL zeb_str * string_new(unsigned int length){
	zeb_str * str = new zeb_str;
	str->len = length;
	str->data = (char *) malloc(++length);
	memset(str->data, 0, length);
	return str;
}

GLOBAL void string_print(zeb_str * str){
	printf("%s", str->data);
}

GLOBAL unsigned int string_length(zeb_str * str){
	return str->len;
}

GLOBAL zeb_str * string_subscr(zeb_str * str, unsigned int i){
	if(i > str->len){
		i = str->len;
	}
	zeb_str * subscr = new zeb_str;
	subscr->len = 1;
	subscr->data = (char *) malloc(2);
	subscr->data[0] = str->data[i];
	subscr->data[1] = 0;
	return subscr;
}

GLOBAL zeb_str * string_subscr_set(zeb_str * str, unsigned int i, zeb_str * subscr){
	if(i >= str->len){
		return str;
	}
	str->data[i] = subscr->data[0];
	return str;
}

GLOBAL zeb_str * string_copy(char * str){
	zeb_str * newstr = new zeb_str;
	unsigned int length = strlen(str);
	newstr->len = length;
	newstr->data = (char *) malloc(length + 1);
	memcpy(newstr->data, str, length + 1);
	return newstr;
}

GLOBAL zeb_str * string_concat(zeb_str * a, zeb_str * b){
	zeb_str * str = new zeb_str;
	str->len = a->len + b->len;
	str->data = (char *) malloc(str->len + 1);
	memcpy(str->data, a->data, a->len);
	memcpy(str->data + a->len, b->data, b->len + 1);
	return str;
}

GLOBAL signed int string_to_int(zeb_str * str){
	return atoi(str->data);
}

GLOBAL zeb_str * string_from_int(signed int i){
	zeb_str * str = new zeb_str;
	str->data = (char *) malloc(11);
	str->len = sprintf(str->data, "%i", i);
	return str;
}

GLOBAL void print_num(signed int i){
	printf("Integer Output: %i\n", i);
}





