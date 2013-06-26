/*
** PLCM/Melinda
** Copyright 2009, 2010 Grenoble University
** Original LCM algorithm from Takaki Uno and Hiroki Arimura.
**
** The program was design and implemented in colaboration with
** Alexandre Termier and Jean-François Méhaut.
** 
** See the README file for installation, usage, and details.
** See the LICENSE file for licensing.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include "Transactions.hpp"

/*** BEG loadTransactionsFromFile ***/
int loadTransactionsFromFile(char *fileName, TransactionTable *tt, int threshold, Array<item_t> *permutations, int *dbSize){
  int nbTransactions; 
  Array<int> itemSupport = countItemsSupport(fileName,  &nbTransactions); 

  tt->trans = new Array<Transaction>(nbTransactions);
  tt->weights = new Weights(nbTransactions);

  Array<Transaction> *transactions = tt->trans; 
  Weights *weights = tt->weights; 

  int nbItems=itemSupport.getSize(); 
  
#ifdef REORDER_ITEMS_BASE
  /* Reorder items by frequency and store permutations */
  
  int nbFrequentItems = reorderFrequentItems(&itemSupport, permutations, threshold, dbSize);
#else
  int nbFrequentItems = nbItems; 
#endif //REORDER_ITEMS_BASE
  
  /* Second pass, allocates and fills the transaction array */

  //Allocate enought memory for storing all items in the db. 
  //Transactions in transactions will point to different part of this memory bloc.
  tt->data = (item_t *)malloc(sizeof(item_t) * (*dbSize)); 
  item_t *transactionsData = tt->data; 
  int tdOffset = 0; //offset for the first free cell in transactionsData (initial 0). 

  std::ifstream reader ;

  reader.open(fileName) ;
  if(reader.bad()){
    cerr<<"Cannot open input file : "<<fileName<<endl; 
    exit(EXIT_FAILURE); 
  }


  //  cout<<"itemsupport "<<itemSupport<<endl<<"perms "<<*permutations<<endl;

  /* TODO find the initial db size so the transactions array is not resized log_2(nb_transactions) times */

  char buf[TRANSACTION_BUFFER_SIZE] = {0};
  int transactionIdx = -1; 
  
  int lineNmbr=0;

  /* Reads each transaction line */
  for(;;){
    
    reader.getline(buf, TRANSACTION_BUFFER_SIZE);
    if(reader.fail()){
      if(reader.eof())
	break; 
      else{
	cerr<<"Cannot read line from input file, be sure that the TRANSACTION_BUFFER_SIZE is big enough for each line of the file (line : "<<lineNmbr<<")."<<endl;
	exit(EXIT_FAILURE); 
      }
    }
    
    lineNmbr++;
    
    /* Create a new empty transaction if needed */
    if(transactionIdx == -1){
      assert(tdOffset <= *dbSize);
#ifdef NDEBUG
      transactionIdx = transactions->pushBack(Transaction(transactionsData+tdOffset, 0)); 
#else
      transactionIdx = transactions->pushBack(Transaction(transactionsData+tdOffset, *dbSize - tdOffset)); 
#endif
      weights->pushBack(1); 
    }
    Transaction *currentTransaction = &(*transactions)[transactionIdx];
      
    std::istringstream strStream(buf);
    int currentItem;
    
    while(strStream>>currentItem){		 
      /* Read items in the transaction, pushes them into the current transaction. */
#ifdef REORDER_ITEMS_BASE
      currentItem = (*permutations)[currentItem]; /* Get the permuted value of the red item */
#endif //REORDER_ITEMS_BASE
      if(itemSupport[currentItem] > 0){
	currentTransaction->pushBack(currentItem); 	  
      }
      else{
	//TODO remember removed tid. 
      }
    }
    if(!strStream.eof()){
      cerr<<"Cannot parse input file (line "<<lineNmbr<<")."<<endl;
      exit(EXIT_FAILURE); 
    }

     
      /* If the transaction is empty currentTransactionIdx is kept at the same value, 
	 so the slot is reused.*/
      if (currentTransaction->getSize() > 0){
	if (currentTransaction->getSize() > 1)
	  currentTransaction->sort();
	transactionIdx = -1; 

	/* Moves the offset in transactionsBuffer */
	tdOffset+=currentTransaction->getSize(); 
      }
    }


    /* remove the last transaction if it was empty */
    if(transactionIdx != -1){
      transactions->resize(transactions->getSize() -1); 
    }

  reader.close() ;
  //  cout<<"TRANSACTIONS :"<<endl<<*transactions<<endl; 

  tt->maxItem = nbFrequentItems - 1; 
  tt->item = -1; // set item to a dummy value so suffix intersection is not calles
  timer(); 
#ifdef DB_REDUCTION_REDUCE_INITIAL_DB
  mergeIdenticalTransactions(tt, true);
#endif
  return NULL; 

}
/*** END loadTransactionsFromFile ***/


/*** countItemsSupport ***/
Array<int> countItemsSupport(char *fileName, int *nbTransactions){
  int i, j, found, item, maxItem=0;
  int fc=TRANSACTION_BUFFER_SIZE, p= TRANSACTION_BUFFER_SIZE;
  char ch, *buf;
  FILE *fp;

  fopen2r ( fp, fileName, "TRSACT_file_count");
  *nbTransactions=0; /* # transactions */
  Array<int> E(128);
  malloc2 ( buf, char, TRANSACTION_BUFFER_SIZE, "TRSACT_file_count", "buf");
  int nbLines = 0; 
  while (1){
    nbLines++;
    do { /* Reads a line */ 
      /* pp stores the begening of the number, p the offset in the buffer fc the size of the effective buffer*/
      for (found=0, item=0 ; 1 ; item=item*10 +(int)(ch-'0') ){
	/* Reads a number */
        if ( p == fc ){
          if ( fc < TRANSACTION_BUFFER_SIZE ) goto END;
          fc = fread(buf, 1, TRANSACTION_BUFFER_SIZE, fp);
          if ( fc <= 0 ) goto END;
          p = 0;
        }
        ch = buf[p++];
        if ( (ch < '0') || (ch > '9')) break; 
	found=1; 
      }
      if ( found ){
	if(item>=maxItem){
	  maxItem = item;
	  E.resizeInit(item+1, 0);
	}
	E[item]++;
      }
    } while (ch != '\n');
    (*nbTransactions)++;  /* increase #transaction */
    
  }
  END:;

  fclose ( fp );
  free ( buf );
  return ( E );
}
/*** END countItemsSupport ***/


struct pairCmpFirstGt:public std::binary_function<const std::pair<int,item_t> &,const std::pair<int,item_t> &, bool>{
    bool operator()( const std::pair<int,item_t> &p1, const std::pair<int,item_t> &p2){
      return (p1.first>p2.first);
    }
};


/*** reorderFrequentItems ***/
int reorderFrequentItems ( Array<int> *itemSupport, Array<item_t> *permutations, 
			   int threshold, int *dbSize){

  int Enum = 0; 
  *dbSize = 0; 
  int nbFrequentsItems = 0; 

  permutations->clear();
  permutations->expand(itemSupport->getSize());

  //cout<<*itemSupport<<endl;

  /* pPerms contains itemsets associated with their support. So it can
     be sorted keeping a track of the initial order. */
  std::pair<int, item_t> *pPerms = (std::pair<int, item_t> *)
    malloc(sizeof(std::pair<int, item_t>) * itemSupport->getSize());

  //std::pair<int, int> *pPerms = permutations->pData(); 

  for (int i = 0; i < itemSupport->getSize(); i++){
    pPerms[i].first = (*itemSupport)[i];
    pPerms[i].second = i;
  }

  std::sort(pPerms, pPerms+itemSupport->getSize(), pairCmpFirstGt()); 
  
  for(int i = 0; i < itemSupport->getSize(); i++){
    if(pPerms[i].first >= threshold){
      (*itemSupport)[i]=(pPerms[i].first); 
      *dbSize += pPerms[i].first; 
      nbFrequentsItems++;
      }
    else{
      (*itemSupport)[i]=-1;
    }
    (*permutations)[pPerms[i].second] = i;
  }
  
  free(pPerms);
  return nbFrequentsItems; 
}
/*** END reorderFrequentItems ***/

/*** createTransactionTable ***/
void createTransactionTable(const TransactionTable &parentTT, item_t item, 
			    const Occurences &itemOccs, const Array<freq_t> &frequencies,
			    item_t maxCandidate, unsigned int dbSize,
			    TransactionTable *newTT, bool merge){

  int nbTransactions = itemOccs.size(); 
  /* Create array for transaction structures and weights.*/
  newTT->trans = new Array<Transaction>(nbTransactions); 
  newTT->weights = new Array<weight_t>(nbTransactions); 
  newTT->item = item; 
  
  /* Allocate enough memory for the items of all transactions */
  newTT->data = (item_t*)malloc(sizeof(item_t) * dbSize);
  item_t *transactionsData = newTT->data; 

  Array<Transaction> *pNewTT = newTT->trans; 
  pNewTT->expand(nbTransactions); 
  Array<weight_t> *pNewWeights = newTT->weights; 
  pNewWeights->expand(nbTransactions); 

  Array<Transaction> *parentTrans = parentTT.trans; 
  Array<weight_t> *parentWeights = parentTT.weights; 

  item_t maxItem = 0; 

  assert(transactionsData);
  int tdOffset=0; 

  //     Transactions *newTransaction; //pointer to a Transaction to be added into the new CDB
  int newTransIndex = -1; 
  Transaction *pNewTrans = NULL; 
  // loop over the tids in occ
  //     std::cout<<itemOccs.getSize()<<std::endl;
  const tid_t *occEnd = itemOccs.pEnd(); 
  for(const tid_t *tid = itemOccs.pData(); tid < occEnd; ++tid){
    const Transaction *trans = &(*parentTrans)[*tid];
    // if first element of the transaction is bigger than item, the
    //   transaction does not contains item, drop it.
    if((*trans)[0] > maxCandidate)
      continue;
    
    // loop over the items in the current transaction
    const item_t *transEnd = trans->pEnd();        
    const item_t *i = trans->pData();
    item_t vi = *i; 
    //       for( ; i < transEnd && vi <= parentDB._maxItem; vi=*(++i)){

    item_t parentMaxItem = parentTT.maxItem;
    /* Create a Transaction inside newTT, if needed (the previous
       transaction may be empty, in this case we skip this part to reuse the spot)*/
    if(pNewTrans == NULL){
      pNewTrans = &(*pNewTT)[++newTransIndex];
#ifndef NDEBUG //ifdebug mode
      new (pNewTrans) Transaction(transactionsData+tdOffset, dbSize-tdOffset);
#else
      new (pNewTrans) Transaction(transactionsData+tdOffset, 0);
#endif 
    }
    /* Set the weigh from the parent transaction */
    (*pNewWeights)[newTransIndex] = (*parentWeights)[*tid]; 
   
    for( ; i < transEnd && vi <= parentMaxItem; ++i){
	vi = *i;
	 /* if item appears in the parent conditional DB (ie. if its frequency has not been set to 0), keep it in this transaction*/ 
	 if(frequencies[vi] > 0){
	   if(vi > maxItem)
	     maxItem=vi;
	   pNewTrans->pushBack(vi);
	 }
    }
    if(pNewTrans->size() != 0){
      tdOffset+=pNewTrans->size();
      pNewTrans = NULL; /* A new Transaction will be created at next iter.*/      
    }
  }
  pNewTT->resize(newTransIndex+1);

  //  cout<<"Created DB for item "<<item<<" ("<<newTT->trans->size()<<endl;
  newTT->maxItem = maxItem;   
  //  mergeIdenticalTransactions(newTT); 
  //  cout<<item<<" "<<maxCandidate<<" "<<newTT->trans->size()<<" "<<maxItem<<endl;
  //    printTransactionTable(*newTT); 
  //    exit(1); 
}
/*** END createTransactionTable ***/


/*** createTransactionTablePermuteItems ***/
void createTransactionTablePermuteItems(const TransactionTable &parentTT, 
					item_t item, const Occurences &itemOccs,
					const Array<freq_t> &frequencies,
					item_t maxCandidate, unsigned int dbSize,
					TransactionTable *newTT, bool merge, 
					const Array<int> &permutations){

  int nbTransactions = itemOccs.size(); 
  /* Create array for transaction structures and weights.*/
  newTT->trans = new Array<Transaction>(nbTransactions); 
  newTT->weights = new Array<weight_t>(nbTransactions); 
  newTT->item = item; 
  
  /* Allocate enough memory for the items of all transactions */
  newTT->data = (item_t*)malloc(sizeof(item_t) * dbSize);
  item_t *transactionsData = newTT->data; 

  Array<Transaction> *pNewTT = newTT->trans; 
  pNewTT->expand(nbTransactions); 
  Array<weight_t> *pNewWeights = newTT->weights; 
  pNewWeights->expand(nbTransactions); 

  Array<Transaction> *parentTrans = parentTT.trans; 
  Array<weight_t> *parentWeights = parentTT.weights; 

  item_t maxItem = 0; 

  assert(transactionsData);
  int tdOffset=0; 

  //     Transactions *newTransaction; //pointer to a Transaction to be added into the new CDB
  int newTransIndex = -1; 
  Transaction *pNewTrans = NULL; 
  // loop over the tids in occ
  //     std::cout<<itemOccs.getSize()<<std::endl;
  const tid_t *occEnd = itemOccs.pEnd(); 
  for(const tid_t *tid = itemOccs.pData(); tid < occEnd; ++tid){
    const Transaction *trans = &(*parentTrans)[*tid];
    // if first element of the transaction is bigger than item, the
    //   transaction does not contains item, drop it.
    if((*trans)[0] > maxCandidate)
      continue;
    
    // loop over the items in the current transaction
    const item_t *transEnd = trans->pEnd();        
    const item_t *i = trans->pData();
    item_t vi = *i; 
    //       for( ; i < transEnd && vi <= parentDB._maxItem; vi=*(++i)){

    item_t parentMaxItem = parentTT.maxItem;
    /* Create a Transaction inside newTT, if needed (the previous
       transaction may be empty, in this case we skip this part to reuse the spot)*/
    if(pNewTrans == NULL){
      pNewTrans = &(*pNewTT)[++newTransIndex];
#ifndef NDEBUG //ifdebug mode
      new (pNewTrans) Transaction(transactionsData+tdOffset, dbSize-tdOffset);
#else
      new (pNewTrans) Transaction(transactionsData+tdOffset, 0);
#endif 
    }
    /* Set the weigh from the parent transaction */
    (*pNewWeights)[newTransIndex] = (*parentWeights)[*tid]; 
   
    for( ; i < transEnd && vi <= parentMaxItem; ++i){
      vi = *i;
      //	cout<<"Transformed "<<*i<<"in "<<permutations[*i]<<endl;
      item_t pvi = permutations[vi];
      /* if item appears in the parent conditional DB (ie. if its frequency has not been set to 0), keep it in this transaction*/ 
      /* frequencies are NOT permuted, we must access with preivous item naming*/
      if(frequencies[vi] > 0){
	if(pvi > maxItem)
	  maxItem=pvi;
	pNewTrans->pushBack(pvi);
      }
    }
    if(pNewTrans->size() != 0){
      tdOffset+=pNewTrans->size();
      pNewTrans->sort(); 
      pNewTrans = NULL; /* A new Transaction will be created at next iter.*/      
    }
  }
  pNewTT->resize(newTransIndex+1);

  //  cout<<"Created DB for item "<<item<<" ("<<newTT->trans->size()<<endl;
  newTT->maxItem = maxItem;   
  //  mergeIdenticalTransactions(newTT); 
  //  cout<<item<<" "<<maxCandidate<<" "<<newTT->trans->size()<<" "<<maxItem<<endl;
  //    printTransactionTable(*newTT); 
  //    exit(1); 
}
/*** END createTransactionTablePermuteItems ***/


/*** mergeIdenticalTransactions ***/  
void mergeIdenticalTransactions(TransactionTable *tt, bool forceRadix){
  //  cout<<"SIZE "<<tt->trans->size()<<" item "<<tt->item<<endl;
  Array<Transaction> *pTransactions = tt->trans; 
  Weights *weights = tt->weights; 

  Array<tid_t> sorted(pTransactions->getSize());
  sorted.expand(pTransactions->size()); 
  tid_t *pSorted = sorted.pData(); 
  for(tid_t i = 0; i< pTransactions->size(); i++){
    pSorted[i] = i; 
  }
  sortTids(&sorted, tt, forceRadix);

  tid_t *refTid = sorted.pData(); //sort transactions, lexical order
  const tid_t *end = sorted.pEnd(); 
  for(tid_t *currentTid = refTid+1; currentTid < end; ++currentTid){
    if((*pTransactions)[*currentTid] == (*pTransactions)[*refTid]){
      (*weights)[*refTid] += (*weights)[*currentTid]; 
      (*pTransactions)[*currentTid].clear(); 
    }
    else{
      refTid=currentTid;
    }
  }

  removeEmptyTransactions(tt);
}
/*** END mergeIdenticalTransactions ***/  

/*** BEG sortTids ***/
void sortTids(Array<tid_t> *tids, TransactionTable *tt, bool forceRadix){
  if((tids->size() < DB_REDUCTION_QUICKSORT_THRESHOLD) && !forceRadix){
    //    quickSortTids(*tt->trans, tids, 0, tids->size());
    //    if(tids->size() <= DB_REDUCTION_INSERTIONSORT_THRESHOLD)
    //      insertionSortTids(*tt->trans, tids, 0, tids->size());
    //    else
    quickSortTids(*tt->trans, tids, 0, tids->size(), tt);

    
//     if(tt->item == -1)
//       return; 
    
//     int blockStart = 0; 
//     int blockItemPos; 
//     for(int i = 1; i < tids->size(); i++){
//       int itemPos; 
//       const Transaction &refTrans = (*tt->trans)[(*tids)[blockStart]];
//       if( (itemPos = 
// 	   refTrans.valueLimitedCompare((*tt->trans)[(*tids)[i]], tt->item)) == -1){
// 	/* if Transactions are not prefix equals */
// 	suffixIntersection(tt, tids, blockItemPos, blockStart, i-1);
// 	blockStart = i;
//       }
//       blockItemPos = itemPos; 
//     }
  }
  else{ //RADIXSORT_TIDS
    //    cout<<"Runing radix"<<endl;
    if(tids->getSize() > 1){
      int *marks = (int*)malloc(sizeof(int) * tids->getSize());
      Array< Array<tid_t> *> buckets(tt->maxItem+1);
      buckets.expandInit(tt->maxItem+1, NULL);
      radixSort__(tids, 0, 0, tids->getSize(), buckets, marks, tt);
            
      for (item_t item=0 ; item <= tt->maxItem ; ++item) { 
	delete buckets[item];
      }
      
      free(marks); 
    }
  }
}
/*** END sortTids ***/

/*** BEG radixSort__ ***/
void radixSort__(Array <tid_t> *input, int index, int begin, int end, 
		 Array <Array <tid_t> *> &buckets, int *marks, TransactionTable *tt){
  //  cout<<"RADIX SORT on "<<begin<<" "<<end<<" item : "<<_item<<" index "<<index<<endl;
  const Array<Transaction> &transactions = *tt->trans;
  item_t currentItem = tt->item; 

  for (int i = begin; i < end; ++i) {
    Array<tid_t> **bucket; 
    tid_t currentTid = (*input)[i];
    if(transactions[currentTid].getSize() <= index){ //if trans is shorter than index
      (*input)[begin++] = currentTid;
    }
    else{
      bucket = &buckets[transactions[currentTid][index]];
      if(*bucket == NULL)
	*bucket = new Array<tid_t>(2);
      (*bucket)->pushBack(currentTid);
    }
  }

  if (begin != end){ /* if there is something in the buckets */ 
    Array<tid_t> *bucket; 
    int windowStart = begin; 
    /* Reorder tids in input according to what is in the buckets */
    for (item_t item=0 ; item <= tt->maxItem ; ++item) { 
      if (bucket = buckets[item]){
	const tid_t *bucketEnd = bucket->pEnd();
	for(tid_t *tid = bucket->pData(); tid != bucketEnd; ++tid){
	  (*input)[begin++] = *tid;
	  //we use the last transaction item as mark to identify up-to-here equal transactions
	  marks[*tid] = item;
	}
	bucket->clear();
      }
    }

    /* Recursive calls on the segments of input with the same mark */
    int refMark = marks[(*input)[windowStart]];
    int windowPos = windowStart; 

    do{
      int mark = marks[(*input)[windowPos]];
      if(refMark == mark) 
	continue; 
      if ( (windowPos - windowStart) > 1)
	radixSort__(input, index+1, windowStart, windowPos, buckets, marks, tt);
      windowStart = windowPos; 
      refMark = mark;
    }while (++windowPos < end); 
  
    if ( (windowPos - windowStart) > 1)
#ifdef DB_REDUCTION_ENABLE_SUFFIX_INTERSECTION
      if(refMark != currentItem){
	//    cout<<"RADIX "<<windowStart<<" "<<windowPos<<endl;
	radixSort__(input, index+1, windowStart, windowPos, buckets, marks, tt);
      }
      else{
	//	cout<<"SUFFIX "<<windowStart<<" "<<windowPos<<endl;
	suffixIntersection(tt, input, index+1, windowStart, windowPos);
      }
#else
    radixSort__(input, index+1, windowStart, windowPos, buckets, marks, tt);

#endif
  }
}
/*** END radixSort__ ***/



/*** BEG quickSortTids ***/
void quickSortTids( const Array<Transaction> &transactions, Array<tid_t> *tids, 
		    int begin, int end, TransactionTable *tt){  

  //  cout<<"ITER "<<begin<<" "<<end<<endl;
  //  getchar();
  if( (end - begin) <= 1)
    return; 

  int siIdx=-1; 
  //  Array<item_t> siTids; 

  /* Select the middle as a pivot */
  //  int pivotIdx = (end+begin)/2; 
  //  const Transaction &pivot = transactions[(*tids)[pivotIdx]];
  int pivotIdx = end-1; 
  const Transaction &pivot = transactions[(*tids)[pivotIdx]];
  //  cout<<pivotIdx<<" "<<pivot<<endl;
  int storeIdx = begin; 
  for (int i = begin; i < end-1; ++i) {
    const tid_t currentTid = (*tids)[i];
    //    cout<<"COMPARE "<<endl<<pivot<<endl<<"AND"<<endl<<transactions[currentTid]<<endl<<"RETURNED"<<pivot.lexicalGt(transactions[currentTid])<<endl;
    int cmp; 
    if((cmp = pivot.lexicalGt(transactions[currentTid], tt->item)) == 1){
      /* swap the tids */
      tid_t tmp = (*tids)[storeIdx];
      (*tids)[storeIdx++] = currentTid; 
      (*tids)[i] = tmp; 
    }
    if(cmp > 9){
      siIdx = cmp - 10; 
      //      cout<<transactions[currentTid]<<endl<<"AND"<<pivot<<endl<<"are prefix equal up to "<<tt->item<<" index "<<siIdx<<endl;
      //      siTids.pushBack(currentTid);
    }
  }
  tid_t tmp = (*tids)[storeIdx];
  (*tids)[storeIdx] = (*tids)[pivotIdx];
  (*tids)[pivotIdx] = tmp; 

  quickSortTids(transactions, tids, begin, storeIdx,tt);   

  if(siIdx!= -1){
    //    siTids.pushBack((*tids)[storeIdx]);
    /* Partition of second window */
    int storeIdx2 = storeIdx+1; 
    for(int i = storeIdx+1; i < end; i++){
      tid_t currentTid = (*tids)[i]; 
      if(pivot.lexicalGt(transactions[currentTid], tt->item) > 1){
	/* swap the tids */
	tid_t tmp = (*tids)[storeIdx2];
	(*tids)[storeIdx2++] = currentTid;
	(*tids)[i] = tmp;
      }
    }
    
    //    cout<<tt->item<<COLOR_PINK<<" SI :"<<endl; 
    //    for(int i = 0; i < siTids.size(); i++){
    //      cout<<transactions[siTids[i]]<<endl;
    //    }
    //    cout<<COLOR_BACK<<endl;
    //    suffixIntersection(tt, &siTids, siIdx, 0, siTids.size());
    suffixIntersection(tt, tids, siIdx, storeIdx, storeIdx2);
    storeIdx = storeIdx2-1; 
  }


  //  cout<<"NEXT "<<begin<<" "<<storeIdx<<" "<<end<<endl;; 
  //  cout<<*tids<<endl;
  //  if(storeIdx - begin <= DB_REDUCTION_INSERTIONSORT_THRESHOLD)

    //  else
    //    insertionSortTids(transactions, tids, begin, storeIdx); 
    //if(end - storeIdx+1 <= DB_REDUCTION_INSERTIONSORT_THRESHOLD)
  quickSortTids(transactions, tids, storeIdx+1, end,tt); 
    //  else
    //    insertionSortTids(transactions, tids, storeIdx+1, end); 
}
/*** END quickSortTids ***/

// /*** BEG quickSortTids ***/
// void quickSortTids( const Array<Transaction> &transactions, Array<tid_t> *tids, 
// 		    int begin, int end, TransactionTable *tt){  

//   //  cout<<"ITER "<<begin<<" "<<end<<endl;
//   //  getchar();
//   if( (end - begin) <= 1)
//     return; 

//   int siIdx=-1; 
//   Array<item_t> siTids; 

//   /* Select the middle as a pivot */
//   //  int pivotIdx = (end+begin)/2; 
//   //  const Transaction &pivot = transactions[(*tids)[pivotIdx]];
//   int pivotIdx = end-1; 
//   const Transaction &pivot = transactions[(*tids)[pivotIdx]];
//   //  cout<<pivotIdx<<" "<<pivot<<endl;
//   int storeIdx = begin; 
//   for (int i = begin; i < end-1; ++i) {
//     const tid_t currentTid = (*tids)[i];
//     //    cout<<"COMPARE "<<endl<<pivot<<endl<<"AND"<<endl<<transactions[currentTid]<<endl<<"RETURNED"<<pivot.lexicalGt(transactions[currentTid])<<endl;
//     int cmp; 
//     if((cmp = pivot.lexicalGt(transactions[currentTid], tt->item)) == 1){
//       /* swap the tids */
//       tid_t tmp = (*tids)[storeIdx];
//       (*tids)[storeIdx++] = currentTid; 
//       (*tids)[i] = tmp; 
//     }
//     if(cmp > 9){
//       siIdx = cmp - 10; 
//       //      cout<<transactions[currentTid]<<endl<<"AND"<<pivot<<endl<<"are prefix equal up to "<<tt->item<<" index "<<siIdx<<endl;
//       siTids.pushBack(currentTid);
//     }
//   }
//   tid_t tmp = (*tids)[storeIdx];
//   (*tids)[storeIdx] = (*tids)[pivotIdx];
//   (*tids)[pivotIdx] = tmp; 
  

//   if(siIdx!= -1){
//     siTids.pushBack((*tids)[storeIdx]);

    
//     //    cout<<tt->item<<COLOR_PINK<<" SI :"<<endl; 
//     //    for(int i = 0; i < siTids.size(); i++){
//     //      cout<<transactions[siTids[i]]<<endl;
//     //    }
//     //    cout<<COLOR_BACK<<endl;
//     suffixIntersection(tt, &siTids, siIdx, 0, siTids.size());
//   }


//   //  cout<<"NEXT "<<begin<<" "<<storeIdx<<" "<<end<<endl;; 
//   //  cout<<*tids<<endl;
//   //  if(storeIdx - begin <= DB_REDUCTION_INSERTIONSORT_THRESHOLD)
//   quickSortTids(transactions, tids, begin, storeIdx,tt); 
//     //  else
//     //    insertionSortTids(transactions, tids, begin, storeIdx); 
//     //if(end - storeIdx+1 <= DB_REDUCTION_INSERTIONSORT_THRESHOLD)
//   quickSortTids(transactions, tids, storeIdx+1, end,tt); 
//     //  else
//     //    insertionSortTids(transactions, tids, storeIdx+1, end); 
// }
// /*** END quickSortTids ***/

void insertionSortTids( const Array<Transaction> &transactions,
			Array<tid_t> *tids, int begin, int end){  
  int j; 
  int value;

  for (int i = begin; i < end-1; ++i) {
    {
    
      value = (*tids)[i]; 
      for (j = i; j > begin && transactions[(*tids)[j - 1]].lexicalGt(transactions[value],0); --j)
	{
	  (*tids)[j] = (*tids)[j - 1];
	}
      (*tids)[j] = value;
    }
  }
}

/*** BEG suffixIntersection ***/
void suffixIntersection(TransactionTable *tt, 
			Array<tid_t> *input,
			int index, int begin, int end){
  
  if((end - begin)  <= 1)
    return; 

  Array<Transaction> *transactions = tt->trans;
  Weights *weights = tt->weights;
  item_t item = tt->item; 
  //TODO : do not proceed to anything if one transaction is <= index (shold to a first pass) */

  tid_t refTransIndex = (*input)[begin];
  Transaction *refTrans = &(*transactions)[refTransIndex]; 
  Array<int> buckets(1); 
  bool flag = false; 
  buckets.expandInit(tt->maxItem+1, 0); 
  for(int i = begin; i < end; ++i){
    tid_t currentTid = (*input)[i];
    //    weight+=(*transactions)[currentTid].weight; 
    if((*transactions)[currentTid].getSize() <= index){ //intersection is empty !
      flag = true;
      break;
    }    
    else{
      //TODO if one item is not present we can remove it (decrease MAXITEM ?)
      item_t *x = (*transactions)[currentTid].pData(); 
      const item_t *tEnd = (*transactions)[currentTid].pEnd(); 
      for(; x < tEnd; ++x)
	buckets[*x]++;        
    }
  }

  /* Troncate the current refTrans at item */
  refTrans->resize(index); 
  
  /* Add the intersection if non-empty */
  if(!flag)
    for(int i = item+1 ; i <= tt->maxItem; ++i){
      if(buckets[i] == (end-begin)) //if item is present in all transactions, 
	(*refTrans).pushBack(i);    //it belongs to the intersection
    }
  
  /* Clears others transactions */ 
  weight_t refTransWeight = 0; 
  for(int i = begin+1; i < end; ++i){
    refTransWeight += (*weights)[(*input)[i]];
    (*transactions)[(*input)[i]].clear();
  }
  (*weights)[refTransIndex] += refTransWeight;
}
/*** END suffixIntersection ***/

/*** removeEmptyTransactions ***/
void removeEmptyTransactions(TransactionTable *tt){
  Transaction *current = tt->trans->pData(); 
  weight_t *currentWeight = tt->weights->pData(); 
 

  Transaction *ref = current; 
  weight_t *refWeight = currentWeight; 
  const Transaction *tEnd = tt->trans->pEnd(); 
  int newSize = 0; 
  for(; current < tEnd; ++current, ++currentWeight){
    if(current->size() != 0){
      if(current != ref){
	current->leaveData(ref);
	*refWeight = *currentWeight; 
      }
      newSize++; ref++; refWeight++; 
    }
  }
  tt->trans->resize(newSize); 
  tt->weights->resize(newSize); 
}
/*** END removeEmptyTransactions ***/

/*** deleteTransactionTable ***/
void deleteTransactionTable(TransactionTable *tt){
  assert(tt->trans->size() > 0); 
  free(tt->data);
  delete tt->trans; 
  delete tt->weights; 
}
/*** END deleteTransactionTable ***/


/*** printTransactionTable ***/
void printTransactionTable(const TransactionTable &tt){  
  cout<<COLOR_GREEN; 
  for(int i = 0; i < tt.trans->size(); i++){
    cout<<i<<":("<<(*tt.weights)[i]<<"):"<<(*tt.trans)[i]<<endl; 
  }
  cout<<COLOR_BACK<<endl;
}
/*** END printTransactionTable ***/
