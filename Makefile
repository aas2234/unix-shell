CC := gcc
CFLAGS := -Wall  -g
LDFLAGS :=


OBJECTS := shell.o dir_stack.o dir_list.o

all: w4118_sh


w4118_sh: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f w4118_sh
	rm -f shell.o dir_stack.o dir_list.o

.PHONY: clean
