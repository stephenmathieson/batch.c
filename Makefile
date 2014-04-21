
CC ?= gcc
CFLAGS = -std=c99 -Wall -Wextra -Isrc
LDFLAGS = -pthread
SRC = $(wildcard src/*.c)
SRC += $(wildcard deps/*/*.c)
OBJS = $(SRC:.c=.o)
TEST_SOURCES = $(wildcard test/*.c)
TEST_EXECUTABLES = $(TEST_SOURCES:.c=)

test: $(TEST_EXECUTABLES)

$(TEST_EXECUTABLES): CFLAGS += -Wunused-parameter
$(TEST_EXECUTABLES): CFLAGS += -Ideps/describe -Ideps
$(TEST_EXECUTABLES): $(OBJS)
	@$(CC) $@.c $^ -o $@ $(CFLAGS) $(LDFLAGS)
	@$@

example: CFLAGS += -Wint-to-void-pointer-cast
example: example.o $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

clean:
	rm -f $(OBJS)
	rm -f example example.o
	rm -f $(TEST_OBJECTS) $(TEST_EXECUTABLES)

.PHONY: test clean
