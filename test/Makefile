CC := gcc
CFLAGS := -Wall -Wextra -Werror -I. -O3 -std=c11 -g
SRC := $(shell find src -type f -name "*.c")
OBJ := $(SRC:.c=.o)

MODS_DIR := mods

MOD_MAKE_INCLUDES := $(shell find mods -type f -name $(MODS_DIR)/*/mod.mk)

$(foreach inc,$(MOD_MAKE_INCLUDES),$(eval include $(inc)))

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(OBJ)
	$(CC) $^ -o $@ $(CFLAGS)