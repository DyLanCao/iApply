###########################################
#Makefile for simple programs
###########################################
INC=
LIB= -lpthread
 
CC=gcc
CC_FLAG=-Wall

PRG=threadtest
#OBJ=thread.o CThreadPool.o CThread.o CWorkerThread.o threadpooltest.o
OBJ=thread.o 
 
$(PRG):$(OBJ)
	$(CC) $(INC) -o $@ $(OBJ) $(LIB)
	
.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o
 
.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)

