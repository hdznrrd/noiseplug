#include <inttypes.h>

#ifndef PROGMEM
#define PROGMEM /* nix */
#endif

#define APREGGIO_RANGE 4
#define APREGGIO_DELAY 256

#define BIT(x,n) (((x)&(1<<(n)))>>(n))
#define SO(x) (sizeof((x))/sizeof(*(x)))

static const PROGMEM uint8_t sin[] = {0, 49, 97, 141, 180, 212, 235, 250, 254, 250, 235, 212, 180, 141, 97, 49 };
static const PROGMEM uint8_t octave_delay[] = { 36, 34, 32, 31, 29, 27, 26, 24, 23, 22, 20, 19, 18, 17, 16, 15, 14, 14, 13, 12, 11, 11, 10, 10 };
static const PROGMEM struct { uint8_t a; uint8_t b; } synth[] = { { 7, 6}, {7, 5}, {7,5}, {6,5} };
static const PROGMEM uint8_t apreggiobase[] =	{ 3, 4, 4, 5 };
static const PROGMEM uint8_t bassdrum[] =	{ 1, 1, 1, 1 };
static const PROGMEM uint8_t snare[] = 	{ 1, 1, 1, 1 };

static inline uint8_t next_rnd()
{
	static unsigned short rnd = 13373;
	uint8_t f1 = (rnd&(3<<13))>>13;
	uint8_t f2 = (rnd&(3<<10))>>10;
	rnd <<=1;
	rnd |= f1^f2;

	return sin[rnd%SO(sin)];
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

static uint8_t t8=0;
static uint8_t barevent=0;
static uint8_t bars=0;

static uint8_t pc = 0;
static uint8_t apreggiocnt = 1;

static uint16_t next_sin_time = -1;
static uint8_t current_tone = 0;
static uint8_t current_tone_base = 0;

unsigned short snaredelay = 0;
unsigned short bassdelay = 0;

uint8_t synth1 = 0;
uint8_t synth2 = 0;

if(t%1024 == 0)
{
	// implicit rollover of t roughly every 2 seconds
	if(t==0) t8 = 0;
	else ++t8;

	// determine which note we're playing
	barevent |= 8;
	if(t8%2 == 0) barevent |= 4;
	if(t8%4 == 0) barevent |= 2;
	if(t8%8 == 0) barevent |= 1;
}
else barevent = 0;

// increment bar counter
if(barevent & 1) ++bars;

// increment pattern counter
if(bars % 8 == 0)
{
	++pc;
	pc %= SO(apreggiobase)-1;
}

// increment apreggio
if(t % APREGGIO_DELAY == 0)
{
	// apreggio
	++apreggiocnt;
	apreggiocnt %= APREGGIO_RANGE;
	current_tone = current_tone_base + apreggiocnt;
}

// render synth

synth1 = (t&(t>>(synth[pc].a))) | (t&(t>>(synth[pc].b)));
synth1 += synth1;

/*
if(t == next_sin_time)
{
	next_sin_time = 
	synth2 = next_sin();
}
*/
//synth2 = t<<(apreggiobase[pc]+apreggiocnt);

// mix two synth lines
unsigned int mix = (synth1>>1) | (synth2>>1);

// load or decrement snare delay
if(barevent & 8 && snare[pc])	snaredelay = 800;
else if(snaredelay > 0)		--snaredelay;

// load or decrement bass delay
if(barevent & 4 && bassdrum[pc])	bassdelay = 800;
else if(bassdelay > 0)			--bassdelay;



// add bass drum
if(bassdelay>0) mix = next_sin(1);

// add snare drum
if(snaredelay>0)	mix ^= (next_rnd() & 7) << 3;

++t;
// here comes the noize!
return mix;
}

int main()
{
	while(1) putchar(next_sample());
}

