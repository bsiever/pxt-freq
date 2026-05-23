#include "pxt.h"
#include "MicroBit.h"

using namespace pxt;

#if MICROBIT_CODAL

#define SAMPLE_COUNT 256

namespace frequencies {

class FreqSampler : public codal::DataSink {
public:
    SplitterChannel *channel;
    int8_t buf[SAMPLE_COUNT];
    volatile int count;
    volatile bool capturing;

    FreqSampler() : channel(nullptr), count(0), capturing(false) {}

    void setup() {
        MicroBitAudio::requestActivation();
        channel = uBit.audio.splitter->createChannel();
        channel->connect(*this);
    }

    void startCapture() {
        count = 0;
        __asm__ volatile("" ::: "memory");
        capturing = true;
    }

    virtual int pullRequest() override {
        ManagedBuffer data = channel->pull();
        if (!capturing)
            return DEVICE_OK;

        int8_t *samples = (int8_t *)data.getBytes();
        int n = data.length();
        for (int i = 0; i < n && count < SAMPLE_COUNT; i++)
            buf[count++] = samples[i];  // int8_t: splitter output is DATASTREAM_FORMAT_8BIT_SIGNED

        if (count >= SAMPLE_COUNT) {
            __asm__ volatile("" ::: "memory");
            capturing = false;
        }

        return DEVICE_OK;
    }
};

static FreqSampler *sampler = nullptr;

static void ensureInit() {
    if (!sampler) {
        sampler = new FreqSampler();
        sampler->setup();
    }
}

} // namespace frequencies

#endif // MICROBIT_CODAL

namespace frequencies {

//%
void dumpSamples() {
#if MICROBIT_CODAL
    uBit.serial.printf("Starting Capture (waiting for sample\n");
    uBit.audio.activateMic();
    ensureInit();
    sampler->startCapture();
    while (sampler->capturing)
        fiber_sleep(1);
    for (int i = 0; i < SAMPLE_COUNT; i++)
        uBit.serial.printf("%d\n", sampler->buf[i]);
#else
    target_panic(PANIC_VARIANT_NOT_SUPPORTED);
#endif
}

} // namespace frequencies
