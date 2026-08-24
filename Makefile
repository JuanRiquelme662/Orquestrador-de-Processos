
CC = gcc
CFLAGS = -Wall -Wextra
TARGET = processflow
SRC = main.c funcoes.c
 
.PHONY: all clean run test
 
all: $(TARGET)
 
$(TARGET): $(SRC) funcoes.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)
 
clean:
	rm -f $(TARGET) *.o *.txt
 
run: $(TARGET)
	./$(TARGET)
 
test: $(TARGET)
	./$(TARGET) test.pf
 