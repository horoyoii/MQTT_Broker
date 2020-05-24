CC = g++
TARGET = main
OBJS = main.o Connection.o Node.o
SRC_DIR = src

all : $(TARGET)

Connection.o : $(SRC_DIR)/Connection.cpp
	$(CC) -c -o Connection.o $(SRC_DIR)/Connection.cpp

Node.o : $(SRC_DIR)/Node.cpp
	$(CC) -c -o Node.o $(SRC_DIR)/Node.cpp

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) -w

clean : 
	rm *.o 
