#include "pxt.h"
#include "MicroBit.h"
#include "cmsis-dsp/rfft_q15_api.h"

using namespace pxt;

#if MICROBIT_CODAL

#define SAMPLE_COUNT 256

namespace frequencies {

static int16_t q15buf[SAMPLE_COUNT];
// arm_rfft_q15 requires 2*N output values (complex spectrum)
static int16_t fftOutput[SAMPLE_COUNT * 2];

static arm_rfft_instance_q15 fftInstance;



class FreqSampler : public codal::DataSink {
public:
    SplitterChannel *channel;
    int16_t buf[SAMPLE_COUNT];
    volatile int count;
    volatile bool capturing;
    int32_t dcSum;

    FreqSampler() : channel(nullptr), count(0), capturing(false), dcSum(0) {}

    void setup() {
        MicroBitAudio::requestActivation();
        channel = uBit.audio.rawSplitter->createChannel();
        channel->connect(*this);
    }

    void startCapture() {
        count = 0;
        dcSum = 0;
        __asm__ volatile("" ::: "memory");
        capturing = true;
    }

    virtual int pullRequest() override {
        ManagedBuffer data = channel->pull();
        // if (!capturing)
        //     return DEVICE_OK;

        int16_t *samples = (int16_t *)data.getBytes();
        int n = data.length() / sizeof(int16_t);
        for (int i = 0; i < n && count < SAMPLE_COUNT; i++) {
            buf[count] = samples[i];
            dcSum += samples[i];
            count++;
        }

        if (count >= SAMPLE_COUNT) {
            int16_t dc = (int16_t)(dcSum / SAMPLE_COUNT);
            for (int i = 0; i < SAMPLE_COUNT; i++)
                q15buf[i] = (int16_t)(((int32_t)buf[i] - dc) << 2);
            arm_rfft_q15(&fftInstance, q15buf, fftOutput);

                // __asm__ volatile("" ::: "memory");
            // capturing = false;
        }

        return DEVICE_OK;
    }
};

static FreqSampler *sampler = nullptr;

} // namespace frequencies

#endif // MICROBIT_CODAL

namespace frequencies {

//% advanced=true
void setup() {
    uBit.serial.printf("C++ setup() / configuring samples and starting capture\n");

    if (sampler) return;
    sampler = new FreqSampler();
    sampler->setup();
    arm_rfft_init_q15(&fftInstance, SAMPLE_COUNT, 0, 1);
    sampler->startCapture();
    uBit.audio.activateMic();
}

//%
void dumpSamples() {
#if MICROBIT_CODAL
    setup();
    // sampler->startCapture();
    // while (sampler->capturing)
    //     fiber_sleep(1);
    arm_rfft_q15(&fftInstance, q15buf, fftOutput);
    uBit.serial.printf("FFT (bin,real,imag) \n");
    for (int k = 0; k <= SAMPLE_COUNT / 2; k++)
        uBit.serial.printf("%d %d hZ \t %d,%d,%d\n", k, (int)(k*43.4), fftOutput[2 * k], fftOutput[2 * k + 1]);
#else
    target_panic(PANIC_VARIANT_NOT_SUPPORTED);
#endif
}

// //%
// void runFFT() {
// #if MICROBIT_CODAL
//     uBit.audio.activateMic();
//     ensureInit();
//     sampler->startCapture();
//     while (sampler->capturing)
//         fiber_sleep(1);
//     // q15buf is used as scratch and modified in place by arm_rfft_q15
//     arm_rfft_q15(&fftInstance, q15buf, fftOutput);
// #else
//     target_panic(PANIC_VARIANT_NOT_SUPPORTED);
// #endif
// }

} // namespace frequencies
