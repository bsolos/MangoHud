#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <algorithm>

#include "vega3_workaround.h"

#define SAMPLE_BUF_SIZE 128
#define SAMPLING_RATE 256

void samplerMain();
bool bit31NotZero();

bool sampler_thread_running = false;
std::thread sampler_thread;
std::mutex sample_buf_mutex;
std::vector<bool> sample_buf(SAMPLE_BUF_SIZE);

bool v3w::samplerThreadRunning() {
    return sampler_thread_running;
}

void v3w::startSamplerThread() {
    sampler_thread = std::thread(samplerMain);
    sampler_thread_running = true;
}

double v3w::getBusy() {
    sample_buf_mutex.lock();

    size_t accumulator = 0;
    for (bool sample : sample_buf) {
        if (sample) accumulator++;
    }

    sample_buf_mutex.unlock();
    return (double)accumulator / (double)SAMPLE_BUF_SIZE;
}

void samplerMain() {
    while (true) {
        sample_buf_mutex.lock();

        std::rotate(sample_buf.begin(), sample_buf.begin() + 1, sample_buf.end());
        sample_buf[SAMPLE_BUF_SIZE - 1] = bit31NotZero();

        sample_buf_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(1000000 / SAMPLING_RATE));
    }
}

bool bit31NotZero() {
    // TODO: Implement
    return false;
}