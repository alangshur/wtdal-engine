#include "util/semaphore.hpp"
using namespace std;

BaseSemaphore::BaseSemaphore(int32_t count) : m_count(count) {}

void BaseSemaphore::post() {
    unique_lock<mutex> lock(this->m_mutex);
    this->m_count++;
    this->m_cv.notify_one();
}

void BaseSemaphore::wait() {
    unique_lock<mutex> lock(this->m_mutex);
    this->m_cv.wait(lock, [this]() { return this->m_count != 0; });
    this->m_count--;
}

EffSemaphore::EffSemaphore() : m_count(0), m_semaphore(0) {}
EffSemaphore::EffSemaphore(int32_t count) : m_count(count), m_semaphore(0) {}

void EffSemaphore::post() {
    int32_t count = m_count.fetch_add(1, memory_order_release);
    if (count < 0) m_semaphore.post();
}

void EffSemaphore::wait() {
    int32_t count = m_count.fetch_sub(1, memory_order_acquire);
    if (count < 1) m_semaphore.wait();
}

