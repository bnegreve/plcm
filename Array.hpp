//
// Array.hpp
// 
// Made by Benjamin Negrevergne
// Login   <bengreve@confiance.imag.fr>
// 
// Started on  Thu Mar  5 17:40:30 2009 Benjamin Negrevergne
// Last update Mon Mar 16 16:15:33 2009 Benjamin Negrevergne
//

#ifndef   	_ARRAY_HPP_
#define   	_ARRAY_HPP_

#include <vector>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <memory> 
#include <functional> //std::greater

#include "Alloc.hpp"



/* Call destructors in array for all unknowned types */

template<class T>
struct useConstructors {
    static const bool VALUE = true ;
};

template<>
struct useConstructors<unsigned int> {
    static const bool VALUE = false ;
};

template<>
struct useConstructors<int> {
    static const bool VALUE = false ;
};

template<>
struct useConstructors<short int> {
    static const bool VALUE = false ;
};


template<>
struct useConstructors<unsigned short int> {
    static const bool VALUE = false ;
};


template <class T, bool USE_CTORS=useConstructors<T>::VALUE, class ALLOCATOR = MallocAlloc<T> >
class Array{

private:
  ALLOCATOR _allocator; 
  T *_data; 
  unsigned int _size; 
  
public: 

  inline const T *pEnd() const{
    return _data+_size; 
  }
  
  inline const unsigned int size() const{
    return _size; 
  }

  inline const unsigned int getSize() const{
    return _size; 
  }
  
  inline const T *pData() const{
    return _data; 
  }

  inline T *pData(){
    return _data;
  }

  /** 
   * Constructor, creates a vector of size size. 
   * 
   * @param size Initial size. 
   */
  Array(unsigned int memSize = 0):
    _allocator(ALLOCATOR()){
    if(memSize)
      _data = _allocator.alloc(memSize); 
    else
      _data = NULL; //so the realloc does not says invalid point
    _size = 0; 
  }

  Array(T *pData, unsigned int memSize):
#ifndef NDEBUG // if debug mode
    _allocator(ALLOCATOR(pData, memSize)){
    _data = _allocator.alloc(memSize);
    _size = 0;
#else
    _allocator(ALLOCATOR(0, 0)),
      _data(pData),
      _size(0){ 
#endif
      }

  /** 
   * Destructor
   */
  ~Array(){
    //Call the destructor on every items, if needed
    if(USE_CTORS){
      for(T *p= pData(); p < pEnd(); ++p){
	p->~T();
      }
    }
    _size = 0; 
    _allocator.free(_data);
    //    _data=0; 
  }


private: 
  void clone(const Array<T, USE_CTORS, ALLOCATOR> &copy){
    if(USE_CTORS){
      T *p = _data;
      const T *pCopyEnd = copy.pEnd(); 
      for(T *pCopy = copy._data; pCopy < pCopyEnd ; ++pCopy, ++p){	
	new (p) T(*pCopy); //call the constructor at given adress
      }
    }//TODO SLOWER WHY ????
    else{
      memcpy(_data, copy._data, copy._size*sizeof(T)); 
    }
    _size = copy._size;
  }
  
public: 
  /** 
   * \brief Copy constructor.
   *
   * Copy the array into a new one. 
   * The content of _data is copied from _data to _data+_end.
   *
   * @param copy 
   */
  Array(const Array<T> &copy):
    _data(_allocator.alloc(copy._size)){
    assert(this != &copy);
    clone(copy); 
  }
  
  inline const Array<T> &operator=(const Array<T> &copy){
    _allocator.free(_data); 
    _data = (_allocator.alloc(copy._size));
    assert(this != &copy);
    clone(copy);     
    return *this; 
  }

  inline void reserve(const unsigned int size){
    _data = _allocator.expand(_data, size);
  }

  /** 
   * \brief Increase the size of the array to \size
   * 
   * \warning If may be \size lower than the current size of the array
   * use \resize instead. 
   * 
   * @param size New size of the array.
   */
  inline void expand (const unsigned int size){
    assert(_size < size);
    _data = _allocator.expand(_data, size);
    _size = size; 
    assert(_data); 
  }

  /** 
   * \brief Same as \expand but fills the new cells with \c constant value.
   *
   * \warning If may be size lower than the current size of the array
   * use \resize instead.
   *
   * @param newSize New size of the array.
   * @param c constant integer value to fill the array.  
   */
  inline void expandConst(unsigned int newSize, int c){
    assert(newSize > _size);
    unsigned int previousSize = _size; 
    expand(newSize);
    assert(false);
    memset(_data+previousSize, c, (size() - previousSize) * sizeof(T));
    }  

  /** 
   * \brief Same as \expand but Initialize new objects from \t object. 
   *
   * Copy contructor will be called on every new cell with \t object. 
   *
   * \warning If may be size lower than the current size of the array
   * use \resizeInit instead.
   *
   * @param newSize New size of the array.
   * @param T t
   */
  inline void expandInit(unsigned int newSize, const T &t){
    assert(newSize > _size);
    unsigned int previousSize = _size; 
    expand(newSize); 
    for(T *p = _data+previousSize; p < pEnd(); ++p){
	new (p) T(t);
      }
  }

/** 
   * \brief Cut the array to \size
   * 
   * 
   * @param size New size of the array.
   */
  inline void shrink (const unsigned int size){
    if(USE_CTORS)
      assert(false);
    assert(_size > size);
    _size = size; 
    assert(_data); 
  }

  /** 
   * Insert \value at the end of array, increases the size of
   * the allocated memory if necessary. 
   * 
   * @param value Element to insert. 
   * @return The index of the added element.
   */
  inline unsigned int pushBack(const T &value = T()){
    expand(_size+1);
    if(USE_CTORS)
      new (_data+_size-1) T(value); //call the copy constructor at given adress
    else
      _data[_size-1]=value;
    return _size - 1; 
  }

  /** 
   * \brief Resize the array to \newSize elements
   * 
   * If \size is less than the actual size of the array, 
   * it will be trucated to \newSize otherwise expand will be called. 
   * 
   * @param newSize New size of the array.
   */
  inline void resize(const unsigned int newSize){
    if(newSize <= _size)
      _size = newSize; 
    else
      expand(newSize);
  }



  /** 
   * \brief Same as \resize but fills the new cells with \c constant value.
   *
   *
   * @param newSize New size of the array.
   * @param c constant integer value to fill the array.  
   */
  inline void resizeConst(const unsigned int newSize, const int c){
    if(newSize > _size){
      unsigned int oldSize = _size; 
      expand(newSize); 
      memset(oldSize, c, _size - oldSize);       
    }
    else{
      _size = newSize; 
    }
  }


  /** 
   * \brief Same as \resize but Initialize new objects with \t object. 
   *
   * @param newSize New size of the array.
   * @param c constant integer value to fill the array.  
   */
  inline void resizeInit(unsigned int newSize, const T &t){
    unsigned int oldSize = _size;
    if(newSize > _size){
      expand(newSize);
      for(T *p = _data+oldSize; p < pEnd(); ++p)
 	new (p) T(t); //Call the constructor
    }
    else{
      _size = newSize; 
      for(T *p = _data+oldSize; p < pEnd(); ++p)
      	p->~T();  //Call the destructor
    }
  }




  inline T &operator[](unsigned int index) {
    //    cout<<"idex"<<index<<" size"<<_size<<endl;
    assert(index < _size);
    return _data[index];
  }


  inline const T &operator[](unsigned int index) const {
    assert(index < _size);
    return _data[index];
  }

  
  inline void sort(){
    std::sort(_data, _data+_size);
  }

  inline void rSort(){
    std::sort(_data, _data+_size, std::greater<T>()); 
  }

  /** 
   * \brief Returns true if parameter \t belongs to the array, false if not. 
   * 
   * @param t Value to be compared. 
   * @param begin Start index.
   *
   * @return true if t is in the array, false if not. 
   */
  inline bool member(const T &t, unsigned int begin = 0) const{
    assert(begin < _size);
    for(const T *tp = _data+begin; tp < pEnd(); ++tp){
      if(*tp == t)
	return true; 
    }
    return false; 
  }


  /** 
   * \brief Assuming the array is sorted, returns true if parameter \t belongs to the array, false if not. 
   * 
   * @param t Value to be compared. 
   * @param begin Start index.
   *
   * @return true if t is in the array, false if not. 
   */
  inline bool memberSorted(const T &t, int begin = 0) const{
    assert(false);
    //NOT IMPLEMENTED
    for(const T *tp = _data+begin; tp != pEnd(); ++tp){
      if(*tp == t)
	return true; 
      if(*tp > t)
	return false; 
    }
    return false; 
  }

  inline const bool isSorted(){
    if(_size <= 1)
      return true; 
    for(int i = 1 ; i < _size; i++)
      if(_data[i-1] > _data[i])
	return false; 
    return true; 
  }

  /** 
   * \brief Copy data in *this into a std vector and returns it.
   * 
   * @return a std::vector<T> containing the data in *this.  
   */
  inline const std::vector<T> toStdVector(){
    std::vector<T> stdVector; 
    stdVector.reserve(size()); 
    for(T *p = _data ; p < pEnd(); ++p){
      stdVector.push_back(*p); 
    }
    return stdVector; 
  }



  /** 
   * \brief Removes all the elements in the array with out freeing any memory. 
   * 
   */
  inline void clear(){
    //TODO WARNING DO NOT CALL DESTRUCTORS
    if(USE_CTORS)
      assert(false); 
    else
      _size = 0 ; 
  }

  inline void swap(unsigned int i, unsigned int j){
    T tmp(_data[j]); 
    _data[j] = _data[i];
    _data[i] = tmp;
  }

  bool operator==(const Array<T, USE_CTORS, ALLOCATOR> &other) const{
    if(other._size != _size)
      return false; 
    for(unsigned int i = 0; i < _size; i++)
      if(unlikely(_data[i] != other._data[i]))
	return false;
    return true;
  }


  /**
   * \brief Lexical greater than operator 
   * \return 0, 1 or -1, 1 if \this is greater than \other concidering the lexical order.
   */
  inline int lexicalGt( const Array<T, USE_CTORS, ALLOCATOR> &other, const T &prefixBound) const{
    const T *tp = _data, *tpEnd = pEnd();
    const T *op = other._data, *opEnd = other.pEnd();
    int i = 0; 
    for(; tp < tpEnd && op < opEnd; ++tp, ++op, i++){
      if(*tp == *op){
	if(unlikely (*tp == prefixBound))
	  return i+10; /* if the transactions are equals up to the prefix */
	continue; 
      }
      if(*tp > *op)
	return 1; 
      else
	return -1; 
    }
    
    if(tp == tpEnd && op == opEnd)
      return 0; 
    else
      if(tp != tpEnd) 
	return 1; 
      else
	return -1; 
  }


  /** 
   * \brief value limited comparaison 
   * \warning assuming array is sorted.
   *
   * Returns true when the two arrays are equal up to the first
   * occurence of value \value.
   *
   * \warning return false if does not contains value (even if both equals).
   *
   * @param T 
   * @param USE_CTORS 
   * @param other 
   */
  inline int valueLimitedCompare(const Array<T, USE_CTORS, ALLOCATOR> &other,
				  const T &value) const{
    const T *tp = _data, *tpEnd = pEnd();
    const T *op = other._data, *opEnd = other.pEnd();    
    for(; tp < tpEnd && op < opEnd; ++tp, ++op){
      if(*tp != *op)
	return -1;
      if(*tp == value)
	return tp - _data;       
    }
    return -1; 
  }
    
  /** 
   * \brief Transfer datas from an array to another without coping
   * memory.  
   * \warning No memory is allocated, no copies are
   * done. After a call this become an empty array.  @param other
   */
  void leaveData(Array<T, USE_CTORS, ALLOCATOR> *other){
    assert(other != this);
    other->~Array<T, USE_CTORS, ALLOCATOR>(); 
    other->_data = _data; 
    other->_size = _size; 
    other->_allocator = _allocator; 
    assert((*other)[0] == _data[0]); 
    _data = NULL; 
    _size = 0; 
  }

  unsigned int memSize(){
    return _allocator.memSize();
  }

};


template <class T, bool USE_CTORS, class ALLOCATOR>
std::ostream &operator<<(std::ostream &os, const Array<T, USE_CTORS, ALLOCATOR> &a){
  
  os<<"size : "<<a.size()<<" [ "; 
  for(unsigned int i = 0; i < a.size(); i++){
    //        os<<i<<":"<<a[i]<<" ";
        os<<a[i]<<" ";
      }
      os<<"]";
      return os; 
  }


#endif	    /* _ARRAY_HPP_ */
