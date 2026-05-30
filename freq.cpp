#include "pxt.h"
#include "MicroBit.h"

#include "dsp/transform_functions.h"

using namespace pxt;


namespace frequencies {

static void processFiber();

static const int   NUM_NOTES  = 36;
static const int   BLOCK_SIZE = 4096;
#define MAKECODE_NOTES NUM_NOTES

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
    987.76660251f  // [35] B5
};


// Sample rate: CODAL rounds 11000 Hz request to 16 MHz / 1440 = 11111.1̄ Hz
static const float SAMPLE_RATE = 11111.111f;
static const float BIN_WIDTH   = SAMPLE_RATE / BLOCK_SIZE;  // ≈2.713 Hz/bin
static float   avgNotePower = 0;
static float   maxNotePower = 0;

static float   maxBinPower = 0;
static int    maxBinIndex = 0;
static Action notesUpdatedAction = nullptr;

// CMSIS-DSP Q15 real FFT state and buffers
static arm_rfft_instance_q15 rfftInst;
static q15_t fftIn[BLOCK_SIZE];           // q15 input (modified in-place by CFFT stage)
static q15_t fftOut[BLOCK_SIZE * 2 + 2]; // complex output: 4 * (N/2) + 2 = 8194 elements

// Per-note results written by processFiber, read by getters
static float notePowers[NUM_NOTES];
static int   noteCents[NUM_NOTES];


class FreqSampler : public codal::DataSink {
public:
    SplitterChannel *channel;
    int16_t cbuf[BLOCK_SIZE];    // circular buffer of most-recent samples
    volatile int  head;          // next write index
    volatile bool getData;       // processFiber sets this to request a snapshot
    volatile bool dataReady;     // sampler sets this after filling fftIn

    FreqSampler() : channel(nullptr), head(0), getData(false), dataReady(false) {}

    void setup() {
        MicroBitAudio::requestActivation();
        channel = uBit.audio.rawSplitter->createChannel();
        channel->connect(*this);
    }

    virtual int pullRequest() override {
        ManagedBuffer data = channel->pull();
        int16_t *samples = (int16_t *)data.getBytes();
        int n = data.length() / sizeof(int16_t);
        for (int i = 0; i < n; i++) {
            cbuf[head] = samples[i];
            if (++head >= BLOCK_SIZE) head = 0;
        }
        if (getData) {
            // Compute integer DC over the circular buffer (12-bit unsigned, mean ≈ 2048)
            int32_t sum = 0;
            int j = head;
            for (int i = 0; i < BLOCK_SIZE; i++) {
                sum += cbuf[j];
                if (++j >= BLOCK_SIZE) j = 0;
            }
            int32_t dc = sum / BLOCK_SIZE;
            // DC-remove and scale to Q15 in one pass:
            // 12-bit AC component is ±2047; shift left 4 fills the Q15 range (±32767).
            j = head;
            for (int i = 0; i < BLOCK_SIZE; i++) {
                int32_t v = ((int32_t)cbuf[j] - dc) << 4;
                if (v < -32768) v = -32768;
                if (v >  32767) v =  32767;
                fftIn[i] = (q15_t)v;
                if (++j >= BLOCK_SIZE) j = 0;
            }
            getData = false;
            __asm__ volatile("" ::: "memory");
            dataReady = true;
        }
        return DEVICE_OK;
    }
};

static FreqSampler *sampler = nullptr;


//% advanced=true
void setup() {
    if (sampler) return;
    sampler = new FreqSampler();
    sampler->setup();
    uBit.audio.activateMic();
    arm_rfft_init_4096_q15(&rfftInst, 0, 1);
    create_fiber(processFiber);
}



static void processFiber() {
    // NOTE: This only uses MAKECODE_NOTES
    while (true) {
        sampler->getData = true;
        while (!sampler->dataReady)
            fiber_sleep(1);
        sampler->dataReady = false;
        __asm__ volatile("" ::: "memory");

        // Compute 4096-point real FFT; fftIn is consumed/modified in-place
        arm_rfft_q15(&rfftInst, fftIn, fftOut);

        // Compute per-note power and sub-bin cents offset.
        // Q15 RFFT with N=4096 downscales by 12 bits internally (output in 13.3 format).
        // Full-scale amplitude 32767 → peak bin magnitude ≈ 32767/2^12 ≈ 8 → power ≈ 64.
        static const float FULL_SCALE_POW = 64.0f;
        float totalPower = 0.0f;
        float curMax     = 0.0f;

        for (int n = 0; n < NUM_NOTES; n++) {
            int k = (int)(noteFreq[n] / BIN_WIDTH + 0.5f);
            if (k < 1 || k >= BLOCK_SIZE / 2) {
                notePowers[n] = 0.0f;
                noteCents[n]  = 0;
                continue;
            }

            auto binPow = [](const q15_t *buf, int bin) -> float {
                float r  = (float)buf[2 * bin];
                float im = (float)buf[2 * bin + 1];
                return r * r + im * im;
            };

            float p0 = binPow(fftOut, k - 1);
            float p1 = binPow(fftOut, k);
            float p2 = binPow(fftOut, k + 1);

            // Parabolic interpolation for fractional bin offset
            float denom    = p0 - 2.0f * p1 + p2;
            float delta    = (fabsf(denom) > 0.001f) ? 0.5f * (p0 - p2) / denom : 0.0f;
            if (delta >  1.0f) delta =  1.0f;
            if (delta < -1.0f) delta = -1.0f;

            float peakHz   = (k + delta) * BIN_WIDTH;
            float centsErr = 1200.0f * log2f(peakHz / noteFreq[n]);
            noteCents[n]   = (int)(centsErr * 40.0f);

            float normPow  = p1 / FULL_SCALE_POW * 1000.0f;
            //if (normPow > 1000.0f) normPow = 1000.0f;
            notePowers[n]  = normPow;
        }

        // Harmonic suppression: for each note, subtract its expected contribution from
        // higher notes whose frequencies fall near its overtones (harmonics 2–6).
        // Processed low-to-high so cascade suppression (harmonic-of-a-harmonic) is automatic.
        // Power model: harmonic h has expected power ≈ P_fund / h².
        for (int fund = 0; fund < NUM_NOTES; fund++) {
            float fp = notePowers[fund];
            if (fp < 1.0f) continue;
            for (int h = 2; h <= 6; h++) {
                float hFreq = (float)h * noteFreq[fund];
                int   best  = -1;
                float bestD = 1e9f;
                for (int j = fund + 1; j < NUM_NOTES; j++) {
                    float d = fabsf(1200.0f * log2f(noteFreq[j] / hFreq));
                    if (d < bestD) { bestD = d; best = j; }
                    else if (noteFreq[j] > hFreq) break;
                }
                if (best >= 0 && bestD < 50.0f) {
                    float sub = fp / (float)(h * h) * 0.8f;
                    notePowers[best] = fmaxf(0.0f, notePowers[best] - sub);
                }
            }
        }

        for (int n = 0; n < NUM_NOTES; n++) {
            totalPower += notePowers[n];
            if (notePowers[n] > curMax) curMax = notePowers[n];
        }
        maxNotePower = curMax;
        avgNotePower = totalPower / NUM_NOTES;

        float maxPower = 0;
        int maxIndex = 0;
        // Find bin with max power
        for (int i = 0;i< BLOCK_SIZE / 2; i++) {
            float p = ((float)fftOut[2 * i] * (float)fftOut[2 * i] + (float)fftOut[2 * i + 1] * (float)fftOut[2 * i + 1]);
            if(p > maxPower) {
                maxPower = p;
                maxIndex = i;
            }
        }
        maxBinPower = maxPower;
        maxBinIndex = maxIndex;

        if (notesUpdatedAction)
            pxt::runAction0(notesUpdatedAction);
    }
}



//%
void onNotesUpdated(Action a) {
    if (notesUpdatedAction) pxt::decr(notesUpdatedAction);
    notesUpdatedAction = a;
    if (a) pxt::incr(a);
}

//%
int getNumNotes() {
    return MAKECODE_NOTES;
}

// Returns normalized power for note i, scaled by 1000 (i.e. 1000 = full-scale).
//%
float getNotePower(int i) {
    if (i < 0 || i >= MAKECODE_NOTES) return 0;
    return notePowers[i];
}

// Returns the mean normalized power across all notes.
//%
float getAvgNotePower() {
    return avgNotePower;
}

// Returns the max note power
//%
float getMaxNotePower() {
    return maxNotePower;
}

//% 
float getBin(int binIndex) {
    if (binIndex < 0 || binIndex >= BLOCK_SIZE / 2) return 0;
    return ((float)fftOut[2 * binIndex] * (float)fftOut[2 * binIndex] + (float)fftOut[2 * binIndex + 1] * (float)fftOut[2 * binIndex + 1]);
}

//% 
float getMaxBinPower() {
    return maxBinPower;
}   

//%
float getMaxBinIndex() {
    return maxBinIndex;
}

//% 
float getBinWidth() {
    return BIN_WIDTH;
}


// Returns the cents error for note i (range roughly -2000 to +2000; divide by 40 for cents).
//%
int getNoteCents(int i) {
    if (i < 0 || i >= MAKECODE_NOTES) return 0;
    return noteCents[i];
}

} // namespace frequencies
