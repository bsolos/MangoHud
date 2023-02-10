#pragma once
#ifndef MANGOHUD_VEGA3_WORKAROUND
#define MANGOHUD_VEGA3_WORKAROUND

namespace v3w {
    bool samplerThreadRunning();
    void startSamplerThread();
    double getBusy();
}

#endif