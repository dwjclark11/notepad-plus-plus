// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../Threading.h"
#include <windows.h>
#include <processthreadsapi.h>
#include <chrono>
#include <string>

namespace PlatformLayer {

// Thread implementation for Windows
class Thread::Impl {
public:
    std::thread _thread;
    ThreadFunc _func;
    bool _running = false;
};

Thread::Thread() : _impl(std::make_unique<Impl>()) {}

Thread::Thread(ThreadFunc func) : _impl(std::make_unique<Impl>()) {
    start(std::move(func));
}

Thread::~Thread() {
    if (_impl && _impl->_thread.joinable()) {
        _impl->_thread.detach();
    }
}

Thread::Thread(Thread&& other) noexcept : _impl(std::move(other._impl)) {}

Thread& Thread::operator=(Thread&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

void Thread::start(ThreadFunc func) {
    if (_impl->_running) {
        return;
    }
    _impl->_func = std::move(func);
    _impl->_thread = std::thread([this]() {
        _impl->_running = true;
        if (_impl->_func) {
            _impl->_func();
        }
        _impl->_running = false;
    });
}

void Thread::join() {
    if (_impl->_thread.joinable()) {
        _impl->_thread.join();
    }
}

bool Thread::tryJoin(uint32_t timeoutMs) {
    if (!_impl->_thread.joinable()) {
        return true;
    }
    auto result = WaitForSingleObject(_impl->_thread.native_handle(), timeoutMs);
    if (result == WAIT_OBJECT_0) {
        _impl->_thread.detach();
        return true;
    }
    return false;
}

void Thread::detach() {
    if (_impl->_thread.joinable()) {
        _impl->_thread.detach();
    }
}

bool Thread::isRunning() const {
    return _impl->_running;
}

bool Thread::joinable() const {
    return _impl->_thread.joinable();
}

void* Thread::nativeHandle() {
    return reinterpret_cast<void*>(_impl->_thread.native_handle());
}

void Thread::setPriority(ThreadPriority priority) {
    HANDLE hThread = _impl->_thread.native_handle();
    int winPriority = THREAD_PRIORITY_NORMAL;
    switch (priority) {
        case ThreadPriority::Idle: winPriority = THREAD_PRIORITY_IDLE; break;
        case ThreadPriority::Lowest: winPriority = THREAD_PRIORITY_LOWEST; break;
        case ThreadPriority::BelowNormal: winPriority = THREAD_PRIORITY_BELOW_NORMAL; break;
        case ThreadPriority::Normal: winPriority = THREAD_PRIORITY_NORMAL; break;
        case ThreadPriority::AboveNormal: winPriority = THREAD_PRIORITY_ABOVE_NORMAL; break;
        case ThreadPriority::Highest: winPriority = THREAD_PRIORITY_HIGHEST; break;
        case ThreadPriority::TimeCritical: winPriority = THREAD_PRIORITY_TIME_CRITICAL; break;
    }
    SetThreadPriority(hThread, winPriority);
}

void Thread::setName(const char* name) {
    // Windows 10 version 1607+ supports SetThreadDescription
    typedef HRESULT (WINAPI *SetThreadDescriptionFunc)(HANDLE, PCWSTR);
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    if (kernel32) {
        auto setThreadDesc = reinterpret_cast<SetThreadDescriptionFunc>(
            GetProcAddress(kernel32, "SetThreadDescription"));
        if (setThreadDesc) {
            std::wstring wname(name, name + strlen(name));
            setThreadDesc(_impl->_thread.native_handle(), wname.c_str());
        }
    }
}

void Thread::sleep(uint32_t milliseconds) {
    Sleep(milliseconds);
}

void Thread::yield() {
    SwitchToThread();
}

uint64_t Thread::currentThreadId() {
    return GetCurrentThreadId();
}

uint32_t Thread::hardwareConcurrency() {
    return std::thread::hardware_concurrency();
}

// Mutex implementation using std::mutex (header-only in Threading.h)
Mutex::Mutex() : _impl(nullptr) {}
Mutex::~Mutex() = default;
void Mutex::lock() {}
bool Mutex::tryLock() { return true; }
void Mutex::unlock() {}
void* Mutex::nativeHandle() { return nullptr; }

// RecursiveMutex implementation (header-only in Threading.h)
RecursiveMutex::RecursiveMutex() = default;
RecursiveMutex::~RecursiveMutex() = default;
void RecursiveMutex::lock() { _mutex.lock(); }
bool RecursiveMutex::tryLock() { return _mutex.try_lock(); }
void RecursiveMutex::unlock() { _mutex.unlock(); }

// ReadWriteLock implementation for Windows
class ReadWriteLock::Impl {
public:
    SRWLOCK _lock;
    Impl() { InitializeSRWLock(&_lock); }
};

ReadWriteLock::ReadWriteLock() : _impl(std::make_unique<Impl>()) {}
ReadWriteLock::~ReadWriteLock() = default;

void ReadWriteLock::lockRead() {
    AcquireSRWLockShared(&_impl->_lock);
}

bool ReadWriteLock::tryLockRead() {
    return TryAcquireSRWLockShared(&_impl->_lock) != FALSE;
}

void ReadWriteLock::unlockRead() {
    ReleaseSRWLockShared(&_impl->_lock);
}

void ReadWriteLock::lockWrite() {
    AcquireSRWLockExclusive(&_impl->_lock);
}

bool ReadWriteLock::tryLockWrite() {
    return TryAcquireSRWLockExclusive(&_impl->_lock) != FALSE;
}

void ReadWriteLock::unlockWrite() {
    ReleaseSRWLockExclusive(&_impl->_lock);
}

// ConditionVariable implementation using std::condition_variable (header-only)
ConditionVariable::ConditionVariable() = default;
ConditionVariable::~ConditionVariable() = default;

void ConditionVariable::wait(std::unique_lock<std::mutex>& lock) {
    _cv.wait(lock);
}

bool ConditionVariable::waitFor(std::unique_lock<std::mutex>& lock, uint32_t timeoutMs) {
    return _cv.wait_for(lock, std::chrono::milliseconds(timeoutMs)) != std::cv_status::timeout;
}

void ConditionVariable::notifyOne() {
    _cv.notify_one();
}

void ConditionVariable::notifyAll() {
    _cv.notify_all();
}

// Semaphore implementation for Windows
class Semaphore::Impl {
public:
    HANDLE _handle;
    explicit Impl(int initialCount) {
        _handle = CreateSemaphoreW(nullptr, initialCount, LONG_MAX, nullptr);
    }
    ~Impl() {
        if (_handle) CloseHandle(_handle);
    }
};

Semaphore::Semaphore(int initialCount) : _impl(std::make_unique<Impl>(initialCount)) {}
Semaphore::~Semaphore() = default;

void Semaphore::acquire() {
    WaitForSingleObject(_impl->_handle, INFINITE);
}

bool Semaphore::tryAcquire(uint32_t timeoutMs) {
    return WaitForSingleObject(_impl->_handle, timeoutMs) == WAIT_OBJECT_0;
}

void Semaphore::release(int count) {
    ReleaseSemaphore(_impl->_handle, count, nullptr);
}

// ThreadPool implementation
class ThreadPool::Impl {
public:
    std::vector<std::thread> _threads;
    std::atomic<bool> _shutdown{false};
};

ThreadPool::ThreadPool(size_t numThreads) : _impl(std::make_unique<Impl>()) {
    if (numThreads == 0) {
        numThreads = std::thread::hardware_concurrency();
    }
    for (size_t i = 0; i < numThreads; ++i) {
        _impl->_threads.emplace_back([this]() {
            while (!_impl->_shutdown.load()) {
                // Task execution loop
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

size_t ThreadPool::size() const {
    return _impl->_threads.size();
}

bool ThreadPool::isActive() const {
    return !_impl->_shutdown.load();
}

void ThreadPool::shutdown() {
    _impl->_shutdown.store(true);
    for (auto& t : _impl->_threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void ThreadPool::shutdownNow() {
    _impl->_shutdown.store(true);
    for (auto& t : _impl->_threads) {
        if (t.joinable()) {
            t.detach();
        }
    }
}

// OnceFlag implementation using std::call_once pattern
void callOnce(OnceFlag& flag, std::function<void()> func) {
    bool expected = false;
    if (flag._called.compare_exchange_strong(expected, true)) {
        func();
    }
}

// Barrier implementation using Windows synchronization
class Barrier::Impl {
public:
    size_t _count;
    size_t _remaining;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::atomic<size_t> _generation{0};

    explicit Impl(size_t count) : _count(count), _remaining(count) {}
};

Barrier::Barrier(size_t count) : _impl(std::make_unique<Impl>(count)) {}
Barrier::~Barrier() = default;

void Barrier::arriveAndWait() {
    std::unique_lock<std::mutex> lock(_impl->_mutex);
    size_t gen = _impl->_generation.load();

    if (--_impl->_remaining == 0) {
        _impl->_generation++;
        _impl->_remaining = _impl->_count;
        _impl->_cv.notify_all();
    } else {
        _impl->_cv.wait(lock, [this, gen] { return _impl->_generation.load() != gen; });
    }
}

bool Barrier::isComplete() const {
    return _impl->_generation.load() > 0;
}

// Timer implementation
class Timer::Impl {
public:
    std::thread _thread;
    std::atomic<bool> _active{false};
    std::atomic<bool> _stop{false};
};

Timer::Timer() : _impl(std::make_unique<Impl>()) {}
Timer::~Timer() {
    stop();
}

void Timer::startOneShot(uint32_t delayMs, std::function<void()> callback) {
    stop();
    _impl->_stop = false;
    _impl->_active = true;
    _impl->_thread = std::thread([this, delayMs, callback]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        if (!_impl->_stop.load()) {
            callback();
        }
        _impl->_active = false;
    });
    _impl->_thread.detach();
}

void Timer::startPeriodic(uint32_t intervalMs, std::function<void()> callback) {
    stop();
    _impl->_stop = false;
    _impl->_active = true;
    _impl->_thread = std::thread([this, intervalMs, callback]() {
        while (!_impl->_stop.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
            if (!_impl->_stop.load()) {
                callback();
            }
        }
        _impl->_active = false;
    });
    _impl->_thread.detach();
}

void Timer::stop() {
    _impl->_stop.store(true);
}

bool Timer::isActive() const {
    return _impl->_active.load();
}

// WorkerThread implementation
class WorkerThread::Impl {
public:
    std::thread _thread;
    std::queue<std::function<void()>> _tasks;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::atomic<bool> _running{false};
    std::atomic<bool> _stop{false};
    std::string _name;

    explicit Impl(const char* name) : _name(name ? name : "WorkerThread") {}
};

WorkerThread::WorkerThread() : _impl(std::make_unique<Impl>(nullptr)) {}
WorkerThread::WorkerThread(const char* name) : _impl(std::make_unique<Impl>(name)) {}
WorkerThread::~WorkerThread() {
    stop();
}

void WorkerThread::start() {
    if (_impl->_running.load()) return;
    _impl->_running = true;
    _impl->_thread = std::thread([this]() {
        ThreadUtils::setCurrentThreadName(_impl->_name.c_str());
        while (!_impl->_stop.load()) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(_impl->_mutex);
                _impl->_cv.wait(lock, [this] { return !_impl->_tasks.empty() || _impl->_stop.load(); });
                if (_impl->_stop.load()) break;
                if (!_impl->_tasks.empty()) {
                    task = std::move(_impl->_tasks.front());
                    _impl->_tasks.pop();
                }
            }
            if (task) {
                task();
            }
        }
        _impl->_running = false;
    });
}

void WorkerThread::stop() {
    _impl->_stop.store(true);
    _impl->_cv.notify_all();
    if (_impl->_thread.joinable()) {
        _impl->_thread.join();
    }
}

void WorkerThread::stopNow() {
    _impl->_stop.store(true);
    _impl->_cv.notify_all();
    if (_impl->_thread.joinable()) {
        _impl->_thread.detach();
    }
}

void WorkerThread::postTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(_impl->_mutex);
        _impl->_tasks.push(std::move(task));
    }
    _impl->_cv.notify_one();
}

bool WorkerThread::isRunning() const {
    return _impl->_running.load();
}

bool WorkerThread::isIdle() const {
    std::lock_guard<std::mutex> lock(_impl->_mutex);
    return _impl->_tasks.empty();
}

void WorkerThread::waitForIdle() {
    std::unique_lock<std::mutex> lock(_impl->_mutex);
    _impl->_cv.wait(lock, [this] { return _impl->_tasks.empty(); });
}

// ThreadUtils implementation
namespace ThreadUtils {

static thread_local uint64_t g_mainThreadId = 0;

uint64_t getCurrentThreadId() {
    return GetCurrentThreadId();
}

void setCurrentThreadName(const char* name) {
    typedef HRESULT (WINAPI *SetThreadDescriptionFunc)(HANDLE, PCWSTR);
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    if (kernel32) {
        auto setThreadDesc = reinterpret_cast<SetThreadDescriptionFunc>(
            GetProcAddress(kernel32, "SetThreadDescription"));
        if (setThreadDesc) {
            std::wstring wname(name, name + strlen(name));
            setThreadDesc(GetCurrentThread(), wname.c_str());
        }
    }
}

unsigned int getHardwareConcurrency() {
    return std::thread::hardware_concurrency();
}

void sleep(uint32_t milliseconds) {
    Sleep(milliseconds);
}

void yield() {
    SwitchToThread();
}

bool isMainThread() {
    return g_mainThreadId == GetCurrentThreadId();
}

void setMainThreadId() {
    g_mainThreadId = GetCurrentThreadId();
}

} // namespace ThreadUtils

} // namespace PlatformLayer