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

#include "Occurences.hpp"


/*** occurenceDeliverAll ***/
void occurenceDeliverAll(const TransactionTable &tt, OccurencesTable *ot, item_t maxItem){
  assert(maxItem>=0); 

  Array<Occurences> *occurences = ot->occs;
  const Array<Transaction> &transactions = *tt.trans;

#ifndef NDEBUG
  assert(occurences->size() >= maxItem); 
  for(int i = 0 ; i <= maxItem; i++){
    assert(!(*occurences)[i].size()); 
    //cout<<i<<" : "<<(void*)&(*workspace)[i]<<endl;
  }
#endif
  
  assert(occurences->size() > maxItem);
  for(tid_t tid = 0; tid < transactions.getSize();++tid){
    const item_t *transEnd = transactions[tid].pEnd(); 
    for(const item_t *x = transactions[tid].pData(); x < transEnd && *x <= maxItem; ++x){
      (*occurences)[*x].pushBack(tid);
    }
  }  
}
/*** END occurenceDeliverAll ***/


/*** frequencyCount ***/
int frequencyCount(const TransactionTable &tt, const Occurences &tids,
		   Frequencies *itemsFrequency, Array<item_t> *presentItems){  
  int dbSize = 0;
  Array<Transaction> *transactions = tt.trans; 
  Array<weight_t> *weights = tt.weights; 

  //  freq_t *pItemsFrequency = itemsFrequency->pData(); 

#ifndef NDEBUG
  for (int i = 0; i < tt.maxItem; i++){
    assert((*itemsFrequency)[i] == 0);
  }
#endif 

  const tid_t *tidsEnd = tids.pEnd();

  for(const tid_t *tid = tids.pData(); tid < tidsEnd ;++tid){    
    weight_t weight = (*weights)[*tid];
    const item_t *transEnd =  (*transactions)[*tid].pEnd(); 
    for(item_t *x = (*transactions)[*tid].pData(); x < transEnd; ++x){
      ++dbSize; 
      if( (*itemsFrequency)[*x] == 0)
	presentItems->pushBack(*x);      
      (*itemsFrequency)[*x] += weight;
      //pItemsFrequency[*x]+=weight; 
    }
  }
  return dbSize;
}
/*** END frequencyCount ***/


/*** frequencyCountAndSize ***/
int frequencyCountAndSize(const TransactionTable &tt, const Occurences &tids,
		       Frequencies *itemsFrequency, Array<item_t> *presentItems, 
		       Array<int> *workspace){  
  int dbSize = 0;
  Array<Transaction> *transactions = tt.trans; 
  Array<weight_t> *weights = tt.weights; 
  workspace->expandInit(tt.maxItem+1, 0); 

  //  freq_t *pItemsFrequency = itemsFrequency->pData(); 

  itemsFrequency->expandInit(tt.maxItem+1, 0); 

  const tid_t *tidsEnd = tids.pEnd();

  for(const tid_t *tid = tids.pData(); tid < tidsEnd ;++tid){    
    weight_t weight = (*weights)[*tid];
    const item_t *transEnd =  (*transactions)[*tid].pEnd(); 
    for(item_t *x = (*transactions)[*tid].pData(); x < transEnd; ++x){
      ++dbSize; 
      if( (*itemsFrequency)[*x] == 0)
	presentItems->pushBack(*x);      
      (*itemsFrequency)[*x] += weight;
      //TODO Double the number of caches misses maybe keep this info in itemfrequency
      (*workspace)[*x] += 1;

      //pItemsFrequency[*x]+=weight; 
    }
  }
  return dbSize;
}
/*** END frequencyCount ***/

/*** BEG computeOccurencesSizes ***/
void computeOccurencesSizes(const TransactionTable &tt, const Occurences &tids,
			    Array<int> *workspace){  
  Array<Transaction> *transactions = tt.trans; 
  Array<weight_t> *weights = tt.weights; 
  workspace->expandInit(tt.maxItem+1, 0); 

  //  freq_t *pItemsFrequency = itemsFrequency->pData(); 

  const tid_t *tidsEnd = tids.pEnd();

  for(const tid_t *tid = tids.pData(); tid < tidsEnd ;++tid){    
    weight_t weight = (*weights)[*tid];
    const item_t *transEnd =  (*transactions)[*tid].pEnd(); 
    for(item_t *x = (*transactions)[*tid].pData(); x < transEnd; ++x){
      (*workspace)[*x] += 1;
    }
  }  
}
/*** END computeOccurencesSizes ***/

/*** createOccurencesTable ***/
void createOccurencesTable(OccurencesTable *ot, int dbSize, const Array<int>* workspace){
  tid_t *occurencesData = (tid_t*)malloc(dbSize * sizeof(tid_t)); 
  item_t nbItems = workspace->size();
  assert(occurencesData); 

  int odOffset = 0; 

  ot->occs=new Array<Occurences>(nbItems);  
  ot->occs->expand(nbItems);
  assert(ot->occs->size() <= workspace->size()); 

  Occurences *occ = ot->occs->pData(); 
  const Occurences *occsEnd = ot->occs->pEnd();
  const int *occSize = workspace->pData(); 

  /* Attribute a piece of the memory bloc pointed by occurencesData to
     each occurences.*/
  for(; occ < occsEnd; ++occ){
#ifndef NDEBUG
    new (occ) Occurences(occurencesData+odOffset, dbSize-odOffset); 
#else     
    new (occ) Occurences(occurencesData+odOffset, 0); 
#endif
    odOffset+=*occSize;
    occSize++;
  }
}
/*** END createOccurencesTable ***/


/*** deleteOccurencesTable ***/
void deleteOccurencesTable(OccurencesTable *ot){
  /* This point to the begining of the allocated blocks */
  free((*ot->occs)[0].pData()); 
  delete ot->occs; 
}
/*** END deleteOccurencesTable ***/


/*** printOccurencesTable ***/
void printOccurencesTable(const OccurencesTable &ot){
  cout<<COLOR_RED; 
  for(int i = 0; i < ot.occs->size(); i++){
    cout<<i<<":("<</*(*ot.weights)[i]<<*/"):"<<(*ot.occs)[i]<<endl; 
  }
  cout<<COLOR_BACK<<endl;
}
/*** END printOccurencesTable ***/
