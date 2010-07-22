## Makefile
## Made by Alexandre Termier
## Login   <alexandre@belem>
## Started on  yyyy/mm/dd : 2004/04/09 Alexandre Termier
## Last update Fri Apr  3 11:27:24 2009 Benjamin Negrevergne

#
# Makefile for PLCM
#
#

#CC = g++ -m32
#CC = icc -O1
#CC = g++ -O3 
CC = g++ -g -Wall

MELINDA_DIR=../../melinda/

LIBS= -lmelinda -lpthread
#HEAPALLOC

##Compilation parameters


ifeq ($(debug), 1)
  CFLAGS+= -g3
else
  CFLAGS+= -O3 -DNDEBUG
endif

ifeq ($(prof), 1)
  CFLAGS+=-pg
  LDFLAGS+=-pg
endif

ifdef nt
  CFLAGS+=-DNUM_THREADS_MACRO=$(nt)
else
  CFLAGS+=-DNUM_THREADS_MACRO=2
endif

ifdef ni
  CFLAGS+=-DNUM_INTERNALS_MACRO=$(ni)
else
  CFLAGS+=-DNUM_INTERNALS_MACRO=2
endif



CFLAGS+=  -I/usr/local/include $(INCS) -I$(MELINDA_DIR) 
#CFLAGS= -g3  -I/usr/local/include  -I$(MELINDA_DIR) -I$(LINUX_TUPLES)

##Link parameters
LDFLAGS+= -L/usr/local/lib -L$(MELINDA_DIR) 


#for assembly debug
CFLAGSASM= -O3  -S -DNDEBUG  -I/usr/local/include 
LDFLAGSASM= 

EXE_NAME = plcm

#Default target
all:$(EXE_NAME)


#All ar	ch independant source files
SRCS= plcm.cpp Transactions.cpp Occurences.cpp Permutations.cpp

#All arch independant object files	
OBJS=$(addsuffix .o,$(basename $(SRCS))) 

ASMS=$(addsuffix .s,$(basename $(SRCS))) 




.PHONY :  dep tags doc


#How to build PLCM objets files
%.o:%.cpp depends
	$(CC) -c $(CFLAGS) -o $@ $<

%.s:%.cpp depends
	$(CC) -c $(CFLAGSASM) -o $@ $<




$(EXE_NAME): $(OBJS) libmelinda.a 
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

asm_debug: $(ASMS) 
	$(CC) $(LDFLAGSASM) $(ASMS) -o $@


#How to construct de dep file which contain all de dependences (included at the very end of this Makefile)
dep: 
	$(CC)  $(CFLAGS) -MM $(SRCS) > depends

libmelinda.a:
	if [ $(debug) -eq 1 ]; then \
	cmake ../../melinda/ -DCMAKE_BUILD_TYPE=Debug ;\
	else \
	cmake ../../melinda/ -DCMAKE_BUILD_TYPE=Release ;\
	fi
	make -C$(MELINDA_DIR) melinda

# liblt.a:
# 	make -C $(LINUX_TUPLES) tuple.o
# 	ar -crs $(LINUX_TUPLES)/liblt.a $(LINUX_TUPLES)/tuple.o

clean:
	rm -f *.o ; rm -f *.o ; rm -f *~ ; rm -f *~ ; rm -f $(EXE_NAME) *.s depends

tags:
	etags *.cpp *.hpp *.h */*.cpp */*.hpp */*.h

doc:
	doxygen dox

mt:
	for i in `seq 1 24`; do make -B -j4 ni=$$i nt=$$i; cp plcm plcm$$i;done

-include depends
