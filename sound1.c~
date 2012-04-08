#define SO(x) (sizeof((x))/sizeof(*(x)))

static inline unsigned char getRand()
{
	static unsigned short rnd = 13373;
	unsigned char f1 = (rnd&(3<<13))>>13;
	unsigned char f2 = (rnd&(3<<10))>>10;
	rnd <<=1;
	rnd |= f1^f2;
	return (unsigned char)(rnd&0x0f);
}

static unsigned char sin[] = {0, 49, 97, 141, 180, 212, 235, 250, 254, 250, 235, 212, 180, 141, 97, 49 };

static inline unsigned char sin_adv()
{
	static unsigned char sinoff=SO(sin)-1;
	++sinoff;
	sinoff %= SO(sin);
	return sin[sinoff];
}

int main()
{
unsigned short t=0;
unsigned char t8=0;
unsigned char barevent=0;
unsigned char bars=0;

#define APREGGIO_RANGE 3
#define APREGGIO_DELAY 200
unsigned char apreggiobase[] =	{ 3, 4, 4, 5 };
unsigned char bassdrum[] =	{ 1, 0, 1, 1 };
unsigned char snare[] = 	{ 1, 0, 0, 1 };
unsigned char pc = 0;
unsigned char apreggiocnt = 1;

#define BIT(x,n) (((x)&(1<<(n)))>>(n))


unsigned short snaredelay = 0;
unsigned short bassdelay = 0;


unsigned char synth1 = 0;
unsigned char synth2 = 0;


for(t=0;;++t)
{
unsigned char rnd = getRand();

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
//if(bassdelay>0) mix = sin_adv(); }

// add snare drum
//if(snaredelay>0)	mix ^= (rnd & 7) << 3;

// here comes the noize!
putchar(mix);
}

}

