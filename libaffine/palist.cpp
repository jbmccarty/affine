#include "palist.h"

namespace PAList {

  node* list::pool_ = 0;

  //precondition: psize is a power of 2
  //Returns the number of variables of size vsize required to allocate an
  //integer multiple of psize bytes.
  inline size_t calc_poolsize (size_t psize, size_t vsize)
  {
    while (!(vsize & 0x1))
    {
      vsize >>= 1;
      psize >>= 1;
    }
    return psize;
  }
  
  const size_t pagesize = 4096;
  size_t list::poolsize_ = calc_poolsize(pagesize, sizeof(node));

  //precondition: pool_ = 0
  //postconditions: newpool points to a zero-terminated linked list of
  //size oldpoolsize;
  //poolsize_ <<= 1;
  void list::grow ()
  {
    pool_ = new node[poolsize_];

    size_t i = 0;
    for (;i < poolsize_ - 1; ++i)
      pool_[i].next_ = &pool_[i + 1]; //link nodes together
    pool_[i].next_ = 0;
    
    poolsize_ <<= 1;
  }

  list& list::operator= (const list& a)
  { //fails on self-assignment
    clear();
    if (a.begin_)
    {
      if (!pool_)
        grow();
      begin_ = pool_;
      node* A = a.begin_;
      do
      {
        pool_->val_ = A->val_;
        A = A->next_;
        if (A)
          if (pool_->next_)
            pool_ = pool_->next_;
          else
          {
            node* t = pool_;
            grow();
            t->next_ = pool_;
          }
      } while (A);
      end_ = pool_;
      pool_ = pool_->next_;
      end_->next_ = 0;
    }
    return *this;
  }

  void list::push_back (const T& a)
  {
    if (!pool_)
      grow();

    if (end_)
      end_ = end_->next_ = pool_;
    else
      begin_ = end_ = pool_;
    pool_ = pool_->next_;
    end_->next_ = 0;
    end_->val_ = a;
  }

  list::iterator list::insert (list::iterator i, const T& a)
  {
    if (!pool_)
      grow();

    iterator ret = i;
    ret.curr_ = pool_;
    pool_ = pool_->next_;
    *ret = a;
    ret.curr_->next_ = i.curr_;

    if (ret.prev_)
      ret.prev_->next_ = ret.curr_;
    else
      begin_ = ret.curr_;
    if (!i.curr_)
      end_ = ret.curr_;

    return ret;
  }

  list::iterator list::erase (list::iterator i)
  {
    iterator ret = i;
    ret.curr_ = i.curr_->next_;

    if (ret.prev_)
      ret.prev_->next_ = ret.curr_;
    else
      begin_ = ret.curr_;

    if (!ret.curr_)
      end_ = ret.prev_;

    i.curr_->next_ = pool_;
    pool_ = i.curr_;

    return ret;
  }

  void list::clear ()
  { //simply prepend list to pool_
    if (end_)
    {
      end_->next_ = pool_;
      pool_ = begin_;
      end_ = begin_ = 0;
    }
  }
}
