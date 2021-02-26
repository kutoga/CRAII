TARGETS=main

.PHONY: all clean

all: $(TARGETS)

clean:
	$(RM) $(TARGETS)

main: main.c craii.h
	gcc -Ofast -Wall -Werror -Wextra $< -o $@ -std=gnu11

