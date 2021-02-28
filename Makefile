TARGETS=main

.PHONY: all clean

all: $(TARGETS)

clean:
	$(RM) $(TARGETS)

main: main.c craii.h
	gcc -O0 -Wall -Werror -Wextra $< -o $@ -std=gnu11

