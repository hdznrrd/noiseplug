#include <inttypes.h>

#define APREGGIO_RANGE 3
#define APREGGIO_DELAY 200


#define BIT(x,n) (((x)&(1<<(n)))>>(n))
#define SO(x) (sizeof((x))/sizeof(*(x)))

static uint8_t sin[] = {0, 49, 97, 141, 180, 212, 235, 250, 254, 250, 235, 212, 180, 141, 97, 49 };

static inline uint8_t getRand()
{
	static unsigned short rnd = 13373;
	uint8_t f1 = (rnd&(3<<13))>>13;
	uint8_t f2 = (rnd&(3<<10))>>10;
	rnd <<=1;
	rnd |= f1^f2;
	return (uint8_t)(rnd&0x0f);
}

static inline uint8_t next_rnd()
{
	return sin[getRand()%SO(sin)];
}

static inline uint8_t next_sin()
{
	static uint8_t sinoff=SO(sin)-1;
	++sinoff;
	sinoff %= SO(sin);
	return sin[sinoff];
}


uint8_t next_sample()
{
static uint16_t t=0;
static uint8_t t8=0;
static uint8_t barevent=0;
static uint8_t bars=0;

static uint8_t apreggiobase[] =	{ 3, 4, 4, 5 };
static uint8_t bassdrum[] =	{ 1, 0, 1, 1 };
static uint8_t snare[] = 	{ 1, 0, 0, 1 };
static uint8_t pc = 0;
static uint8_t apreggiocnt = 1;



unsigned short snaredelay = 0;
unsigned short bassdelay = 0;

uint8_t synth1 = 0;
uint8_t synth2 = 0;


uint8_t rnd = getRand();

if(t%1000 == 0)
{
	if(t==16000)
	{
		t=0;
		t8 = 0;
	}
	else	++t8;

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
	// pattern counter
	++pc;
	pc %= SO(apreggiobase)-1;
}

// increment apreggio
if(t % APREGGIO_DELAY == 0)
{
	// apreggio
	++apreggiocnt;
	apreggiocnt %= APREGGIO_RANGE;
}

// render synth

if(t<8000)	synth1 = t&t>>5 | t&t>>4;
else		synth1 = t&t>>7 | t&t>>5;
synth1 += synth1;

//synth2 = t<<(apreggiobase[pc]+apreggiocnt);

// mix two synth lines
unsigned int mix = (synth1>>1) | (synth2>>1);

// load or decrement snare delay
if(barevent & 8 && snare[pc])	snaredelay = 400;
else if(snaredelay > 0)		--snaredelay;

// load or decrement bass delay
if(barevent & 4 && bassdrum[pc])	bassdelay = 800;
else if(bassdelay > 0)			--bassdelay;



// add bass drum
//if(bassdelay>0) mix = next_sin(); }

// add snare drum
//if(snaredelay>0)	mix ^= (rnd & 7) << 3;

++t;
// here comes the noize!
return mix;
}

int main()
{
	while(1) putchar(next_sample());
}

