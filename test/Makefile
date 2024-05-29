all: test
CC := ./hook gcc
CFLAGS := -Wall -Wextra -Werror -I. -O3
SRC := src/main.c # $(shell find src -type f -name "*.c")
OBJ := $(SRC:.c=.o)

MODS_DIR := mods

MOD_MAKE_INCLUDES := $(shell find $(MODS_DIR) -type f -name "mod.mk")

$(foreach inc,$(MOD_MAKE_INCLUDES),$(eval include $(inc)))

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f test $(OBJ)