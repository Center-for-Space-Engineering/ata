NAME = data_aquisition
OBJ = $(NAME).o
LIBS = -ldaqhats
CFLAGS = -Wall -I/usr/local/include -I./ -g
CC = gcc
EXTENSION = .c

all: $(NAME)

%.o: %$(EXTENSION) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -lm

$(NAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) -lm

.PHONY: clean

clean:
	@rm -f *.o *~ core $(NAME)
