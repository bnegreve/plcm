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

#include "Permutations.hpp" 

#include <vector>

static  int compare2int(const void *a, const void *b){
  return (*(int *)b) - (*(int *)a); 
}

/*** BEG sortItems ***/
void sortItems(item_t item, const Frequencies &frequencies, Array<int> *permutations, int th){
  assert(permutations->size() == 0); 
  permutations->expand(frequencies.size()); 

  size_t permutationsSize = 2*frequencies.size();
  item_t *tmp =     
    (item_t*)malloc(permutationsSize*sizeof(item_t)); 
  
  int i,j;

  item_t *current = tmp; 
  for(i = 0; i < frequencies.size(); i++){
    *current++ = frequencies[i]; 
    *current++ = i; 
  }
  
  qsort(tmp, i, sizeof(int)*2, compare2int); 
  
  /* rename items < item && > th as lower items */
  for(i = 0, j = 0; i < frequencies.size()  && tmp[2*i] >= th; i++){
    int value = tmp[2*i+1]; 
    if(value < item){
      (*permutations)[value] = j++;       
    }
  }

  /* rename item  */
  (*permutations)[item] = j++;

  /* rename items > item && > th as higher items */
  for(i = 0; i < frequencies.size() && tmp[2*i] >= th; i++){
    int value = tmp[2*i+1]; 
    if(value > item)
      (*permutations)[value] = j++; 
  }

  /* rename unfrequent items higher than higher items */
  for(i = 0; i < frequencies.size(); i++){
    int value = tmp[2*i+1]; 
    if(tmp[2*i] < th && value != item)
      (*permutations)[value] = j++; 
  }

  //  cout<<"PERMUTATIONS "<<item<<" "<<*permutations<<endl;
  //  exit(1); 

}
/*** END sortItems ***/

/*** BEG permuteValues ***/ 
void permuteValues(Array<int> *array, const Array<int> &permutations){
  int *pArray = array->pData(); 
  const int *end = array->pEnd();
  const int *pPermutations = permutations.pData(); 

  for(int *current = pArray; current < end; ++current){
    assert(*pArray < permutations.size());
    *pArray = pPermutations[*pArray++];
  }
}
/*** END permuteValues ***/

/*** BEG permuteIndexes ***/
void permuteIndexes(Array<int> *array, const Array<int> &permutations){
  //  Array<int> save(*array); 
  std::vector<bool> done(array->size(), false);
  int *pArray = array->pData(); 

  for(int i = 0; i < array->size(); i++){
    if(done[i]) continue; 
    int nextValue, value = pArray[i]; 
    for(int j = permutations[i]; !done[j]; value = nextValue){
      nextValue = (*array)[j]; 
      (*array)[j] = value; 
      done[j] = true; 
      j = permutations[j]; 
    }
  }
//   for(int i = 0; i < save.size(); i++){
//     if((*array)[permutations[i]] != save[i]){      
//       assert(false); 
//     }
}
/*** END permuteIndexes ***/

/*** BEG mergePermutations ***/
void mergePermutations(Array<int> *newPerms, const Array<int> &oldPerms){
  //TODO use a pointer based loop 
  for(int i = 0; i < newPerms->size(); i++){
    (*newPerms)[i] = oldPerms[(*newPerms)[i]];
  }
}
/*** END mergePermutations ***/


/*** BEG invertPermutations ***/
void invertPermutations(Array<int> *permutations){
  //  Array<int> save(*array); 
  std::vector<bool> done(permutations->size(), false);  
  int size = permutations->size(); 
  int next, prev, j; 
  for(int i = 0; i < size; i++){
    if(done[i]) continue;     
    j = (*permutations)[i];
    prev = i; 
    while(!done[j]){
      next = (*permutations)[j]; 
      (*permutations)[j] = prev;
      done[j] = true; 
      prev = j; 
      j = next;
    }
  }
//   for(int i = 0; i < save.size(); i++){
//     if((*array)[permutations[i]] != save[i]){      
//       assert(false); 
//     }
}
/*** END invertPermutations ***/
