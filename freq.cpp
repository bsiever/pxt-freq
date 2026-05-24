#include "pxt.h"
#include "MicroBit.h"
#include "cmsis-dsp/arm_math.h"

using namespace pxt;

#if MICROBIT_CODAL

#define SAMPLE_COUNT 256

namespace frequencies {

class FreqSampler : public codal::DataSink {
public:
    SplitterChannel *channel;
    int16_t buf[SAMPLE_COUNT];
    volatile int count;
    volatile bool capturing;

    FreqSampler() : channel(nullptr), count(0), capturing(false) {}

    void setup() {
        MicroBitAudio::requestActivation();
        channel = uBit.audio.rawSplitter->createChannel();
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

        int16_t *samples = (int16_t *)data.getBytes();
        int n = data.length() / sizeof(int16_t);
        for (int i = 0; i < n && count < SAMPLE_COUNT; i++)
            buf[count++] = samples[i];

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

static int16_t q15buf[SAMPLE_COUNT];
// arm_rfft_q15 requires 2*N output values (complex spectrum)
static int16_t fftOutput[SAMPLE_COUNT * 2];
static arm_rfft_instance_q15 fftInstance;
static bool fftInitialized = false;

// Converts 14-bit signed ADC samples (stored as int16_t) to Q15.
// rawSplitter data has a DC bias (the StreamNormalizer on the splitter path
// exists specifically to remove it). We estimate DC as the mean of the
// captured buffer, subtract it, then left-shift by 2 to scale the 14-bit
// range to Q15 [-32768, 32764].
static void convertToQ15() {
    int32_t sum = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++)
        sum += sampler->buf[i];
    int16_t dc = (int16_t)(sum / SAMPLE_COUNT);

    for (int i = 0; i < SAMPLE_COUNT; i++)
        q15buf[i] = (int16_t)(((int32_t)sampler->buf[i] - dc) << 2);
}

static void ensureFFTInit() {
    if (!fftInitialized) {
        arm_rfft_init_q15(&fftInstance, SAMPLE_COUNT, 0, 1);
        fftInitialized = true;
    }
}

} // namespace frequencies

#endif // MICROBIT_CODAL

namespace frequencies {

//%
void dumpSamples() {
#if MICROBIT_CODAL
    uBit.serial.printf("Starting Capture\n");
    uBit.audio.activateMic();
    ensureInit();
    sampler->startCapture();
    while (sampler->capturing)
        fiber_sleep(1);

    uBit.serial.printf("Samples:\n");
    for (int i = 0; i < SAMPLE_COUNT; i++)
        uBit.serial.printf("%d\n", sampler->buf[i]);

    convertToQ15();
    ensureFFTInit();
    // q15buf is modified in place as scratch during the FFT
    arm_rfft_q15(&fftInstance, q15buf, fftOutput);

    // Print unique bins 0..N/2 (real and imaginary parts)
    uBit.serial.printf("FFT (bin,real,imag):\n");
    for (int k = 0; k <= SAMPLE_COUNT / 2; k++)
        uBit.serial.printf("%d,%d,%d\n", k, fftOutput[2 * k], fftOutput[2 * k + 1]);
#else
    target_panic(PANIC_VARIANT_NOT_SUPPORTED);
#endif
}

//%
void runFFT() {
#if MICROBIT_CODAL
    uBit.audio.activateMic();
    ensureInit();
    sampler->startCapture();
    while (sampler->capturing)
        fiber_sleep(1);
    convertToQ15();
    ensureFFTInit();
    // q15buf is used as scratch and modified in place by arm_rfft_q15
    arm_rfft_q15(&fftInstance, q15buf, fftOutput);
#else
    target_panic(PANIC_VARIANT_NOT_SUPPORTED);
#endif
}

} // namespace frequencies
