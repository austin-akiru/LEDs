#include<FastLED.h>

CRGB la[512];
int z16,h8,lc=0;
unsigned char Mask[]= {0xFC, 0xC6, 0xC6, 0xFC, 0xC6, 0xC6, 0xFC, 0x00,
                       0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00,
                       0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00,
                       0xFC, 0xC6, 0xC6, 0xFC, 0xC6, 0xC6, 0xFC, 0x00,
                       0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00,
                       0xFE, 0xC0, 0xC0, 0xF8, 0xC0, 0xC0, 0xFE, 0x00,
                       0x7C, 0xC6, 0xC0, 0x7C, 0x06, 0xC6, 0x7C, 0x00};
void setup() {
  delay(1000);
  FastLED.addLeds<WS2812B,2,GRB>(la,512);
}

void loop() {
  sl2(Mask);
}

void sl2(unsigned char *P) {
  uint8_t z=z16>>4;
  uint8_t zf=z16++&0x0F;
  uint8_t fb=128-(zf*8+8);
  uint8_t sb=128-fb;
  uint8_t h=h8>>2;
  uint8_t x,y=0;
  ++h8%2^11;
  if(z16>=128) {
    z16=0;
    ++lc%=7;
  }
  for(y=0; y<8; y++) {
    for(x=0; x<8; x++) {
      if((*(P+8*lc+y)>>x)&0x01) {
        la[((7-z)*64)+((7-x)*8)+(7-y)].setHSV( h, 255, fb); 
        z<7 && la[((7-z-1)*64)+((7-x)*8)+(7-y)].setHSV( h, 255, 128); // extra girth
        z<6 && la[((7-z-2)*64)+((7-x)*8)+(7-y)].setHSV( h, 255, sb); 
      }
    }
  }   
  FastLED.show();
} 
