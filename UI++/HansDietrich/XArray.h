// XArray.h   release 3.3  May 3, 2007
//
// Author: Samuel R. Buss
//
// A general purpose dynamically resizable array. Implemented with templates.
// Items are stored contiguously, for quick indexing.   However, allocation 
// may require the array to be copied into new memory locations. Because of 
// the dynamic resizing, you should be careful to understand how the array 
// code works before using it.  Care should be taken if the array might be 
// resized during an expression evaluation.
//
// All rights reserved.  May be used for any purpose as long
// as use is acknowledged.
//
///////////////////////////////////////////////////////////////////////////////
// Updated June 2010 by Hans Dietrich
///////////////////////////////////////////////////////////////////////////////

#ifndef XARRAY_H
#define XARRAY_H

#include <assert.h>
#include <stdlib.h>

// if you want to see the TRACE output, 
// uncomment this line:
//#include "XTrace.h"
//=============================================================================
#undef TRACEXARRAY
#undef TRACEXARRAYRECT
#ifdef XTRACE_H
	#define TRACEXARRAY TRACE
	#define TRACEXARRAYRECT TRACERECT
#else
	#ifndef __noop
		#if _MSC_VER < 1300
			#define __noop ((void)0)
		#endif
	#endif
	#define TRACEXARRAY __noop
	#define TRACEXARRAYRECT __noop
#endif
//=============================================================================

#pragma warning(push)
#pragma warning(disable : 4710)	// function not inlined

template <class T> class XArray 
{

public:
	XArray();						// Constructor
	XArray(int initialSize);		// Constructor
	~XArray();						// Destructor

	void RemoveAll(int newsize);
	void Reset() { ReducedSizeUsed(0); }

	// Next two routines: Main purpose to control allocation.
	//   Note: Allocation can occur without warning if you do not take care.
	//	 When allocation occurs, pointer and references become bad.
	// Use this judiciously to control when the array may be resized
	void PreallocateMore(int numAdditional) { Resize(SizeUsed()+numAdditional); }
	void Resize(int newMaxSize);	// Increases allocated size (will not decrease allocated size)

	// Next routines are used to update the "SizeUsed" of the stack.
	void Touch(int i);			// Makes entry i active.  Increases SizeUsed (MaxEntryPlus)
	void ReducedSizeUsed(int i); // "i" is the number of used entries now.
	void MakeEmpty() { MaxEntryPlus = 0; };	// Makes the length "SizeUsed" equal zero.

	// Next four functions give the functionality of a stack.
	T& Top() { return *(TheEntries+(MaxEntryPlus-1)); }
	const T& Top() const { return *(TheEntries+(MaxEntryPlus-1)); }
	T& Pop();
	T* Push();					// Push with no argument returns pointer to top element
	T* Push(const T& newElt);
	BOOL IsEmpty() const { return (MaxEntryPlus == 0); }

	void DisallowDynamicResizing() { DynamicResizingOK = FALSE; }
	void AllowDynamicResizing() { DynamicResizingOK = TRUE; }

	// Access function - if i is out of range, it resizes
	T& operator[](int i);
	const T& operator[](int i) const;

	// Access functions - Do not check whether i is out of range!!
	//		No resizing will occur!  Use only to get existing entries.
	T& GetEntry(int i);
	const T& GetEntry(int i) const;
	T& GetFirstEntry();
	const T& GetFirstEntry() const;
	T& GetLastEntry();
	const T& GetLastEntry() const;
	T* GetFirstEntryPtr();
	const T* GetFirstEntryPtr() const;

	int SizeUsed() const;			// Number of elements used (= 1+(max index));

	int AllocSize() const;			// Size allocated for the array
	BOOL IsFull() const { return MaxEntryPlus == Allocated; }
	// int SizeAvailable() const { return Allocated-MaxEntryPlus; }

	XArray<T>& operator=(const XArray<T>& other);
	XArray<T>(const XArray<T>& other);

	// Higher-level functions
	BOOL IsMember(const T& queryElt) const;	// Check if present in array

private:

	int MaxEntryPlus;				// Maximum entry used, plus one (Usually same as size)
	int Allocated;					// Number of entries allocated
	T* TheEntries;					// Pointer to the array of entries

	BOOL DynamicResizingOK;			// TRUE = array can be dynamically resized.
};

template<class T> inline XArray<T>::XArray()
{ 
	TRACEXARRAY(_T("in XArray()\n"));
	TheEntries = 0;
	MaxEntryPlus = 0;
	Allocated = 0;
	DynamicResizingOK = TRUE;
	Resize(10);
}

template<class T> inline XArray<T>::XArray(int initialSize)
{
	TRACEXARRAY(_T("in XArray(%d)\n"), initialSize);
	TheEntries = 0;
	MaxEntryPlus = 0;
	Allocated = 0;
	DynamicResizingOK = TRUE;
	Resize(initialSize);
}

template<class T> inline XArray<T>::~XArray()
{
	TRACEXARRAY(_T("in ~XArray:  MaxEntryPlus=%d  Allocated=%d\n"), MaxEntryPlus, Allocated);
	delete [] TheEntries;
	TheEntries = 0;
	MaxEntryPlus = 0;
	Allocated = 0;
}

template<class T> inline void XArray<T>::RemoveAll(int newsize)
{
	TRACEXARRAY(_T("in RemoveAll:  MaxEntryPlus=%d  Allocated=%d\n"), MaxEntryPlus, Allocated);
	delete [] TheEntries;
	TheEntries = 0;
	MaxEntryPlus = 0;
	Allocated = 0;
	DynamicResizingOK = TRUE;
	if (newsize > 0)
		Resize(newsize);
}

template<class T> inline void XArray<T>::Resize(int newMaxSize)
{
	if (newMaxSize <= Allocated) 
	{
		return;
	}
	if (!DynamicResizingOK) 
	{
		assert(FALSE);
		return;
	}
	Allocated = __max(2*Allocated+1, newMaxSize);
	T* newArray = new T [Allocated];
	T* toPtr = newArray;
	T* fromPtr = TheEntries;
	if (fromPtr)
	{
		for (int i = 0; i < MaxEntryPlus; i++) 
		{
			*(toPtr++) = *(fromPtr++);
		}
	}
	delete [] TheEntries;
	TheEntries = newArray;	
}

template<class T> inline void XArray<T>::Touch(int i)
{
	if (i >= Allocated) 
	{
		Resize(i+1);
	}
	if (i >= MaxEntryPlus) 
	{
		MaxEntryPlus = i+1;
	}
}

template<class T> inline void XArray<T>::ReducedSizeUsed(int i)
{
	// "i" is the number of used entries now.
	if (i < MaxEntryPlus) 
	{
		MaxEntryPlus = i;
	}
}

template<class T> inline T& XArray<T>::Pop()
{
	if (MaxEntryPlus > 0)
		MaxEntryPlus--;
	return *(TheEntries+MaxEntryPlus);
}

template<class T> inline T* XArray<T>::Push()
{
	if (MaxEntryPlus >= Allocated) 
	{
		Resize(MaxEntryPlus+1);
	}
	T* ret = TheEntries + MaxEntryPlus;
	MaxEntryPlus++;
	return ret;
}

template<class T> inline T* XArray<T>::Push(const T& newElt)
{
	T* top = Push();
	*top = newElt;
	return top;
}

template<class T> inline T& XArray<T>::operator[](int i)
{
	if (i >= Allocated) 
	{
		Resize(i+1);
	}
	if (i >= MaxEntryPlus) 
	{
		MaxEntryPlus = i+1;
	}
	return TheEntries[i];
}

template<class T> inline const T& XArray<T>::operator[](int i) const
{
	if (i >= Allocated) 
	{
		const_cast<XArray<T>*>(this)->Resize(i+1);
	}
	if (i >= MaxEntryPlus) 
	{
		const_cast<XArray<T>*>(this)->MaxEntryPlus = i+1;
	}
	return TheEntries[i];
}

template<class T> inline T& XArray<T>::GetEntry(int i)
{
	assert(i < MaxEntryPlus);
	return TheEntries[i];
}

template<class T> inline const T& XArray<T>::GetEntry(int i) const
{
	assert(i < MaxEntryPlus);
	return TheEntries[i];
}

template<class T> inline T& XArray<T>::GetFirstEntry()
{
	assert(Allocated > 0);
	return *TheEntries;
}

template<class T> inline const T& XArray<T>::GetFirstEntry() const
{
	assert(Allocated > 0);
	return *TheEntries;
}

template<class T> inline T* XArray<T>::GetFirstEntryPtr()
{
	assert(Allocated > 0);
	return TheEntries;
}

template<class T> inline const T* XArray<T>::GetFirstEntryPtr() const
{
	assert(Allocated > 0);
	return TheEntries;
}

template<class T> inline T& XArray<T>::GetLastEntry()
{
	assert(MaxEntryPlus > 0);
	return *(TheEntries+(MaxEntryPlus-1));
}

template<class T> inline const T& XArray<T>::GetLastEntry() const
{ 
	assert(MaxEntryPlus > 0);
	return *(TheEntries+(MaxEntryPlus-1));
}

template<class T> inline int XArray<T>::SizeUsed() const
{
	return MaxEntryPlus;
}

template<class T> inline int XArray<T>::AllocSize() const
{
	return Allocated;
}

// Check if queryElt present in array
template<class T> inline BOOL XArray<T>::IsMember(const T& queryElt) const
{
	T* tPtr = TheEntries;
	for (int i = MaxEntryPlus; i > 0; i--, tPtr++) 
	{
		if ((*tPtr) == queryElt) 
		{
			return TRUE;
		}
	}
	return FALSE;
}

// assignment operator
template<class T> inline XArray<T>& XArray<T>::operator=(const XArray<T>& other)
{
	if (TheEntries != other.TheEntries)
	{
		Resize(other.MaxEntryPlus);
		MaxEntryPlus = other.MaxEntryPlus;
		T* toPtr = TheEntries;
		const T* fromPtr = other.TheEntries;

		for (int i = MaxEntryPlus; i > 0; i--, toPtr++, fromPtr++) 
		{
			*toPtr = *fromPtr;
		}
	}

	return *this;
}

// copy ctor
template<class T> inline XArray<T>::XArray(const XArray<T>& other)
{
	TRACEXARRAY(_T("in copy ctor\n"));
	TheEntries = 0;
	Allocated = 0;
	DynamicResizingOK = other.DynamicResizingOK;
	Resize(other.MaxEntryPlus);
	MaxEntryPlus = other.MaxEntryPlus;

	T* toPtr = TheEntries;
	const T* fromPtr = other.TheEntries;

	for (int i = MaxEntryPlus; i > 0; i--, toPtr++, fromPtr++) 
	{
		*toPtr = *fromPtr;
	}
}

#pragma warning(pop)

#endif // XARRAY_H
