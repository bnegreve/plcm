AM_CPPFLAGS = -I$(srcdir)/../melinda/
AM_CFLAGS = -std=c99
AM_LDFLAGS = 

if DEBUG
AM_CPPFLAGS +=  
else 
AM_CPPFLAGS += -DNDEBUG
endif 

if PROF
AM_CPPFLAGS += -pg --coverage
AM_LDFLAGS += -pg --coverage
endif 

COMMON = Alloc.hpp Array.hpp defines.hpp head.cpp \
	melinda_local.hpp Occurences.cpp Occurences.hpp \
	Permutations.cpp Permutations.hpp plcm.cpp plcm.hpp\
	 Transactions.cpp Transactions.hpp


bin_PROGRAMS = plcm

LDADD = ../melinda/libmelinda.a -lpthread

AM_DEFAULT_SOURCE_EXT = .cpp

plcm_SOURCES = $(COMMON)

