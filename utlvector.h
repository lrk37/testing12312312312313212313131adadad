#pragma once

template <class T>
inline T* Construct( T* pMemory )
{
	return ::new( pMemory ) T;
}

template <class T, typename ARG1>
inline T* Construct( T* pMemory, ARG1 a1 )
{
	return ::new( pMemory ) T( a1 );
}

template <class T, typename ARG1, typename ARG2>
inline T* Construct( T* pMemory, ARG1 a1, ARG2 a2 )
{
	return ::new( pMemory ) T( a1, a2 );
}

template <class T, typename ARG1, typename ARG2, typename ARG3>
inline T* Construct( T* pMemory, ARG1 a1, ARG2 a2, ARG3 a3 )
{
	return ::new( pMemory ) T( a1, a2, a3 );
}

template <class T, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
inline T* Construct( T* pMemory, ARG1 a1, ARG2 a2, ARG3 a3, ARG4 a4 )
{
	return ::new( pMemory ) T( a1, a2, a3, a4 );
}

template <class T, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
inline T* Construct( T* pMemory, ARG1 a1, ARG2 a2, ARG3 a3, ARG4 a4, ARG5 a5 )
{
	return ::new( pMemory ) T( a1, a2, a3, a4, a5 );
}

template <class T>
inline T* CopyConstruct( T* pMemory, T const& src )
{
	return ::new( pMemory ) T( src );
}

template <class T>
inline void Destruct( T* pMemory )
{
	pMemory->~T();
}

template < class T, class I = int >
class CUtlMemory
{
public:
	// constructor, destructor
	CUtlMemory( int nGrowSize = 0, int nInitSize = 0 );
	CUtlMemory( T* pMemory, int numElements );
	CUtlMemory( const T* pMemory, int numElements );
	~CUtlMemory();

	// Set the size by which the memory grows
	void Init( int nGrowSize = 0, int nInitSize = 0 );

	class Iterator_t
	{
	public:
		Iterator_t( I i ) :
			index( i )
		{
		}
		I index;

		bool operator==( const Iterator_t it ) const { return index == it.index; }
		bool operator!=( const Iterator_t it ) const { return index != it.index; }
	};
	Iterator_t First() const { return Iterator_t( IsIdxValid( 0 ) ? 0 : InvalidIndex() ); }
	Iterator_t Next( const Iterator_t& it ) const { return Iterator_t( IsIdxValid( it.index + 1 ) ? it.index + 1 : InvalidIndex() ); }
	I GetIndex( const Iterator_t& it ) const { return it.index; }
	bool IsIdxAfter( I i, const Iterator_t& it ) const { return i > it.index; }
	bool IsValidIterator( const Iterator_t& it ) const { return IsIdxValid( it.index ); }
	Iterator_t InvalidIterator() const { return Iterator_t( InvalidIndex() ); }

	// element access
	T& operator[]( I i );
	const T& operator[]( I i ) const;
	T& Element( I i );
	const T& Element( I i ) const;

	bool IsIdxValid( I i ) const;

	static const I INVALID_INDEX = (I)-1; // For use with COMPILE_TIME_ASSERT
	static I InvalidIndex() { return INVALID_INDEX; }

	T* Base();
	const T* Base() const;

	void SetExternalBuffer( T* pMemory, int numElements );
	void SetExternalBuffer( const T* pMemory, int numElements );
	void AssumeMemory( T* pMemory, int nSize );
	T* Detach();
	void* DetachMemory();

	void Swap( CUtlMemory< T, I >& mem );
	void ConvertToGrowableMemory( int nGrowSize );
	int NumAllocated() const;
	int Count() const;
	void Grow( int num = 1 );
	void EnsureCapacity( int num );
	void Purge();
	void Purge( int numElements );
	bool IsExternallyAllocated() const;
	bool IsReadOnly() const;
	void SetGrowSize( int size );

protected:
	void ValidateGrowSize()
	{
	}

	enum
	{
		EXTERNAL_BUFFER_MARKER = -1,
		EXTERNAL_CONST_BUFFER_MARKER = -2,
	};

public:
	T* m_pMemory;
	int m_nAllocationCount;
	int m_nGrowSize;
};

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------

template < class T, class I >
CUtlMemory< T, I >::CUtlMemory( int nGrowSize, int nInitAllocationCount ) :
	m_pMemory( 0 ),
	m_nAllocationCount( nInitAllocationCount ), m_nGrowSize( nGrowSize )
{
	ValidateGrowSize();
	assert( nGrowSize >= 0 );
	if ( m_nAllocationCount )
	{
		m_pMemory = (T*)g_csgo.m_mem_alloc->Alloc( m_nAllocationCount * sizeof( T ) );
		//m_pMemory = (T*)m_pMemAlloc->Alloc(m_nAllocationCount * sizeof(T));
	}
}

template < class T, class I >
CUtlMemory< T, I >::CUtlMemory( T* pMemory, int numElements ) :
	m_pMemory( pMemory ),
	m_nAllocationCount( numElements )
{
	// Special marker indicating externally supplied modifyable memory
	m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}

template < class T, class I >
CUtlMemory< T, I >::CUtlMemory( const T* pMemory, int numElements ) :
	m_pMemory( (T*)pMemory ),
	m_nAllocationCount( numElements )
{
	// Special marker indicating externally supplied modifyable memory
	m_nGrowSize = EXTERNAL_CONST_BUFFER_MARKER;
}

template < class T, class I >
CUtlMemory< T, I >::~CUtlMemory()
{
	Purge();
}

template < class T, class I >
void CUtlMemory< T, I >::Init( int nGrowSize /*= 0*/, int nInitSize /*= 0*/ )
{
	Purge();

	m_nGrowSize = nGrowSize;
	m_nAllocationCount = nInitSize;
	ValidateGrowSize();
	assert( nGrowSize >= 0 );
	if ( m_nAllocationCount )
	{
		UTLMEMORY_TRACK_ALLOC();
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemory = (T*)g_csgo.m_mem_alloc->Alloc( m_nAllocationCount * sizeof( T ) );
	}
}

//-----------------------------------------------------------------------------
// Fast swap
//-----------------------------------------------------------------------------
template < class T, class I >
void CUtlMemory< T, I >::Swap( CUtlMemory< T, I >& mem )
{
	V_swap( m_nGrowSize, mem.m_nGrowSize );
	V_swap( m_pMemory, mem.m_pMemory );
	V_swap( m_nAllocationCount, mem.m_nAllocationCount );
}

//-----------------------------------------------------------------------------
// Switches the buffer from an external memory buffer to a reallocatable buffer
//-----------------------------------------------------------------------------
template < class T, class I >
void CUtlMemory< T, I >::ConvertToGrowableMemory( int nGrowSize )
{
	if ( !IsExternallyAllocated() )
		return;

	m_nGrowSize = nGrowSize;
	if ( m_nAllocationCount )
	{
		int nNumBytes = m_nAllocationCount * sizeof( T );
		T* pMemory = (T*)g_csgo.m_mem_alloc->Alloc( nNumBytes );
		memcpy( pMemory, m_pMemory, nNumBytes );
		m_pMemory = pMemory;
	}
	else
	{
		m_pMemory = NULL;
	}
}

//-----------------------------------------------------------------------------
// Attaches the buffer to external memory....
//-----------------------------------------------------------------------------
template < class T, class I >
void CUtlMemory< T, I >::SetExternalBuffer( T* pMemory, int numElements )
{
	// Blow away any existing allocated memory
	Purge();

	m_pMemory = pMemory;
	m_nAllocationCount = numElements;

	// Indicate that we don't own the memory
	m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}

template < class T, class I >
void CUtlMemory< T, I >::SetExternalBuffer( const T* pMemory, int numElements )
{
	// Blow away any existing allocated memory
	Purge();

	m_pMemory = const_cast<T*>( pMemory );
	m_nAllocationCount = numElements;

	// Indicate that we don't own the memory
	m_nGrowSize = EXTERNAL_CONST_BUFFER_MARKER;
}

template < class T, class I >
void CUtlMemory< T, I >::AssumeMemory( T* pMemory, int numElements )
{
	// Blow away any existing allocated memory
	Purge();

	// Simply take the pointer but don't mark us as external
	m_pMemory = pMemory;
	m_nAllocationCount = numElements;
}

template < class T, class I >
void* CUtlMemory< T, I >::DetachMemory()
{
	if ( IsExternallyAllocated() )
		return NULL;

	void* pMemory = m_pMemory;
	m_pMemory = 0;
	m_nAllocationCount = 0;
	return pMemory;
}

template < class T, class I >
inline T* CUtlMemory< T, I >::Detach()
{
	return (T*)DetachMemory();
}

//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------
template < class T, class I >
inline T& CUtlMemory< T, I >::operator[]( I i )
{
	assert( !IsReadOnly() );
	// men
	// assert( IsIdxValid( i ) );
	return m_pMemory[ i ];
}

template < class T, class I >
inline const T& CUtlMemory< T, I >::operator[]( I i ) const
{
	// can you
	// assert( IsIdxValid( i ) );
	return m_pMemory[ i ];
}

template < class T, class I >
inline T& CUtlMemory< T, I >::Element( I i )
{
	assert( !IsReadOnly() );
	// FUCK
	// assert( IsIdxValid( i ) );
	return m_pMemory[ i ];
}

template < class T, class I >
inline const T& CUtlMemory< T, I >::Element( I i ) const
{
	// my balls ?
	// assert( IsIdxValid( i ) );
	return m_pMemory[ i ];
}

//-----------------------------------------------------------------------------
// is the memory externally allocated?
//-----------------------------------------------------------------------------
template < class T, class I >
bool CUtlMemory< T, I >::IsExternallyAllocated() const
{
	return ( m_nGrowSize < 0 );
}

//-----------------------------------------------------------------------------
// is the memory read only?
//-----------------------------------------------------------------------------
template < class T, class I >
bool CUtlMemory< T, I >::IsReadOnly() const
{
	return ( m_nGrowSize == EXTERNAL_CONST_BUFFER_MARKER );
}

template < class T, class I >
void CUtlMemory< T, I >::SetGrowSize( int nSize )
{
	assert( !IsExternallyAllocated() );
	assert( nSize >= 0 );
	m_nGrowSize = nSize;
	ValidateGrowSize();
}

//-----------------------------------------------------------------------------
// Gets the base address (can change when adding elements!)
//-----------------------------------------------------------------------------
template < class T, class I >
inline T* CUtlMemory< T, I >::Base()
{
	assert( !IsReadOnly() );
	return m_pMemory;
}

template < class T, class I >
inline const T* CUtlMemory< T, I >::Base() const
{
	return m_pMemory;
}

//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------
template < class T, class I >
inline int CUtlMemory< T, I >::NumAllocated() const
{
	return m_nAllocationCount;
}

template < class T, class I >
inline int CUtlMemory< T, I >::Count() const
{
	return m_nAllocationCount;
}

//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------
template < class T, class I >
inline bool CUtlMemory< T, I >::IsIdxValid( I i ) const
{
	// GCC warns if I is an unsigned type and we do a ">= 0" against it (since the comparison is always 0).
	// We Get the warning even if we cast inside the expression. It only goes away if we assign to another variable.
	long x = i;
	return ( x >= 0 ) && ( x < m_nAllocationCount );
}

//-----------------------------------------------------------------------------
// Grows the memory
//-----------------------------------------------------------------------------
inline int UtlMemory_CalcNewAllocationCount( int nAllocationCount, int nGrowSize, int nNewSize, int nBytesItem )
{
	if ( nGrowSize )
	{
		nAllocationCount = ( ( 1 + ( ( nNewSize - 1 ) / nGrowSize ) ) * nGrowSize );
	}
	else
	{
		if ( !nAllocationCount )
		{
			// Compute an allocation which is at least as big as a cache line...
			nAllocationCount = ( 31 + nBytesItem ) / nBytesItem;
		}

		while ( nAllocationCount < nNewSize )
		{
#ifndef _X360
			nAllocationCount *= 2;
#else
			int nNewAllocationCount = ( nAllocationCount * 9 ) / 8; // 12.5 %
			if ( nNewAllocationCount > nAllocationCount )
				nAllocationCount = nNewAllocationCount;
			else
				nAllocationCount *= 2;
#endif
		}
	}

	return nAllocationCount;
}

template < class T, class I >
void CUtlMemory< T, I >::Grow( int num )
{
	assert( num > 0 );

	if ( IsExternallyAllocated() )
	{
		// Can't grow a buffer whose memory was externally allocated
		assert( 0 );
		return;
	}

	auto oldAllocationCount = m_nAllocationCount;
	// Make sure we have at least numallocated + num allocations.
	// Use the grow rules specified for this memory (in m_nGrowSize)
	int nAllocationRequested = m_nAllocationCount + num;

	int nNewAllocationCount = UtlMemory_CalcNewAllocationCount( m_nAllocationCount, m_nGrowSize, nAllocationRequested, sizeof( T ) );

	// if m_nAllocationRequested wraps index type I, recalculate
	if ( (int)(I)nNewAllocationCount < nAllocationRequested )
	{
		if ( (int)(I)nNewAllocationCount == 0 && (int)(I)( nNewAllocationCount - 1 ) >= nAllocationRequested )
		{
			--nNewAllocationCount; // deal w/ the common case of m_nAllocationCount == MAX_USHORT + 1
		}
		else
		{
			if ( (int)(I)nAllocationRequested != nAllocationRequested )
			{
				// we've been asked to grow memory to a size s.t. the index type can't address the requested amount of memory
				assert( 0 );
				return;
			}
			while ( (int)(I)nNewAllocationCount < nAllocationRequested )
			{
				nNewAllocationCount = ( nNewAllocationCount + nAllocationRequested ) / 2;
			}
		}
	}

	m_nAllocationCount = nNewAllocationCount;

	if ( m_pMemory )
	{
		auto ptr = (T*)g_csgo.m_mem_alloc->Alloc( m_nAllocationCount * sizeof( T ) );

		memcpy( ptr,
			m_pMemory,
			oldAllocationCount * sizeof( T ) );
		m_pMemory = (T*)ptr;
	}
	else
	{
		m_pMemory = (T*)g_csgo.m_mem_alloc->Alloc( m_nAllocationCount * sizeof( T ) );
	}
}

//-----------------------------------------------------------------------------
// Makes sure we've got at least this much memory
//-----------------------------------------------------------------------------
template < class T, class I >
inline void CUtlMemory< T, I >::EnsureCapacity( int num )
{
	if ( m_nAllocationCount >= num )
		return;

	if ( IsExternallyAllocated() )
	{
		// Can't grow a buffer whose memory was externally allocated
		assert( 0 );
		return;
	}
	m_nAllocationCount = num;

	if ( m_pMemory )
	{
		m_pMemory = (T*)g_csgo.m_mem_alloc->Realloc( m_pMemory, m_nAllocationCount * sizeof( T ) );
	}
	else
	{
		m_pMemory = (T*)g_csgo.m_mem_alloc->Alloc( m_nAllocationCount * sizeof( T ) );
	}
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------
template < class T, class I >
void CUtlMemory< T, I >::Purge()
{
	if ( !IsExternallyAllocated() )
	{
		if ( m_pMemory )
		{
			g_csgo.m_mem_alloc->Free( (void*)m_pMemory );
			m_pMemory = 0;
		}
		m_nAllocationCount = 0;
	}
}

template < class T, class I >
void CUtlMemory< T, I >::Purge( int numElements )
{
	assert( numElements >= 0 );

	if ( numElements > m_nAllocationCount )
	{
		// Ensure this isn't a grow request in disguise.
		assert( numElements <= m_nAllocationCount );
		return;
	}

	// If we have zero elements, simply do a purge:
	if ( numElements == 0 )
	{
		Purge();
		return;
	}

	if ( IsExternallyAllocated() )
	{
		// Can't shrink a buffer whose memory was externally allocated, fail silently like purge
		return;
	}

	// If the number of elements is the same as the allocation count, we are done.
	if ( numElements == m_nAllocationCount )
	{
		return;
	}

	if ( !m_pMemory )
	{
		// Allocation count is non zero, but memory is null.
		assert( m_pMemory );
		return;
	}
	m_nAllocationCount = numElements;
	m_pMemory = (T*)realloc( m_pMemory, m_nAllocationCount * sizeof( T ) );
}

//-----------------------------------------------------------------------------
// The CUtlMemory class:
// A growable memory class which doubles in size by default.
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
class CUtlMemoryAligned : public CUtlMemory< T >
{
public:
	// constructor, destructor
	CUtlMemoryAligned( int nGrowSize = 0, int nInitSize = 0 );
	CUtlMemoryAligned( T* pMemory, int numElements );
	CUtlMemoryAligned( const T* pMemory, int numElements );
	~CUtlMemoryAligned();

	// Attaches the buffer to external memory....
	void SetExternalBuffer( T* pMemory, int numElements );
	void SetExternalBuffer( const T* pMemory, int numElements );

	// Grows the memory, so that at least allocated + num elements are allocated
	void Grow( int num = 1 );

	// Makes sure we've got at least this much memory
	void EnsureCapacity( int num );

	// Memory deallocation
	void Purge();

	// Purge all but the given number of elements (NOT IMPLEMENTED IN CUtlMemoryAligned)
	void Purge( int numElements ) { assert( 0 ); }

private:
	void* Align( const void* pAddr );
};

//-----------------------------------------------------------------------------
// Aligns a pointer
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
void* CUtlMemoryAligned< T, nAlignment >::Align( const void* pAddr )
{
	size_t nAlignmentMask = nAlignment - 1;
	return (void*)( ( (size_t)pAddr + nAlignmentMask ) & ( ~nAlignmentMask ) );
}

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
CUtlMemoryAligned< T, nAlignment >::CUtlMemoryAligned( int nGrowSize, int nInitAllocationCount )
{
	CUtlMemory< T >::m_pMemory = 0;
	CUtlMemory< T >::m_nAllocationCount = nInitAllocationCount;
	CUtlMemory< T >::m_nGrowSize = nGrowSize;
	this->ValidateGrowSize();

	// Alignment must be a power of two
	COMPILE_TIME_ASSERT( ( nAlignment & ( nAlignment - 1 ) ) == 0 );
	assert( ( nGrowSize >= 0 ) && ( nGrowSize != CUtlMemory< T >::EXTERNAL_BUFFER_MARKER ) );
	if ( CUtlMemory< T >::m_nAllocationCount )
	{
		UTLMEMORY_TRACK_ALLOC();
		MEM_ALLOC_CREDIT_CLASS();
		CUtlMemory< T >::m_pMemory = (T*)_aligned_malloc( nInitAllocationCount * sizeof( T ), nAlignment );
	}
}

template < class T, int nAlignment >
CUtlMemoryAligned< T, nAlignment >::CUtlMemoryAligned( T* pMemory, int numElements )
{
	// Special marker indicating externally supplied memory
	CUtlMemory< T >::m_nGrowSize = CUtlMemory< T >::EXTERNAL_BUFFER_MARKER;

	CUtlMemory< T >::m_pMemory = (T*)Align( pMemory );
	CUtlMemory< T >::m_nAllocationCount = ( (int)( pMemory + numElements ) - (int)CUtlMemory< T >::m_pMemory ) / sizeof( T );
}

template < class T, int nAlignment >
CUtlMemoryAligned< T, nAlignment >::CUtlMemoryAligned( const T* pMemory, int numElements )
{
	// Special marker indicating externally supplied memory
	CUtlMemory< T >::m_nGrowSize = CUtlMemory< T >::EXTERNAL_CONST_BUFFER_MARKER;

	CUtlMemory< T >::m_pMemory = (T*)Align( pMemory );
	CUtlMemory< T >::m_nAllocationCount = ( (int)( pMemory + numElements ) - (int)CUtlMemory< T >::m_pMemory ) / sizeof( T );
}

template < class T, int nAlignment >
CUtlMemoryAligned< T, nAlignment >::~CUtlMemoryAligned()
{
	Purge();
}

//-----------------------------------------------------------------------------
// Attaches the buffer to external memory....
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
void CUtlMemoryAligned< T, nAlignment >::SetExternalBuffer( T* pMemory, int numElements )
{
	// Blow away any existing allocated memory
	Purge();

	CUtlMemory< T >::m_pMemory = (T*)Align( pMemory );
	CUtlMemory< T >::m_nAllocationCount = ( (int)( pMemory + numElements ) - (int)CUtlMemory< T >::m_pMemory ) / sizeof( T );

	// Indicate that we don't own the memory
	CUtlMemory< T >::m_nGrowSize = CUtlMemory< T >::EXTERNAL_BUFFER_MARKER;
}

template < class T, int nAlignment >
void CUtlMemoryAligned< T, nAlignment >::SetExternalBuffer( const T* pMemory, int numElements )
{
	// Blow away any existing allocated memory
	Purge();

	CUtlMemory< T >::m_pMemory = (T*)Align( pMemory );
	CUtlMemory< T >::m_nAllocationCount = ( (int)( pMemory + numElements ) - (int)CUtlMemory< T >::m_pMemory ) / sizeof( T );

	// Indicate that we don't own the memory
	CUtlMemory< T >::m_nGrowSize = CUtlMemory< T >::EXTERNAL_CONST_BUFFER_MARKER;
}

//-----------------------------------------------------------------------------
// Grows the memory
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
void CUtlMemoryAligned< T, nAlignment >::Grow( int num )
{
	assert( num > 0 );

	if ( this->IsExternallyAllocated() )
	{
		// Can't grow a buffer whose memory was externally allocated
		assert( 0 );
		return;
	}

	UTLMEMORY_TRACK_FREE();

	// Make sure we have at least numallocated + num allocations.
	// Use the grow rules specified for this memory (in m_nGrowSize)
	int nAllocationRequested = CUtlMemory< T >::m_nAllocationCount + num;

	CUtlMemory< T >::m_nAllocationCount = UtlMemory_CalcNewAllocationCount( CUtlMemory< T >::m_nAllocationCount, CUtlMemory< T >::m_nGrowSize, nAllocationRequested, sizeof( T ) );

	UTLMEMORY_TRACK_ALLOC();

	if ( CUtlMemory< T >::m_pMemory )
	{
		MEM_ALLOC_CREDIT_CLASS();
		CUtlMemory< T >::m_pMemory = (T*)g_csgo.m_mem_alloc->Realloc( CUtlMemory< T >::m_pMemory, CUtlMemory< T >::m_nAllocationCount * sizeof( T ), nAlignment );
		assert( CUtlMemory< T >::m_pMemory );
	}
	else
	{
		MEM_ALLOC_CREDIT_CLASS();
		CUtlMemory< T >::m_pMemory = (T*)g_csgo.m_mem_alloc->Alloc( CUtlMemory< T >::m_nAllocationCount * sizeof( T ), nAlignment );
		assert( CUtlMemory< T >::m_pMemory );
	}
}

//-----------------------------------------------------------------------------
// Makes sure we've got at least this much memory
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
inline void CUtlMemoryAligned< T, nAlignment >::EnsureCapacity( int num )
{
	if ( CUtlMemory< T >::m_nAllocationCount >= num )
		return;

	if ( this->IsExternallyAllocated() )
	{
		// Can't grow a buffer whose memory was externally allocated
		assert( 0 );
		return;
	}

	UTLMEMORY_TRACK_FREE();

	CUtlMemory< T >::m_nAllocationCount = num;

	UTLMEMORY_TRACK_ALLOC();

	if ( CUtlMemory< T >::m_pMemory )
	{
		MEM_ALLOC_CREDIT_CLASS();
		CUtlMemory< T >::m_pMemory = (T*)g_csgo.m_mem_alloc->Realloc( CUtlMemory< T >::m_pMemory, CUtlMemory< T >::m_nAllocationCount * sizeof( T ), nAlignment );
	}
	else
	{
		MEM_ALLOC_CREDIT_CLASS();
		CUtlMemory< T >::m_pMemory = (T*)g_csgo.m_mem_alloc->Alloc( CUtlMemory< T >::m_nAllocationCount * sizeof( T ), nAlignment );
	}
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
void CUtlMemoryAligned< T, nAlignment >::Purge()
{
	if ( !this->IsExternallyAllocated() )
	{
		if ( CUtlMemory< T >::m_pMemory )
		{
			UTLMEMORY_TRACK_FREE();
			g_csgo.m_mem_alloc->Free( CUtlMemory< T >::m_pMemory );
			CUtlMemory< T >::m_pMemory = 0;
		}
		CUtlMemory< T >::m_nAllocationCount = 0;
	}
}

template< class T, class A = CUtlMemory< T > >
class CUtlVector
{
	typedef A CAllocator;

	typedef T* iterator;
	typedef const T* const_iterator;
public:
	T& operator[]( int i );
	const T& operator[]( int i ) const;

	T& Element( int i )
	{
		return m_Memory[ i ];
	}

	const T& Element( int i ) const
	{
		//assert( IsValidIndex( i ) );
		return m_Memory[ i ];
	}

	T* Base()
	{
		return m_Memory.Base();
	}

	int Count() const
	{
		return m_Size;
	}

	void RemoveAll()
	{
		for ( int i = m_Size; --i >= 0; )
			Destruct( &Element( i ) );

		m_Size = 0;
	}

	bool IsValidIndex( int i ) const
	{
		return ( i >= 0 ) && ( i < m_Size );
	}

	void ShiftElementsRight( int elem, int num = 1 )
	{
		//assert( IsValidIndex( elem ) || ( m_Size == 0 ) || ( num == 0 ) );
		int numToMove = m_Size - elem - num;
		if ( ( numToMove > 0 ) && ( num > 0 ) )
		{
			memmove( &Element( elem + num ), &Element( elem ), numToMove * sizeof( T ) );
		}
	}

	void ShiftElementsLeft( int elem, int num = 1 )
	{
		//assert( IsValidIndex( elem ) || ( m_Size == 0 ) || ( num == 0 ) );
		int numToMove = m_Size - elem - num;
		if ( ( numToMove > 0 ) && ( num > 0 ) )
		{
			memmove( &Element( elem ), &Element( elem + num ), numToMove * sizeof( T ) );
		}
	}

	__forceinline void FillWithValue( const T& src )
	{
		for ( int i = 0; i < Count(); i++ )
		{
			Element( i ) = src;
		}
	}

	void GrowVector( int num = 1 )
	{
		if ( m_Size + num > m_Memory.NumAllocated() )
		{
			m_Memory.Grow( m_Size + num - m_Memory.NumAllocated() );
		}

		m_Size += num;
	}

	int InsertBefore( int elem )
	{
		// Can insert at the end
		//assert( ( elem == Count( ) ) || IsValidIndex( elem ) );

		GrowVector();
		ShiftElementsRight( elem );
		Construct( &Element( elem ) );
		return elem;
	}

	int AddToHead()
	{
		return InsertBefore( 0 );
	}

	int AddToTail()
	{
		return InsertBefore( m_Size );
	}

	int InsertBefore( int elem, const T& src )
	{
		// Can't insert something that's in the list... reallocation may hose us
		assert( ( Base() == NULL ) || ( &src < Base() ) || ( &src >= ( Base() + Count() ) ) );

		// Can insert at the end
		//assert( ( elem == Count( ) ) || IsValidIndex( elem ) );

		GrowVector();
		ShiftElementsRight( elem );
		CopyConstruct( &Element( elem ), src );
		return elem;
	}

	int AddToTail( const T& src )
	{
		// Can't insert something that's in the list... reallocation may hose us
		return InsertBefore( m_Size, src );
	}

	int Find( const T& src ) const
	{
		for ( int i = 0; i < Count(); ++i )
		{
			if ( Element( i ) == src )
			{
				return i;
			}
		}
		return -1;
	}

	void Remove( int elem )
	{
		Destruct( &Element( elem ) );
		ShiftElementsLeft( elem );
		--m_Size;
	}

	bool FindAndRemove( const T& src )
	{
		int elem = Find( src );
		if ( elem != -1 )
		{
			Remove( elem );
			return true;
		}
		return false;
	}

	iterator begin()
	{
		return Base();
	}

	const_iterator begin() const
	{
		return Base();
	}

	iterator end()
	{
		return Base() + Count();
	}

	const_iterator end() const
	{
		return Base() + Count();
	}

	//protected:
	CAllocator m_Memory;
	int m_Size;
	T* m_pElements;
};

template< typename T, class A >
T& CUtlVector< T, A >::operator[]( int i )
{
	assert( i < m_Size );
	return m_Memory[ i ];
}

template< typename T, class A >
const T& CUtlVector< T, A >::operator[]( int i ) const
{
	assert( i < m_Size );
	return m_Memory[ i ];
}

//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#ifndef BITVEC_H
#define BITVEC_H

#include <limits.h>

#define PAD_NUMBER(number, boundary) \
	( ((number) + ((boundary)-1)) / (boundary) ) * (boundary)

class CBitVecAccessor
{
public:
	CBitVecAccessor(unsigned int* pDWords, int iBit);

	void		operator=(int val);
	operator unsigned int();

private:
	unsigned int* m_pDWords;
	int	m_iBit;
};


//-----------------------------------------------------------------------------
// Support functions
//-----------------------------------------------------------------------------

#define LOG2_BITS_PER_INT	5
#define BITS_PER_INT		32

#if _WIN32 && !defined(_X360)
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
#endif

inline int FirstBitInWord(unsigned int elem, int offset)
{
#if _WIN32
	if (!elem)
		return -1;
#if defined( _X360 )
	// this implements CountTrailingZeros() / BitScanForward()
	unsigned int mask = elem - 1;
	unsigned int comp = ~elem;
	elem = mask & comp;
	return (32 - _CountLeadingZeros(elem)) + offset;
#else
	unsigned long out;
	_BitScanForward(&out, elem);
	return out + offset;
#endif

#else
	static unsigned firstBitLUT[256] =
	{
		0,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,0,1,0,2,0,1,0,
		3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
		4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,
		3,0,1,0,2,0,1,0,7,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
		5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,0,1,0,2,0,1,0,
		3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
		4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0
	};
	unsigned elemByte;

	elemByte = (elem & 0xFF);
	if (elemByte)
		return offset + firstBitLUT[elemByte];

	elem >>= 8;
	offset += 8;
	elemByte = (elem & 0xFF);
	if (elemByte)
		return offset + firstBitLUT[elemByte];

	elem >>= 8;
	offset += 8;
	elemByte = (elem & 0xFF);
	if (elemByte)
		return offset + firstBitLUT[elemByte];

	elem >>= 8;
	offset += 8;
	elemByte = (elem & 0xFF);
	if (elemByte)
		return offset + firstBitLUT[elemByte];

	return -1;
#endif
}

//-------------------------------------

inline unsigned GetEndMask(int numBits)
{
	static unsigned bitStringEndMasks[] =
	{
		0xffffffff,
		0x00000001,
		0x00000003,
		0x00000007,
		0x0000000f,
		0x0000001f,
		0x0000003f,
		0x0000007f,
		0x000000ff,
		0x000001ff,
		0x000003ff,
		0x000007ff,
		0x00000fff,
		0x00001fff,
		0x00003fff,
		0x00007fff,
		0x0000ffff,
		0x0001ffff,
		0x0003ffff,
		0x0007ffff,
		0x000fffff,
		0x001fffff,
		0x003fffff,
		0x007fffff,
		0x00ffffff,
		0x01ffffff,
		0x03ffffff,
		0x07ffffff,
		0x0fffffff,
		0x1fffffff,
		0x3fffffff,
		0x7fffffff,
	};

	return bitStringEndMasks[numBits % BITS_PER_INT];
}


inline int GetBitForBitnum(int bitNum)
{
	static int bitsForBitnum[] =
	{
		(1 << 0),
		(1 << 1),
		(1 << 2),
		(1 << 3),
		(1 << 4),
		(1 << 5),
		(1 << 6),
		(1 << 7),
		(1 << 8),
		(1 << 9),
		(1 << 10),
		(1 << 11),
		(1 << 12),
		(1 << 13),
		(1 << 14),
		(1 << 15),
		(1 << 16),
		(1 << 17),
		(1 << 18),
		(1 << 19),
		(1 << 20),
		(1 << 21),
		(1 << 22),
		(1 << 23),
		(1 << 24),
		(1 << 25),
		(1 << 26),
		(1 << 27),
		(1 << 28),
		(1 << 29),
		(1 << 30),
		(1 << 31),
	};

	return bitsForBitnum[(bitNum) & (BITS_PER_INT - 1)];
}

inline int GetBitForBitnumByte(int bitNum)
{
	static int bitsForBitnum[] =
	{
		(1 << 0),
		(1 << 1),
		(1 << 2),
		(1 << 3),
		(1 << 4),
		(1 << 5),
		(1 << 6),
		(1 << 7),
	};

	return bitsForBitnum[bitNum & 7];
}

inline int CalcNumIntsForBits(int numBits) { return (numBits + (BITS_PER_INT - 1)) / BITS_PER_INT; }

// http://bits.stephan-brumme.com/PopulationCount.html
// http://graphics.stanford.edu/~seander/bithacks.html#PopulationCountSetParallel
inline unsigned int PopulationCount(unsigned int v)
{
	unsigned int const w = v - ((v >> 1) & 0x55555555);
	unsigned int const x = (w & 0x33333333) + ((w >> 2) & 0x33333333);
	return ((x + (x >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

inline unsigned int PopulationCount(unsigned long long v)
{
	unsigned long long const w = v - ((v >> 1) & 0x5555555555555555ull);
	unsigned long long const x = (w & 0x3333333333333333ull) + ((w >> 2) & 0x3333333333333333ull);
	return ((x + (x >> 4) & 0x0F0F0F0F0F0F0F0Full) * 0x0101010101010101ull) >> 56; // [Sergiy] I'm not sure if it's faster to multiply here to reduce the bit sum further first, so feel free to optimize, please
}

inline unsigned int PopulationCount(unsigned short v)
{
	unsigned short const w = v - ((v >> 1) & 0x5555);
	unsigned short const x = (w & 0x3333) + ((w >> 2) & 0x3333);
	return ((x + (x >> 4) & 0x0F0F) * 0x101) >> 8;
}


inline unsigned int PopulationCount(unsigned char v)
{
	unsigned char const w = v - ((v >> 1) & 0x55);
	unsigned char const x = (w & 0x33) + ((w >> 2) & 0x33);
	return x + (x >> 4) & 0x0F;
}


#ifdef _X360
#define BitVec_Bit( bitNum ) GetBitForBitnum( bitNum )
#define BitVec_BitInByte( bitNum ) GetBitForBitnumByte( bitNum )
#else
#define BitVec_Bit( bitNum ) ( 1 << ( (bitNum) & (BITS_PER_INT-1) ) )
#define BitVec_BitInByte( bitNum ) ( 1 << ( (bitNum) & 7 ) )
#endif
#define BitVec_Int( bitNum ) ( (bitNum) >> LOG2_BITS_PER_INT )


//-----------------------------------------------------------------------------
// template CBitVecT
//
// Defines the operations relevant to any bit array. Simply requires a base
// class that implements GetNumBits(), Base(), GetNumDWords() & ValidateOperand()
//
// CVarBitVec and CBitVec<int> are the actual classes generally used
// by clients
//

template <class BASE_OPS>
class CBitVecT : public BASE_OPS
{
public:
	CBitVecT();
	explicit CBitVecT(int numBits);			// Must be initialized with the number of bits

	void	Init(int val = 0);

	// Access the bits like an array.
	CBitVecAccessor	operator[](int i);
	unsigned int operator[](int i)const;

	// Do NOT override bitwise operators (see note in header)
	void	And(const CBitVecT& andStr, CBitVecT* out) const;
	void	Or(const CBitVecT& orStr, CBitVecT* out) const;
	void	Xor(const CBitVecT& orStr, CBitVecT* out) const;

	void	Not(CBitVecT* out) const;

	void	CopyTo(CBitVecT* out) const;
	void	Copy(const CBitVecT<BASE_OPS>& other, int nBits = -1);
	bool	Compare(const CBitVecT<BASE_OPS>& other, int nBits = -1) const;

	bool	IsAllClear(void) const;		// Are all bits zero?
	bool	IsAllSet(void) const;		// Are all bits one?

	unsigned int	Get(unsigned int bitNum) const;
	bool 	IsBitSet(int bitNum) const;
	void 	Set(int bitNum);
	void 	Set(int bitNum, bool bNewVal);
	void 	Clear(int bitNum);

	bool	TestAndSet(int bitNum);

	void 	Set(unsigned int offset, unsigned int mask);
	void 	Clear(unsigned int offset, unsigned int mask);
	unsigned int	Get(unsigned int offset, unsigned int mask);

	void	SetAll(void);			// Sets all bits
	void	ClearAll(void);			// Clears all bits

	unsigned int	GetDWord(int i) const;
	void	SetDWord(int i, unsigned int val);
	unsigned int    PopulationCount()const;

	CBitVecT<BASE_OPS>& operator=(const CBitVecT<BASE_OPS>& other) { other.CopyTo(this); return *this; }
	bool			operator==(const CBitVecT<BASE_OPS>& other) { return Compare(other); }
	bool			operator!=(const CBitVecT<BASE_OPS>& other) { return !operator==(other); }

	static void GetOffsetMaskForBit(unsigned int bitNum, unsigned int* pOffset, unsigned int* pMask) { *pOffset = BitVec_Int(bitNum); *pMask = BitVec_Bit(bitNum); }
};

//-----------------------------------------------------------------------------
// class CVarBitVecBase
//
// Defines the operations necessary for a variable sized bit array
template <typename BITCOUNTTYPE>
class CVarBitVecBase
{
public:
	bool	IsFixedSize() const { return false; }
	int		GetNumBits(void) const { return m_numBits; }
	void	Resize(int numBits, bool bClearAll = false);		// resizes bit array

	int 	GetNumDWords() const { return m_numInts; }
	unsigned int* Base() { return m_pInt; }
	const unsigned int* Base() const { return m_pInt; }

	void Attach(unsigned int* pBits, int numBits);
	bool Detach(unsigned int** ppBits, int* pNumBits);

	int		FindNextSetBit(int iStartBit) const; // returns -1 if no set bit was found

protected:
	CVarBitVecBase();
	explicit CVarBitVecBase(int numBits);
	CVarBitVecBase(const CVarBitVecBase<BITCOUNTTYPE>& from);
	CVarBitVecBase& operator=(const CVarBitVecBase<BITCOUNTTYPE>& from);
	~CVarBitVecBase(void);

	void 		ValidateOperand(const CVarBitVecBase<BITCOUNTTYPE>& operand) const { assert(GetNumBits() == operand.GetNumBits()); }

	unsigned	GetEndMask() const { return ::GetEndMask(GetNumBits()); }

private:

	BITCOUNTTYPE	m_numBits;					// Number of bits in the bitstring
	BITCOUNTTYPE	m_numInts;					// Number of ints to needed to store bitstring
	unsigned int			m_iBitStringStorage;		// If the bit string fits in one int, it goes here
	unsigned int* m_pInt;					// Array of ints containing the bitstring

	void	AllocInts(int numInts);	// Free the allocated bits
	void	ReallocInts(int numInts);
	void	FreeInts(void);			// Free the allocated bits
};

//-----------------------------------------------------------------------------
// class CFixedBitVecBase
//
// Defines the operations necessary for a fixed sized bit array. 
// 

template <int bits> struct BitCountToEndMask_t {};
template <> struct BitCountToEndMask_t< 0> { enum { MASK = 0xffffffff }; };
template <> struct BitCountToEndMask_t< 1> { enum { MASK = 0x00000001 }; };
template <> struct BitCountToEndMask_t< 2> { enum { MASK = 0x00000003 }; };
template <> struct BitCountToEndMask_t< 3> { enum { MASK = 0x00000007 }; };
template <> struct BitCountToEndMask_t< 4> { enum { MASK = 0x0000000f }; };
template <> struct BitCountToEndMask_t< 5> { enum { MASK = 0x0000001f }; };
template <> struct BitCountToEndMask_t< 6> { enum { MASK = 0x0000003f }; };
template <> struct BitCountToEndMask_t< 7> { enum { MASK = 0x0000007f }; };
template <> struct BitCountToEndMask_t< 8> { enum { MASK = 0x000000ff }; };
template <> struct BitCountToEndMask_t< 9> { enum { MASK = 0x000001ff }; };
template <> struct BitCountToEndMask_t<10> { enum { MASK = 0x000003ff }; };
template <> struct BitCountToEndMask_t<11> { enum { MASK = 0x000007ff }; };
template <> struct BitCountToEndMask_t<12> { enum { MASK = 0x00000fff }; };
template <> struct BitCountToEndMask_t<13> { enum { MASK = 0x00001fff }; };
template <> struct BitCountToEndMask_t<14> { enum { MASK = 0x00003fff }; };
template <> struct BitCountToEndMask_t<15> { enum { MASK = 0x00007fff }; };
template <> struct BitCountToEndMask_t<16> { enum { MASK = 0x0000ffff }; };
template <> struct BitCountToEndMask_t<17> { enum { MASK = 0x0001ffff }; };
template <> struct BitCountToEndMask_t<18> { enum { MASK = 0x0003ffff }; };
template <> struct BitCountToEndMask_t<19> { enum { MASK = 0x0007ffff }; };
template <> struct BitCountToEndMask_t<20> { enum { MASK = 0x000fffff }; };
template <> struct BitCountToEndMask_t<21> { enum { MASK = 0x001fffff }; };
template <> struct BitCountToEndMask_t<22> { enum { MASK = 0x003fffff }; };
template <> struct BitCountToEndMask_t<23> { enum { MASK = 0x007fffff }; };
template <> struct BitCountToEndMask_t<24> { enum { MASK = 0x00ffffff }; };
template <> struct BitCountToEndMask_t<25> { enum { MASK = 0x01ffffff }; };
template <> struct BitCountToEndMask_t<26> { enum { MASK = 0x03ffffff }; };
template <> struct BitCountToEndMask_t<27> { enum { MASK = 0x07ffffff }; };
template <> struct BitCountToEndMask_t<28> { enum { MASK = 0x0fffffff }; };
template <> struct BitCountToEndMask_t<29> { enum { MASK = 0x1fffffff }; };
template <> struct BitCountToEndMask_t<30> { enum { MASK = 0x3fffffff }; };
template <> struct BitCountToEndMask_t<31> { enum { MASK = 0x7fffffff }; };

//-------------------------------------

template <int NUM_BITS>
class CFixedBitVecBase
{
public:
	bool	IsFixedSize() const { return true; }
	int		GetNumBits(void) const { return NUM_BITS; }
	void	Resize(int numBits, bool bClearAll = false) { assert(numBits == NUM_BITS); if (bClearAll) memset(m_Ints, 0, NUM_INTS * sizeof(unsigned int)); }// for syntatic consistency (for when using templates)

	int 			GetNumDWords() const { return NUM_INTS; }
	unsigned int* Base() { return m_Ints; }
	const unsigned int* Base() const { return m_Ints; }

	int		FindNextSetBit(int iStartBit) const; // returns -1 if no set bit was found

protected:
	CFixedBitVecBase() {}
	explicit CFixedBitVecBase(int numBits)
	{
		// doesn't make sense, really. Supported to simplify templates & allow easy replacement of variable 
		assert(numBits == NUM_BITS);
	}

	void 		ValidateOperand(const CFixedBitVecBase<NUM_BITS>& operand) const {} // no need, compiler does so statically

public: // for test code
	unsigned	GetEndMask() const { return static_cast<unsigned>(BitCountToEndMask_t<NUM_BITS % BITS_PER_INT>::MASK); }

private:
	enum
	{
		NUM_INTS = (NUM_BITS + (BITS_PER_INT - 1)) / BITS_PER_INT
	};

	unsigned int m_Ints[(NUM_BITS + (BITS_PER_INT - 1)) / BITS_PER_INT];
};

//-----------------------------------------------------------------------------
//
// The actual classes used
//

// inheritance instead of typedef to allow forward declarations
template< class CountType = unsigned short >
class CVarBitVecT : public CBitVecT< CVarBitVecBase< CountType > >
{
public:
	CVarBitVecT()
	{
	}

	explicit CVarBitVecT(int numBits)
		: CBitVecT<CVarBitVecBase< CountType > >(numBits)
	{
	}
};

class CVarBitVec : public CVarBitVecT<unsigned short>
{
public:
	CVarBitVec() : CVarBitVecT<unsigned short>() {}
	explicit CVarBitVec(int nBitCount) : CVarBitVecT<unsigned short>(nBitCount) {}
};

class CLargeVarBitVec : public CBitVecT< CVarBitVecBase<int> >
{
public:
	CLargeVarBitVec()
	{
	}

	explicit CLargeVarBitVec(int numBits)
		: CBitVecT< CVarBitVecBase<int> >(numBits)
	{
	}
};

//-----------------------------------------------------------------------------

template < int NUM_BITS >
class CBitVec : public CBitVecT< CFixedBitVecBase<NUM_BITS> >
{
public:
	CBitVec()
	{
	}

	explicit CBitVec(int numBits)
		: CBitVecT< CFixedBitVecBase<NUM_BITS> >(numBits)
	{
	}
};


//-----------------------------------------------------------------------------

typedef CBitVec<32> CDWordBitVec;

//-----------------------------------------------------------------------------

template <typename BITCOUNTTYPE>
inline CVarBitVecBase<BITCOUNTTYPE>::CVarBitVecBase()
{
	memset(this, 0, sizeof(*this));
}

//-----------------------------------------------------------------------------

template <typename BITCOUNTTYPE>
inline CVarBitVecBase<BITCOUNTTYPE>::CVarBitVecBase(int numBits)
{
	assert(numBits);
	m_numBits = numBits;

	// Figure out how many ints are needed
	m_numInts = CalcNumIntsForBits(numBits);
	m_pInt = NULL;
	AllocInts(m_numInts);
}

//-----------------------------------------------------------------------------

template <typename BITCOUNTTYPE>
inline CVarBitVecBase<BITCOUNTTYPE>::CVarBitVecBase(const CVarBitVecBase<BITCOUNTTYPE>& from)
{
	if (from.m_numInts)
	{
		m_numBits = from.m_numBits;
		m_numInts = from.m_numInts;
		m_pInt = NULL;
		AllocInts(m_numInts);
		memcpy(m_pInt, from.m_pInt, m_numInts * sizeof(int));
	}
	else
		memset(this, 0, sizeof(*this));
}

//-----------------------------------------------------------------------------

template <typename BITCOUNTTYPE>
inline CVarBitVecBase<BITCOUNTTYPE>& CVarBitVecBase<BITCOUNTTYPE>::operator=(const CVarBitVecBase<BITCOUNTTYPE>& from)
{
	Resize(from.GetNumBits());
	if (m_pInt)
		memcpy(m_pInt, from.m_pInt, m_numInts * sizeof(int));
	return (*this);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
// Input  :
// Output :
//-----------------------------------------------------------------------------

template <typename BITCOUNTTYPE>
inline CVarBitVecBase<BITCOUNTTYPE>::~CVarBitVecBase(void)
{
	FreeInts();
}

//-----------------------------------------------------------------------------

template <typename BITCOUNTTYPE>
inline void CVarBitVecBase<BITCOUNTTYPE>::Attach(unsigned int* pBits, int numBits)
{
	FreeInts();
	m_numBits = numBits;
	m_numInts = CalcNumIntsForBits(numBits);
	if (m_numInts > 1)
	{
		m_pInt = pBits;
	}
	else
	{
		m_iBitStringStorage = *pBits;
		m_pInt = &m_iBitStringStorage;
		free(pBits);
	}
}

//-----------------------------------------------------------------------------

template <typename BITCOUNTTYPE>
inline bool CVarBitVecBase<BITCOUNTTYPE>::Detach(unsigned int** ppBits, int* pNumBits)
{
	if (!m_numBits)
	{
		return false;
	}

	*pNumBits = m_numBits;
	if (m_numInts > 1)
	{
		*ppBits = m_pInt;
	}
	else
	{
		*ppBits = (unsigned int*)malloc(sizeof(unsigned int));
		**ppBits = m_iBitStringStorage;
		free(m_pInt);
	}

	memset(this, 0, sizeof(*this));
	return true;
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline CBitVecT<BASE_OPS>::CBitVecT()
{
	// undef this is ints are not 4 bytes
	// generate a compile error if sizeof(int) is not 4 (HACK: can't use the preprocessor so use the compiler)

	static_assert(sizeof(int) == 4);

	// Initialize bitstring by clearing all bits
	ClearAll();
}

//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline CBitVecT<BASE_OPS>::CBitVecT(int numBits)
	: BASE_OPS(numBits)
{
	// undef this is ints are not 4 bytes
	// generate a compile error if sizeof(int) is not 4 (HACK: can't use the preprocessor so use the compiler)

	static_assert(sizeof(int) == 4);

	// Initialize bitstring by clearing all bits
	ClearAll();
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline CBitVecAccessor CBitVecT<BASE_OPS>::operator[](int i)
{
	assert(i >= 0 && i < this->GetNumBits());
	return CBitVecAccessor(this->Base(), i);
}

template <class BASE_OPS>
inline unsigned int CBitVecT<BASE_OPS>::operator[](int i)const
{
	assert(i >= 0 && i < this->GetNumBits());
	return this->Base()[i >> 5] & (1 << (i & 31));
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::Init(int val)
{
	if (this->Base())
		memset(this->Base(), (val) ? 0xff : 0, this->GetNumDWords() * sizeof(int));
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline unsigned int CBitVecT<BASE_OPS>::Get(unsigned int bitNum) const
{
	assert(bitNum < (unsigned int)this->GetNumBits());
	const unsigned int* pInt = this->Base() + BitVec_Int(bitNum);
	return (*pInt & BitVec_Bit(bitNum));
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline bool CBitVecT<BASE_OPS>::IsBitSet(int bitNum) const
{
	assert(bitNum >= 0 && bitNum < this->GetNumBits());
	const unsigned int* pInt = this->Base() + BitVec_Int(bitNum);
	return ((*pInt & BitVec_Bit(bitNum)) != 0);
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::Set(int bitNum)
{
	assert(bitNum >= 0 && bitNum < this->GetNumBits());
	unsigned int* pInt = this->Base() + BitVec_Int(bitNum);
	*pInt |= BitVec_Bit(bitNum);
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline bool CBitVecT<BASE_OPS>::TestAndSet(int bitNum)
{
	assert(bitNum >= 0 && bitNum < this->GetNumBits());
	unsigned int bitVecBit = BitVec_Bit(bitNum);
	unsigned int* pInt = this->Base() + BitVec_Int(bitNum);
	bool bResult = ((*pInt & bitVecBit) != 0);
	*pInt |= bitVecBit;
	return bResult;
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::Clear(int bitNum)
{
	assert(bitNum >= 0 && bitNum < this->GetNumBits());
	unsigned int* pInt = this->Base() + BitVec_Int(bitNum);
	*pInt &= ~BitVec_Bit(bitNum);
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::Set(int bitNum, bool bNewVal)
{
	unsigned int* pInt = this->Base() + BitVec_Int(bitNum);
	unsigned int bitMask = BitVec_Bit(bitNum);
	if (bNewVal)
	{
		*pInt |= bitMask;
	}
	else
	{
		*pInt &= ~bitMask;
	}
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::Set(unsigned int offset, unsigned int mask)
{
	unsigned int* pInt = this->Base() + offset;
	*pInt |= mask;
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::Clear(unsigned int offset, unsigned int mask)
{
	unsigned int* pInt = this->Base() + offset;
	*pInt &= ~mask;
}

//-----------------------------------------------------------------------------

template <class BASE_OPS>
inline unsigned int CBitVecT<BASE_OPS>::Get(unsigned int offset, unsigned int mask)
{
	unsigned int* pInt = this->Base() + offset;
	return (*pInt & mask);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::And(const CBitVecT& addStr, CBitVecT* out) const
{
	ValidateOperand(addStr);
	ValidateOperand(*out);

	unsigned int* pDest = out->Base();
	const unsigned int* pOperand1 = this->Base();
	const unsigned int* pOperand2 = addStr.Base();

	for (int i = this->GetNumDWords() - 1; i >= 0; --i)
	{
		pDest[i] = pOperand1[i] & pOperand2[i];
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::Or(const CBitVecT& orStr, CBitVecT* out) const
{
	ValidateOperand(orStr);
	ValidateOperand(*out);

	unsigned int* pDest = out->Base();
	const unsigned int* pOperand1 = this->Base();
	const unsigned int* pOperand2 = orStr.Base();

	for (int i = this->GetNumDWords() - 1; i >= 0; --i)
	{
		pDest[i] = pOperand1[i] | pOperand2[i];
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::Xor(const CBitVecT& xorStr, CBitVecT* out) const
{
	unsigned int* pDest = out->Base();
	const unsigned int* pOperand1 = this->Base();
	const unsigned int* pOperand2 = xorStr.Base();

	for (int i = this->GetNumDWords() - 1; i >= 0; --i)
	{
		pDest[i] = pOperand1[i] ^ pOperand2[i];
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::Not(CBitVecT* out) const
{
	this->ValidateOperand(*out);

	unsigned int* pDest = out->Base();
	const unsigned int* pOperand = this->Base();

	for (int i = this->GetNumDWords() - 1; i >= 0; --i)
	{
		pDest[i] = ~(pOperand[i]);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Copy a bit string
// Input  :
// Output :
//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::CopyTo(CBitVecT* out) const
{
	out->Resize(this->GetNumBits());

	this->ValidateOperand(*out);
	assert(out != this);

	memcpy(out->Base(), this->Base(), this->GetNumDWords() * sizeof(int));
}

//-----------------------------------------------------------------------------
// Purpose: Are all bits zero?
// Input  :
// Output :
//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline bool CBitVecT<BASE_OPS>::IsAllClear(void) const
{
	// Number of available bits may be more than the number
	// actually used, so make sure to mask out unused bits
	// before testing for zero
	(const_cast<CBitVecT*>(this))->Base()[this->GetNumDWords() - 1] &= CBitVecT<BASE_OPS>::GetEndMask(); // external semantics of const retained

	for (int i = this->GetNumDWords() - 1; i >= 0; --i)
	{
		if (this->Base()[i] != 0)
		{
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Are all bits set?
// Input  :
// Output :
//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline bool CBitVecT<BASE_OPS>::IsAllSet(void) const
{
	// Number of available bits may be more than the number
	// actually used, so make sure to mask out unused bits
	// before testing for set bits
	(const_cast<CBitVecT*>(this))->Base()[this->GetNumDWords() - 1] |= ~CBitVecT<BASE_OPS>::GetEndMask();  // external semantics of const retained

	for (int i = this->GetNumDWords() - 1; i >= 0; --i)
	{
		if (this->Base()[i] != ~0)
		{
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Sets all bits
// Input  :
// Output :
//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::SetAll(void)
{
	if (this->Base())
		memset(this->Base(), 0xff, this->GetNumDWords() * sizeof(int));
}

//-----------------------------------------------------------------------------
// Purpose: Clears all bits
// Input  :
// Output :
//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::ClearAll(void)
{
	if (this->Base())
		memset(this->Base(), 0, this->GetNumDWords() * sizeof(int));
}

//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::Copy(const CBitVecT<BASE_OPS>& other, int nBits)
{
	if (nBits == -1)
	{
		nBits = other.GetNumBits();
	}

	this->Resize(nBits);

	this->ValidateOperand(other);
	assert(&other != this);

	memcpy(this->Base(), other.Base(), this->GetNumDWords() * sizeof(unsigned int));
}

//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline bool CBitVecT<BASE_OPS>::Compare(const CBitVecT<BASE_OPS>& other, int nBits) const
{
	if (nBits == -1)
	{
		if (other.GetNumBits() != this->GetNumBits())
		{
			return false;
		}

		nBits = other.GetNumBits();
	}

	if (nBits > other.GetNumBits() || nBits > this->GetNumBits())
	{
		return false;
	}

	(const_cast<CBitVecT*>(this))->Base()[this->GetNumDWords() - 1] &= CBitVecT<BASE_OPS>::GetEndMask(); // external semantics of const retained
	(const_cast<CBitVecT*>(&other))->Base()[this->GetNumDWords() - 1] &= other.CBitVecT<BASE_OPS>::GetEndMask(); // external semantics of const retained

	int nBytes = PAD_NUMBER(nBits, 8) >> 3;

	return (memcmp(this->Base(), other.Base(), nBytes) == 0);
}

//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline unsigned int CBitVecT<BASE_OPS>::GetDWord(int i) const
{
	assert(i >= 0 && i < this->GetNumDWords());
	return this->Base()[i];
}

//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline void CBitVecT<BASE_OPS>::SetDWord(int i, unsigned int val)
{
	assert(i >= 0 && i < this->GetNumDWords());
	this->Base()[i] = val;
}

//-----------------------------------------------------------------------------
template <class BASE_OPS>
inline unsigned int CBitVecT<BASE_OPS>::PopulationCount() const
{
	int nDwordCount = this->GetNumDWords();
	const unsigned int* pBase = this->Base();
	unsigned int nCount = 0;
	for (int i = 0; i < nDwordCount; ++i)
	{
		nCount += ::PopulationCount(pBase[i]);
	}
	return nCount;
}

//-----------------------------------------------------------------------------

inline unsigned GetStartBitMask(int startBit)
{
	static unsigned int g_StartMask[32] =
	{
		0xffffffff,
		0xfffffffe,
		0xfffffffc,
		0xfffffff8,
		0xfffffff0,
		0xffffffe0,
		0xffffffc0,
		0xffffff80,
		0xffffff00,
		0xfffffe00,
		0xfffffc00,
		0xfffff800,
		0xfffff000,
		0xffffe000,
		0xffffc000,
		0xffff8000,
		0xffff0000,
		0xfffe0000,
		0xfffc0000,
		0xfff80000,
		0xfff00000,
		0xffe00000,
		0xffc00000,
		0xff800000,
		0xff000000,
		0xfe000000,
		0xfc000000,
		0xf8000000,
		0xf0000000,
		0xe0000000,
		0xc0000000,
		0x80000000,
	};

	return g_StartMask[startBit & 31];
}


#ifdef _X360
#define BitVec_Bit( bitNum ) GetBitForBitnum( bitNum )
#define BitVec_BitInByte( bitNum ) GetBitForBitnumByte( bitNum )
#else
#define BitVec_Bit( bitNum ) ( 1 << ( (bitNum) & (BITS_PER_INT-1) ) )
#define BitVec_BitInByte( bitNum ) ( 1 << ( (bitNum) & 7 ) )
#endif
#define BitVec_Int( bitNum ) ( (bitNum) >> LOG2_BITS_PER_INT )

inline bool BitVec_IsBitSet(const unsigned int* pBase, int nBitNum)
{
	const unsigned int* pInt = pBase + BitVec_Int(nBitNum);
	return ((*pInt & BitVec_Bit(nBitNum)) != 0);
}

inline void Bitvec_Set(unsigned int* pBits, int nBitNum)
{
	unsigned int* pInt = pBits + BitVec_Int(nBitNum);
	*pInt |= BitVec_Bit(nBitNum);
}

inline void Bitvec_Unset(unsigned int* pBits, int nBitNum)
{
	unsigned int* pInt = pBits + BitVec_Int(nBitNum);
	*pInt &= ~BitVec_Bit(nBitNum);
}

inline bool BitVec_TestAndSetBit(unsigned int* pBase, int nBitNum)
{
	unsigned int* pInt = pBase + BitVec_Int(nBitNum);
	unsigned int nBits = *pInt;
	unsigned int nSetMask = BitVec_Bit(nBitNum);
	*pInt = nBits | nSetMask;
	return (nBits & nSetMask) ? true : false;
}

inline void BitVec_ClearAll(unsigned int* pDst, int nDWords, unsigned int nEndMask)
{
	if (nDWords > 1)
	{
		memset(pDst, 0, (nDWords - 1) * sizeof(unsigned int));
	}
	pDst[nDWords - 1] &= ~nEndMask;
}

inline void BitVec_ClearAll(unsigned int* pDst, int nDWords)
{
	memset(pDst, 0, nDWords * sizeof(unsigned int));
}

inline void BitVec_SetAll(unsigned int* pDst, int nDWords, unsigned int nEndMask)
{
	if (nDWords > 1)
	{
		memset(pDst, 0xff, (nDWords - 1) * sizeof(unsigned int));
	}
	pDst[nDWords - 1] |= nEndMask;
}

inline void BitVec_SetAll(unsigned int* pDst, int nDWords)
{
	memset(pDst, 0xff, nDWords * sizeof(unsigned int));
}

inline void BitVec_Or(unsigned int* pDst, const unsigned int* pSrc, int nDWords, unsigned int nEndMask)
{
	unsigned int* pEnd = pDst + nDWords - 1;
	for (; pDst < pEnd; ++pSrc, ++pDst)
	{
		*pDst = *pDst | *pSrc;
	}
	*pDst = *pDst | (*pSrc & nEndMask);
}

inline void BitVec_Or(unsigned int* pDst, const unsigned int* pSrc, int nDWords)
{
	unsigned int* pEnd = pDst + nDWords;
	for (; pDst < pEnd; ++pSrc, ++pDst)
	{
		*pDst = *pDst | *pSrc;
	}
}

inline void BitVec_AndNot(unsigned int* pDst, const unsigned int* pSrc, const unsigned int* pAndNot, int nDWords)
{
	unsigned int* pEnd = pDst + nDWords;
	for (; pDst < pEnd; ++pSrc, ++pDst, ++pAndNot)
	{
		*pDst = *pSrc & ~(*pAndNot);
	}
}

inline bool BitVec_IsAnySet(const unsigned int* pDst, int nDWords)
{
	const unsigned int* pEnd = pDst + nDWords;
	for (; pDst < pEnd; ++pDst)
	{
		if (*pDst != 0)
			return true;
	}
	return false;
}

inline int BitVec_CountNewBits(const unsigned int* pOld, const unsigned int* pNew, int nDWords, unsigned int nEndMask)
{
	// NOTE - this assumes that any unused bits are unchanged between pOld and pNew

	int nNewBits = 0;

	const unsigned int* pEnd = pNew + nDWords - 1;
	for (; pNew < pEnd; ++pOld, ++pNew)
	{
		unsigned int diff = *pNew & ~*pOld;
		nNewBits += PopulationCount(diff);
	}
	nNewBits += PopulationCount((*pNew & ~*pOld) & nEndMask);

	return nNewBits;
}


//template <typename BITCOUNTTYPE>
//inline int CVarBitVecBase<BITCOUNTTYPE>::FindNextSetBit(int startBit) const
//{
//	if (startBit < GetNumBits())
//	{
//		int wordIndex = BitVec_Int(startBit);
//		unsigned int startMask = GetStartBitMask(startBit);
//		int lastWord = GetNumDWords() - 1;
//
//		// handle non dword lengths
//		if ((GetNumBits() % BITS_PER_INT) != 0)
//		{
//			unsigned int elem = Base()[wordIndex];
//			elem &= startMask;
//			if (wordIndex == lastWord)
//			{
//				elem &= (GetEndMask());
//				// there's a bit remaining in this word
//				if (elem)
//					return FirstBitInWord(elem, wordIndex << 5);
//			}
//			else
//			{
//				// there's a bit remaining in this word
//				if (elem)
//					return FirstBitInWord(elem, wordIndex << 5);
//
//				// iterate the words
//				for (int i = wordIndex + 1; i < lastWord; i++)
//				{
//					elem = Base()[i];
//					if (elem)
//						return FirstBitInWord(elem, i << 5);
//				}
//				elem = Base()[lastWord] & GetEndMask();
//				if (elem)
//					return FirstBitInWord(elem, lastWord << 5);
//			}
//		}
//		else
//		{
//			const unsigned int* RESTRICT pCurElem = Base() + wordIndex;
//			unsigned int elem = *pCurElem;
//			elem &= startMask;
//			do
//			{
//				if (elem)
//					return FirstBitInWord(elem, wordIndex << 5);
//				++pCurElem;
//				elem = *pCurElem;
//				++wordIndex;
//			} while (wordIndex <= lastWord);
//		}
//
//	}
//
//	return -1;
//}

//template <int NUM_BITS>
//inline int CFixedBitVecBase<NUM_BITS>::FindNextSetBit(int startBit) const
//{
//	if (startBit < NUM_BITS)
//	{
//		int wordIndex = BitVec_Int(startBit);
//		unsigned int startMask = GetStartBitMask(startBit);
//
//		// handle non dword lengths
//		if ((NUM_BITS % BITS_PER_INT) != 0)
//		{
//			unsigned int elem = Base()[wordIndex];
//			elem &= startMask;
//			if (wordIndex == NUM_INTS - 1)
//			{
//				elem &= (GetEndMask());
//				// there's a bit remaining in this word
//				if (elem)
//					return FirstBitInWord(elem, wordIndex << 5);
//			}
//			else
//			{
//				// there's a bit remaining in this word
//				if (elem)
//					return FirstBitInWord(elem, wordIndex << 5);
//
//				// iterate the words
//				for (int i = wordIndex + 1; i < NUM_INTS - 1; i++)
//				{
//					elem = Base()[i];
//					if (elem)
//						return FirstBitInWord(elem, i << 5);
//				}
//				elem = Base()[NUM_INTS - 1] & GetEndMask();
//				if (elem)
//					return FirstBitInWord(elem, (NUM_INTS - 1) << 5);
//			}
//		}
//		else
//		{
//			const unsigned int* RESTRICT pCurElem = Base() + wordIndex;
//			unsigned int elem = *pCurElem;
//			elem &= startMask;
//			do
//			{
//				if (elem)
//					return FirstBitInWord(elem, wordIndex << 5);
//				++pCurElem;
//				elem = *pCurElem;
//				++wordIndex;
//			} while (wordIndex <= NUM_INTS - 1);
//		}
//
//	}
//
//	return -1;
//}

//-----------------------------------------------------------------------------
// Unrolled loops for some common sizes

template<>
__forceinline void CBitVecT< CFixedBitVecBase<256> >::And(const CBitVecT& addStr, CBitVecT* out) const
{
	unsigned int* pDest = out->Base();
	const unsigned int* pOperand1 = Base();
	const unsigned int* pOperand2 = addStr.Base();

	pDest[0] = pOperand1[0] & pOperand2[0];
	pDest[1] = pOperand1[1] & pOperand2[1];
	pDest[2] = pOperand1[2] & pOperand2[2];
	pDest[3] = pOperand1[3] & pOperand2[3];
	pDest[4] = pOperand1[4] & pOperand2[4];
	pDest[5] = pOperand1[5] & pOperand2[5];
	pDest[6] = pOperand1[6] & pOperand2[6];
	pDest[7] = pOperand1[7] & pOperand2[7];
}

template<>
__forceinline  bool CBitVecT< CFixedBitVecBase<256> >::IsAllClear(void) const
{
	const unsigned int* pInts = Base();
	return (pInts[0] == 0 && pInts[1] == 0 && pInts[2] == 0 && pInts[3] == 0 && pInts[4] == 0 && pInts[5] == 0 && pInts[6] == 0 && pInts[7] == 0);
}

template<>
__forceinline  void CBitVecT< CFixedBitVecBase<256> >::CopyTo(CBitVecT* out) const
{
	unsigned int* pDest = out->Base();
	const unsigned int* pInts = Base();

	pDest[0] = pInts[0];
	pDest[1] = pInts[1];
	pDest[2] = pInts[2];
	pDest[3] = pInts[3];
	pDest[4] = pInts[4];
	pDest[5] = pInts[5];
	pDest[6] = pInts[6];
	pDest[7] = pInts[7];
}

template<>
__forceinline  void CBitVecT< CFixedBitVecBase<128> >::And(const CBitVecT& addStr, CBitVecT* out) const
{
	unsigned int* pDest = out->Base();
	const unsigned int* pOperand1 = Base();
	const unsigned int* pOperand2 = addStr.Base();

	pDest[0] = pOperand1[0] & pOperand2[0];
	pDest[1] = pOperand1[1] & pOperand2[1];
	pDest[2] = pOperand1[2] & pOperand2[2];
	pDest[3] = pOperand1[3] & pOperand2[3];
}

template<>
__forceinline  bool CBitVecT< CFixedBitVecBase<128> >::IsAllClear(void) const
{
	const unsigned int* pInts = Base();
	return (pInts[0] == 0 && pInts[1] == 0 && pInts[2] == 0 && pInts[3] == 0);
}

template<>
__forceinline  void CBitVecT< CFixedBitVecBase<128> >::CopyTo(CBitVecT* out) const
{
	unsigned int* pDest = out->Base();
	const unsigned int* pInts = Base();

	pDest[0] = pInts[0];
	pDest[1] = pInts[1];
	pDest[2] = pInts[2];
	pDest[3] = pInts[3];
}

template<>
inline void CBitVecT< CFixedBitVecBase<96> >::And(const CBitVecT& addStr, CBitVecT* out) const
{
	unsigned int* pDest = out->Base();
	const unsigned int* pOperand1 = Base();
	const unsigned int* pOperand2 = addStr.Base();

	pDest[0] = pOperand1[0] & pOperand2[0];
	pDest[1] = pOperand1[1] & pOperand2[1];
	pDest[2] = pOperand1[2] & pOperand2[2];
}

template<>
inline bool CBitVecT< CFixedBitVecBase<96> >::IsAllClear(void) const
{
	const unsigned int* pInts = Base();
	return (pInts[0] == 0 && pInts[1] == 0 && pInts[2] == 0);
}

template<>
inline void CBitVecT< CFixedBitVecBase<96> >::CopyTo(CBitVecT* out) const
{
	unsigned int* pDest = out->Base();
	const unsigned int* pInts = Base();

	pDest[0] = pInts[0];
	pDest[1] = pInts[1];
	pDest[2] = pInts[2];
}

template<>
inline void CBitVecT< CFixedBitVecBase<64> >::And(const CBitVecT& addStr, CBitVecT* out) const
{
	unsigned int* pDest = out->Base();
	const unsigned int* pOperand1 = Base();
	const unsigned int* pOperand2 = addStr.Base();

	pDest[0] = pOperand1[0] & pOperand2[0];
	pDest[1] = pOperand1[1] & pOperand2[1];
}

template<>
inline bool CBitVecT< CFixedBitVecBase<64> >::IsAllClear(void) const
{
	const unsigned int* pInts = Base();
	return (pInts[0] == 0 && pInts[1] == 0);
}

template<>
inline void CBitVecT< CFixedBitVecBase<64> >::CopyTo(CBitVecT* out) const
{
	unsigned int* pDest = out->Base();
	const unsigned int* pInts = Base();

	pDest[0] = pInts[0];
	pDest[1] = pInts[1];
}

template<>
inline void CBitVecT< CFixedBitVecBase<32> >::And(const CBitVecT& addStr, CBitVecT* out) const
{
	unsigned int* pDest = out->Base();
	const unsigned int* pOperand1 = Base();
	const unsigned int* pOperand2 = addStr.Base();

	pDest[0] = pOperand1[0] & pOperand2[0];
}

template<>
inline bool CBitVecT< CFixedBitVecBase<32> >::IsAllClear(void) const
{
	const unsigned int* pInts = Base();

	return (pInts[0] == 0);
}

template<>
inline void CBitVecT< CFixedBitVecBase<32> >::CopyTo(CBitVecT* out) const
{
	unsigned int* pDest = out->Base();
	const unsigned int* pInts = Base();

	pDest[0] = pInts[0];
}

//-----------------------------------------------------------------------------

template <>
inline unsigned int CBitVecT< CFixedBitVecBase<32> >::Get(unsigned int bitNum) const
{
	return (*Base() & BitVec_Bit(bitNum));
}

//-----------------------------------------------------------------------------

template <>
inline bool CBitVecT< CFixedBitVecBase<32> >::IsBitSet(int bitNum) const
{
	return ((*Base() & BitVec_Bit(bitNum)) != 0);
}

//-----------------------------------------------------------------------------

template <>
inline void CBitVecT< CFixedBitVecBase<32> >::Set(int bitNum)
{
	*Base() |= BitVec_Bit(bitNum);
}

//-----------------------------------------------------------------------------

template <>
inline void CBitVecT< CFixedBitVecBase<32> >::Clear(int bitNum)
{
	*Base() &= ~BitVec_Bit(bitNum);
}

//-----------------------------------------------------------------------------

template <>
inline void CBitVecT< CFixedBitVecBase<32> >::Set(int bitNum, bool bNewVal)
{
	unsigned int bitMask = BitVec_Bit(bitNum);
	if (bNewVal)
	{
		*Base() |= bitMask;
	}
	else
	{
		*Base() &= ~bitMask;
	}
}


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Resizes the bit string to a new number of bits
// Input  : resizeNumBits - 
//-----------------------------------------------------------------------------
template <typename BITCOUNTTYPE>
inline void CVarBitVecBase<BITCOUNTTYPE>::Resize(int resizeNumBits, bool bClearAll)
{
	assert(resizeNumBits >= 0 && ((BITCOUNTTYPE)resizeNumBits == resizeNumBits));

	int newIntCount = CalcNumIntsForBits(resizeNumBits);
	if (newIntCount != GetNumDWords())
	{
		if (Base())
		{
			ReallocInts(newIntCount);
			if (!bClearAll && resizeNumBits >= GetNumBits())
			{
				Base()[GetNumDWords() - 1] &= GetEndMask();
				memset(Base() + GetNumDWords(), 0, (newIntCount - GetNumDWords()) * sizeof(int));
			}
		}
		else
		{
			// Figure out how many ints are needed
			AllocInts(newIntCount);
			// Initialize bitstring by clearing all bits
			bClearAll = true;
		}

		m_numInts = newIntCount;
	}
	else if (!bClearAll && resizeNumBits >= GetNumBits() && Base())
	{
		Base()[GetNumDWords() - 1] &= GetEndMask();
	}

	if (bClearAll && Base())
	{
		memset(Base(), 0, newIntCount * sizeof(int));
	}

	// store the new size and end mask
	m_numBits = resizeNumBits;
}

//-----------------------------------------------------------------------------
// Purpose: Allocate the storage for the ints
// Input  : numInts - 
//-----------------------------------------------------------------------------
template <typename BITCOUNTTYPE>
inline void CVarBitVecBase<BITCOUNTTYPE>::AllocInts(int numInts)
{
	assert(!m_pInt);

	if (numInts == 0)
		return;

	if (numInts == 1)
	{
		m_pInt = &m_iBitStringStorage;
		return;
	}

	m_pInt = (unsigned int*)malloc(numInts * sizeof(int));
}


//-----------------------------------------------------------------------------
// Purpose: Reallocate the storage for the ints
// Input  : numInts - 
//-----------------------------------------------------------------------------
template <typename BITCOUNTTYPE>
inline void CVarBitVecBase<BITCOUNTTYPE>::ReallocInts(int numInts)
{
	assert(Base());
	if (numInts == 0)
	{
		FreeInts();
		return;
	}

	if (m_pInt == &m_iBitStringStorage)
	{
		if (numInts != 1)
		{
			m_pInt = ((unsigned int*)malloc(numInts * sizeof(int)));
			*m_pInt = m_iBitStringStorage;
		}

		return;
	}

	if (numInts == 1)
	{
		m_iBitStringStorage = *m_pInt;
		free(m_pInt);
		m_pInt = &m_iBitStringStorage;
		return;
	}

	m_pInt = (unsigned int*)realloc(m_pInt, numInts * sizeof(int));
}


//-----------------------------------------------------------------------------
// Purpose: Free storage allocated with AllocInts
//-----------------------------------------------------------------------------
template <typename BITCOUNTTYPE>
inline void CVarBitVecBase<BITCOUNTTYPE>::FreeInts(void)
{
	if (m_numInts > 1)
	{
		free(m_pInt);
	}
	m_pInt = NULL;
}

// ------------------------------------------------------------------------ //
// CBitVecAccessor inlines.
// ------------------------------------------------------------------------ //

inline CBitVecAccessor::CBitVecAccessor(unsigned int* pDWords, int iBit)
{
	m_pDWords = pDWords;
	m_iBit = iBit;
}


inline void CBitVecAccessor::operator=(int val)
{
	if (val)
		m_pDWords[m_iBit >> 5] |= (1 << (m_iBit & 31));
	else
		m_pDWords[m_iBit >> 5] &= ~(unsigned long)(1 << (m_iBit & 31));
}

inline CBitVecAccessor::operator unsigned int()
{
	return m_pDWords[m_iBit >> 5] & (1 << (m_iBit & 31));
}


//=============================================================================

#endif // BITVEC_H