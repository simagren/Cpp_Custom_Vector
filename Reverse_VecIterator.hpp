#pragma once
#include <iterator>

template<typename CT>
class Reverse_VecIterator
{
private:
	CT* vecPtr = nullptr;

public:
	using difference_type = ptrdiff_t;
	using iterator_category = std::random_access_iterator_tag;
	using value_type = CT;
	using pointer = CT*;
	using reference = CT&;


	// p will be assigned either first or last element memory slot in array
	Reverse_VecIterator(CT* p) : vecPtr(p)
	{
	}

	Reverse_VecIterator() = default;


	Reverse_VecIterator(const Reverse_VecIterator& other)
	{
		vecPtr = other.vecPtr;
	}


	Reverse_VecIterator& operator=(const Reverse_VecIterator& other)
	{
		vecPtr = other.vecPtr;
		return *this;
	}


	CT& operator*() const
	{
		return *vecPtr;
	}


	CT* operator->() const
	{
		return vecPtr;
	}


	CT& operator[](size_t i) const
	{
		return vecPtr[i*-1];
	}


	// ++i
	// Increase and then return
	Reverse_VecIterator& operator++()
	{
		vecPtr = vecPtr - 1;
		return *this;
	}


	// --i
	// Decrease and then return
	Reverse_VecIterator& operator--()
	{
		vecPtr = vecPtr + 1;
		return *this;
	}


	// i++
	// Save current ptr into temp iterator and return that iterator
	// This will return an iterator and increment the ptr AFTER
	Reverse_VecIterator operator++(int)
	{
		Reverse_VecIterator tempIter(vecPtr);
		vecPtr = vecPtr - 1;
		return tempIter;
	}


	// i--
	// Same as i++, return ptr with current value and THEN decrease by 1.
	Reverse_VecIterator operator--(int)
	{
		Reverse_VecIterator tempIter(vecPtr);
		vecPtr = vecPtr + 1;
		return tempIter;
	}


	Reverse_VecIterator& operator+=(difference_type i)
	{
		vecPtr -= i;
		return *this;
	}


	Reverse_VecIterator operator+(difference_type i) const
	{
		CT* tmpPtr = vecPtr - i;
		Reverse_VecIterator tmp(tmpPtr);
		return tmp;
	}


	Reverse_VecIterator operator-(difference_type i) const
	{
		//CT* tmpPtr = vecPtr - (i * DIR);
		CT* tmpPtr = vecPtr + i;
		Reverse_VecIterator tmp(tmpPtr);
		return tmp;
	}


	difference_type operator-(const Reverse_VecIterator& rhs) const
	{
		// calculate the difference between the subtraction.
		// ex: 4-2=2, there are 2 integers between 2 and 4
		// 	// totala antal element i vecArray, "Fox" = 3
		// it3 pekar på 'x', sista elementet = 3
		// it pekar på 'F', första elementet = 1
		// it3 - it ==> 3 - 1 = 2.
		difference_type dif = std::distance(vecPtr, rhs.vecPtr);	
		return dif;
	}


	friend bool operator==(const Reverse_VecIterator& lhs, const Reverse_VecIterator& rhs)
	{
		return lhs.vecPtr == rhs.vecPtr;
	}

	friend bool operator!=(const Reverse_VecIterator& lhs, const Reverse_VecIterator& rhs)
	{
		return !(lhs == rhs);
	}
	friend bool operator<(const Reverse_VecIterator& lhs, const Reverse_VecIterator& rhs)
	{
		return rhs.vecPtr < lhs.vecPtr;
	}
	friend bool operator>(const Reverse_VecIterator& lhs, const Reverse_VecIterator& rhs)
	{
		return rhs.vecPtr > lhs.vecPtr;
	}
	friend bool operator<=(const Reverse_VecIterator& lhs, const Reverse_VecIterator& rhs)
	{
		return !(rhs.vecPtr > lhs.vecPtr);
	}
	friend bool operator>=(const Reverse_VecIterator& lhs, const Reverse_VecIterator& rhs)
	{
		return !(rhs.vecPtr < lhs.vecPtr);
	}
};