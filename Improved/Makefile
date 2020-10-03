CC = g++
CFLAGS = -std=c++11
OBJS = main.o HTTPBench.o
LIBS = -pthread
BIN  = HTTPBench

$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -fr *.o $(BIN)