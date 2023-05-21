#ifndef NETWORKING_TS_DEQUE_HPP
#define NETWORKING_TS_DEQUE_HPP

#include "../pch.h"

// Thread-safe deque

namespace net {
    template<typename T>
    class ts_deque {
    public:
        ts_deque() = default;

        ts_deque(const ts_deque<T> &) = delete;

        virtual ~ts_deque() { clear(); }

    public:
        // Returns and maintains item at front of Queue
        const T &front() {
            std::scoped_lock lock(_deque_mutex);
            return _deque.front();
        }

        // Returns and maintains item at back of Queue
        const T &back() {
            std::scoped_lock lock(_deque_mutex);
            return _deque.back();
        }

        // Removes and returns item from front of Queue
        T pop_front() {
            std::scoped_lock lock(_deque_mutex);
            auto t = std::move(_deque.front());
            _deque.pop_front();
            return t;
        }

        // Removes and returns item from back of Queue
        T pop_back() {
            std::scoped_lock lock(_deque_mutex);
            auto t = std::move(_deque.back());
            _deque.pop_back();
            return t;
        }

        // Adds an item to back of Queue
        void push_back(const T &item) {
            std::scoped_lock lock(_deque_mutex);
            _deque.emplace_back(std::move(item));

            std::unique_lock<std::mutex> ul(_blocking_mutex);
            _cv_blocking.notify_one();
        }

        // Adds an item to front of Queue
        void push_front(const T &item) {
            std::scoped_lock lock(_deque_mutex);
            _deque.emplace_front(std::move(item));

            std::unique_lock<std::mutex> ul(_blocking_mutex);
            _cv_blocking.notify_one();
        }

        // Returns true if Queue has no items
        bool empty() {
            try {
                std::scoped_lock lock(_deque_mutex);
                return _deque.empty();
            }
            catch (std::exception &e) {
                std::cerr << "Client Exception: " << e.what() << "\n";
                return false;
            }
        }

        // Returns number of items in Queue
        size_t count() {
            std::scoped_lock lock(_deque_mutex);
            return _deque.size();
        }

        // Clears Queue
        void clear() {
            std::scoped_lock lock(_deque_mutex);
            _deque.clear();
        }

        void wait() {
            while (empty()) {
                std::unique_lock<std::mutex> ul(_blocking_mutex);
                _cv_blocking.wait(ul);
            }
        }

    protected:
        std::mutex _deque_mutex;
        std::deque<T> _deque;
        std::condition_variable _cv_blocking;
        std::mutex _blocking_mutex;
    };
}

#endif //NETWORKING_TS_DEQUE_HPP