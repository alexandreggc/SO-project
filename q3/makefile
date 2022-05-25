# Name of the project
PROJ_NAME=cypher
 
# .c files
C_SOURCE=$(wildcard *.c)
 
# .h files
H_SOURCE=$(wildcard *.h)
 
# Object files
OBJ=$(C_SOURCE:.c=.o)
 
# Compiler
CC=gcc
 
# Flags for compiler
CC_FLAGS=-c                 \
         -w                 \
         -W                 \
         -Wall              \
         -Wextra            \
         -ansi              \
         -pedantic          \
         -pedantic-errors   \
         -std=c99           \

#
# Compilation and linking
#

all: $(PROJ_NAME)

$(PROJ_NAME): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c %.h
	$(CC) -o $@ $< $(CC_FLAGS)

clean:
	rm -rf *.o $(PROJ_NAME) *~