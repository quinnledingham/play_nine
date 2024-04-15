internal s64
win32_create_mutex() {
    s64 mutex = (s64)CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL) {
        print("CreateMutex error: %d\n", GetLastError());
    }
    return mutex;
}

internal void
win32_wait_mutex(s64 handle) {
    DWORD wait_result = WaitForSingleObject((HANDLE)handle, INFINITE);
    if (wait_result == WAIT_ABANDONED) {}

    if (wait_result == WAIT_OBJECT_0) {}
}

internal void
win32_release_mutex(s64 handle) {
    if (!ReleaseMutex((HANDLE)handle)) {
        print("ReleaseMutex error: %d\n", GetLastError());
    }
}

internal s64
win32_create_thread(void *function, void *data) {
    DWORD thread_id;
    return (s64)CreateThread(0, 0, LPTHREAD_START_ROUTINE(function), data, 0, &thread_id);
}

internal void
win32_terminate_thread(s64 handle) {
    TerminateThread((HANDLE)handle, NULL);
}

internal void
win32_wait_for_thread(s64 handle) {
    WaitForSingleObject((HANDLE)handle, INFINITE);
}
