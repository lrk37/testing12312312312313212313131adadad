#include "includes.h"

GrimaceThread g_threading{};

void GrimaceThread::startup( int core )
{
	m_mutex.lock();
	{
		allocate_thread_id();

		SetThreadAffinityMask( GetCurrentThread(), 1 << ( core + 1 ) );
	}
	m_mutex.unlock();

	while ( true )
	{
		std::unique_lock lock( m_mutex ); // automatically unlocks the mutex when we exit this scope.

		m_condition.wait( lock, [this]
			{
				return !m_queued_tasks.empty() || m_thread_shutdown; // sleeps the thread if we dont need to perform any processing on it.
			} );

		if ( m_thread_shutdown && m_queued_tasks.empty() )
			break;

		// transfer ownership out of the queue.
		auto func = std::move( m_queued_tasks.front() );
		m_queued_tasks.pop_front();

		++m_active_tasks;

		lock.unlock();

		func();

		lock.lock();

		--m_active_tasks;

		m_condition.notify_all();
	}

	m_mutex.lock();
	{
#ifdef _DEBUG
		g_notify.add( tfm::format( "%i exited work loop\n", std::this_thread::get_id() ) );
#endif

		free_thread_id();
	}
	m_mutex.unlock();
}

unsigned int GrimaceThread::get_free_tls_slots()
{
	static constexpr auto max_threads = 32;

	auto highest_index = 1;
	while ( highest_index < 128 && reinterpret_cast<uint8_t*>( (unsigned char*)( (BYTE*)_tier0 + 0x51AA0 )[ highest_index ] ) )
		highest_index++;

	return std::clamp( max_threads - highest_index, 0, max_threads );
}

void GrimaceThread::init()
{
	_tier0 = GetModuleHandleA( "tier0.dll" );

	allocate_thread_id = reinterpret_cast<allocate_thread_id_t>( GetProcAddress( _tier0, "AllocateThreadID" ) );
	free_thread_id = reinterpret_cast<free_thread_id_t>( GetProcAddress( _tier0, "FreeThreadID" ) );

	// not sure if need to call on the main thread
	// but AllocateThreadID checks if its already allocated, so it wont matter.
	allocate_thread_id();

	unsigned int max_threads = std::clamp( std::min( static_cast<unsigned int>( std::floor( 0.5f * std::thread::hardware_concurrency() ) ) - 2u, get_free_tls_slots() ), 2u, 4u );

#ifdef _DEBUG
	g_notify.add( std::to_string( max_threads ) + " threads initialized\n" );
#endif

	for ( unsigned int i = 0; i < max_threads; i++ )
		m_threads.emplace_back( &GrimaceThread::startup, this, i );
}

void GrimaceThread::shutdown()
{
	m_mutex.lock();
	{
		m_thread_shutdown = true;

		// main thread here.
		free_thread_id();
	}
	m_mutex.unlock();

	// wakeup all sleeping threads so they can exit.
	m_condition.notify_all();
}

// infinite loop fucks this, rip.
void GrimaceThread::join_all()
{
	for ( int i = 0; i < m_threads.size(); i++ )
	{
		auto* thread = &m_threads[ i ];
		if ( thread->joinable() ) // indicates current execution.
			thread->join();
	}
}

void GrimaceThread::wait_for_task_completion()
{
	std::unique_lock lock( m_mutex );

	m_condition.wait( lock, [&]()
	{
		return m_queued_tasks.empty() && m_active_tasks == 0;
	} );
}