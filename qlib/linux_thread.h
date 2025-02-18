#include <pthread.h>

MUTEX LINUX_FUNC(create_mutex)() {
  pthread_mutex_t mutex;
  if (pthread_mutex_init(&mutex, NULL)) {
    logprint("linux_create_mutex()", "pthread_mutex_init failed\n");
  }
  return mutex;
}

void LINUX_FUNC(wait_handle)(THREAD handle) {
  //pthread_mutex_lock(&handle);
  if (pthread_join(handle, NULL)) {
    logprint("linux_wait_handle()", "pthread_join failed\n");
  }
}

void LINUX_FUNC(wait_mutex)(MUTEX handle) {
  //LINUX_FUNC(wait_handle)(handle);
  if (pthread_mutex_lock(&handle)) {
    logprint("linux_wait_mutex()", "pthread_mutex_lock failed\n");
  }
  //logprint("linux_wait_mutex()", "mutex locked (%d)\n", handle);
}

void LINUX_FUNC(release_mutex)(MUTEX handle) {
  int returnval;
  if (returnval = pthread_mutex_unlock(&handle)) {
    logprint("linux_release_mutex()", "pthread_mutex_unlock failed (%s)\n", strerror(returnval));
  }
}

THREAD LINUX_FUNC(create_thread)(THREAD_RETURN (*function)(void *), void *data) {
  pthread_t thread_id;
  if (pthread_create(&thread_id, NULL, function, data)) {
    logprint("linux_create_thread()", "pthread_create_failed\n");
  }
  return thread_id;
}

void LINUX_FUNC(terminate_thread)(THREAD handle) {
  if (pthread_cancel(handle)) {
    logprint("linux_terminate_thread()", "pthread_cancel failed\n");
  }
}

bool8 LINUX_FUNC(thread_in_use)(s64 handle) {
  // IMPLEMENT FUNCTION
  logprint("thread_in_use", "IMPLEMENT thread_in_use for Linux\n");
  ASSERT(0);
  return false;
}
