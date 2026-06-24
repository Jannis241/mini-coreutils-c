#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdlib.h>

#define VECTOR_DEFINE(T, Name)                                      \
typedef struct {                                                    \
    T *data;                                                        \
    size_t len;                                                     \
    size_t cap;                                                     \
} Name;                                                            \
                                                                    \
void Name##_init(Name *v) {                                         \
    v->data = NULL;                                                 \
    v->len = 0;                                                     \
    v->cap = 0;                                                     \
}                                                                   \
                                                                    \
void Name##_push(Name *v, T value) {                                \
    if (v->len >= v->cap) {                                         \
        v->cap = v->cap == 0 ? 4 : v->cap * 2;                      \
        v->data = realloc(v->data, v->cap * sizeof(T));             \
    }                                                               \
    v->data[v->len++] = value;                                      \
}                                                                   \
                                                                    \
void Name##_free(Name *v) {                                         \
    free(v->data);                                                  \
    v->data = NULL;                                                 \
    v->len = 0;                                                     \
    v->cap = 0;                                                     \
}

#endif
