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

#ifndef   	_PERMUTATIONS_HPP_
#define   	_PERMUTATIONS_HPP_

#include "defines.hpp"


/** 
 * \brief Compute the permutations for rebasing items of a
 * \TransactionTable.
 *
 * Compute the permutations to sort the items from their frequencies
 * without mixing frequent items lowers than \item and frequent items
 * greater than items.
 *
 * @param item Split value to determine wether an item is a hight or a low item.
 * @param frequencies Frequencies of each items. 
 * @param permutations Empty permutation array to be filled with the permutations. 
 * @param th Frequency threshold.
 */
void sortItems(item_t item, const Frequencies &frequencies, 
	       Array<int> *permutations, int th);


/** 
 * \brief Apply permutations to all the values inside an array. 
 * 
 * Given the \permutations array, this will update \array
 * so that array[i] becomes permutations[array[i]].
 *
 * @param array Array to permute. 
 * @param permutations The permutations.
 */
void permuteValues(Array<int> *array, const Array<int> &permutations); 


/** 
 * \brief Permute all the values according a permutaions array.
 * 
 * Given the \permutations array, this function will move the value
 * at array[i] to array[permutations[i]]. 
 *
 * @param array Array to permute. 
 * @param permutations The permutations (permutations[old] = new). 
 */
void permuteIndexes(Array<int> *array, const Array<int> &permutations); 


/** 
 * \brief Merge two permutation arrays to keep tracks of the original
 * items name.
 * 
 * Merge \oldPerms into \newPerms. If \oldPerms and \newPerms are both
 * backward permutations, after the call, we have :
 *
 * T[newPemrs[oldPerms[new]] = old.
 *
 * @param newPerms Permutations array to merge into.
 * @param oldPerms Other permutatations. 
 * 
 */
void mergePermutations(Array<int> *newPerms, const Array<int> &oldPerms);

/** 
 * \brief In-place invert a permutation array.
 *
 * Transform \permutations so if permutations[old] = new before the
 * call, then \permutations[new] = old after the call.
 *
 * It is permuting \permutations with itself. 
 *
 * @param permutations 
 */
void invertPermutations(Array<int> *permutations);

#endif	    /* _PERMUTATIONS_HPP_ */
