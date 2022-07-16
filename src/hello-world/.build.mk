HELLO_WORLD = $(BUILD)/hello-world.elf
HELLO_WORLD_SRC = $(wildcard src/hello-world/*.c)
BUILD_HELLO_WORLD = $(BUILD)/hello-world
HELLO_WORLD_OBJ = $(patsubst %, $(BUILD_HELLO_WORLD)/%.o, $(HELLO_WORLD_SRC))

$(BUILD_HELLO_WORLD)/%.c.o: %.c
	@$(MKCWD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_HELLO_WORLD)/%.s.o: %.s
	@$(MKCWD)
	$(AS) -o $@ $< $(ASFLAGS)

$(HELLO_WORLD): $(HELLO_WORLD_OBJ) $(LIBC_OBJ) $(COPLAND_OBJ)
	@$(MKCWD)
	$(LD) $(LDFLAGS) -o $@ $^ -T src/hello-world/link.ld

hello-world: $(HELLO_WORLD)
	cp $(HELLO_WORLD) $(SYSROOT)/bin/hello-world.elf
	@bash -c 'echo -e "\nMODULE_PATH=boot:///bin/hello-world.elf\nMODULE_STRING=hello-world\n" >> $(SYSROOT)/boot/limine.cfg'