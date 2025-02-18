internal s64
WIN32_FUNC(create_mutex)() {
    s64 mutex = (s64)CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL) {
        print("CreateMutex error: %d\n", GetLastError());
    }
    return mutex;
}

internal void
WIN32_FUNC(wait_handle)(s64 handle) {
    DWORD wait_result = WaitForSingleObject((HANDLE)handle, INFINITE);
    if (wait_result == WAIT_ABANDONED) {}

    if (wait_result == WAIT_OBJECT_0) {}
}

internal void
WIN32_FUNC(wait_mutex)(s64 handle) {
    WIN32_FUNC(wait_handle)(handle);
}

internal void
WIN32_FUNC(release_mutex)(s64 handle) {
    if (!ReleaseMutex((HANDLE)handle)) {
        print("ReleaseMutex error: %d\n", GetLastError());
    }
}

internal s64
WIN32_FUNC(create_thread)(void *function, void *data) {
    DWORD thread_id;
    return (s64)CreateThread(0, 0, LPTHREAD_START_ROUTINE(function), data, 0, &thread_id);
}

internal void
WIN32_FUNC(terminate_thread)(s64 handle) {
    TerminateThread((HANDLE)handle, NULL);
}

internal void
WIN32_FUNC(wait_for_thread)(s64 handle) {
    WaitForSingleObject((HANDLE)handle, INFINITE);
}

internal bool8
WIN32_FUNC(thread_in_use)(s64 handle) {
    if (!handle)
        return false;
    
    DWORD wait_result = WaitForSingleObject((HANDLE)handle, 0);
    if (wait_result == WAIT_TIMEOUT) {
        return true;
    }
    
    return false;
}
