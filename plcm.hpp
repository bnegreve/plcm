//
// plcm.hpp
// 
// Made by Benjamin Negrevergne
// Login   <bengreve@confiance.imag.fr>
// 
// Started on  Thu Jul  9 15:05:25 2009 Benjamin Negrevergne
// Last update Thu Jul  9 15:05:25 2009 Benjamin Negrevergne
//

#ifndef   	_PLCM_HPP_
#define   	_PLCM_HPP_

#include "defines.hpp"
#include "Transactions.hpp"
#include "Occurences.hpp"



/** 
 * Removes infrequent items in cdb from \candidates. Additionaly
 * insert into \itemset the items that are 100% frequents (ie. belongs
 * to the closure of the current itemset).  Returns the maximum value
 * of the clo(itemset) (usefull for prunning redundent branches
 * 
 * @param frequencies frquencies of the items. 
 * @param itemset The current itemset
 * @param item current item. 
 * @param candidates Array fills with items that appears within the cdb
 * with the current itemset. 
 * @param threshold min sup. 
 * @param freq Frequency of the current itemset.
 * @return max (clo(itemset)). 
 */
item_t removeInfrequentItems(Frequencies *frequencies, Itemset *itemset, 
			     item_t item, Array<item_t> *candidates, 
			     int threshold, int freq, 
			     const Array<int> &permutations);


void lcmIter(const TransactionTable &tt, OccurencesTable *ot, 
	     Frequencies *frequencies, Itemset *itemset, item_t item,
	     int threshold, item_t previous,
	     bool rebase, int depth);

void dumpItemset(const Itemset &itemset, freq_t freq);

int main(int argc, char **argv); 

#ifdef PARALLEL_PROCESS
void processTupleThread(int id);

#endif //PARALLEL_PROCESS


#endif	    /* _PLCM_HPP_ */
