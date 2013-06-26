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

#ifndef   	_OCCURENCES_HPP_
# define   	_OCCURENCES_HPP_

#include "defines.hpp" 

/** 
 * \brief Compute the occurences for a given transactions.
 *
 * Given the \TransactionTable \tt occurenceDeliverAll will store into
 * \ot->occs[item] all the tids in tt where item apears.  
 *
 * \warning for performences issues \ot.trans[i] must exist and be
 * empty for each i < maxItem.
 * 
 * @param tt 
 * @param ot 
 * @param maxItem 
 */
void occurenceDeliverAll(const TransactionTable &tt, OccurencesTable *ot, item_t maxItem);



/** 
 * \brief Count frequency of items. 
 *
 * Given a TransactionTable and a list of tids, count the frequency of
 * all items up to maxItem, 
 *
 * After the call itemsFrequency[i] contains
 * frequency of i in transactions \tids.
 * 
 * @param tids Tids of the transactions to concider for frequency counting.
 * @param itemsFrequency Array to be filled with item frequency.
 * 
 * @return The size of the database (sum of sizes of all transactions). 
 */
int frequencyCount(const TransactionTable &tt, const Occurences &tids,
		   Frequencies *itemsFrequency, Array<item_t> *presentItems);


/** 
 * Same as \frequencyCount, but stores the size for each occurences in
 * workspace. To be used when no previous occurencesTable has been
 * allocated.
 * 
 * @param tt 
 * @param tids 
 * @param itemsFrequency 
 * @param presentItems 
 * @param workspace 
 * 
 * @return 
 */
int frequencyCountAndSize(const TransactionTable &tt, const Occurences &tids,
		       Frequencies *itemsFrequency, Array<item_t> *presentItems, 
		       Array<int> *workspace); 


/** 
 * \brief Compute the sizes of the next occurences table.
 * 
 * @param tt Previous TransactionTable.
 * @param tids Tids of the transactions to concider for the next OccurenceTable. 
 * @param workspace Input Array to be filled with the size of each item. 
 */
void computeOccurencesSizes(const TransactionTable &tt, const Occurences &tids,
			    Array<int> *workspace);


void createOccurencesTable(OccurencesTable *ot, int dbSize, const Array<int>* workspace);

/** 
 * \brief Free the memory allocated for an \OccurenceTable.
 * 
 * @param ot The \OccurenceTable to be freed;
 */
void deleteOccurencesTable(OccurencesTable *ot);

/** 
 * \brief Print the content of \ot.
 * 
 * @param ot
 */
void printOccurencesTable(const OccurencesTable &ot); 

#endif	    /* _OCCURENCES_HPP_ */
