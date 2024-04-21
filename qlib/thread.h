#ifdef OS_WINDOWS

typedef s64 THREAD;
typedef u32 THREAD_RETURN;
typedef s64 MUTEX;

#elif OS_LINUX

typedef pthread_t THREAD;
typedef void* THREAD_RETURN;
typedef pthread_mutex_t MUTEX;

#endif // OS
