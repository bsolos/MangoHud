#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <algorithm>
#include <libdrm/amdgpu.h>
#include <fcntl.h>

#include "vega3_workaround.h"

#define SAMPLE_BUF_SIZE 256
#define SAMPLING_RATE 512

#define GRBM_STATUS 0x8010
#define UTIL_BIT (1U<<31)

void samplerMain();
bool utilBitNotZero();

amdgpu_device_handle amdgpu_dev;

bool sampler_thread_running = false;
std::thread sampler_thread;
std::mutex sample_buf_mutex;
std::vector<bool> sample_buf(SAMPLE_BUF_SIZE);

bool v3w::samplerThreadRunning() {
    return sampler_thread_running;
}

void v3w::startSamplerThread() {
    int fd = open("/dev/dri/card0", O_RDWR); // horrible hardcode
    uint32_t drm_minor, drm_major; 
    amdgpu_device_initialize(fd, &drm_minor, &drm_major, &amdgpu_dev);

    sampler_thread = std::thread(samplerMain);
    sampler_thread.detach();
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
        sample_buf[SAMPLE_BUF_SIZE - 1] = utilBitNotZero();

        sample_buf_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(1000000 / SAMPLING_RATE));
    }
}

bool utilBitNotZero() {
    uint32_t registers;
    amdgpu_read_mm_registers(amdgpu_dev, GRBM_STATUS / 4, 1, 0xffffffff, 0, &registers);

    return (registers & UTIL_BIT) != 0;
}