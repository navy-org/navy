LIBALLOC = $(BUILD)/liballoc.a
LIBALLOC_BUILD = $(BUILD)/liballoc

LIBALLOC_SRC = $(wildcard src/liballoc/*.c)
LIBALLOC_OBJ := $(patsubst %, $(LIBALLOC_BUILD)/%.o, $(LIBALLOC_SRC))
LIBALLOC_DEPENDENCIES += $(LIBALLOC:.o=.d)

$(LIBALLOC_BUILD)/%.c.o: %.c
	@$(MKCWD)
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBALLOC): $(LIBALLOC_OBJ)
	@$(MKCWD)
	$(AR) $(ARFLAGS) $@ $<