//
// defines.hpp
// 
// Made by Benjamin Negrevergne
// Login   <bengreve@confiance.imag.fr>
// 
// Started on  Wed Jul  8 14:21:43 2009 Benjamin Negrevergne
// Last update Wed Jul  8 14:21:43 2009 Benjamin Negrevergne
//

#ifndef   	_DEFINES_HPP_
#define   	_DEFINES_HPP_



#include <iostream>
using std::cout; 
using std::cerr; 
using std::endl;

#include <fstream>
extern std::ostream *output; 
static int nbItemsets = 0; 

#include <cassert>

#include "Array.hpp"

/* TYPES DEFINITIONS */

typedef int item_t; 
typedef int tid_t; 
typedef int weight_t;
typedef int freq_t; 


typedef Array<item_t, false, NullAlloc<item_t> >
	      Transaction;

typedef Array<tid_t, false, NullAlloc<tid_t> > Occurences;

typedef Array<weight_t> Weights; 
typedef Array<freq_t> Frequencies; 

typedef struct {
  Array<Transaction> *trans; 
  Weights *weights;
  item_t item;
  item_t maxItem;   
  item_t *data; 
} TransactionTable;

typedef struct{
  Array<Occurences> *occs;
  Array<int> *perms; 
} OccurencesTable;

typedef Array<item_t> Itemset;
/* PROGRAM PARAMS */

#define PARALLEL_PROCESS

#define REORDER_ITEMS_BASE

//#define MEMORY_OUTPUT 


/* Enable db reduction on initial database*/
#define DB_REDUCTION_REDUCE_INITIAL_DB 
#define DB_REDUCTION_ENABLE_SUFFIX_INTERSECTION 
static const unsigned int DB_REDUCTION_MIN_DB_SIZE = 7;
static const unsigned int DB_REDUCTION_MIN_CANDIDATES_NMBR = 2;
#define DB_REDUCTION_QUICKSORT_THRESHOLD 100000//200000//500
#define DB_REDUCTION_INSERTIONSORT_THRESHOLD 0//12

#define REBASE_PERMUTE_ITEMS


static const int TRANSACTION_BUFFER_SIZE = 32768; 



#define COLOR_YELLOW "\033[33m"
#define COLOR_PINK "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_BLUE "\033[34m"
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_BACK "\033[0m"

/* Simple static timer using gettimeofday() */
#include <sys/time.h>
inline double timer(){
  static struct timeval tv ;
  static bool running = false; 
  if(running = !running){
    /* if it was not running before ...*/
    gettimeofday(&tv, NULL); 
    return 0; 
  }
  else{
    static timeval tv2; 
    gettimeofday(&tv2, NULL); 
    return (tv2.tv_sec+(double)tv2.tv_usec/1000000.) - (tv.tv_sec+(double)tv.tv_usec/1000000);
  }  
}


/* MISC */

#include <cstdlib>
#include <cstdio>

#define   fopen2r(f,a,c)     if(!(f=fopen(a,"r"))){printf("%s:file open error %s\n",c,a);exit(1);}
#define   fopen2w(f,a,c)     if(!(f=fopen(a,"w"))){printf("%s:file open error %s\n",c,a);exit(1);}

#define   malloc2(f,a,b,c,d)     if(!(f=(a *)malloc(sizeof(a)*(b)))){printf("%s:memory error %s",c,d);exit(1);}
#define   realloc2(f,a,b,c)     if(!(f=(a *)realloc(f,sizeof(a)*(b)))){printf("memory error, %s\n",c);exit(1);}
#define  free2(a)      {if(a){free(a);(a)=NULL;}a=0;}


#define STR(x)   #x
/* Print the current state of a given DEFINE*/
#define SHOW_DEFINE(x) printf("%s %s\n", #x, strcmp(STR(x),#x)!=0?"is defined.":"is NOT defined.")

#endif	    /* _DEFINES_HPP_ */
