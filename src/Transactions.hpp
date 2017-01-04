/*
** PLCM/Melinda
** Copyright 2010-2013, 2016-2017, Benjamin Negrevergne, Grenoble University
** Original LCM algorithm from Takaki Uno and Hiroki Arimura.
**
** The program was design and implemented in colaboration with
** Alexandre Termier and Jean-François Méhaut.
** 
** See the README file for installation, usage, and details.
** See the LICENSE file for licensing.
*/

#ifndef   	_TRANSACTIONS_HPP_
#define   	_TRANSACTIONS_HPP_

#include "defines.hpp"



/** 
 * Load the transactions from \filename to \transactionTable.
 
 * the items are
 * reordered by frequency. 0 is the most frequent item. 
 * 
 * @param fileName file where to read the transactions. 
 * @param transactionTable 2D Array which will contain the transactions. 
 * @param maxItem maximum item number. 
 * @param threshold minimum support. 
 *
 * @return maximum frequent Item. 
 */
int loadTransactionsFromFile(char *fileName, TransactionTable *transactionTable, 
			     int threshold, Array<item_t> *permutations, int *dbSize);


/** 
 * \brief Count the support of each item in the file @filename.
 *
 * An Array<int> is return. Support value for item i is in Array[i].
 * 
 * @param fileName Transaction file.
 * @param numTransactions After the call, numTransactions will be set
 * to the number of transactions.
 * 
 * @return An array containing support value for each item. 
 */
Array<int> countItemsSupport(char *fileName, int *nbTransactions);




/** 
 * \brief Reorder frequent items by frequency. 
 * 
 * The most frequent item will be 0 etc.  Permutations are stored into
 * the \permutations array so the original items can be restored at
 * the end of the execution
 *
 * @param itemSupport Array containing support of all items (itemSupport[i] = support(i)). 
 * @param permutations After the call this will contain correspondancies between items
 * name before and after permutation.
 * @param threshold Minimum support. 
 * @param dbSize After the call, will contains an upperbound to the database size (sum of the size of all transactions).
 * @return number of frequent items. 
 */
int reorderFrequentItems ( Array<int> *itemSupport, Array<item_t> *permutations, 
			   int threshold, int *dbSize);

/** 
 * 
 * \brief Creates a new \TransactionTable from another one. 
 *
 * The newly created TransactionTable \newTT is necessarily embeded in the
 * parent one.  It means that the new TransactionTable will contain at
 * most all the items of its parent.
 *
 * The new database is contructed concidering only the transactions in \itemOccs.
 *
 * If a transaction is not including any items smaller than
 * \maxCandidate, it will not be included.
 * 
 * 
 * @param parentTT The parent \TransactionTable. 
 * @param item The item for which \newTT is build for.
 * @param itemOccs Tids of the transactions in \parentTT look into.
 * @param frequencies Frequencies of each items in \parentTT (should have been computed with \frequencyCount())
 * @param maxCandidate Maximum candidate item. Transaction containing only items greater than this won't be included in \newTT.
 * @param dbSize Upper boud for the total number of items in \newTT
 * @param newTT Pointer to the \TransactionTable structure to be filled.
 * @param merge Enable merging of identical transactions. May be set to false if the \newTT is small. 
 */
void createTransactionTable(const TransactionTable &parentTT, item_t item, 
			    const Occurences &itemOccs, 
			    const Array<freq_t> &frequencies,
			    item_t maxCandidate, unsigned int dbSize,
			    TransactionTable *newTT, bool merge);


void createTransactionTablePermuteItems(const TransactionTable &parentTT, 
					item_t item, const Occurences &itemOccs,
					const Array<freq_t> &frequencies,
					item_t maxCandidate, unsigned int dbSize,
					TransactionTable *newTT, bool merge, 
					const Array<int> &permutations);



/** 
 * \brief Merge the transactions that are identical in \TransactionTable \tt. 
 * 
 * @param tt The \TransactionTable to be process.
 */
void mergeIdenticalTransactions(TransactionTable *tt, bool forceRadix = false); 

void sortTids(Array<tid_t> *tids, TransactionTable *tt, bool forceRadix = false);

void radixSort__(Array <tid_t> *input, int index, int begin, int end, 
		 Array <Array <tid_t> *> &buckets, int *marks,
		 TransactionTable *tt);


void suffixIntersection(TransactionTable *tt, 
			Array<tid_t> *input,
			int index, int begin, int end);

void removeEmptyTransactions(TransactionTable *tt);

/** 
 * \brief free memory allocated for a TransactionTable. 
 * 
 * @param tt The TransactionTable object to be freed. 
 */
void deleteTransactionTable(TransactionTable *tt); 

/** 
 * \brief Print the content of \tt.
 * 
 * @param tt 
 */
void printTransactionTable(const TransactionTable &tt);


void quickSortTids( const Array<Transaction> &transactions, Array<tid_t> *tids, 
		    int begin, int end, TransactionTable *tt); 

void insertionSortTids( const Array<Transaction> &transactions,
			Array<tid_t> *tids, int begin, int end); 
#endif	    /* _TRANSACTIONS_HPP_ */
