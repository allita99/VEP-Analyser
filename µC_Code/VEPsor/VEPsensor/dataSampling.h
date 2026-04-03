#ifndef  DATASAMPLING_H
#define DATASAMPLING_H

#include <stdint.h>
#include <Arduino.h>
#include <arduinoFFT.h>
#include <string.h>

// DATA RESAMPLING CODE

void setBit(uint8_t* array, int index, bool value); // compressing data into an 8bit array
void setBit32(uint32_t* array, int index, bool value); // compressing data into an 32 bit array

bool getBit(uint8_t* array, int index);  // getting data from a compressed 8 bit array
bool getBit32(uint32_t* array, int index);  // getting data from a compressed 32 bit array

void printSavedData(uint8_t* sensorData,int byteSize, Stream& serial);
void printSavedData32(uint32_t* sensorData,int byteSize, Stream& serial);

String getPolynomial(uint8_t tapPoints);

int shortestVSlongestChanges(uint8_t* reData, int arraySizeBits);
int shortestVSlongestChanges32(uint8_t* reData, int arraySizeBits);

bool frequencyAnalysis(uint8_t* inputArray, uint8_t& peakFreqArray, uint8_t fs);

void downSample(uint8_t* array, uint8_t* reData, int byteSize, int targetFrequency); // downsampling algorithm that works with binary values
void downSample32(uint32_t* array, uint32_t* reData, int byteSize, int targetFrequency); // downsampling algorithm that works with binary values

//void findSequence(uint8_t* reData, int byteSize);

bool compareSequences(uint8_t* inputArray, int inputBitSize, int sequenceCount);
uint8_t compareSequences32(uint32_t* inputArray, int inputBitSize, int sequenceCount);

struct MSequence { // organizing sequence arrays with their sizes and tap points as efficient as possible
    //uint8_t* sequence;
    uint32_t* sequence;
    uint8_t bitLength;
    uint8_t tapPoints;
};

struct GSequence { // organizing sequence arrays with their sizes and tap points as efficient as possible
    //uint8_t* sequence;
    uint32_t* sequence;
    uint8_t bitLength;
    uint8_t identifier;
};

//31 bit m sequences
extern uint32_t sequence5n1[1];extern uint32_t sequence5n2[1];extern uint32_t sequence5n3[1];
extern uint32_t sequence5n4[1];extern uint32_t sequence5n5[1];extern uint32_t sequence5n6[1];
//63 bit m sequences
extern uint32_t sequence6n1[2];extern uint32_t sequence6n2[2];extern uint32_t sequence6n3[2];
extern uint32_t sequence6n4[2];extern uint32_t sequence6n5[2];extern uint32_t sequence6n6[2];
//127bit m sequences
extern uint32_t sequence7n1[4];extern uint32_t sequence7n2[4];extern uint32_t sequence7n3[4];
extern uint32_t sequence7n4[4];extern uint32_t sequence7n5[4];extern uint32_t sequence7n6[4];
extern uint32_t sequence7n7[4];extern uint32_t sequence7n8[4];extern uint32_t sequence7n9[4];
extern uint32_t sequence7n10[4];extern uint32_t sequence7n11[4];extern uint32_t sequence7n12[4];
extern uint32_t sequence7n13[4];extern uint32_t sequence7n14[4];extern uint32_t sequence7n15[4];
extern uint32_t sequence7n16[4];extern uint32_t sequence7n17[4];extern uint32_t sequence7n18[4];

// //31 bit golden sequences
// extern uint32_t  gSequence5n1[1];// ignore position 31(259605760)
// extern uint32_t  gSequence5n2[1];// ignore position 31(487282432)
// extern uint32_t  gSequence5n3[1];// ignore position 31(1591455744)
// extern uint32_t  gSequence5n4[1];// ignore position 31(2121198080)
// extern uint32_t  gSequence5n5[1];// ignore position 31(1792157184)
// extern uint32_t  gSequence5n6[1];// ignore position 31(309657344)
// extern uint32_t  gSequence5n7[1];// ignore position 31(1342087424)
// extern uint32_t  gSequence5n8[1];// ignore position 31(1994531712)
// extern uint32_t  gSequence5n9[1];// ignore position 31(1636667392)
// extern uint32_t gSequence5n10[1];// ignore position 31(584331264)
// extern uint32_t gSequence5n11[1];// ignore position 31(1160580096)
// extern uint32_t gSequence5n12[1];// ignore position 31(1372207104)
// extern uint32_t gSequence5n13[1];// ignore position 31(494980928)
// extern uint32_t gSequence5n14[1];// ignore position 31(1186927488)
// extern uint32_t gSequence5n15[1];// ignore position 31(386863104)

// //63 bit golden Sequences
// extern uint32_t gSequence6n1[2]; // ignore position 63
// extern uint32_t gSequence6n2[2];  // ignore position 63
// extern uint32_t gSequence6n3[2];// ignore position 63
// extern uint32_t gSequence6n4[2]; // ignore position 63
// extern uint32_t gSequence6n5[2]; // ignore position 63
// extern uint32_t gSequence6n6[2]; // ignore position 63
// extern uint32_t gSequence6n7[2]; // ignore position 63
// extern uint32_t gSequence6n8[2]; // ignore position 63
// extern uint32_t gSequence6n9[2]; // ignore position 63
// extern uint32_t gSequence6n10[2]; // ignore position 63
// extern uint32_t gSequence6n11[2]; // ignore position 63
// extern uint32_t gSequence6n12[2]; // ignore position 63
// extern uint32_t gSequence6n13[2];// ignore position 63
// extern uint32_t gSequence6n14[2]; // ignore position 63
// extern uint32_t gSequence6n15[2]; // ignore position 63
// //127 bit golden sequences
// extern uint32_t gSequence7n1[4];extern uint32_t gSequence7n2[4];extern uint32_t gSequence7n3[4];extern uint32_t gSequence7n4[4];
// extern uint32_t gSequence7n5[4];extern uint32_t gSequence7n6[4];extern uint32_t gSequence7n7[4];extern uint32_t gSequence7n8[4];
// extern uint32_t gSequence7n9[4];extern uint32_t gSequence7n10[4];extern uint32_t gSequence7n11[4];extern uint32_t gSequence7n12[4];
// extern uint32_t gSequence7n13[4];extern uint32_t gSequence7n14[4];extern uint32_t gSequence7n15[4];extern uint32_t gSequence7n16[4];
// extern uint32_t gSequence7n17[4];extern uint32_t gSequence7n18[4];extern uint32_t gSequence7n19[4];extern uint32_t gSequence7n20[4];
// extern uint32_t gSequence7n21[4];extern uint32_t gSequence7n22[4];extern uint32_t gSequence7n23[4];extern uint32_t gSequence7n24[4];
// extern uint32_t gSequence7n25[4];extern uint32_t gSequence7n26[4];extern uint32_t gSequence7n27[4];extern uint32_t gSequence7n28[4];
// extern uint32_t gSequence7n29[4];extern uint32_t gSequence7n30[4];extern uint32_t gSequence7n31[4];extern uint32_t gSequence7n32[4];
// extern uint32_t gSequence7n33[4];extern uint32_t gSequence7n34[4];extern uint32_t gSequence7n35[4];extern uint32_t gSequence7n36[4];
// extern uint32_t gSequence7n37[4];extern uint32_t gSequence7n38[4];extern uint32_t gSequence7n39[4];extern uint32_t gSequence7n40[4];
// extern uint32_t gSequence7n41[4];extern uint32_t gSequence7n42[4];extern uint32_t gSequence7n43[4];extern uint32_t gSequence7n44[4];
// extern uint32_t gSequence7n45[4];extern uint32_t gSequence7n46[4];extern uint32_t gSequence7n47[4];extern uint32_t gSequence7n48[4];
// extern uint32_t gSequence7n49[4];extern uint32_t gSequence7n50[4];extern uint32_t gSequence7n51[4];extern uint32_t gSequence7n52[4];
// extern uint32_t gSequence7n53[4];extern uint32_t gSequence7n54[4];extern uint32_t gSequence7n55[4];extern uint32_t gSequence7n56[4];
// extern uint32_t gSequence7n57[4];extern uint32_t gSequence7n58[4];extern uint32_t gSequence7n59[4];extern uint32_t gSequence7n60[4];
// extern uint32_t gSequence7n61[4];extern uint32_t gSequence7n62[4];extern uint32_t gSequence7n63[4];extern uint32_t gSequence7n64[4];
// extern uint32_t gSequence7n65[4];extern uint32_t gSequence7n66[4];extern uint32_t gSequence7n67[4];extern uint32_t gSequence7n68[4];
// extern uint32_t gSequence7n69[4];extern uint32_t gSequence7n70[4];extern uint32_t gSequence7n71[4];extern uint32_t gSequence7n72[4];
// extern uint32_t gSequence7n73[4];extern uint32_t gSequence7n74[4];extern uint32_t gSequence7n75[4];extern uint32_t gSequence7n76[4];
// extern uint32_t gSequence7n77[4];extern uint32_t gSequence7n78[4];extern uint32_t gSequence7n79[4];extern uint32_t gSequence7n80[4];
// extern uint32_t gSequence7n81[4];extern uint32_t gSequence7n82[4];extern uint32_t gSequence7n83[4];extern uint32_t gSequence7n84[4];
// extern uint32_t gSequence7n85[4];extern uint32_t gSequence7n86[4];extern uint32_t gSequence7n87[4];extern uint32_t gSequence7n88[4];
// extern uint32_t gSequence7n89[4];extern uint32_t gSequence7n90[4];extern uint32_t gSequence7n91[4];extern uint32_t gSequence7n92[4];
// extern uint32_t gSequence7n93[4];extern uint32_t gSequence7n94[4];extern uint32_t gSequence7n95[4];extern uint32_t gSequence7n96[4];
// extern uint32_t gSequence7n97[4];extern uint32_t gSequence7n98[4];extern uint32_t gSequence7n99[4];extern uint32_t gSequence7n100[4];
// extern uint32_t gSequence7n101[4];extern uint32_t gSequence7n102[4];extern uint32_t gSequence7n103[4];extern uint32_t gSequence7n104[4];
// extern uint32_t gSequence7n105[4];extern uint32_t gSequence7n106[4];extern uint32_t gSequence7n107[4];extern uint32_t gSequence7n108[4];
// extern uint32_t gSequence7n109[4];extern uint32_t gSequence7n110[4];extern uint32_t gSequence7n111[4];extern uint32_t gSequence7n112[4];
// extern uint32_t gSequence7n113[4];extern uint32_t gSequence7n114[4];extern uint32_t gSequence7n115[4];extern uint32_t gSequence7n116[4];
// extern uint32_t gSequence7n117[4];extern uint32_t gSequence7n118[4];extern uint32_t gSequence7n119[4];extern uint32_t gSequence7n120[4];
// extern uint32_t gSequence7n121[4];extern uint32_t gSequence7n122[4];extern uint32_t gSequence7n123[4];extern uint32_t gSequence7n124[4];
// extern uint32_t gSequence7n125[4];extern uint32_t gSequence7n126[4];extern uint32_t gSequence7n127[4];extern uint32_t gSequence7n128[4];
// extern uint32_t gSequence7n129[4];extern uint32_t gSequence7n130[4];extern uint32_t gSequence7n131[4];extern uint32_t gSequence7n132[4];
// extern uint32_t gSequence7n133[4];extern uint32_t gSequence7n134[4];extern uint32_t gSequence7n135[4];extern uint32_t gSequence7n136[4];
// extern uint32_t gSequence7n137[4];extern uint32_t gSequence7n138[4];extern uint32_t gSequence7n139[4];extern uint32_t gSequence7n140[4];
// extern uint32_t gSequence7n141[4];extern uint32_t gSequence7n142[4];extern uint32_t gSequence7n143[4];extern uint32_t gSequence7n144[4];
// extern uint32_t gSequence7n145[4];extern uint32_t gSequence7n146[4];extern uint32_t gSequence7n147[4];extern uint32_t gSequence7n148[4];
// extern uint32_t gSequence7n149[4];extern uint32_t gSequence7n150[4];extern uint32_t gSequence7n151[4];extern uint32_t gSequence7n152[4];
// extern uint32_t gSequence7n153[4];


extern const MSequence sequences[];
//extern const GSequence gsequences[];

// extern uint8_t sequence5n1[4];
// extern uint8_t sequence5n2[4];
// extern uint8_t sequence5n3[4];
// extern uint8_t sequence5n4[4];
// extern uint8_t sequence5n5[4];
// extern uint8_t sequence5n6[4];

// extern uint8_t sequence6n1[8];
// extern uint8_t sequence6n2[8];
// extern uint8_t sequence6n3[8];
// extern uint8_t sequence6n4[8];
// extern uint8_t sequence6n5[8];
// extern uint8_t sequence6n6[8];

// extern uint8_t sequence7n1[16];
// extern uint8_t sequence7n2[16];
// extern uint8_t sequence7n3[16];
// extern uint8_t sequence7n4[16];
// extern uint8_t sequence7n5[16];
// extern uint8_t sequence7n6[16];
// extern uint8_t sequence7n7[16];
// extern uint8_t sequence7n8[16];
// extern uint8_t sequence7n9[16];
// extern uint8_t sequence7n10[16];
// extern uint8_t sequence7n11[16];
// extern uint8_t sequence7n12[16];
// extern uint8_t sequence7n13[16];
// extern uint8_t sequence7n14[16];
// extern uint8_t sequence7n15[16];
// extern uint8_t sequence7n16[16];
// extern uint8_t sequence7n17[16];
// extern uint8_t sequence7n18[16]; 



    
#endif