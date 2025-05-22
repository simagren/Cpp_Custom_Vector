#pragma once
#include "VecIterator.hpp"
#include "Reverse_VecIterator.hpp"
#include <ostream>
#include <format>
#include <iostream>
#include "Dalloc.hpp"
#include <vector>

#define CHECK assert(Invariant());

template<class T> class Vector
{
private:
	T* vecArray = nullptr;
	size_t currentSize = 0;
	size_t maxCapacity = 0;
	Dalloc<T> dAlloc;

public:
	using value_type = T;
	using pointer = T*;
	using reference = T&;
	using const_reference = const T&;
	using const_pointer = const T*;
	using size_type = size_t;
	using iterator = VecIterator<T>;
	using const_iterator = VecIterator<const T>;
	using reverse_iterator = Reverse_VecIterator<T>;
	using const_reverse_iterator = Reverse_VecIterator<const T>;
	using difference_type = ptrdiff_t;


#pragma region Assignment and Constructors
	Vector() noexcept {}

	Vector(const char* other)
	{
		size_t newSize = strlen(other);
		try
		{
			if (newSize > 0)
				vecArray = dAlloc.allocate(newSize);
	
			for (; currentSize < newSize; ++currentSize)
				new (vecArray + currentSize) T(other[currentSize]);
			maxCapacity = newSize;
		}
		catch (...)
			Deallocate(vecArray, currentSize, newSize);
	}

	// Copy Constructor
	Vector(const Vector& other)
	{
		if (other.currentSize <= 0)
			return;
		vecArray = dAlloc.allocate(other.maxCapacity);
		try {
			for (; currentSize < other.currentSize; ++currentSize) {
				new (vecArray + currentSize) T(other[currentSize]);
			}
			maxCapacity = other.maxCapacity;
		}
		catch (...)
			Deallocate(vecArray, currentSize, maxCapacity);
	}


	// Move Constructor
	Vector(Vector&& other) noexcept
	{
		vecArray = other.data();
		currentSize = other.size();
		maxCapacity = other.capacity();
		other.vecArray = nullptr;
		other.currentSize = 0;
		other.maxCapacity = 0;
	}


	Vector& operator=(const Vector& other)
	{
		size_t counter = 0;

		// If other vector is bigger, reallocate and make bigger
		if (maxCapacity < other.currentSize)
		{
			for (size_t i = 0; i < currentSize; ++i)
				vecArray[i].~T();
			currentSize = 0;
			dAlloc.deallocate(vecArray, maxCapacity);

			vecArray = dAlloc.allocate(other.currentSize);
			maxCapacity = other.currentSize;
		}

		try {
			for (; counter < other.currentSize; ++counter) {
				if (counter < currentSize)
					// Assignment on occupied slots
					vecArray[counter] = other.vecArray[counter];
				else
					// Placement new on empty slots
					new (vecArray + counter) T(other[counter]);
			}
			if (counter < currentSize) {
				for (; counter < currentSize; ++counter)
					vecArray[counter].~T();
			}
			currentSize = other.currentSize;
		}
		catch (...) {
			// Set size to what counter was before error.
			if (counter > currentSize)
				currentSize = counter;
			throw;
		}

		return *this;
	}


	Vector& operator=(Vector&& other) noexcept
	{
		this->~Vector();
		vecArray = other.data();
		currentSize = other.size();
		maxCapacity = other.capacity();
		other.vecArray = nullptr;
		other.currentSize = 0;
		other.maxCapacity = 0;

		return *this;
	}
#pragma endregion

#pragma region Data Return Methods
	T& operator[](size_t i) noexcept
	{
		return vecArray[i];
	}


	const T& operator[](size_t i) const noexcept
	{
		return vecArray[i];
	}


	// TODO: make strong exception safety
	T& at(size_t i)
	{
		if (i < currentSize)
			return vecArray[i];
		throw std::out_of_range(std::format
		("Error: trying to access an index outside the bounds of the array. i = {}, Size of array: {}.", i, currentSize));
	}


	// TODO: make strong exception safety
	const T& at(size_t i) const
	{
		if (i >= 0 && i < currentSize)
			return vecArray[i];
		throw std::out_of_range(std::format
		("Error: trying to access an index outside the bounds of the array. i = {}, Size of array: {}.", i, currentSize));
	}


	T* data() noexcept
	{
		return vecArray;
	}


	const T* data() const noexcept
	{
		return vecArray;
	}


	size_t size() const noexcept
	{
		return currentSize;
	}


	size_t capacity() const noexcept
	{
		return maxCapacity;
	}

#pragma endregion

#pragma region Modifying Methods

	void Deallocate(T* data, size_t size, size_t capacity) {
		for (size_t i = 0; i < size; ++i)
			data[i].~T();
		dAlloc.deallocate(data, capacity);
		throw;
	}

	void AllocateAndCopy(size_t newCap)
	{
		T* tempContainer = dAlloc.allocate(newCap);
		size_t counter = 0;
		try {
			for (; counter < currentSize; ++counter)
				new (tempContainer + counter) T(vecArray[counter]);
			for (int i = 0; i < currentSize; ++i)
				vecArray[i].~T();
			dAlloc.deallocate(vecArray, maxCapacity);
			vecArray = tempContainer;
			maxCapacity = newCap;
		}
		catch (...)
			Deallocate(tempContainer, counter, newCap);
	}


	void reserve(size_t n)
	{
		if (n > maxCapacity)
			AllocateAndCopy(n);
	}


	void shrink_to_fit() // 20240320
	{
		if (maxCapacity > currentSize)
			AllocateAndCopy(currentSize);
	}

	void resize(size_t n)
	{
		if (n > maxCapacity)
			AllocateAndCopy(n);
		if (n > currentSize)
			for (; currentSize < n; ++currentSize)
				new (vecArray + currentSize) T(T{});
		if (n < currentSize)
			for (size_t i = n; i < currentSize; ++i)
				vecArray[i].~T();
		currentSize = n;
	}

	void push_back(const T& c)
	{
		if (currentSize + 1 > maxCapacity)
		{
			size_t newCap = maxCapacity == 0 ? maxCapacity + 1 : maxCapacity * 2;
			AllocateAndCopy(newCap);			
		};
		new (vecArray + currentSize) T(c);
		currentSize++;
	}

	void pop_back()
	{
		vecArray[currentSize - 1].~T();
		currentSize--;
	}
	
	friend void swap(Vector& lhs, Vector& rhs)
	{
		T* tmp = lhs.data();
		size_t tmpSize = lhs.size();
		size_t tmpCap = lhs.capacity();

		lhs.vecArray = rhs.data();
		lhs.currentSize = rhs.size();
		lhs.maxCapacity = rhs.capacity();

		rhs.vecArray = tmp;
		rhs.currentSize = tmpSize;
		rhs.maxCapacity = tmpCap;
	}
#pragma endregion

#pragma region Logic Operators

	/// <summary>
	/// If both are same size/length, then loop through one and compare each element in the array.
	/// If one does not match then brake and return false, else continue and eventually return true;
	/// </summary>
	friend bool operator==(const Vector& lhs, const Vector& rhs)
	{
		if (lhs.currentSize == rhs.currentSize)
		{
			for (size_t i = 0; i < lhs.currentSize; i++)
			{
				if (lhs.vecArray[i] != rhs.vecArray[i])
					return false;
			}
			return true;
		}
		return false;
	}


	// Call already implemented overload on "==" and reverse result with "!"
	friend bool operator!=(const Vector& lhs, const Vector& rhs)
	{
		return !(lhs == rhs);
	}

	// lhs = Abcdef, rhs = Abcdefg
	friend bool operator<(const Vector& lhs, const Vector& rhs)
	{
		for (size_t i = 0; i < lhs.size() && i < rhs.size(); i++)
			if (lhs.data()[i] != rhs.data()[i])
				return lhs.data()[i] < rhs.data()[i];

		if (lhs.size() < rhs.size())
			return true;
		return false;
	}


	friend bool operator>(const Vector& lhs, const Vector& rhs)
	{
		return rhs < lhs;
	}


	friend bool operator<=(const Vector& lhs, const Vector& rhs)
	{
		return !(rhs < lhs);
	}


	friend bool operator>=(const Vector& lhs, const Vector& rhs)
	{
		return !(lhs < rhs);

	}
#pragma endregion

#pragma region  Iterators
	iterator begin() noexcept
	{
		return iterator(vecArray);
	}


	iterator end() noexcept
	{
		return iterator(vecArray + currentSize);
	}


	const_iterator begin() const noexcept
	{
		return const_iterator(vecArray);
	}


	const_iterator end() const noexcept
	{
		return const_iterator(vecArray + currentSize);
	}


	const_iterator cbegin() const noexcept
	{
		return const_iterator(vecArray);
	}


	const_iterator cend() const noexcept
	{
		return const_iterator(vecArray + currentSize);
	}


	reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(vecArray + (currentSize ? currentSize - 1 : 0));
	}


	reverse_iterator rend() noexcept
	{
		return reverse_iterator(vecArray - (currentSize > 0 ? 1 : 0));
	}


	const_reverse_iterator rbegin() const noexcept
	{
		return const_reverse_iterator(vecArray + (currentSize ? currentSize - 1 : 0));
	}


	const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(vecArray - (currentSize > 0 ? 1 : 0));
	}


	const_reverse_iterator crbegin() const noexcept
	{
		return const_reverse_iterator(vecArray + (currentSize ? currentSize - 1 : 0));
	}


	const_reverse_iterator crend() const noexcept
	{
		return const_reverse_iterator(vecArray - (currentSize > 0 ? 1 : 0));
	}
#pragma endregion


	bool Invariant() const
	{
		if (currentSize <= maxCapacity)
			return true;
		return false;
	}


	friend std::ostream& operator<<(std::ostream& cout
		, const Vector& other) {
		for (size_t i = 0; i < other.size(); ++i)
			cout << other[i];
		return cout;
	}


	void DeallocateVector(T* _array, size_t size)
	{
		for (size_t i = 0; i < size; ++i)
			_array[i].~T();
		dAlloc.deallocate(_array, size);
	}

	~Vector() noexcept
	{
		if (vecArray != nullptr)
		{
			for (size_t i = 0; i < currentSize; ++i)
				vecArray[i].~T();
			dAlloc.deallocate(vecArray, maxCapacity);
		}
		currentSize = 0;
		maxCapacity = 0;
	}
};

