#include <Block.h>
#include <logger>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// REMOVE ME !
#include <stdio.h>

#include "re.h"

ReExpr _re_single(int n, char *buff, ...)
{
    va_list args;
    va_start(args, n);

    __block char *chars = buff;
    for (int i = 0; i < n; i++)
    {
        chars[i] = va_arg(args, int);
    }

    va_end(args);

    auto skip = ^(Sscan *self) {
      char c = scanner_peek(self);
      for (int i = 0; i < n; i++)
      {
          char expected = buff[i];
          if (c == expected)
          {
              scanner_next(self);
              return true;
          }
      }

      return false;
    };

    auto match = ^(Sscan *self, size_t sz, char buffer[sz]) {
      char c = scanner_peek(self);

      if (sz < 1)
      {
          return (size_t)0;
      }

      for (int i = 0; i < n; i++)
      {
          char expected = buff[i];
          if (c == expected)
          {
              scanner_next(self);
              buffer[0] = c;
              return (size_t)1;
          }
      }

      return (size_t)0;
    };

    auto free_expr = ^(void) {
      free(chars);
    };

    return (ReExpr){
        .skip = Block_copy(skip),
        .match = Block_copy(match),
        .free = Block_copy(free_expr),
    };
}

ReExpr _re_either(int n, ReExpr *buffer, ...)
{
    va_list args;
    va_start(args, n);

    __block ReExpr *exprs = buffer;

    for (int i = 0; i < n; i++)
    {
        exprs[i] = va_arg(args, ReExpr);
    }

    auto skip = ^(Sscan *self) {
      for (int i = 0; i < n; i++)
      {
          ReExpr subexpr = exprs[i];
          if (subexpr.skip(self))
          {
              return true;
          }
      }

      return false;
    };

    auto match = ^(Sscan *self, size_t sz, char buffer[sz]) {
      size_t backup;
      bool matched;
      for (int i = 0; i < n; i++)
      {
          backup = self->index;
          matched = exprs[i].skip(self);
          self->index = backup;

          if (matched)
          {
              return exprs[i].match(self, sz, buffer);
          }
      }

      return (size_t)0;
    };

    auto free_expr = ^(void) {
      for (int i = 0; i < n; i++)
      {
          ReExpr subexpr = exprs[i];
          subexpr.free();

          Block_release(subexpr.skip);
          Block_release(subexpr.free);
      }

      free(buffer);
    };

    va_end(args);
    return (ReExpr){
        .skip = Block_copy(skip),
        .match = Block_copy(match),
        .free = Block_copy(free_expr),
    };
}

ReExpr _re_chain(int n, ReExpr *buffer, ...)
{
    va_list args;
    va_start(args, n);

    __block ReExpr *exprs = buffer;

    for (int i = 0; i < n; i++)
    {
        exprs[i] = va_arg(args, ReExpr);
    }

    auto skip = ^(Sscan *self) {
      for (int i = 0; i < n; i++)
      {
          if (!exprs[i].skip(self))
          {
              return false;
          }
      }

      return true;
    };

    auto match = ^(Sscan *self, size_t sz, char buffer[sz]) {
      size_t ret = 0;
      size_t r;
      size_t backup;
      for (int i = 0; i < n; i++)
      {
          backup = scanner_tell(self);
          if (!exprs[i].skip(self))
          {
              memset(buffer, 0, sz);
              return (size_t)0;
          }

          scanner_at(self, backup);

          r = exprs[i].match(self, sz - ret, buffer + ret);
          ret += r;
      }

      return ret;
    };

    auto free_expr = ^(void) {
      for (int i = 0; i < n; i++)
      {
          ReExpr subexpr = exprs[i];
          subexpr.free();

          Block_release(subexpr.skip);
          Block_release(subexpr.free);
      }

      free(buffer);
    };

    va_end(args);
    return (ReExpr){
        .skip = Block_copy(skip),
        .match = Block_copy(match),
        .free = Block_copy(free_expr),
    };
}

ReExpr re_until(ReExpr expr)
{
    auto skip = ^(Sscan *self) {
      if (expr.skip(self))
      {
          return false;
      }

      while (!expr.skip(self))
      {
          if (!scanner_next(self))
          {
              return false;
          }
      }

      return true;
    };

    auto match = ^(Sscan *self, size_t sz, char buffer[sz]) {
      size_t ret = 0;

      if (expr.skip(self) || sz < 1)
      {
          return (size_t)0;
      }

      while (!expr.skip(self))
      {
          buffer[ret++] = scanner_peek(self);
          if (sz <= ret || !scanner_next(self))
          {
              return (size_t)0;
          }
      }

      return ret;
    };

    auto free_expr = ^(void) {
      expr.free();

      Block_release(expr.skip);
      Block_release(expr.free);
    };

    return (ReExpr){
        .skip = Block_copy(skip),
        .match = Block_copy(match),
        .free = Block_copy(free_expr),
    };
}

ReExpr re_range(int start, int end)
{
    auto skip = ^(Sscan *self) {
      char c = scanner_peek(self);
      if (c >= start && c <= end)
      {
          scanner_next(self);
          return true;
      }

      return false;
    };

    auto match = ^(Sscan *self, size_t sz, char buffer[sz]) {
      if (sz < 1)
      {
          return (size_t)0;
      }

      char c = scanner_peek(self);
      if (c >= start && c <= end)
      {
          scanner_next(self);
          buffer[0] = c;
          return (size_t)1;
      }

      return (size_t)0;
    };

    auto free_expr = ^(void) {
    };

    return (ReExpr){
        .skip = Block_copy(skip),
        .match = Block_copy(match),
        .free = Block_copy(free_expr),
    };
}

ReExpr re_word(char const *word)
{
    auto skip = ^(Sscan *self) {
      size_t len = strlen(word);
      for (size_t i = 0; i < len; i++)
      {
          char c = scanner_peek(self);
          if (c != word[i])
          {
              return false;
          }

          if (!scanner_next(self))
          {
              return false;
          }
      }

      return true;
    };

    auto match = ^(Sscan *self, size_t sz, char buffer[sz]) {
      size_t len = strlen(word);
      if (sz < len)
      {
          printf("Overflow\n");
          return (size_t)0;
      }

      for (size_t i = 0; i < len; i++)
      {
          char c = scanner_peek(self);
          if (c != word[i])
          {
              printf("%c != %c\n", c, word[i]);
              return (size_t)0;
          }

          if (!scanner_next(self))
          {
              return (size_t)0;
          }
      }

      memcpy(buffer, word, len);
      return len;
    };

    auto free_expr = ^(void) {
    };

    return (ReExpr){
        .skip = Block_copy(skip),
        .match = Block_copy(match),
        .free = Block_copy(free_expr),
    };
}

ReExpr re_while(ReExpr expr)
{
    auto skip = ^(Sscan *self) {
      while (expr.skip(self))
      {
          if (!scanner_next(self))
          {
              return false;
          }
      }

      return true;
    };

    auto match = ^(Sscan *self, size_t sz, char buffer[sz]) {
      size_t ret = 0;
      size_t backup;

      while (true)
      {
          backup = scanner_tell(self);

          if (!expr.skip(self))
          {
              break;
          }

          scanner_at(self, backup);
          self->index = backup;
          buffer[ret++] = scanner_peek(self);

          if (sz <= ret)
          {
              return sz;
          }

          if (!scanner_next(self))
          {
              memset(buffer, 0, sz);
              return (size_t)0;
          }
      }

      return ret;
    };

    auto free_expr = ^(void) {
      expr.free();

      Block_release(expr.skip);
      Block_release(expr.free);
    };

    return (ReExpr){
        .skip = Block_copy(skip),
        .match = Block_copy(match),
        .free = Block_copy(free_expr),
    };
}

void free_block(ReExpr *self)
{
    Block_release(self->skip);
    Block_release(self->match);
    Block_release(self->free);
}
