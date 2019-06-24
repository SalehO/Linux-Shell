CC := gcc
SRCDIR := src
BINDIR := bin
OBJDIR := obj
INCDIR := include
EXEC := shell
INC := -I $(INCDIR)
CFLAGS := $(INC) $(STD)

SRCFILES := $(shell find $(SRCDIR) -type f -name *.c)
OBJFILES := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCFILES))

.PHONY: clean compile

all: compile $(EXEC)

clean:
	rm -rf $(OBJDIR) $(BINDIR)

compile:
	mkdir -p bin obj

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXEC): $(OBJFILES)
	$(CC) $^ -o $(BINDIR)/$@
	echo "The program was compiled successfully."
