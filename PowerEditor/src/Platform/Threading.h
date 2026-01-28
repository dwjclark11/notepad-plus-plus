// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include <functional>
#include <memory>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <queue>
#include <vector>

namespace Platform {

// ============================================================================
// Thread Priority
// ============================================================================
enum class ThreadPriority {
    Idle = 0,
    Lowest = 1,
    BelowNormal = 2,
    Normal = 3,
    AboveNormal = 4,
    Highest = 5,
    TimeCritical = 6
};

// ============================================================================
// Thread class
// ============================================================================
class Thread {
public:
    using ThreadFunc = std::function<void()>;

    Thread();
    explicit Thread(ThreadFunc func);
    ~Thread();

    // Disable copy
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    // Enable move
    Thread(Thread&& other) noexcept;
    Thread& operator=(Thread&& other) noexcept;

    // Start the thread
    void start(ThreadFunc func);

    // Wait for thread to complete
    void join();

    // Try to join (non-blocking)
    bool tryJoin(uint32_t timeoutMs);

    // Detach the thread
    void detach();

    // Check if thread is running
    bool isRunning() const;

    // Check if joinable
    bool joinable() const;

    // Get native handle
    void* nativeHandle();

    // Set thread priority
    void setPriority(ThreadPriority priority);

    // Set thread name (for debugging)
    void setName(const char* name);

    // Static methods
    static void sleep(uint32_t milliseconds);
    static void yield();
    static uint64_t currentThreadId();
    static uint32_t hardwareConcurrency();
};

// ============================================================================
// Mutex wrapper
// ============================================================================
class Mutex {
public:
    Mutex();
    ~Mutex();

    // Disable copy
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    void lock();
    bool tryLock();
    void unlock();

    // Native handle for platform-specific operations
    void* nativeHandle();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

// ============================================================================
// Recursive Mutex wrapper
// ============================================================================
class RecursiveMutex {
public:
    RecursiveMutex();
    ~RecursiveMutex();

    // Disable copy
    RecursiveMutex(const RecursiveMutex&) = delete;
    RecursiveMutex& operator=(const RecursiveMutex&) = delete;

    void lock();
    bool tryLock();
    void unlock();

private:
    std::recursive_mutex _mutex;
};

// ============================================================================
// Read-Write Lock
// ============================================================================
class ReadWriteLock {
public:
    ReadWriteLock();
    ~ReadWriteLock();

    // Disable copy
    ReadWriteLock(const ReadWriteLock&) = delete;
    ReadWriteLock& operator=(const ReadWriteLock&) = delete;

    void lockRead();
    bool tryLockRead();
    void unlockRead();

    void lockWrite();
    bool tryLockWrite();
    void unlockWrite();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

// ============================================================================
// Scoped Lock Guards
// ============================================================================
template<typename T>
class LockGuard {
public:
    explicit LockGuard(T& lock) : _lock(lock) { _lock.lock(); }
    ~LockGuard() { _lock.unlock(); }

    // Disable copy and move
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
    LockGuard(LockGuard&&) = delete;
    LockGuard& operator=(LockGuard&&) = delete;

private:
    T& _lock;
};

// Read lock guard
class ReadLockGuard {
public:
    explicit ReadLockGuard(ReadWriteLock& lock) : _lock(lock) { _lock.lockRead(); }
    ~ReadLockGuard() { _lock.unlockRead(); }

    ReadLockGuard(const ReadLockGuard&) = delete;
    ReadLockGuard& operator=(const ReadLockGuard&) = delete;

private:
    ReadWriteLock& _lock;
};

// Write lock guard
class WriteLockGuard {
public:
    explicit WriteLockGuard(ReadWriteLock& lock) : _lock(lock) { _lock.lockWrite(); }
    ~WriteLockGuard() { _lock.unlockWrite(); }

    WriteLockGuard(const WriteLockGuard&) = delete;
    WriteLockGuard& operator=(const WriteLockGuard&) = delete;

private:
    ReadWriteLock& _lock;
};

// ============================================================================
// Condition Variable
// ============================================================================
class ConditionVariable {
public:
    ConditionVariable();
    ~ConditionVariable();

    // Disable copy
    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

    // Wait indefinitely
    void wait(std::unique_lock<std::mutex>& lock);

    // Wait with timeout
    bool waitFor(std::unique_lock<std::mutex>& lock, uint32_t timeoutMs);

    // Notify one waiting thread
    void notifyOne();

    // Notify all waiting threads
    void notifyAll();

private:
    std::condition_variable _cv;
};

// ============================================================================
// Semaphore
// ============================================================================
class Semaphore {
public:
    explicit Semaphore(int initialCount = 0);
    ~Semaphore();

    // Disable copy
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

    // Acquire (decrement)
    void acquire();

    // Try to acquire with timeout
    bool tryAcquire(uint32_t timeoutMs);

    // Release (increment)
    void release(int count = 1);

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

// ============================================================================
// Thread-safe Queue
// ============================================================================
template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue();
    ~ThreadSafeQueue();

    // Disable copy
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    // Add item to queue
    void push(const T& item);
    void push(T&& item);

    // Try to pop an item (non-blocking)
    bool tryPop(T& item);

    // Pop with timeout
    bool popWithTimeout(T& item, uint32_t timeoutMs);

    // Wait and pop (blocking)
    void waitAndPop(T& item);

    // Check if empty
    bool empty() const;

    // Get size
    size_t size() const;

    // Clear the queue
    void clear();

private:
    mutable std::mutex _mutex;
    std::condition_variable _cv;
    std::queue<T> _queue;
    bool _shutdown = false;
};

// ============================================================================
// Thread Pool
// ============================================================================
class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = 0);
    ~ThreadPool();

    // Disable copy
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Submit work to the pool
    template<typename Func, typename... Args>
    auto submit(Func&& func, Args&&... args) -> std::future<typename std::invoke_result<Func, Args...>::type>;

    // Get number of threads
    size_t size() const;

    // Check if pool is active
    bool isActive() const;

    // Shutdown the pool (wait for all tasks to complete)
    void shutdown();

    // Shutdown immediately (cancel pending tasks)
    void shutdownNow();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

// ============================================================================
// Atomic operations wrapper
// ============================================================================
template<typename T>
class Atomic {
public:
    Atomic() : _value(T()) {}
    explicit Atomic(T initial) : _value(initial) {}

    // Load
    T load() const { return _value.load(std::memory_order_seq_cst); }
    T load(std::memory_order order) const { return _value.load(order); }

    // Store
    void store(T val) { _value.store(val, std::memory_order_seq_cst); }
    void store(T val, std::memory_order order) { _value.store(val, order); }

    // Exchange
    T exchange(T val) { return _value.exchange(val, std::memory_order_seq_cst); }

    // Compare and exchange
    bool compareExchange(T& expected, T desired) {
        return _value.compare_exchange_strong(expected, desired, std::memory_order_seq_cst);
    }

    // Fetch and add
    T fetchAdd(T val) { return _value.fetch_add(val, std::memory_order_seq_cst); }

    // Fetch and sub
    T fetchSub(T val) { return _value.fetch_sub(val, std::memory_order_seq_cst); }

    // Increment/decrement operators
    T operator++() { return ++_value; }
    T operator++(int) { return _value++; }
    T operator--() { return --_value; }
    T operator--(int) { return _value--; }

    // Assignment
    T operator=(T val) { store(val); return val; }

    // Conversion operator
    operator T() const { return load(); }

private:
    std::atomic<T> _value;
};

// ============================================================================
// Once Flag and call_once
// ============================================================================
class OnceFlag {
public:
    OnceFlag() : _called(false) {}

    // Disable copy
    OnceFlag(const OnceFlag&) = delete;
    OnceFlag& operator=(const OnceFlag&) = delete;

private:
    friend void callOnce(OnceFlag& flag, std::function<void()> func);
    std::atomic<bool> _called;
    std::mutex _mutex;
};

void callOnce(OnceFlag& flag, std::function<void()> func);

// ============================================================================
// Barrier (synchronization primitive)
// ============================================================================
class Barrier {
public:
    explicit Barrier(size_t count);
    ~Barrier();

    // Disable copy
    Barrier(const Barrier&) = delete;
    Barrier& operator=(const Barrier&) = delete;

    // Wait for all threads to arrive
    void arriveAndWait();

    // Check if barrier is complete
    bool isComplete() const;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

// ============================================================================
// Timer (for delayed execution)
// ============================================================================
class Timer {
public:
    Timer();
    ~Timer();

    // Disable copy
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    // Start a one-shot timer
    void startOneShot(uint32_t delayMs, std::function<void()> callback);

    // Start a periodic timer
    void startPeriodic(uint32_t intervalMs, std::function<void()> callback);

    // Stop the timer
    void stop();

    // Check if timer is active
    bool isActive() const;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

// ============================================================================
// Worker Thread (for background tasks)
// ============================================================================
class WorkerThread {
public:
    WorkerThread();
    explicit WorkerThread(const char* name);
    ~WorkerThread();

    // Disable copy
    WorkerThread(const WorkerThread&) = delete;
    WorkerThread& operator=(const WorkerThread&) = delete;

    // Start the worker
    void start();

    // Stop the worker (wait for current task)
    void stop();

    // Stop immediately (cancel current task)
    void stopNow();

    // Post a task to the worker
    void postTask(std::function<void()> task);

    // Check if worker is running
    bool isRunning() const;

    // Check if worker is idle
    bool isIdle() const;

    // Wait for all tasks to complete
    void waitForIdle();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

// ============================================================================
// Thread Utilities
// ============================================================================
namespace ThreadUtils {

// Get current thread ID
uint64_t getCurrentThreadId();

// Set current thread name (for debugging)
void setCurrentThreadName(const char* name);

// Get hardware concurrency (number of logical processors)
unsigned int getHardwareConcurrency();

// Sleep for specified milliseconds
void sleep(uint32_t milliseconds);

// Yield the current thread
void yield();

// Check if running on main thread
bool isMainThread();

// Set main thread ID (call from main thread at startup)
void setMainThreadId();

} // namespace ThreadUtils

// ============================================================================
// Template implementations
// ============================================================================

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue() : _shutdown(false) {}

template<typename T>
ThreadSafeQueue<T>::~ThreadSafeQueue() {
    clear();
}

template<typename T>
void ThreadSafeQueue<T>::push(const T& item) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(item);
    }
    _cv.notify_one();
}

template<typename T>
void ThreadSafeQueue<T>::push(T&& item) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(std::move(item));
    }
    _cv.notify_one();
}

template<typename T>
bool ThreadSafeQueue<T>::tryPop(T& item) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_queue.empty()) {
        return false;
    }
    item = std::move(_queue.front());
    _queue.pop();
    return true;
}

template<typename T>
bool ThreadSafeQueue<T>::popWithTimeout(T& item, uint32_t timeoutMs) {
    std::unique_lock<std::mutex> lock(_mutex);
    bool hasItem = _cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] {
        return !_queue.empty() || _shutdown;
    });

    if (!hasItem || _queue.empty()) {
        return false;
    }

    item = std::move(_queue.front());
    _queue.pop();
    return true;
}

template<typename T>
void ThreadSafeQueue<T>::waitAndPop(T& item) {
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [this] {
        return !_queue.empty() || _shutdown;
    });

    if (!_queue.empty()) {
        item = std::move(_queue.front());
        _queue.pop();
    }
}

template<typename T>
bool ThreadSafeQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.empty();
}

template<typename T>
size_t ThreadSafeQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.size();
}

template<typename T>
void ThreadSafeQueue<T>::clear() {
    std::lock_guard<std::mutex> lock(_mutex);
    _shutdown = true;
    while (!_queue.empty()) {
        _queue.pop();
    }
    _cv.notify_all();
}

template<typename Func, typename... Args>
auto ThreadPool::submit(Func&& func, Args&&... args) -> std::future<typename std::invoke_result<Func, Args...>::type> {
    using ReturnType = typename std::invoke_result<Func, Args...>::type;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
    );

    std::future<ReturnType> result = task->get_future();

    // Post to thread pool implementation
    // This is a placeholder - actual implementation would queue to worker threads
    (*task)(); // Execute immediately for now

    return result;
}

} // namespace Platform
