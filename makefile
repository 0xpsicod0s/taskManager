CC = gcc
CFLAGS = -Wformat=0 -pthread
TARGET = monitor

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)
