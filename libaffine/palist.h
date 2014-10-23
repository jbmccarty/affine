#ifndef _PALIST_H
#define _PALIST_H
#include <utility> //for pair
#include <cstddef> //for size_t
#include <iterator>
#include <affine/config.h>

namespace PAList { // Pooled Allocation List

typedef std::pair<size_t, Real> T; //fake template classes

struct node;
struct node { //one element of a list
  T val_;
  node *next_;
};

class iterator : public std::iterator<std::forward_iterator_tag, T>
{ //based on GNU libstdc++4's iterators
public:
  reference operator* () const { return curr_->val_; } //dereference
  pointer operator-> () const { return &(curr_->val_); } //arror operator

  iterator& operator++ () //preincrement
  { prev_ = curr_; curr_ = curr_->next_; return *this; }

  iterator operator++ (int) //postincrement
  { iterator temp(*this); ++*this; return temp; }

  bool operator!= (const iterator& a) { return (curr_ != a.curr_); }
  bool operator== (const iterator& a) { return (curr_ == a.curr_); }

private:
  friend class list;
  friend class const_iterator;
  node *prev_, *curr_; //previous and current values
};

class const_iterator : public std::iterator<std::forward_iterator_tag, const T>
{ //based on GNU libstdc++4's iterators
public:

  const_iterator () {}
  const_iterator (const iterator& a) : prev_(a.prev_), curr_(a.curr_) {}
  const_iterator& operator= (const iterator& a)
  { prev_ = a.prev_; curr_ = a.curr_; return *this; }
  
  reference operator* () const { return curr_->val_; } //dereference
  pointer operator-> () const { return &(curr_->val_); } //arror operator

  const_iterator& operator++ () //preincrement
  { prev_ = curr_; curr_ = curr_->next_; return *this; }

  const_iterator operator++ (int) //postincrement
  { const_iterator temp(*this); ++*this; return temp; }

  bool operator!= (const const_iterator& a) { return (curr_ != a.curr_); }
  bool operator== (const const_iterator& a) { return (curr_ == a.curr_); }

private:
  friend class list;
  const node *prev_, *curr_; //previous and current values
};

class list { //interface matches std::list
public:
  list () : begin_(0), end_(0) {}
  list (const list& a) : begin_(0), end_(0) { *this = a; }
  ~list () { clear(); }

  list& operator= (const list&); //fails on self-assignment

  typedef PAList::iterator iterator;
  typedef PAList::const_iterator const_iterator;
  iterator begin ()
  { iterator ret; ret.prev_ = 0; ret.curr_ = begin_; return ret; }
  const_iterator begin () const
  { const_iterator ret; ret.prev_ = 0; ret.curr_ = begin_; return ret; }
  iterator end ()
  { iterator ret; ret.prev_ = end_; ret.curr_ = 0; return ret; }
  const_iterator end () const
  { const_iterator ret; ret.prev_ = end_; ret.curr_ = 0; return ret; }

  T& front () { return begin_->val_; }
  const T& front () const { return begin_->val_; }
  T& back () { return end_->val_; }
  const T& back () const { return end_->val_; }

  void push_back (const T& a);
  
  iterator insert (iterator, const T&); //invalidates its first argument
  iterator erase (iterator);

  void clear ();
private:
  void grow (); //double size of the pool when it's empty
  static size_t poolsize_; //size of pool after next allocation
  node* begin_; //first node
  node* end_; //last node
  static node* pool_; //linked list of nodes in pool
};
}

#endif //_PALIST_H
