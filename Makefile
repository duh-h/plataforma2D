CC = gcc
CFLAGS = -Wall -std=c99 -Wno-missing-braces
TARGET = jogo

INCLUDES = -I/opt/homebrew/include
LIBRARIES = -L/opt/homebrew/lib

FRAMEWORKS = -framework IOKit -framework Cocoa -framework OpenGL -framework CoreVideo

LIBS = -lraylib


all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET) $(INCLUDES) $(LIBRARIES) $(LIBS) $(FRAMEWORKS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean