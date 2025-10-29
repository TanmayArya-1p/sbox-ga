#pragma once
#include <bits/stdc++.h>

// TODO:

class ThreadPool {
public:
    ThreadPool(std::size_t num_threads);

private:
    std::vector<std::thread> threads;
    std::size_t num_threads;

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::queue<std::function<void()>> tasks;

};
