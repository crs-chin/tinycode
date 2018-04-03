default:all

all:test

CFLAGS += -O2
LDFLAGS += -s

test:test.c tinycode.c
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	@rm test

.phony:clean
