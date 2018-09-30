#######################
# Makefile
#######################

# compile and lib parameter
CC      := g++
LIBS    :=
LDFLAGS :=
DEFINES :=
INCLUDE := -I.
CFLAGS  := 
CXXFLAGS:= 

# link parameter
#LIB := libfunc.so
LIB := libfunc.a

#link
$(LIB):func.o
	$(CC) -shared -o -fPIC -o $@ $^
#compile
func.o:thread.c
	$(CC) -c -fPIC $^ -o $@ 

# clean
clean:
	rm -fr *.o
