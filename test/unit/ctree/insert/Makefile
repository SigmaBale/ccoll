VPATH = ../../../../src
CC = gcc
OPTIM = -O0
DEPFLAGS = -MP -MD
INCLUDES = ../../include ../../../../src/
FLAGS = -Wall -Wextra -g $(foreach dir,$(INCLUDES),-I$(dir)) $(OPTIM) $(DEPFLAGS)

BIN = insert_test
CFILES = test_insert.c ctree.c
OBJFILES = $(patsubst %.c,%.o,$(CFILES))
DEPFILES = $(patsubst %.c,%.d,$(CFILES))

all: $(BIN)

$(BIN): $(OBJFILES)
	@$(CC) -o $@ $^

%.o:%.c
	@$(CC) $(FLAGS) -c -o $@ $<

clean:
	@rm -rf $(BIN) $(OBJFILES) $(DEPFILES)

test: $(BIN)
	@./$(BIN);

-include $(DEPFILES)

.PHONY: all clean test
