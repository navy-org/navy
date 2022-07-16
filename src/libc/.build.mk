LIBC = $(BUILD)/libc.a
LIBC_BUILD = $(BUILD)/libc

LIBC_SRC = $(wildcard src/libc/*.c)
LIBC_OBJ := $(patsubst %, $(LIBC_BUILD)/%.o, $(LIBC_SRC))
LIBC_DEPENDENCIES += $(LIBC:.o=.d)

$(LIBC_BUILD)/%.c.o: %.c
	@$(MKCWD)
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBC): $(LIBC_OBJ)
	@$(MKCWD)
	$(AR) $(ARFLAGS) $@ $<