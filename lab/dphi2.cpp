#include <CL/sycl.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>

constexpr size_t num_philosophers = 5;

std::atomic<bool> done(false);

void philosopher(size_t id, std::vector<std::atomic<bool>>& forks) {
    size_t left_fork = id;
    size_t right_fork = (id + 1) % num_philosophers;

    while (!done) {
        // Try to pick up left fork
        bool expected = false;
        while (!forks[left_fork].compare_exchange_weak(expected, true)) {
            expected = false;
            std::this_thread::yield();
        }

        // Try to pick up right fork
        expected = false;
        while (!forks[right_fork].compare_exchange_weak(expected, true)) {
            expected = false;
            std::this_thread::yield();
        }

        // Eating
        std::cout << "Philosopher " << id << " is eating." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Put down forks
        forks[left_fork] = false;
        forks[right_fork] = false;

        // Thinking
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    std::vector<std::atomic<bool>> forks(num_philosophers);
    std::fill(forks.begin(), forks.end(), false);

    std::vector<std::thread> philosophers(num_philosophers);
    for (size_t i = 0; i < num_philosophers; ++i) {
        philosophers[i] = std::thread(philosopher, i, std::ref(forks));
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    done = true;

    for (auto& philosopher_thread : philosophers) {
        philosopher_thread.join();
    }

    return 0;
}
