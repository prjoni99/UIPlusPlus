// XStack.h   release 3.2  May 3, 2007
//
// Author: Samuel R. Buss
//
// A general purpose dynamically resizable stack. Implemented with templates.
// Items are stored contiguously, for quick accessing. However, allocation 
// may require the stack to be copied into new memory locations.
//
// All rights reserved.  May be used for any purpose as long
// as use is acknowledged.
//
///////////////////////////////////////////////////////////////////////////////
// v3.3  2011 Apr 12  Fix assert in Pop()
///////////////////////////////////////////////////////////////////////////////

#ifndef XSTACK_H
#define XSTACK_H

//#include <assert.h>

template <class T> class XStack 
{

public:
	XStack();					// Constructor
	XStack(int initialSize);	// Constructor
	~XStack();					// Destructor

	void Reset();

	void Resize(int newMaxSize);	// Increases allocated size (will not decrease size)

	T& Top() const { return *TopElement; };
	T& Pop();

	T * Push();					// New top element is arbitrary
	T * Push(const T& newElt);	// Push newElt onto stack.

	BOOL IsEmpty() const { return (SizeUsed == 0); }

	int Size() const { return SizeUsed; }
	int SizeAllocated() const { return Allocated; }

private:

	int SizeUsed;				// Number of elements in the stack
	T *TopElement;				// Pointer to the top element of the stack
	int Allocated;				// Number of entries allocated
	T *TheStack;				// Pointer to the array of entries
};

template<class T> inline XStack<T>::XStack()
{ 
	SizeUsed = 0;
	TheStack = 0;
	Allocated = 0;
	Resize(10);
}

template<class T> inline XStack<T>::XStack(int initialSize)
{
	SizeUsed = 0;
	TheStack = 0;
	Allocated = 0;
	Resize(initialSize);
}

template<class T> inline XStack<T>::~XStack()
{
	delete [] TheStack;
}

template<class T> inline void XStack<T>::Reset()
{
	SizeUsed = 0;
	TopElement = TheStack-1;
}

template<class T> inline void XStack<T>::Resize(int newMaxSize)
{
	if (newMaxSize <= Allocated) 
	{
		return;
	}
	int newSize = __max(2*Allocated+1, newMaxSize);
	T *newArray = new T [newSize];
	T *toPtr = newArray;
	T *fromPtr = TheStack;
	for (int i =  0; i < SizeUsed; i++) 
	{
		*(toPtr++) = *(fromPtr++);
	}
	delete [] TheStack;
	TheStack = newArray;
	Allocated = newSize;
	TopElement = TheStack + (SizeUsed-1);
}

template<class T> inline T& XStack<T>::Pop()
{
	T *ret = TopElement;
	//+++3.3
	if (SizeUsed > 0)		// Should be non-empty
	{
		SizeUsed--;
		TopElement--;
	}
	return *ret;
}

// Enlarge the stack but do not update the top element.
// Returns a pointer to the top element (which is unchanged/uninitialized)
template<class T> inline T* XStack<T>::Push()
{
	if (SizeUsed >= Allocated) 
	{
		Resize(SizeUsed+1);
	}
	SizeUsed++;
	TopElement++;
	return TopElement;
}

template<class T> inline T* XStack<T>::Push(const T& newElt)
{
	Push();
	*TopElement = newElt;
	return TopElement;
}

#endif	// XSTACK_H
