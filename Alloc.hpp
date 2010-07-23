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

#ifndef   	_ALLOC_HPP_
#define   	_ALLOC_HPP_

#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <iostream>


/* help the branch predixtor of GCC */
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)



template <class T>
class MallocAlloc {
private: 
  static const int EXPAND_FACTOR = 3;

public :

  inline T *alloc( unsigned int size){
    _memSize = size;
    T *p = (T*)malloc(_memSize*sizeof(T));
    assert(p);
    return p; 
  }

  inline T *expand(T *p, unsigned int size){
    if(unlikely(size > _memSize)){
      _memSize = std::max(_memSize*EXPAND_FACTOR, size);
      //#define COUNT_REALLOCS
#ifndef COUNT_REALLOCS
      p = (T*)realloc(p, _memSize*sizeof(T));
#else      
      T *q = (T*)realloc(p, _memSize*sizeof(T));
      if(q != p)
	cout<<"REALLOC"<<endl; 
      p=q;
#endif      
      assert(p); 
    }
    return p; 
  }

  inline void free(T *p){
    std::free(p);
    _memSize = 0; 
  }

  inline unsigned int memSize(){
    return _memSize; 
  }

private: 
  unsigned int _memSize; 
};


template <class T>
class NullAlloc {
public :


#ifdef NDEBUG
  NullAlloc(T *p, unsigned int size){
  }
#else
  NullAlloc(T *p, unsigned int size):_p(p), _memSize(size){
  }
#endif 


  inline T *alloc(unsigned int size){
#ifndef NDEBUG //if debug mode 
    assert(size<=_memSize); 
    return _p; 
#else
    cerr<<__FILE__<<":"<<__LINE__<<" call to NullAlloc::alloc() invalid in NDEBUG mode."<<endl;
    abort(); 
#endif
  }

  inline T *expand(T *p, unsigned int size){
#ifndef NDEBUG
    assert(size<=_memSize); 
#endif 
    return p;
  }

  inline void free(T *p){
#ifndef NDEBUG
    _memSize = 0; 
#endif
  }

  inline unsigned int memSize(){
#ifndef NDEBUG
    return _memSize; 
#else
    cerr<<__FILE__<<":"<<__LINE__<<" call to memSize invalid in NDEBUG mode."<<endl;
    abort(); 
#endif
  }
private: 
#ifndef NDEBUG //if in debug mode
  T *_p; 
  unsigned int _memSize;
#endif
};

#endif	    /* _ALLOC_HPP_ */
