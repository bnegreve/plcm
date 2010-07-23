/*
** PLCM/Melinda
** (c) Benjamin Negrevergne, 2010
** Original LCM algorithm from Takaki Uno and Hiroki Arimura.
**
** The program was design and implemented in colaboration with
** Alexandre Termier and Jean-François Méhaut.
** 
** See the README file for installation, usage, and details.
** See the LICENSE file for licensing.
*/

#include "plcm.hpp"

#include "Permutations.hpp" 
#include <unistd.h>
#ifdef PARALLEL_PROCESS
#include <ostream>
#include <sstream>

extern "C" {
#include <tuplespace.h>
#include <thread.h>
}

#include "melinda_local.hpp"
tuplespace_t ts;

//const int NUM_THREADS = NUM_THREADS_MACRO;
int numThreads = 1; 
std::ofstream **outputs;

#endif //PARALLEL_PROCESS

/*** BEG removeInfrequentItems ***/
item_t removeInfrequentItems(Frequencies *frequencies, Itemset *itemset, 
			     item_t item, Array<item_t> *candidates,
			     int threshold, int freq,
			     const Array<int> &permutations){
  item_t flag=-1; //This will tell if we need to call lcmIter on this itemset  

  const item_t *presentEnd = candidates->pEnd(); 
  int candidateIdx = 0; 
  //  for(int i = candidates->size(); i >= 0; i++){
    //    presentItemValue = (*candidates)[i];
  for(const item_t *presentItem = candidates->pData();
      presentItem != presentEnd; ++presentItem){
    item_t presentItemValue = *presentItem; 
    int presentItemFreq = (*frequencies)[presentItemValue];
    if(presentItemFreq >= threshold){//if current item is frequent in cdb
      if(presentItemFreq < freq){ // but not as the item (not 100% frequent)
	//	if(presentItemValue < item){//TODO SURE ??
	(*candidates)[candidateIdx++] = presentItemValue;
	continue;
      }
      else{ /* if it is 100% frequent, it can be added to current itemset*/
	if(presentItemValue > item) return presentItemValue; 
	itemset->pushBack(permutations[presentItemValue]); 
	if(presentItemValue > flag)
	    flag=presentItemValue; 
      }
    }
    (*frequencies)[presentItemValue] = 0; 
  }
  candidates->resize(candidateIdx); 
  return flag;
}
/*** END removeInfrequentItems ***/


/*** BEG lcmIter ***/
void lcmIter(const TransactionTable &tt, OccurencesTable *ot, 
	     Frequencies *frequencies, Itemset *itemset, item_t item,
	     int threshold, item_t previous, int depth, bool rebase){

  //  cout<<"DEPTH : "<<depth<<"rebase : "<<rebase<<endl;
  itemset->pushBack((*ot->perms)[item]);

  //get a copy of item occurences 
  const Occurences &itemOccs((*ot->occs)[item]);
  
  Array<item_t> candidates(tt.maxItem+1); 
 

  int dbSize; 

#if defined(PARALLEL_PROCESS) || defined(REBASE_PERMUTE_ITEMS)
  Array<int> workspace;
  Array<int> *perms = ot->perms; 
  frequencies->resizeInit(tt.maxItem+1, 0);
  if(rebase){ 
    /* If rebase has been forced from previous call, frequency count
       and compute the sizes for the new occurencesTable */
    frequencies = new Frequencies(tt.maxItem+1);
    dbSize = frequencyCountAndSize(tt, itemOccs,
				    frequencies, &candidates, &workspace);
  }
  else{
    /* Clear the frequency array and reuse it for frequency counting */
  assert(frequencies->memSize() >= tt.maxItem+1); 
  frequencies->resize(tt.maxItem+1); 
  memset(frequencies->pData(), 0, (tt.maxItem+1)*sizeof(item_t));       
  /* Compute the frequencies of each item */
  dbSize = frequencyCount(tt, itemOccs,
			  frequencies, &candidates);
  }

#else /* if sequential, there is no need to rebase */
  
  /* Clean the frequency array and reuse it */
  assert(frequencies->memSize() >= tt.maxItem+1); 
  frequencies->resize(tt.maxItem+1); 
  memset(frequencies->pData(), 0, (tt.maxItem+1)*sizeof(item_t));       
  /* And compute the frequencies of each item */
  dbSize = frequencyCount(tt, itemOccs,
			  frequencies, &candidates);
  
#endif //PP RPI
 
  /* freq is used to know whether other items belong to the closure
     (as frequent as item) or are extention candidates (less frequent as item)*/  
  int freq = (*frequencies)[item]; 

  (*frequencies)[item] = 0; 
  if(previous != -1){
    (*frequencies)[previous] = 0; 
  }
  
  item_t flag;  
  flag = removeInfrequentItems(frequencies, itemset, item, &candidates, threshold, freq, *ot->perms); 
  /* If there is another item with higher value (ie. lower frequency)
     can be added to the current itemset, no recursive call, it will be
     done later on the higher value item of the itemset*/    
  if(flag > item){
    return; 
  }
  
  dumpItemset(*itemset, freq); 
  nbItemsets++; 

  item_t maxCandidate = -1;
  int nbCandidates = 0; 
  for(item_t *candidate = candidates.pData();
      candidate < candidates.pEnd(); ++candidate){
    if(*candidate < item){
      if(*candidate > maxCandidate){
	maxCandidate = *candidate;
      }
      nbCandidates++;
    }
  }

  (*frequencies)[item] = freq; 

   if(nbCandidates > 0){
     /* To restore the itemset as it was before */
     int core = itemset->size(); 
	  
     Array<int> permutations;
#if defined(PARALLEL_PROCESS) || defined(REBASE_PERMUTE_ITEMS)
#ifdef REBASE_PERMUTE_ITEMS
     /* Decide wether we should reorder the items or not */
     bool reorder = nbCandidates > 8;
#else
     bool reorder = false; 
#endif 
     if(rebase || reorder){
       ot = new OccurencesTable;
       itemset = new Itemset(*itemset);
       ot->perms = perms;

#ifdef REBASE_PERMUTE_ITEMS
       //TODO !!!!!! Do not work with high level tuples, the permutation array has to be duplicated somewhere
       if(!rebase){
	 /* It means the occurences sizes have not been calculated, do it. */
	 computeOccurencesSizes(tt, itemOccs, &workspace);
       }
       sortItems(item, *frequencies, &permutations, threshold);
       
       /* Permute candidates that has been generated from the previous
	  database and thus with another order.*/
       permuteValues(&candidates, permutations); 
       
       /* Permutes the sizes in workspace the it matches new items.*/
       permuteIndexes(&workspace, permutations); 
       
       item = permutations[item];
#endif// PARALLEL PROCESS || REBASE_PERMUTE_ITEMS 
       createOccurencesTable(ot, dbSize, &workspace); 
       
       rebase |= reorder;
     }
#endif //PARALLEL_PROCESS
     candidates.sort();      
     //TODO if nbcandidates < 1 ne pas refaire de TT
     
     /* Create a transaction table for item */
     TransactionTable &newTT = *new TransactionTable; //TODO change this to a pointer

     //  printTransactionTable(tt);
#ifdef REBASE_PERMUTE_ITEMS
     if(!rebase){
       createTransactionTable(tt, item, itemOccs, *frequencies,
			      maxCandidate, dbSize+(*ot->occs)[item].size(),
			      &newTT, true);
     }
     else{
       createTransactionTablePermuteItems(tt, item, itemOccs, *frequencies,
					  maxCandidate, 
					  dbSize+(*ot->occs)[item].size(),
					  &newTT, true, permutations);

       /* Compute the backward permutation to remember the original items names */
       invertPermutations(&permutations);
       mergePermutations(&permutations, *ot->perms); 
       ot->perms = &permutations;
     }
     
#else 
     createTransactionTable(tt, item, itemOccs, *frequencies,
			    maxCandidate, dbSize+(*ot->occs)[item].size(),
			    &newTT, true);
#endif //REBASE_PERMUTE_ITEMS
     
     /* Shrink the database only if it is going to be used more than once and it is big */
     if(nbCandidates >= DB_REDUCTION_MIN_CANDIDATES_NMBR
	&& newTT.trans->size() > DB_REDUCTION_MIN_DB_SIZE)
       mergeIdenticalTransactions(&newTT); 


     /* Compute the occurences for the newTT. */
     assert(item != 0); 
     occurenceDeliverAll(newTT, ot, item-1);

     /* Parallel call, if possible */

#ifdef MULTI_LEVEL_TUPLES

     if(rebase  && depth <= 1){ 
       tuple_t tuples[candidates.size()]; 
       int i = 0; 
       for(const item_t *candidate = candidates.pData(); 
	   candidate < candidates.pEnd() && *candidate < item; ++candidate){	 
	 tuple_t *t = &tuples[i++]; 
	 t->tt = &newTT; 
	 t->ot = ot; 
	 t->frequencies = frequencies; 
	 t->itemset = new Itemset(*itemset);
	 t->item = *candidate;
	 t->threshold = threshold; 
	 t->previous = item; 
       }
       m_tuplespace_put(&ts, (opaque_tuple_t*)&tuples, i); 
     }
  
     /* Recursive call for each candidates items. */
     else{
#endif //MULTI_LEVEL_TUPLES
       for(const item_t *candidate = candidates.pData(); 
	   candidate < candidates.pEnd() && *candidate < item; ++candidate){
	 lcmIter(newTT, ot, frequencies, itemset, *candidate, threshold, item, depth+1, false); 
	 
	 /* Once the candidate is process, reset the occurences in ot so
	    it can be reuse for next candidates with greater value */
	 (*ot->occs)[*candidate].clear();
	 
	 /* Restore the Itemset as it was before recursive call. */
	 itemset->shrink(core); 
       }
     
       deleteTransactionTable(&newTT); 
#ifdef MULTI_LEVEL_TUPLES
     }
#endif //MULTI_LEVEL_TUPLES

#ifdef REBASE_PERMUTE_ITEMS
     if(rebase)
       deleteOccurencesTable(ot); 
#endif // REBASE_PERMUTE_ITEMS
   }
   
#ifdef REBASE_PERMUTE_ITEMS
   if(rebase){
     //     delete frequencies; 
     //     delete itemset; 
   }
#endif // REBASE_PERMUTE_ITEMS
}
/*** END lcmIter ***/ 



/*** BEG dumpItemsent ***/
void dumpItemset(const Itemset &itemset, freq_t freq){  
  std::ostream *os = outputs[m_thread_id()];
  const item_t *iEnd = itemset.pEnd(); 
  for(const item_t *item = itemset.pData(); item != iEnd; ++item){
    *os<<*item<<" ";
  }
  *os<<"("<<freq<<")\n"; 
}
/*** END dumpItemset ***/


/*** BEG processTupleThread ***/
#ifdef PARALLEL_PROCESS
void processTupleThread(int id){
  m_thread_register(); 
  
  /* Retreives a Tuple */
  for(;;){
    tuple_t t; 
    
    int r = m_tuplespace_get(&ts, 1, (tuple_t*)&t);
    if(r == TUPLESPACE_CLOSED)
      break; 
  
    // tt = t->getValue<TransactionTable *>(0);
    // ot = t->getValue<OccurencesTable *>(1);
    // frequencies = t->getValue<Frequencies *>(2);    
    // itemset = t->getValue<Itemset *>(3);
    // item = t->getValue<item_t>(4);
    // threshold = t->getValue<int>(5); 
    // previous = t->getValue<item_t>(6); 

    /* call */
    //    usleep(200000); 
    lcmIter(*t.tt, t.ot, t.frequencies, t.itemset, t.item, t.threshold,
	    t.previous, (t.previous==-1?1:2), true); 

    //    delete t; 
  }
}
#endif // PARALLEL_PROCESS
/*** END processTupleThread ***/

void usage(char *a){
  cerr<<"Usage: "<<a<<" dataset asbolute_threshold output_prefix [-t nbthreads] \n"<<endl;
  exit(EXIT_FAILURE);
}

/*** BEG main ***/
int main(int argc, char **argv){

  /* Recover optional arguments */
  int opt; 


  while ((opt = getopt(argc, argv, "t:")) != -1) {
    switch (opt) {
    case 't':
      numThreads = atoi(optarg);
      break;
    default: /* '?' */
      usage(argv[0]); 
    }
  }
  /* Recover mandatory arguments, (ie. input file and threadhold) */
  argv+=optind; 
  if (argc - optind != 3) {
    usage(argv[0]); 
  }

  char* inputFileName(argv[0]) ;
  int threshold = atoi(argv[1]) ;
  std::string outputPrefix(argv[2]); 

  if(outputPrefix.empty()){
    cout<<"No output file, standard output."<<endl; 
    outputs = NULL;
  }
  else{    
    outputs = new std::ofstream *[numThreads+1]; 
    for(int i = 0; i <= numThreads; i++){
      std::ostringstream oss; 
      oss<<outputPrefix<<i<<".dat"; 
      outputs[i] = new std::ofstream(oss.str().c_str()); 
    }
  }


  m_tuplespace_init(&ts, sizeof(tuple_t), 0, TUPLESPACE_OPTIONAUTOCLOSE); 
  m_thread_register(); 
  
  SHOW_DEFINE(NDEBUG); 
  SHOW_DEFINE(PARALLEL_PROCESS);
  SHOW_DEFINE(REBASE_PERMUTE_ITEMS);
  SHOW_DEFINE(DB_REDUCTION_REDUCE_INITIAL_DB);
  SHOW_DEFINE(MULTI_LEVEL_TUPLES); 



  TransactionTable itt; 
  Array<item_t> permutations; 
  int dbSize; 

  loadTransactionsFromFile(inputFileName, &itt,
			   threshold, &permutations, &dbSize); 
  
  //  printTransactionTable(itt);   
  
  Array<tid_t> presentItems; 
  Occurences allTids((tid_t*)malloc(itt.trans->size() * sizeof(tid_t)),
		     itt.trans->size()); //TODO something cleaner and free the mem.
  for(int i = 0 ; i < itt.trans->size(); i++){
    allTids.pushBack(i); 
  }  
 
  
  /* Array used among all recursives calls for storing the frequencies
     of each items. It will never need more than maxItem+1 elements,
     however it must be reset before each call to frequencyCount.*/
  Frequencies frequencies;

  /* First call to FrequencyCountAndSize() to determine frequent items
     and to determine the sizes of the Occurences in occurenceTable */
  Array<item_t> workspace; 
  frequencyCountAndSize(itt, allTids, &frequencies, &presentItems, &workspace); 
  
  OccurencesTable ot; 
  createOccurencesTable(&ot, dbSize, &workspace);  

  /* Transform forward permutations into backward permutations in order 
     to retaure the original item names when dumping itemsets */
  invertPermutations(&permutations); 
  ot.perms = &permutations; 

  occurenceDeliverAll(itt, &ot, itt.maxItem); 

#ifndef PARALLEL_PROCESS
  /* Sequential */
  Itemset itemset; 

  for (item_t item = 0; item <= itt.maxItem; item++){
    lcmIter(itt,  &ot, &frequencies,
	    &itemset,  item, threshold, -1, 1, false);
    itemset.resize(0); 
    (*ot.occs)[item].clear(); 
  }
  cout<<"TIME "<<timer()<<endl;

#else // PARALLEL_PROCESS

  /* WARNING for timing purpose, all the tuples are pushed before the
     creation of the threads, in the real life it is better to
     procecss the tuples while pushing the tuples */

  int nbTuples = itt.maxItem+1;
  tuple_t *tuples = new tuple_t[nbTuples];

  cout<<"Pushing "<<nbTuples<<" tuples"<<endl;
  for (item_t item = 0; item <= itt.maxItem; item++){
    tuple_t *t = &tuples[item]; 
    t->tt = &itt; 
    t->ot = &ot; 
    t->frequencies = &frequencies; 
    t->itemset = new Itemset; 
    t->item = item; 
    t->threshold = threshold; 
    t->previous = -1;     

  }

  m_tuplespace_put(&ts, (opaque_tuple_t*)tuples, nbTuples); 
  m_tuplespace_close_at(&ts, numThreads);   



  //  Run the threads
  pthread_t *tids = new pthread_t[numThreads];
  for(int i = 0; i < numThreads; i++){
    cout<<"Creating thread"<<endl;
    if(pthread_create(&tids[i], NULL, 
  		      (void*(*)(void*))processTupleThread, (void*)i)){
      perror("pthread_create ");
      exit(EXIT_FAILURE); 
    }
  }



  for(int i = 0; i < numThreads; i++)
    pthread_join(tids[i], NULL);

  for(int i = 0; i <= numThreads; i++){
    outputs[i]->close();
    delete outputs[i]; 
  }
  delete [] outputs;

  cout<<"TIME "<<timer()<<endl;

#endif //PARALLEL_PROCESS

  //printTransactionTable(newTT); 
  deleteOccurencesTable(&ot); 
  //  deleteTransactionTable(&itt); 

  cout<<nbItemsets<<endl;

  exit(EXIT_SUCCESS); 

}
/*** END main ***/

