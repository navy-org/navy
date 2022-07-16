KERNEL = $(BUILD)/kernel.elf
KERNEL_BUILD = $(BUILD)/kernel

KERNEL_CFLAGS = 					\
	$(CFLAGS)						\
	-mno-red-zone					\
	-fno-zero-initialized-in-bss	\
	-mcmodel=kernel					\
	-m64							\
	-Isrc/							

KERNEL_LDFLAGS +=					\
	$(LDFLAGS)						\
	-static							\
	-z max-page-size=0x1000			\

KERNEL_SRC += $(wildcard src/kernel/*.c)

KERNEL_OBJ := $(patsubst %, $(KERNEL_BUILD)/%.o, $(KERNEL_SRC))
DEPENDENCIES += $(KERNEL_OBJ:.o=.d)

$(KERNEL_BUILD)/%.c.o: %.c
	@$(MKCWD)
	$(CC) -c -o $@ $< $(KERNEL_CFLAGS)

$(KERNEL_BUILD)/%.s.o: %.s
	@$(MKCWD)
	$(AS) -o $@ $< $(ASFLAGS)

$(KERNEL): $(LIBALLOC_OBJ) $(LIBC_OBJ) $(COPLAND_OBJ) $(KERNEL_OBJ)
	@$(MKCWD)
	$(LD) -o $@ $^ $(KERNEL_LDFLAGS)