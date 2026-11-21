CC = gcc

CFLAGS = -Wall

TARGET = assemble
SOURCES = main.c helpers.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -rf $(TARGET)