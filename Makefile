#include Makefile.link

PNAME = ATA_DAQ

# Directory structure
SDIR = src
ODIR = obj
BDIR = bin

# Compiler option
INC = $(SDIR)
INC_PARAMS = $(foreach d, $(INC), -I$d)
LIBS = -ldaqhats -lm
CFLAGS = -Wall -I/usr/local/include -I./src/ -g

# Compilation commands
CC = gcc $(CFLAGS) $(INC_PARAMS) $(LIBS)
CL = gcc $(CFLAGS) $(LIBS)

#Default make target: setup the environment, then build the program
.PHONY: all
all: | toolchain $(PNAME)

SRCS = data_aquisition.c
SOBJ = $(patsubst %.c, $(ODIR)/%.o, $(SRCS))

$(BDIR)/$(PNAME): $(SOBJ)
	$(CL) -o $@ $^

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $^

%.o: %.c
	$(CC) -c -o $@ $<

.PHONY: clean
clean:
	rm -rf $(ODIR) $(BDIR)

.PHONY: toolchain pmain $(PNAME)
toolchain: | $(BDIR) $(ODIR)
pmain: $(BDIR)/$(PNAME)
$(PNAME): | toolchain pmain

$(ODIR):
	@mkdir $(ODIR)

$(BDIR):
	@mkdir $(BDIR)
