#include "pxt.h"
#include "MicroBit.h"

using namespace pxt;

#if MICROBIT_CODAL


namespace frequencies {

static const int   NUM_NOTES  = 48;     // C3–B6 = 4 octaves × 12 semitones
static const int   BLOCK_SIZE = 1559;   // ~140 ms at 11136 Hz
static const float FS         = 11136.0f;

// noteFreq[48]: C3 (130.81 Hz) through B6 (1975.53 Hz)
// Computed as 440 * 2^((semitone - 9) / 12), where semitone 0 = C3
static const float noteFreq[NUM_NOTES] = {
    130.81278265f,  // [ 0] C3
    138.59131549f,  // [ 1] C#3
    146.83238396f,  // [ 2] D3
    155.56349186f,  // [ 3] D#3
    164.81377846f,  // [ 4] E3
    174.61411572f,  // [ 5] F3
    184.99721136f,  // [ 6] F#3
    195.99771799f,  // [ 7] G3
    207.65234879f,  // [ 8] G#3
    220.00000000f,  // [ 9] A3
    233.08188076f,  // [10] A#3
    246.94165063f,  // [11] B3
    261.62556530f,  // [12] C4
    277.18263098f,  // [13] C#4
    293.66476792f,  // [14] D4
    311.12698372f,  // [15] D#4
    329.62755691f,  // [16] E4
    349.22823143f,  // [17] F4
    369.99442271f,  // [18] F#4
    391.99543598f,  // [19] G4
    415.30469758f,  // [20] G#4
    440.00000000f,  // [21] A4
    466.16376152f,  // [22] A#4
    493.88330126f,  // [23] B4
    523.25113060f,  // [24] C5
    554.36526195f,  // [25] C#5
    587.32953583f,  // [26] D5
    622.25396744f,  // [27] D#5
    659.25511383f,  // [28] E5
    698.45646287f,  // [29] F5
    739.98884542f,  // [30] F#5
    783.99087196f,  // [31] G5
    830.60939516f,  // [32] G#5
    880.00000000f,  // [33] A5
    932.32752304f,  // [34] A#5
    987.76660251f,  // [35] B5
   1046.50226120f,  // [36] C6
   1108.73052391f,  // [37] C#6
   1174.65907167f,  // [38] D6
   1244.50793489f,  // [39] D#6
   1318.51022765f,  // [40] E6
   1396.91292573f,  // [41] F6
   1479.97769085f,  // [42] F#6
   1567.98174393f,  // [43] G6
   1661.21879032f,  // [44] G#6
   1760.00000000f,  // [45] A6
   1864.65504607f,  // [46] A#6
   1975.53320502f,  // [47] B6
};

// goertzelCoeff[48]: 2*cos(2π*f/fs) for each note at fs=11136 Hz
static const float goertzelCoeff[NUM_NOTES] = {
    1.99455493f,  // [ 0] C3   ( 130.8128 Hz)
    1.99388846f,  // [ 1] C#3  ( 138.5913 Hz)
    1.99314050f,  // [ 2] D3   ( 146.8324 Hz)
    1.99230096f,  // [ 3] D#3  ( 155.5635 Hz)
    1.99135898f,  // [ 4] E3   ( 164.8138 Hz)
    1.99030144f,  // [ 5] F3   ( 174.6141 Hz)
    1.98911482f,  // [ 6] F#3  ( 184.9972 Hz)
    1.98778766f,  // [ 7] G3   ( 195.9977 Hz)
    1.98628881f,  // [ 8] G#3  ( 207.6523 Hz)
    1.98461182f,  // [ 9] A3   ( 220.0000 Hz)
    1.98271957f,  // [10] A#3  ( 233.0819 Hz)
    1.98061853f,  // [11] B3   ( 246.9417 Hz)
    1.97824932f,  // [12] C4   ( 261.6256 Hz)
    1.97559111f,  // [13] C#4  ( 277.1826 Hz)
    1.97260879f,  // [14] D4   ( 293.6648 Hz)
    1.96926303f,  // [15] D#4  ( 311.1270 Hz)
    1.96550977f,  // [16] E4   ( 329.6276 Hz)
    1.96129970f,  // [17] F4   ( 349.2282 Hz)
    1.95657760f,  // [18] F#4  ( 369.9944 Hz)
    1.95128169f,  // [19] G4   ( 391.9954 Hz)
    1.94534286f,  // [20] G#4  ( 415.3047 Hz)
    1.93868384f,  // [21] A4   ( 440.0000 Hz)
    1.93121826f,  // [22] A#4  ( 466.1638 Hz)
    1.92284966f,  // [23] B4   ( 493.8833 Hz)
    1.91347036f,  // [24] C5   ( 523.2511 Hz)
    1.90296025f,  // [25] C#5  ( 554.3653 Hz)
    1.89118545f,  // [26] D5   ( 587.3295 Hz)
    1.87799687f,  // [27] D#5  ( 622.2540 Hz)
    1.86322865f,  // [28] E5   ( 659.2551 Hz)
    1.84669650f,  // [29] F5   ( 698.4565 Hz)
    1.82819590f,  // [30] F#5  ( 739.9888 Hz)
    1.80750024f,  // [31] G5   ( 783.9909 Hz)
    1.78435885f,  // [32] G#5  ( 830.6094 Hz)
    1.75849503f,  // [33] A5   ( 880.0000 Hz)
    1.72960396f,  // [34] A#5  ( 932.3275 Hz)
    1.69735081f,  // [35] B5   ( 987.7666 Hz)
    1.66136882f,  // [36] C6  (1046.5023 Hz)
    1.62125772f,  // [37] C#6 (1108.7305 Hz)
    1.57658241f,  // [38] D6  (1174.6591 Hz)
    1.52687224f,  // [39] D#6 (1244.5079 Hz)
    1.47162101f,  // [40] E6  (1318.5102 Hz)
    1.41028796f,  // [41] F6  (1396.9129 Hz)
    1.34230024f,  // [42] F#6 (1479.9777 Hz)
    1.26705710f,  // [43] G6  (1567.9817 Hz)
    1.18393651f,  // [44] G#6 (1661.2188 Hz)
    1.09230475f,  // [45] A6  (1760.0000 Hz)
    0.99152986f,  // [46] A#6 (1864.6550 Hz)
    0.88099977f,  // [47] B6  (1975.5332 Hz)
};


// goertzelCoeff_mid[NUM_NOTES + 1]:
//   Each entry is the semitone *boundary* — exactly halfway (in cents)
//   between two adjacent notes, i.e. ±50¢ from both neighbours.
//
//   For note i:
//     -50¢ bin  →  goertzelCoeff_mid[i]
//     +50¢ bin  →  goertzelCoeff_mid[i + 1]
//
//   Replaces the two separate goertzelCoeff_lo2[48] and
//   goertzelCoeff_hi2[48] arrays, saving 192 bytes of flash.
static const float goertzelCoeff_mid[NUM_NOTES + 1] = {
    1.99486040f,  // [ 0]   127.09 Hz  (below C3)
    1.99423129f,  // [ 1]   134.65 Hz  (below C#3  /  above C3)
    1.99352522f,  // [ 2]   142.65 Hz  (below D3   /  above C#3)
    1.99273279f,  // [ 3]   151.13 Hz  (below D#3  /  above D3)
    1.99184344f,  // [ 4]   160.12 Hz  (below E3   /  above D#3)
    1.99084533f,  // [ 5]   169.64 Hz  (below F3   /  above E3)
    1.98972519f,  // [ 6]   179.73 Hz  (below F#3  /  above F3)
    1.98846813f,  // [ 7]   190.42 Hz  (below G3   /  above F#3)
    1.98705744f,  // [ 8]   201.74 Hz  (below G#3  /  above G3)
    1.98547439f,  // [ 9]   213.74 Hz  (below A3   /  above G#3)
    1.98369797f,  // [10]   226.45 Hz  (below A#3  /  above A3)
    1.98170464f,  // [11]   239.91 Hz  (below B3   /  above A#3)
    1.97946800f,  // [12]   254.18 Hz  (below C4   /  above B3)
    1.97695844f,  // [13]   269.29 Hz  (below C#4  /  above C4)
    1.97414282f,  // [14]   285.30 Hz  (below D4   /  above C#4)
    1.97098398f,  // [15]   302.27 Hz  (below D#4  /  above D4)
    1.96744029f,  // [16]   320.24 Hz  (below E4   /  above D#4)
    1.96346514f,  // [17]   339.29 Hz  (below F4   /  above E4)
    1.95900635f,  // [18]   359.46 Hz  (below F#4  /  above F4)
    1.95400551f,  // [19]   380.84 Hz  (below G4   /  above F#4)
    1.94839727f,  // [20]   403.48 Hz  (below G#4  /  above G4)
    1.94210855f,  // [21]   427.47 Hz  (below A4   /  above G#4)
    1.93505765f,  // [22]   452.89 Hz  (below A#4  /  above A4)
    1.92715329f,  // [23]   479.82 Hz  (below B4   /  above A#4)
    1.91829355f,  // [24]   508.36 Hz  (below C5   /  above B4)
    1.90836469f,  // [25]   538.58 Hz  (below C#5  /  above C5)
    1.89723988f,  // [26]   570.61 Hz  (below D5   /  above C#5)
    1.88477784f,  // [27]   604.54 Hz  (below D#5  /  above D5)
    1.87082128f,  // [28]   640.49 Hz  (below E5   /  above D#5)
    1.85519535f,  // [29]   678.57 Hz  (below F5   /  above E5)
    1.83770588f,  // [30]   718.92 Hz  (below F#5  /  above F5)
    1.81813753f,  // [31]   761.67 Hz  (below G5   /  above F#5)
    1.79625192f,  // [32]   806.96 Hz  (below G#5  /  above G5)
    1.77178561f,  // [33]   854.95 Hz  (below A5   /  above G#5)
    1.74444810f,  // [34]   905.79 Hz  (below A#5  /  above A5)
    1.71391980f,  // [35]   959.65 Hz  (below B5   /  above A#5)
    1.67985013f,  // [36]  1016.71 Hz  (below C6   /  above B5)
    1.64185578f,  // [37]  1077.17 Hz  (below C#6  /  above C6)
    1.59951917f,  // [38]  1141.22 Hz  (below D6   /  above C#6)
    1.55238750f,  // [39]  1209.08 Hz  (below D#6  /  above D6)
    1.49997227f,  // [40]  1280.97 Hz  (below E6   /  above D#6)
    1.44174980f,  // [41]  1357.15 Hz  (below F6   /  above E6)
    1.37716289f,  // [42]  1437.85 Hz  (below F#6  /  above F6)
    1.30562407f,  // [43]  1523.34 Hz  (below G6   /  above F#6)
    1.22652096f,  // [44]  1613.93 Hz  (below G#6  /  above G6)
    1.13922426f,  // [45]  1709.90 Hz  (below A6   /  above G#6)
    1.04309917f,  // [46]  1811.57 Hz  (below A#6  /  above A6)
    0.93752107f,  // [47]  1919.29 Hz  (below B6   /  above A#6)
    0.82189647f,  // [48]  2033.42 Hz  (above B6)
};


static const char * const noteName[NUM_NOTES] = {
    "C3",   // [ 0]  130.81 Hz
    "C#3",  // [ 1]  138.59 Hz
    "D3",   // [ 2]  146.83 Hz
    "D#3",  // [ 3]  155.56 Hz
    "E3",   // [ 4]  164.81 Hz
    "F3",   // [ 5]  174.61 Hz
    "F#3",  // [ 6]  185.00 Hz
    "G3",   // [ 7]  196.00 Hz
    "G#3",  // [ 8]  207.65 Hz
    "A3",   // [ 9]  220.00 Hz
    "A#3",  // [10]  233.08 Hz
    "B3",   // [11]  246.94 Hz
    "C4",   // [12]  261.63 Hz
    "C#4",  // [13]  277.18 Hz
    "D4",   // [14]  293.66 Hz
    "D#4",  // [15]  311.13 Hz
    "E4",   // [16]  329.63 Hz
    "F4",   // [17]  349.23 Hz
    "F#4",  // [18]  370.00 Hz
    "G4",   // [19]  392.00 Hz
    "G#4",  // [20]  415.30 Hz
    "A4",   // [21]  440.00 Hz
    "A#4",  // [22]  466.16 Hz
    "B4",   // [23]  493.88 Hz
    "C5",   // [24]  523.25 Hz
    "C#5",  // [25]  554.37 Hz
    "D5",   // [26]  587.33 Hz
    "D#5",  // [27]  622.25 Hz
    "E5",   // [28]  659.26 Hz
    "F5",   // [29]  698.46 Hz
    "F#5",  // [30]  740.00 Hz
    "G5",   // [31]  784.00 Hz
    "G#5",  // [32]  830.61 Hz
    "A5",   // [33]  880.00 Hz
    "A#5",  // [34]  932.33 Hz
    "B5",   // [35]  987.77 Hz
    "C6",   // [36] 1046.50 Hz
    "C#6",  // [37] 1108.73 Hz
    "D6",   // [38] 1174.66 Hz
    "D#6",  // [39] 1244.51 Hz
    "E6",   // [40] 1318.51 Hz
    "F6",   // [41] 1396.91 Hz
    "F#6",  // [42] 1479.98 Hz
    "G6",   // [43] 1567.98 Hz
    "G#6",  // [44] 1661.22 Hz
    "A6",   // [45] 1760.00 Hz
    "A#6",  // [46] 1864.66 Hz
    "B6",   // [47] 1975.53 Hz
};

class FreqSampler : public codal::DataSink {
public:
    SplitterChannel *channel;
    int16_t buf[BLOCK_SIZE];
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
        for (int i = 0; i < n && count < BLOCK_SIZE; i++) {
            buf[count] = samples[i];
            count++;
        }

        if (count >= BLOCK_SIZE) {
            __asm__ volatile("" ::: "memory");
            capturing = false;
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
    sampler->startCapture();
    uBit.audio.activateMic();
}

// Returns 2*cos(2π*freq/FS) — the Goertzel recurrence coefficient for a given frequency.
float goertzelCoeffFor(float freq) {
    return 2.0f * cosf(2.0f * 3.14159265358979323846f * freq / FS);
}

// Mean of the block; subtracted from each sample to remove DC bias from the ADC.
static float computeDC(const int16_t *samples) {
    int32_t sum = 0;
    for (int n = 0; n < BLOCK_SIZE; n++)
        sum += samples[n];
    return (float)sum / (float)BLOCK_SIZE;
}

// Returns power at note i for a block of BLOCK_SIZE samples with DC removed.
float goertzelPower(float coeff, const int16_t *samples, float dc) {
    float s1 = 0.0f, s2 = 0.0f, s0;

    for (int n = 0; n < BLOCK_SIZE; n++) {
        s0 = ((float)samples[n] - dc) + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    return s1*s1 + s2*s2 - coeff*s1*s2;
}

// Detect the loudest note in a block; returns index (0=C3 … 47=B6), or -1 if
// no note exceeds the threshold.
int detectNote(const int16_t *samples, float dc, float threshold) {
    int   best      = -1;
    float bestPower = threshold;

    for (int i = 0; i < NUM_NOTES; i++) {
        const float coeff = goertzelCoeff[i];
        float p = goertzelPower(coeff, samples, dc);
        if (p > bestPower) { bestPower = p; best = i; }
    }
    return best;
}

//%
void dumpSamples() {
#if MICROBIT_CODAL
    setup();
    sampler->startCapture();
    while (sampler->capturing)
        fiber_sleep(1);

    float dc = computeDC(sampler->buf);
    float maxPower = 0.0f;
    int maxIndex = -1;

    for (int i = 0; i < NUM_NOTES; i++) {
        // Normalize by N² so the result is always a manageable float
        const float coeff = goertzelCoeff[i];
        float p = goertzelPower(coeff, sampler->buf, dc);
        if(p > maxPower) { maxPower = p; maxIndex = i; }
        p=p / ((float)BLOCK_SIZE * BLOCK_SIZE);
        // Instead of: uBit.serial.printf("%s %f\n", noteName[i], p);
        int whole = (int)p;
        int frac  = (int)((p - whole) * 1000);   // 3 decimal places
        const char *pad = (frac < 10) ? "00" : (frac < 100) ? "0" : "";
        uBit.serial.printf("%s=%d.%s%d\n", noteName[i], whole, pad, frac);
    }
    if(maxIndex >= 0) {
        uBit.serial.printf("Loudest note %s (power=%d)\n", noteName[maxIndex], (int)(maxPower / ((float)BLOCK_SIZE * BLOCK_SIZE)));
    } else {
        uBit.serial.printf("No note detected above threshold\n");
    }
    maxPower = 0.0f;
    int freqAtMaxPower = 0;
    for(int freq = 120; freq <= 2000; freq += 10) {
        float coeff = goertzelCoeffFor(freq);
        float p = goertzelPower(coeff, sampler->buf, dc);
        if(p > maxPower) {
            maxPower = p;
            freqAtMaxPower = freq;
        }
    }
    uBit.serial.printf("Loudest frequency %d Hz (power=%d)\n", freqAtMaxPower, (int)(maxPower / ((float)BLOCK_SIZE * BLOCK_SIZE)));

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
