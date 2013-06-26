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

extern "C" {
#include <melinda.h>
#include <thread.h>
}

#include "Transactions.hpp"
#include "Occurences.hpp"
typedef struct {
  TransactionTable *tt; 
  OccurencesTable *ot; 
  Frequencies *frequencies; 
  Itemset *itemset; 
  item_t item; 
  int threshold; 
  int previous; 
}tuple_t ; 


int m_distribute(opaque_tuple_t *tuple){
  return m_thread_id(); 
}

int m_retrieve(){
  return m_thread_id(); 
}
