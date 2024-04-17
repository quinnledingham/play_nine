s64 LINUX_FUNC(create_mutex)() {
  
}

void LINUX_FUNC(wait_handle)(s64 handle) {
  
}

void LINUX_FUNC(wait_mutex)(s64 handle) {
  LINUX_FUNC(wait_handle)(handle);
}

void LINUX_FUNC(release_mutex)(s64 handle) {
  
}

s64 LINUX_FUNC(create_thread)(u32 (*function)(void *), void *data) {
  
}

void LINUX_FUNC(terminate_thread)(s64 handle) {
  
}

