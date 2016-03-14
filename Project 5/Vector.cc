// Implementation of the templated Vector class
// ECE4893/8893 lab 3
// YOUR NAME HERE

#include <iostream> // debugging
#include "Vector.h"

// Your implementation here
// Fill in all the necessary functions below
using namespace std;

// Default constructor
template <typename T>
Vector<T>::Vector()
{
	elements = NULL;
	count = 0;
	reserved = 0;
}

// Copy constructor
template <typename T>
Vector<T>::Vector(const Vector& rhs)
{
	count = rhs.count;
	reserved = rhs.reserved;
	elements = (T*)malloc(sizeof(T) * rhs.count);
	for (size_t i = 0; i < count; ++i)
	{
		new (&elements[i]) T(rhs[i]);
	}
	
}

// Assignment operator
template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& rhs)
{
	for (size_t i = 0; i < count; ++i)
    {
    	elements[i].~T();
    } 
    free(elements);
	count = rhs.count;
	reserved = rhs.reserved;
	elements = (T*)malloc(sizeof(T) * rhs.count);
	for (size_t i = 0; i < count; ++i)
	{
		new (&elements[i]) T(rhs[i]);
	}
	return *this;
}

#ifdef GRAD_STUDENT
// Other constructors
template <typename T>
Vector<T>::Vector(size_t nReserved)
{ // Initialize with reserved memory
	elements = (T*)malloc(sizeof(T) * nReserved);
	count = 0;
	reserved = nReserved;
}

template <typename T>
Vector<T>::Vector(size_t n, const T& t)
{ // Initialize with "n" copies of "t"
	count = n;
	reserved = 0;
	elements = (T*)malloc(sizeof(T) * count);
	for (size_t i = 0; i < count; ++i)
	{
		new (&elements[i]) T(t);
	}
}

template <typename T>
void Vector<T>::Reserve(size_t n)
{	
	T* new_elements = (T*)malloc(sizeof(T) * n);
	
	for (size_t i = 0; i < count; ++i)
	{
		new (&new_elements[i]) T(elements[i]);
		elements[i].~T();
	}
	
	free(elements);
	elements = new_elements;
	reserved = n;
}

#endif

// Destructor
template <typename T>
Vector<T>::~Vector()
{
    for (size_t i = 0; i < count; ++i)
    {
    	elements[i].~T();
    }    
    
    free(elements);
    elements = NULL;
    count = 0;
    reserved = 0;
}

// Add and access front and back
template <typename T>
void Vector<T>::Push_Back(const T& rhs)
{
	if (count+1 > reserved)
	{
		Reserve(count+1);
	}
	new (&elements[count]) T(rhs);
	count++;
}

template <typename T>
void Vector<T>::Push_Front(const T& rhs)
{
	if (count < reserved)
	{
		for (size_t i = count; i > 0; --i)
		{
			elements[i] = elements[i-1];
		}
		new (&elements[0]) T(rhs);
		count++;
	}
	else
	{
		T* new_elements = (T*)malloc(sizeof(T) * (count+1));
		for (size_t i = count; i > 0; --i)
		{
			new (&new_elements[i]) T(elements[i-1]);
			elements[i-1].~T();
		}
		free(elements);
		elements = new_elements;
		new (&elements[0]) T(rhs);
		count++;
		reserved = count;
	}	
}

template <typename T>
void Vector<T>::Pop_Back()
{ // Remove last element
	count--;
	elements[count].~T();
}

template <typename T>
void Vector<T>::Pop_Front()
{ // Remove first element
	for (size_t i = 0; i < count-1; ++i)
	{
		elements[i].~T();
		new (&elements[i]) T(elements[i+1]);
	}

	Pop_Back();
}

// Element Access
template <typename T>
T& Vector<T>::Front() const
{
	return elements[0];
}

// Element Access
template <typename T>
T& Vector<T>::Back() const
{
	return elements[count-1];
}

template <typename T>
const T& Vector<T>::operator[](size_t i) const
{
	return this->elements[i];
}

template <typename T>
T& Vector<T>::operator[](size_t i)
{
	return this->elements[i];
}

template <typename T>
size_t Vector<T>::Size() const
{
	return count;
}

template <typename T>
bool Vector<T>::Empty() const
{
	return count == 0;
}

// Implement clear
template <typename T>
void Vector<T>::Clear()
{
	for (size_t i = 0; i < count; ++i)
    {
    	elements[i].~T();
    }  
	count = 0;
}

// Iterator access functions
template <typename T>
VectorIterator<T> Vector<T>::Begin() const
{
  return VectorIterator<T>(elements);
}

template <typename T>
VectorIterator<T> Vector<T>::End() const
{
  return VectorIterator<T>(elements + count);
}

#ifdef GRAD_STUDENT
// Erase and insert
template <typename T>
void Vector<T>::Erase(const VectorIterator<T>& it)
{
    size_t index;
    for (size_t i = 0; i < count; ++i)
    {
    	if (&elements[i] == it.current)
    	{
    		index = i;
    		break;
    	}
    }
    for (size_t i = index; i < count-1; ++i)
    {
    	elements[i].~T();
    	new (&elements[i]) T(elements[i+1]);
    }
    count--;
    elements[count].~T();
}

template <typename T>
void Vector<T>::Insert(const T& rhs, const VectorIterator<T>& it)
{
	size_t index;
    
    for (size_t i = 0; i < count; ++i)
    {
    	if (&elements[i] == it.current)
    	{
    		index = i;
    		break;
    	}
    }
    
	if (count < reserved)
	{
		for (size_t i = count; i > index; --i)
		{
			elements[i] = elements[i-1];
		}
		new (&elements[index]) T(rhs);
		count++;
	}
	else
	{
		T* new_elements = (T*)malloc(sizeof(T) * (count+1));
		for (size_t i = 0; i < index; ++i)
		{
			new (&new_elements[i]) T(elements[i]);
			elements[i].~T();
		}

		for (size_t i = count; i > index; --i)
		{
			new (&new_elements[i]) T(elements[i-1]);
			elements[i-1].~T();
		}
		free(elements);
		elements = new_elements;
		new (&elements[index]) T(rhs);
		count++;
		reserved = count;
	}	
}
#endif

// Implement the iterators

// Constructors
template <typename T>
VectorIterator<T>::VectorIterator()
{
	current = NULL;
}

template <typename T>
VectorIterator<T>::VectorIterator(T* c)
{
	current = c;
}

// Copy constructor
template <typename T>
VectorIterator<T>::VectorIterator(const VectorIterator<T>& rhs)
{
	current = rhs.current;
}

// Iterator defeferencing operator
template <typename T>
T& VectorIterator<T>::operator*() const
{
	return *current;
}

// Prefix increment
template <typename T>
VectorIterator<T>  VectorIterator<T>::operator++()
{
	++current;
	return *this;
}

// Postfix increment
template <typename T>
VectorIterator<T> VectorIterator<T>::operator++(int)
{
	VectorIterator<T> tmp(*this); 
	++current; 
	return tmp;
}

// Comparison operators
template <typename T>
bool VectorIterator<T>::operator !=(const VectorIterator<T>& rhs) const
{
	return current != rhs.current;
}

template <typename T>
bool VectorIterator<T>::operator ==(const VectorIterator<T>& rhs) const
{
	return current == rhs.current;
}




