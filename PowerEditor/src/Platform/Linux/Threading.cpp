// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "../Threading.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <chrono>
#include <cstring>

namespace Platform {

// Thread implementation for Linux
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
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start).count() < timeoutMs) {
        if (!_impl->_running) {
            _impl->_thread.join();
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
    pthread_t handle = _impl->_thread.native_handle();
    int policy;
    sched_param param;
    pthread_getschedparam(handle, &policy, &param);

    int minPriority = sched_get_priority_min(policy);
    int maxPriority = sched_get_priority_max(policy);
    int range = maxPriority - minPriority;

    switch (priority) {
        case ThreadPriority::Idle:
        case ThreadPriority::Lowest:
            param.sched_priority = minPriority;
            break;
        case ThreadPriority::BelowNormal:
            param.sched_priority = minPriority + range / 4;
            break;
        case ThreadPriority::Normal:
            param.sched_priority = minPriority + range / 2;
            break;
        case ThreadPriority::AboveNormal:
            param.sched_priority = minPriority + 3 * range / 4;
            break;
        case ThreadPriority::Highest:
        case ThreadPriority::TimeCritical:
            param.sched_priority = maxPriority;
            break;
    }
    pthread_setschedparam(handle, policy, &param);
}

void Thread::setName(const char* name) {
    pthread_setname_np(_impl->_thread.native_handle(), name);
}

void Thread::sleep(uint32_t milliseconds) {
    usleep(milliseconds * 1000);
}

void Thread::yield() {
    sched_yield();
}

uint64_t Thread::currentThreadId() {
    return syscall(SYS_gettid);
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

// ReadWriteLock implementation for Linux using pthread_rwlock
class ReadWriteLock::Impl {
public:
    pthread_rwlock_t _lock;
    Impl() {
        pthread_rwlock_init(&_lock, nullptr);
    }
    ~Impl() {
        pthread_rwlock_destroy(&_lock);
    }
};

ReadWriteLock::ReadWriteLock() : _impl(std::make_unique<Impl>()) {}
ReadWriteLock::~ReadWriteLock() = default;

void ReadWriteLock::lockRead() {
    pthread_rwlock_rdlock(&_impl->_lock);
}

bool ReadWriteLock::tryLockRead() {
    return pthread_rwlock_tryrdlock(&_impl->_lock) == 0;
}

void ReadWriteLock::unlockRead() {
    pthread_rwlock_unlock(&_impl->_lock);
}

void ReadWriteLock::lockWrite() {
    pthread_rwlock_wrlock(&_impl->_lock);
}

bool ReadWriteLock::tryLockWrite() {
    return pthread_rwlock_trywrlock(&_impl->_lock) == 0;
}

void ReadWriteLock::unlockWrite() {
    pthread_rwlock_unlock(&_impl->_lock);
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

// Semaphore implementation for Linux
class Semaphore::Impl {
public:
    std::mutex _mutex;
    std::condition_variable _cv;
    int _count;
    explicit Impl(int initialCount) : _count(initialCount) {}
};

Semaphore::Semaphore(int initialCount) : _impl(std::make_unique<Impl>(initialCount)) {}
Semaphore::~Semaphore() = default;

void Semaphore::acquire() {
    std::unique_lock<std::mutex> lock(_impl->_mutex);
    _impl->_cv.wait(lock, [this] { return _impl->_count > 0; });
    --_impl->_count;
}

bool Semaphore::tryAcquire(uint32_t timeoutMs) {
    std::unique_lock<std::mutex> lock(_impl->_mutex);
    bool result = _impl->_cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] {
        return _impl->_count > 0;
    });
    if (result) {
        --_impl->_count;
    }
    return result;
}

void Semaphore::release(int count) {
    {
        std::lock_guard<std::mutex> lock(_impl->_mutex);
        _impl->_count += count;
    }
    for (int i = 0; i < count; ++i) {
        _impl->_cv.notify_one();
    }
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

// Barrier implementation
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
    return syscall(SYS_gettid);
}

void setCurrentThreadName(const char* name) {
    pthread_setname_np(pthread_self(), name);
}

unsigned int getHardwareConcurrency() {
    return std::thread::hardware_concurrency();
}

void sleep(uint32_t milliseconds) {
    usleep(milliseconds * 1000);
}

void yield() {
    sched_yield();
}

bool isMainThread() {
    return g_mainThreadId == syscall(SYS_gettid);
}

void setMainThreadId() {
    g_mainThreadId = syscall(SYS_gettid);
}

} // namespace ThreadUtils

} // namespace Platform