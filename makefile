INCLUDE_DIR =
CC=gcc
LINK=gcc
CFLAGS= -I$(INCLUDE_DIR) -pthread
LFLAGS_DLL= -shared -pthread
LFLAGS_TEST= -pthread


#Set all files here
FILES = Queue
OBJ = $(addsuffix .o,$(FILES))

%.o: %.c
		$(CC) -c -o $@ $< $(CFLAGS)

libqueue: $(OBJ) 
		$(LINK) -o $@.dll $^ $(LFLAGS_DLL)
		
queuetest: $(OBJ) QueueTest.o
		$(LINK) -o $@.exe $^ $(LFLAGS_TEST)
clean:
		rm *.o *.exe *.dll 2>nul
