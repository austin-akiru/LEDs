# LED strips

Having previosly filled my ghouse with warm white LED strips, I've now started to pla with indexable RGB strips, specifically the WS2812. Adafruit sell this as NeoPixels, but unbranded ones are also available at less cost.

Programming the strips requires a library, and there is a choice. Adafruit have alibrary for their NeoPiels but this works just a well with any LED strip. There is also an indepenant FastLED library. For a first step I need to compare these two to see which I prefer.

## Adafruit

I followed Adafruits own tutorial from: https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use.

It's fairly simple to download the corectly library using the Ardunio SDK using the tutorial to make a single LED light up:

```c
#include <Adafruit_NeoPixel.h>  // Adafruit library
#define PIN 3 // Data pin from Arduino
#define LEDS 30 // number of LEDS on a strip
// define strip in terms of LEDs, data pin and RGB format
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, PIN, NEO_GRB + NEO_KHZ800);  

void setup() {
  strip.begin(); // enable strip
   strip.setPixelColor(1, 255, 0, 0); // index, red, green, blue
   strip.show(); // the LED array has been changed, this changes it on the actual strip
}

void loop() {
}
```

## FastLED

This library has a compelte tutorial at https://github.com/FastLED/FastLED/wiki. The same example code looks a bit differnet with FastLED:

```c
#include <FastLED.h> // fastLED library
#define PIN 3 // Data pin from Arduino
#define LEDS 30 // number of LEDS on a strip
CRGBArray<LEDS> leds; // create LED array

void setup() {
  FastLED.addLeds<NEOPIXEL, PIN>(leds, LEDS);  // initialise strip
  leds[0].setRGB( 255, 0, 0); // set LED
  FastLED.show(); // update strip
}

void loop() {
}
```

## Comparisons

The Adafruit library is reportedly smaller than the alternative, but that just means it lacks some functionality. FastLED does have some useful functions, can use HSV rather than RBG, and has it's own delay/show combined function. The main advantage is its own trig maths functions that will be useful in coding animations. This, combined with the familiaristy of C arrays to mange stips means the rest of this project will use FastLED.

## Wiring

[This guide](https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/) was great for getting me started. Wiring is simple as there is only 3 wires; 5V, ground and data. Although for basic testing I can tap the power of the arduino, when a longer strip is in use I shuld power the two independently. 

# Experiments

## Kitt

As a child of the 80s this was always going to be a first experiment:

```c
void loop() {
  int width=5;
  int rate=50;
  for(int i=0; i<LEDS-width; i++) {
    leds[i].setRGB(0,0,0);
    for(int j=1; j<width; j++) {
      leds[i+j].setRGB(255,0,0);
    }
    leds[i+width].setRGB(0,0,0);
    FastLED.show();
    delay(rate);
  }
  delay(rate*2);
  for(int i=LEDS; i>=width; i--) {
    leds[i].setRGB(0,0,0);
    for(int j=1; j<width; j++) {
      leds[i-j].setRGB(255,0,0);
    }
    leds[i-width].setRGB(0,0,0);
    FastLED.show();
    delay(rate);
  }
  delay(rate*2);
}
```

## Twinkle

Basic but pleasant:

```c
loop() {
  twinkle();
}

void twinkle() {
  leds[random(LEDS)].setRGB(random(150,255),random(150,255),random(150,255));
  FastLED.show();
}
```

## Rainbows

Not my code, but this demonstrates how to get deeper colours:

```c
void loop() {
  rainbowCycle(10);
}

void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< LEDS; i++) {
      c=Wheel(((i * 256 / LEDS) + j) & 255);
      leds[i].setRGB(*c, *(c+1), *(c+2));
    }
    FastLED.show();
    delay(SpeedDelay);
  }
}

byte * Wheel(byte WheelPos) {
  static byte c[3];  
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }
  return c;
}
```

This code is great for using RGB values, but is made a lit simpler use HSV:

```c
void loop() {
  myrainbow();
}

void myrainbow() {
  for (int c=0; c<255; c++) {
    for (int i=0; i<LEDS; i++) {
      leds[i].setHSV((i*(255/30)+c)%255,255,255);
    }
    FastLED.show();
    delay(10);
  }
}
```

HSV is very different to RGB. The FastLED wiki explains it well, but in my shorthand Hue is the colour, Saturation is how mch colour there is, and vibrance is how bright. So, a vibrance of 0 will be black, and a saturation of 0 will be white. This way of managing LEDs will make some animations easier.

# Classes

If I want to do anything more challenign it makes sense to start using classes. This took a bit of work and is best covered in sections:

### Header

```c
class Sunrise
{
  public:	// functions that can be called from main loop
    Sunrise(int steps); // constuctor
    uint32_t next(); // calculate and display next step
    void start();  // reset counters
    void tshow();  // serial debugging
  private: // internal functionas
    void _fsteps16(uint8_t *, uint32_t); // calculate table of light steps
    void _fstarttime16(uint16_t * , int, uint32_t); // calcualte table fo start times
    uint8_t * _tsteps; // array to hold light level for each step
    uint16_t _tstart[NUM_LEDS];  // array to hoold start times
    uint32_t _cstep;  // counters
    uint32_t _steps;
    uint8_t _hue;  // random hue
};
```

### Constructor

```c
Sunrise::Sunrise(int steps)
{
  _steps = steps; // allows user to define the nubmer of steps at construction time
}
```

### Initialisation

```c
void Sunrise::start() {
  _tsteps = (uint8_t *) malloc(_steps*sizeof(uint8_t)); // allocate a block of memroy to store steps
  _fsteps16(_tsteps, _steps); // run internal functon to calaute steps
  _fstarttime16(_tstart, NUM_LEDS, _steps); // intenral function to calculate start times
  for (uint8_t i = 0; i < NUM_LEDS; i++) { // set each LED to 0
    leds[i].setHSV(0, 0, 0); // HSV = Hue, Saturation, Vibrance.
  }
  _cstep=0;
  FastLED.show();
  randomSeed(analogRead(0)); // set random hue
  _hue=random(255);
}
```

Theres some new concepts here. the `malloc` is required because we only know the size of this array at runtime. It's been a while since I've used a `malloc` in anger but this seems to be the correct syntax.

randomseed is required to randomise the randomiser. the pseudoramdom genertors resets to the same state, so if I don't want this to follow a set pattern i need to randomise it by trying ot read an analogue value from a floating pin.

### Calculations

```c
void Sunrise::_fsteps16(uint8_t *a, uint32_t steps) {
  for (uint32_t i = 0; i < steps; i++) {
    uint8_t num=((128*i/(steps-1)));
    a[i]=255-cos8(num);
  }
}

void Sunrise::_fstarttime16(uint16_t * a, int leds, uint32_t steps) {
  for (int i = 0; i < leds; i++) {
    uint32_t num = (255 * i / (leds - 1)) & 255;
    a[i] = uint16_t(uint32_t(cos8(num)) * steps / 255);
  }
}
```

Lots of trig maths here. the cos8 function generates values in the 0 - 255 range. Details of how to use these functions are in the [documentation](http://fastled.io/docs/3.1/group___trig.html).

### Stepping

```c
uint32_t Sunrise::next() {
  if(_cstep<_steps*2) {

    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      if (_cstep >= _tstart[i]) {
        uint8_t h = _tsteps[min(_cstep - _tstart[i], _steps - 1)];
        leds[i].setHSV(_hue, min((255 + 50 - h), 255), h);
      }
    }
    FastLED.show();
    return ++_cstep;
  } else {
    return 0;
  }
}
```

For each LED, lookup it's HSV value as a functiobn of it's index, start time and number of steps already completed.

### Running

```c
  Sunrise sun(1000); // initialise class
  sun.start(); // calaulte lookup tables
  while(sun.next()>0) {} // keep calling next until compelted
```

## Conclusions

It's been a while since I've written C code in anger and it shows. I'm not convinced I've made the best use of classes, but it works.

Interestingly it doesn't work if the steps array is too big. This is because i've very limited on working memory and it's posisble to make the `malloc`ed array too big. Seeing as the trig functions are meant to be super efficient, it's probably best to do some calculations in real time.

# Dynamic Classes

One for another day.