#include "pxt.h"
#include "MicroBit.h"

using namespace pxt;

#if MICROBIT_CODAL


namespace frequencies {

static const int   NUM_NOTES  = 48;     // C3–B6
static const int   BLOCK_SIZE = 1559;   // ~140 ms at 11136 Hz
static const float FS         = 11136.0f;

// Coverage with 5 bins at -50¢,-25¢,0¢,+25¢,+50¢  (bin spacing = 25 cents)
// Single-filter -3dB half-width = 4.29 Hz → seamless when 25¢ in Hz < 8.57 Hz
//
//  Note    f (Hz)   25¢ (Hz)   overlap/gap        verdict
//  C3       130.81       1.90   -6.67 Hz    overlap 6.67 Hz  seamless ✓
//  B3       246.94       3.59   -4.98 Hz    overlap 4.98 Hz  seamless ✓
//  C4       261.63       3.81   -4.77 Hz    overlap 4.77 Hz  seamless ✓
//  A4       440.00       6.40   -2.17 Hz    overlap 2.17 Hz  seamless ✓
//  C5       523.25       7.61   -0.96 Hz    overlap 0.96 Hz  seamless ✓
//  B5       987.77      14.37   +5.80 Hz    gap ~10¢ dead zone
//  C6      1046.50      15.22   +6.65 Hz    gap ~11¢ dead zone
//  B6      1975.53      28.73   +20.16 Hz   gap ~18¢ dead zone

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

static const char * const noteName[NUM_NOTES] = {
    "C3",  "C#3", "D3",  "D#3", "E3",  "F3",  "F#3", "G3",  "G#3", "A3",  "A#3", "B3",
    "C4",  "C#4", "D4",  "D#4", "E4",  "F4",  "F#4", "G4",  "G#4", "A4",  "A#4", "B4",
    "C5",  "C#5", "D5",  "D#5", "E5",  "F5",  "F#5", "G5",  "G#5", "A5",  "A#5", "B5",
    "C6",  "C#6", "D6",  "D#6", "E6",  "F6",  "F#6", "G6",  "G#6", "A6",  "A#6", "B6",
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

// -25 cents from centre
static const float goertzelCoeff_lo1[NUM_NOTES] = {
    1.99470986f,  // [ 0] C3   (128.94 Hz)
    1.99406234f,  // [ 1] C#3  (136.60 Hz)
    1.99333560f,  // [ 2] D3   (144.73 Hz)
    1.99251998f,  // [ 3] D#3  (153.33 Hz)
    1.99160460f,  // [ 4] E3   (162.45 Hz)
    1.99057729f,  // [ 5] F3   (172.11 Hz)
    1.98942439f,  // [ 6] F#3  (182.34 Hz)
    1.98813056f,  // [ 7] G3   (193.19 Hz)
    1.98667862f,  // [ 8] G#3  (204.68 Hz)
    1.98504929f,  // [ 9] A3   (216.85 Hz)
    1.98322096f,  // [10] A#3  (229.74 Hz)
    1.98116939f,  // [11] B3   (243.40 Hz)
    1.97886742f,  // [12] C4   (257.87 Hz)
    1.97628461f,  // [13] C#4  (273.21 Hz)
    1.97338683f,  // [14] D4   (289.45 Hz)
    1.97013586f,  // [15] D#4  (306.67 Hz)
    1.96648889f,  // [16] E4   (324.90 Hz)
    1.96239795f,  // [17] F4   (344.22 Hz)
    1.95780938f,  // [18] F#4  (364.69 Hz)
    1.95266311f,  // [19] G4   (386.38 Hz)
    1.94689192f,  // [20] G#4  (409.35 Hz)
    1.94042067f,  // [21] A4   (433.69 Hz)
    1.93316536f,  // [22] A#4  (459.48 Hz)
    1.92503215f,  // [23] B4   (486.80 Hz)
    1.91591628f,  // [24] C5   (515.75 Hz)
    1.90570087f,  // [25] C#5  (546.42 Hz)
    1.89425560f,  // [26] D5   (578.91 Hz)
    1.88143533f,  // [27] D#5  (613.33 Hz)
    1.86707854f,  // [28] E5   (649.80 Hz)
    1.85100573f,  // [29] F5   (688.44 Hz)
    1.83301759f,  // [30] F#5  (729.38 Hz)
    1.81289322f,  // [31] G5   (772.75 Hz)
    1.79038815f,  // [32] G#5  (818.70 Hz)
    1.76523238f,  // [33] A5   (867.38 Hz)
    1.73712831f,  // [34] A#5  (918.96 Hz)
    1.70574879f,  // [35] B5   (973.61 Hz)
    1.67073520f,  // [36] C6  (1031.50 Hz)
    1.63169579f,  // [37] C#6 (1092.83 Hz)
    1.58820426f,  // [38] D6  (1157.82 Hz)
    1.53979889f,  // [39] D#6 (1226.67 Hz)
    1.48598229f,  // [40] E6  (1299.61 Hz)
    1.42622220f,  // [41] F6  (1376.89 Hz)
    1.35995347f,  // [42] F#6 (1458.76 Hz)
    1.28658181f,  // [43] G6  (1545.50 Hz)
    1.20548974f,  // [44] G#6 (1637.40 Hz)
    1.11604536f,  // [45] A6  (1734.77 Hz)
    1.01761477f,  // [46] A#6 (1837.92 Hz)
    0.90957892f,  // [47] B6  (1947.21 Hz)
};

// Centre
static const float goertzelCoeff[NUM_NOTES] = {
    1.99455492f,  // [ 0] C3   (130.81 Hz)
    1.99388844f,  // [ 1] C#3  (138.59 Hz)
    1.99314043f,  // [ 2] D3   (146.83 Hz)
    1.99230094f,  // [ 3] D#3  (155.56 Hz)
    1.99135877f,  // [ 4] E3   (164.81 Hz)
    1.99030141f,  // [ 5] F3   (174.61 Hz)
    1.98911478f,  // [ 6] F#3  (185.00 Hz)
    1.98778311f,  // [ 7] G3   (196.00 Hz)
    1.98628872f,  // [ 8] G#3  (207.65 Hz)
    1.98461176f,  // [ 9] A3   (220.00 Hz)
    1.98273000f,  // [10] A#3  (233.08 Hz)
    1.98061850f,  // [11] B3   (246.94 Hz)
    1.97824932f,  // [12] C4   (261.63 Hz)
    1.97559111f,  // [13] C#4  (277.18 Hz)
    1.97260879f,  // [14] D4   (293.66 Hz)
    1.96926303f,  // [15] D#4  (311.13 Hz)
    1.96550977f,  // [16] E4   (329.63 Hz)
    1.96129970f,  // [17] F4   (349.23 Hz)
    1.95657760f,  // [18] F#4  (369.99 Hz)
    1.95128169f,  // [19] G4   (392.00 Hz)
    1.94534286f,  // [20] G#4  (415.30 Hz)
    1.93868384f,  // [21] A4   (440.00 Hz)
    1.93121826f,  // [22] A#4  (466.16 Hz)
    1.92284966f,  // [23] B4   (493.88 Hz)
    1.91347036f,  // [24] C5   (523.25 Hz)
    1.90296025f,  // [25] C#5  (554.37 Hz)
    1.89118545f,  // [26] D5   (587.33 Hz)
    1.87799687f,  // [27] D#5  (622.25 Hz)
    1.86322865f,  // [28] E5   (659.26 Hz)
    1.84669650f,  // [29] F5   (698.46 Hz)
    1.82819590f,  // [30] F#5  (739.99 Hz)
    1.80750024f,  // [31] G5   (783.99 Hz)
    1.78435885f,  // [32] G#5  (830.61 Hz)
    1.75849503f,  // [33] A5   (880.00 Hz)
    1.72960396f,  // [34] A#5  (932.33 Hz)
    1.69735081f,  // [35] B5   (987.77 Hz)
    1.66136882f,  // [36] C6  (1046.50 Hz)
    1.62125772f,  // [37] C#6 (1108.73 Hz)
    1.57658241f,  // [38] D6  (1174.66 Hz)
    1.52687224f,  // [39] D#6 (1244.51 Hz)
    1.47162101f,  // [40] E6  (1318.51 Hz)
    1.41028796f,  // [41] F6  (1396.91 Hz)
    1.34230024f,  // [42] F#6 (1479.98 Hz)
    1.26705710f,  // [43] G6  (1567.98 Hz)
    1.18393651f,  // [44] G#6 (1661.22 Hz)
    1.09230475f,  // [45] A6  (1760.00 Hz)
    0.99152986f,  // [46] A#6 (1864.66 Hz)
    0.88099977f,  // [47] B6  (1975.53 Hz)
};

// +25 cents from centre
static const float goertzelCoeff_hi1[NUM_NOTES] = {
    1.99439544f,  // [ 0] C3   (132.72 Hz)
    1.99370945f,  // [ 1] C#3  (140.61 Hz)
    1.99293955f,  // [ 2] D3   (148.97 Hz)
    1.99207549f,  // [ 3] D#3  (157.83 Hz)
    1.99110576f,  // [ 4] E3   (167.21 Hz)
    1.99001745f,  // [ 5] F3   (177.15 Hz)
    1.98879611f,  // [ 6] F#3  (187.69 Hz)
    1.98742550f,  // [ 7] G3   (198.85 Hz)
    1.98588742f,  // [ 8] G#3  (210.67 Hz)
    1.98416145f,  // [ 9] A3   (223.20 Hz)
    1.98222470f,  // [10] A#3  (236.47 Hz)
    1.98005153f,  // [11] B3   (250.53 Hz)
    1.97761317f,  // [12] C4   (265.43 Hz)
    1.97487738f,  // [13] C#4  (281.21 Hz)
    1.97180806f,  // [14] D4   (297.94 Hz)
    1.96836475f,  // [15] D#4  (315.65 Hz)
    1.96450213f,  // [16] E4   (334.42 Hz)
    1.96016947f,  // [17] F4   (354.31 Hz)
    1.95530998f,  // [18] F#4  (375.38 Hz)
    1.94986013f,  // [19] G4   (397.70 Hz)
    1.94374884f,  // [20] G#4  (421.35 Hz)
    1.93689665f,  // [21] A4   (446.40 Hz)
    1.92921477f,  // [22] A#4  (472.94 Hz)
    1.92060406f,  // [23] B4   (501.07 Hz)
    1.91095383f,  // [24] C5   (530.86 Hz)
    1.90014067f,  // [25] C#5  (562.43 Hz)
    1.88802703f,  // [26] D5   (595.87 Hz)
    1.87445978f,  // [27] D#5  (631.30 Hz)
    1.85926861f,  // [28] E5   (668.84 Hz)
    1.84226435f,  // [29] F5   (708.62 Hz)
    1.82323713f,  // [30] F#5  (750.75 Hz)
    1.80195452f,  // [31] G5   (795.39 Hz)
    1.77815953f,  // [32] G#5  (842.69 Hz)
    1.75156861f,  // [33] A5   (892.80 Hz)
    1.72186963f,  // [34] A#5  (945.89 Hz)
    1.68871994f,  // [35] B5  (1002.13 Hz)
    1.65174455f,  // [36] C6  (1061.72 Hz)
    1.61053456f,  // [37] C#6 (1124.86 Hz)
    1.56464606f,  // [38] D6  (1191.74 Hz)
    1.51359946f,  // [39] D#6 (1262.61 Hz)
    1.45687977f,  // [40] E6  (1337.69 Hz)
    1.39393793f,  // [41] F6  (1417.23 Hz)
    1.32419364f,  // [42] F#6 (1501.50 Hz)
    1.24704011f,  // [43] G6  (1590.79 Hz)
    1.16185133f,  // [44] G#6 (1685.38 Hz)
    1.06799261f,  // [45] A6  (1785.60 Hz)
    0.96483504f,  // [46] A#6 (1891.78 Hz)
    0.85177504f,  // [47] B6  (2004.27 Hz)
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


static float notePowers[NUM_NOTES];
static int noteCents[NUM_NOTES];

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
// Get time at start of computation
    long startTime = uBit.systemTime();
    float p0 = goertzelPower(goertzelCoeff_mid[0], sampler->buf, dc);
    for (int i = 0; i < NUM_NOTES; i++) {
        float p4 = goertzelPower(goertzelCoeff_mid[i+1], sampler->buf, dc);
        float p[5] = {
            p0,
            goertzelPower(goertzelCoeff_lo1[i], sampler->buf, dc),  // -25¢
            goertzelPower(goertzelCoeff[i],     sampler->buf, dc),  //   0¢
            goertzelPower(goertzelCoeff_hi1[i], sampler->buf, dc),  // +25¢
            p4,
        };
        p0 = p4;

        float peak = 0.0f;
        for (int k = 0; k < 5; k++) if (p[k] > peak) peak = p[k];

        float total = p[0]+p[1]+p[2]+p[3]+p[4];
        float x = (-2.0f*p[0] - 1.0f*p[1] + 0.0f*p[2] + 1.0f*p[3] + 2.0f*p[4]) / total;
        int centsError = (int)(x * 1000.0f);
        notePowers[i] = peak;
        noteCents[i] = centsError;
    }

    long endTime = uBit.systemTime();
    long elapsedTime = endTime - startTime;
    uBit.serial.printf("Computation time: %d ms\n", (int)elapsedTime);

    for(int i = 0; i < NUM_NOTES; i++) {
        if(notePowers[i] > maxPower) { maxPower = notePowers[i]; maxIndex = i; }
        float pNorm = notePowers[i] / ((float)BLOCK_SIZE * BLOCK_SIZE);
        int whole = (int)pNorm;
        int frac  = (int)((pNorm - whole) * 1000);   // 3 decimal places
        const char *pad = (frac < 10) ? "00" : (frac < 100) ? "0" : "";
        uBit.serial.printf("%s=%d.%s%d centsError %d\n", noteName[i], whole, pad, frac, noteCents[i]);
        // Wait for the serial buffer to flush before starting the next line, to avoid interleaving output
        uBit.sleep(100);
    }
    if(maxIndex >= 0) {
        uBit.serial.printf("Loudest note %s (power=%d)\n", noteName[maxIndex], (int)(maxPower / ((float)BLOCK_SIZE * BLOCK_SIZE)));
    } else {
        uBit.serial.printf("No note detected above threshold\n");
    }
    // Print computation time
    // maxPower = 0.0f;
    // int freqAtMaxPower = 0;
    // for(int freq = 120; freq <= 2000; freq += 10) {
    //     float coeff = goertzelCoeffFor(freq);
    //     float p = goertzelPower(coeff, sampler->buf, dc);
    //     if(p > maxPower) {
    //         maxPower = p;
    //         freqAtMaxPower = freq;
    //     }
    // }
    // uBit.serial.printf("Loudest frequency %d Hz (power=%d)\n", freqAtMaxPower, (int)(maxPower / ((float)BLOCK_SIZE * BLOCK_SIZE)));

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
