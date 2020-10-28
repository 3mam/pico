#include "str.h"
#include <string.h>

str* str_new(const char* txt) {
  str* self = NEW(str);
  self->size = strlen(txt);
  self->ptr = MAKE(char, self->size);
  strncpy(self->ptr, txt, self->size);
  return self;
}

void str_del(const str* self) {
  free((void*)self->ptr);
  free((void*)self);
}

str* str_add(const str* str1, const str* str2) {
  str* self = NEW(str);
  self->size = str1->size + str2->size;
  self->ptr = MAKE(char, self->size);
  strcat(self->ptr, str1->ptr);
  strcat(self->ptr, str2->ptr);
  return self;
}

str* str_multiply(const str* str1, ui32 multiplay) {
  str* self = NEW(str);
  self->size = str1->size * multiplay;
  self->ptr = MAKE(char, self->size);
  for (ui32 i = 0; i < multiplay; i++)
    strcat(self->ptr, str1->ptr);
  return self;
}

char* str_get(const str* self) {
  return self->ptr;
}
