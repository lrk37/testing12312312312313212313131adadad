#pragma once

class GrimaceThread
{
private:
    typedef int( __cdecl* allocate_thread_id_t )( );
    typedef void( __cdecl* free_thread_id_t )( );

    allocate_thread_id_t allocate_thread_id;
    free_thread_id_t free_thread_id;

public:
	std::condition_variable m_condition;
	bool m_thread_shutdown{ false };

	std::mutex m_mutex;

    int m_active_tasks;
	std::deque<std::function<void()>> m_queued_tasks;

    std::vector<std::thread> m_threads;

    HMODULE _tier0;
    unsigned int get_free_tls_slots();
    void startup( int core );

public:
	void init();
    void shutdown();
	void join_all();

    void wait_for_task_completion();
public:
    template<typename Func, typename... Args>
    void enqueue_task( Func&& f, Args&&... args )
    {
        m_mutex.lock();
        {
            // bind the function and its arguments into a zero-argument callable.
            m_queued_tasks.push_back( std::bind( std::forward<Func>( f ), std::forward<Args>( args )... ) );
        }
        m_mutex.unlock();

        // wakeup a sleeping thread.
        m_condition.notify_one();
    }
};

extern GrimaceThread g_threading;