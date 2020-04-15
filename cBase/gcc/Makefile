#######################
# Makefile
#######################

# compile and lib parameter
CC      := g++
LIBS    :=
LDFLAGS :=
DEFINES :=
INCLUDE := -I.
CFLAGS   := -fno-common -fmessage-length=0 -Wall \
           -fno-exceptions -ffunction-sections \
           -fdata-sections -fomit-frame-pointer
CXXFLAGS:= 
CFLAGS   += -Werror
# link parameter
#LIB := libfunc.so
LIB := libfunc.a

#link
$(LIB):test.o
	$(CC) $(CFLAGS) -shared -o -fPIC -o $@ $^
#compile
func.o:test.c
	$(CC) -c -fPIC $^ -o $@ 

# clean
clean:
	rm -fr *.o
