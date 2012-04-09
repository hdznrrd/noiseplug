#include <inttypes.h>

#ifndef PROGMEM
#define PROGMEM /* nix */
#endif

#define arpeggio_DELAY 128
#define BASS_DURATION 800
#define SNARE_DURATION 200

#define BIT(x,n) (((x)&(1<<(n)))>>(n))
#define SO(x) (sizeof((x))/sizeof(*(x)))

#define A1 0
#define As1 1
#define B1 2
#define C1 3
#define Cs1 4
#define D1 5
#define Ds1 6
#define E1 7
#define F1 8
#define Fs1 9
#define G1 10
#define Gs1 11
#define A2 12
#define Aa2 13
#define B2 14
#define C2 15
#define Cs2 16
#define D2 17
#define Ds2 18
#define E2 19
#define F2 20
#define Fs2 21
#define G2 22
#define Gs2 23
#define HOLD 24
#define OFF 25

static const PROGMEM uint8_t sin[] = {0, 49, 97, 141, 180, 212, 235, 250, 254, 250, 235, 212, 180, 141, 97, 49 };
static const PROGMEM uint8_t octave_delay[] = { 36, 34, 32, 31, 29, 27, 26, 24, 23, 22, 20, 19, 18, 17, 16, 15, 14, 14, 13, 12, 11, 11, 10, 10 };


#define PATTERNS (4)
static const PROGMEM struct { uint8_t a; uint8_t b; } synth[PATTERNS] =
{ { 7, 6}, {9,7}, {8,7}, {3,4} };
static const PROGMEM uint8_t bassdrum[PATTERNS] =	{ 1, 1, 1, 1 };
static const PROGMEM uint8_t snare[PATTERNS] = 	{ 1, 1, 1, 1 };

#define DOUBLE_MASK (0x40)
#define QUAD_MASK (0x80)
#define OCTA_MASK (DOUBLE_MASK|QUAD_MASK)
#define DOUBLE(x) ((x)|DOUBLE_MASK)
#define QUAD(x)   ((x)|QUAD_MASK)
#define OCTA(x)   ((x)|OCTA_MASK)
//#define DOUBLE(x) ((x))
//#define QUAD(x)   ((x))
//#define OCTA(x)   ((x))
static const PROGMEM uint8_t melody[] =
{
E1,
F1,
Fs1,
G1,

Gs1,
A2,
B2,
E1,

OFF,
E1,
OFF,
E1,

OFF,
E1,
OFF,
E1,

OFF,
E1,
E1,
OFF,

OFF,
OFF,
OFF,
OFF,

};

static inline uint8_t next_note()
{
	static uint8_t idx=0;

	const uint8_t v = melody[idx++];
	
	if(idx == SO(melody))
		idx = 0;

	return v;
}

static inline uint8_t get_sin(const uint8_t idx)
{
	return sin[idx%SO(sin)];
}

static inline uint8_t next_rnd()
{
	static unsigned short rnd = 13373;
	const uint8_t f1 = (rnd&(3<<13))>>13;
	const uint8_t f2 = (rnd&(3<<10))>>10;
	rnd <<=1;
	rnd |= f1^f2;

	return get_sin(rnd);
}

static inline uint8_t next_sin(const uint8_t step)
{
	static uint8_t sinoff=SO(sin)-1;
	sinoff += step;
	sinoff %= SO(sin);
	return sin[sinoff];
}


uint8_t next_sample()
{
static uint16_t t=0;

static uint8_t timer =0;
static uint8_t barevent=0;
static uint8_t bars=0;

static uint8_t pc = 0;
static uint8_t arpeggiocnt = 1;

static uint16_t next_sin_time = 0;
static uint8_t current_tone = OFF;
static uint8_t current_tone_base = OFF;

static unsigned short snaredelay = 0;
static unsigned short bassdelay = 0;

static uint8_t synth1 = 0;
static uint8_t synth2 = 0;

static uint8_t sin_speed = 4;

static uint8_t note_rep_cnt = 0;
if(t%128 == 0)
{

	// implicit rollover of t roughly every 2 seconds
	if(t==0) timer = 0;
	else ++timer;

	// determine which note we're playing
	barevent |= 64;
	if(timer%2 == 0) barevent |= 32;
	if(timer%4 == 0) barevent |= 16;
	if(timer%8 == 0) barevent |= 8;
	if(timer%16 == 0) barevent |= 4;
	if(timer%32 == 0) barevent |= 2;
	if(timer%64 == 0) barevent |= 1;
}
else barevent = 0;

if(barevent & 8)
{
	if(note_rep_cnt == (current_tone>>5))
	{
		note_rep_cnt = 0;
		current_tone = current_tone_base = next_note();
	}
	else ++note_rep_cnt;

	if(current_tone&~OCTA_MASK != OFF)
		next_sin_time = t + octave_delay[current_tone&~OCTA_MASK];
	else
		next_sin_time = 0;
}

// increment bar counter
if(barevent & 1) ++bars;

// increment pattern counter
if(bars % 16 == 0)
{
	++pc;
	if(pc == PATTERNS)
		pc = 0;

}

// render synth

static uint8_t synthold1 = 0;
static uint8_t synthold2 = 0;
synthold2 = synthold1;
synthold1 = synth1;
synth1 = 0xfff0 & ( (t&t>>(synth[pc].a)) + (~t&t>>(synth[pc].b)) );

if(t == next_sin_time)
{
	
	if((current_tone&~OCTA_MASK) != OFF)
	{
	  next_sin_time += octave_delay[current_tone&~OCTA_MASK];
		synth2 = ~synth2;//next_sin(sin_speed);
	}
	else synth2 = 0;
}

// mix two synth lines
unsigned int mix = (synth1>>1) + (synth2>>1);

// load or decrement snare delay
if(barevent & 8 && snare[pc])	snaredelay = SNARE_DURATION;
else if(snaredelay > 0)		--snaredelay;

// add snare drum
if(snaredelay>0)	mix ^= (next_rnd() & 7) << 3;


// load or decrement bass delay
if(barevent & 4 && bassdrum[pc])	bassdelay = BASS_DURATION;
else if(bassdelay > 0)			--bassdelay;

// add bass drum
if(bassdelay>0) mix = get_sin(t>>7);

++t;
// here comes the noize!
return mix;
}

#ifndef NOISEPLUG_BIATCH
int main()
{
	while(1) putchar(next_sample());
}
#endif
