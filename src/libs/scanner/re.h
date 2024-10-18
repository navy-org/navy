#pragma once

// A proudly stolen idea from Karm's re-expression library
// https://github.com/skift-org/karm/blob/main/src/karm-io/expr.h

#include <Block.h>
#include <stddef.h>
#include <stdlib.h>
#include <utils.h>

#include "scanner.h"

typedef struct
{
    bool (^skip)(Sscan *self);
    size_t(^match)(Sscan *self, size_t sz, char buffer[sz]);
    void (^free)(void);
} ReExpr;

ReExpr _re_single(int n, char *buffer, ...);
ReExpr _re_either(int n, ReExpr *buffer, ...);
ReExpr _re_chain(int n, ReExpr *buffer, ...);

ReExpr re_until(ReExpr expr);
ReExpr re_while(ReExpr expr);
ReExpr re_range(int start, int end);
ReExpr re_word(char const *word);

ReExpr re_ascii(void);
ReExpr re_digit(void);
ReExpr re_upper(void);
ReExpr re_lower(void);

void free_char(char **buf);
void free_ReExpr(ReExpr **buf);
void free_block(ReExpr *self);

#define re(FN, T, ...) ({                                                 \
    size_t len = get_arg_count$(__VA_ARGS__);                             \
    T *buffer = malloc(sizeof(T) * len);                                  \
    ReExpr retval = FN(get_arg_count$(__VA_ARGS__), buffer, __VA_ARGS__); \
    retval;                                                               \
})

// Each one have 3 arguments:
// 1. The callback function
// 2. The type that the re function will use to treat the arguments
// 3. The arguments

#define re_single(...) re(_re_single, char, __VA_ARGS__)
#define re_either(...) re(_re_either, ReExpr, __VA_ARGS__)
#define re_chain(...)  re(_re_chain, ReExpr, __VA_ARGS__)

#define re_ascii()  re_range(0, 127)
#define re_digit()  re_range('0', '9')
#define re_upper()  re_range('A', 'Z')
#define re_lower()  re_range('a', 'z')
#define re_alpha()  re_either(re_upper(), re_lower())
#define re_alnum()  re_either(re_alpha(), re_digit())
#define re_xdigit() re_either(re_digit(), re_range('a', 'f'), re_range('A', 'F'))
#define re_space()  re_either(re_single(' '), re_single('\t'), re_single('\n'), re_single('\r'), re_single('\v'), re_single('\f'))
#define re_blank()  re_either(re_space(), re_single('\v'), re_single('\f'))
#define re_quote()  re_single('"', '\'')
