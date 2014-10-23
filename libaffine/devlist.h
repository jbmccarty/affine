#ifndef _DEVLIST_H
#define _DEVLIST_H

#include <utility>
#include <affine/config.h>
#include "devalloc.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#  if LIST_TYPE == LIST_PALIST
#    include "palist.h"
#  elif LIST_TYPE == LIST_STDLIST
#    include <list>
#  endif
#endif

class DevList
{
public:
	DevList () {}
	DevList (Real);
	DevList (const DevList&);
	DevList& operator= (const DevList&);
	~DevList ();

	typedef std::pair<size_t, Real> DevType;

  typedef
#if LIST_TYPE == LIST_PALIST
  PAList::list
#elif LIST_TYPE == LIST_STDLIST
	std::list<DevType>
#endif
  DList;

	typedef DList::iterator iterator;
	typedef DList::const_iterator const_iterator;

	void new_dev (Real a = 0.0); //allocate a new deviation
	iterator insert (iterator, const DevType&);
  //insert a deviation from another variable--invalidates its first argument
  void push_back (const DevType&);

	iterator erase (iterator); //erase a deviation
  void clear (); //erase all deviations

	unsigned int get_refcount (const_iterator) const;

	//replicated list operations
	iterator begin () { return devs_.begin(); }
	const_iterator begin () const { return devs_.begin(); }
	iterator end () { return devs_.end(); }
	const_iterator end () const { return devs_.end(); }

private:
  void deallocate_all ();
  void reallocate_all ();
	DList devs_;
	static DevAllocator allocator_;
};

inline DevList::DevList (Real a)
{ devs_.push_back(std::make_pair(allocator_.allocate(), a)); }
	
inline DevList::DevList (const DevList& a) : devs_(a.devs_)
{ reallocate_all(); }

inline DevList& DevList::operator= (const DevList& a)
{
  if (&a != this) {
    deallocate_all();
  	devs_ = a.devs_;
    reallocate_all();
  }
	return *this;
}

inline DevList::~DevList ()
{ deallocate_all(); }

inline void DevList::new_dev (Real a) //a should be positive
{
  if (begin() != end()) {
    DevType& last = devs_.back();
    if (allocator_.get_refcount(last.first) == 1) {
      last.second = std::fabs(last.second) + a;
      return;
    }
  }
  devs_.push_back(std::make_pair(allocator_.allocate(), a));
}

inline DevList::iterator DevList::insert (DevList::iterator i,
		const DevList::DevType& d)
{
	allocator_.reallocate(d.first);
	return devs_.insert(i, d);
}

inline void DevList::push_back (const DevList::DevType& d)
{
  allocator_.reallocate(d.first);
  devs_.push_back(d);
}

inline DevList::iterator DevList::erase (DevList::iterator i)
{
	allocator_.deallocate(i->first);
	return devs_.erase(i);
}

inline void DevList::clear ()
{
  deallocate_all();
  devs_.clear();
}

inline unsigned int DevList::get_refcount (DevList::const_iterator i) const
{ return allocator_.get_refcount(i->first); }

inline void DevList::deallocate_all ()
{
  for (const_iterator i = begin(); i != end(); ++i)
    allocator_.deallocate(i->first);
}

inline void DevList::reallocate_all ()
{
  for (const_iterator i = begin(); i != end(); ++i)
    allocator_.reallocate(i->first);
}

#endif //_DEVLIST_H
