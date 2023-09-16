CC = gcc
RM = rm
VALGRIND = valgrind

CFLAGS = -Wall -I. -I../libs/object_v2 -lm -g
RM_FLAGS = -v
VALGRIND_ARGS = --leak-check=full 

VPATH = ../libs/object_v2
OBJECT = object_v2-object.o object_v2-string.o object_v2-node.o \
 object_v2-hash.o IOStream.o ConfigObject.o FileHash.o FileObject.o \
 QueueObject.o object_v2-simple_chain.o main.o
TARGET = a.out

all: $(TARGET)

$(TARGET): $(OBJECT)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJECT): %.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS)

run: $(TARGET)
	./$(TARGET)

memchk: $(TARGET)
	$(VALGRIND) $(VALGRIND_ARGS) ./$(TARGET) $(COMMAND)

.PHONY: clean
clean:
	$(RM) $(RM_FLAGS) $(OBJECT) $(TARGET)
