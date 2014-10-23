#ifndef _DEVALLOC_H
#define _DEVALLOC_H

#ifdef DEBUG
#include <iostream>
#endif

#include <vector>
#include <affine/config.h>

class DevAllocator //memory management of deviations for Affine
{
public:
	DevAllocator (size_t s = 16384);
	
	size_t allocate ();
	void reallocate (size_t);
	void deallocate (size_t);
	unsigned int get_refcount (size_t) const;

private:
	size_t top_; //index of lowest unallocated deviation
	const size_t size_; //size of the array
	std::vector<unsigned int> devs_;
	//list of how many times each dev has been allocated
};

inline DevAllocator::DevAllocator (size_t s)
#ifdef DEBUG
	: top_(0), size_(s), devs_(s, 0)
#else
	: top_(0), size_(s), devs_(s)
#endif
{}

inline size_t DevAllocator::allocate ()
{
#ifdef DEBUG
	if (top_ >= size_)
	{
		std::cerr << "Too many deviations: " << size_ << "\n";
		throw int(0);
	}
#endif

	devs_[top_] = 1;
	return top_++;
}

inline void DevAllocator::reallocate (size_t index)
{
#ifdef DEBUG
	if ((index >= top_) || (devs_[index] < 1))
	{
		std::cerr << "Attempted to reallocate a non-existent deviation: "
			<< index << "\n";
		throw int(1);
	}
#endif

	devs_[index]++;
}

inline void DevAllocator::deallocate (size_t index)
{
#ifdef DEBUG
	if ((index >= top_) || (devs_[index] < 1))
	{
		std::cerr << "Attempted to deallocate a non-existent deviation: "
			<< index << "\n";
		throw int(2);
	}
#endif

	--devs_[index];
	while (top_ && ! devs_[top_ - 1])
		--top_;
}
inline unsigned int DevAllocator::get_refcount (size_t index) const
{
	//should I even bother with error-checking here?
	return devs_[index];
}

#endif //_DEVALLOC_H
