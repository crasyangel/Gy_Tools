CC := gcc
CFLAGS := -g
CLFAGS += -Wall 
TARGET := Gy_Tools
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

all:$(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(TARGET) *.o
