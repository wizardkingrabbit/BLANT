#ifndef BLANT_SYNTH_GRAPH_H
#define BLANT_SYNTH_GRAPH_H
#include "blant.h"
#include "multisets.h"
#include "heap.h"

// Below is code to help reduce (mostly eliminite if we're lucky) MCMC's duplicate output, which is copious
// Empirically I've found that neither of these need to be very big since most repetition happens with high locality
#define MCMC_KOVACS_CIRC_BUF 999983 // prime, not sure it needs to be but why not... 1M * 4b = 4MB RAM.
#define MCMC_KOVACS_MAX_HASH 2147483647 // this value should be prime; at 2^31/8 it's 256MB of RAM.

#define LOAD_CONCENTRATION 0
#define LOAD_INDEX 1
#define LOAD_DISTRIBUTION 2

#define GEN_NODE_EXPANSION 0
#define GEN_MCMC 1
extern Boolean _GRAPH_GEN;
extern int _GRAPH_GEN_EDGES;
extern int _KS_NUMSAMPLES;
extern float confidence;
extern int _genGraphMethod;

void reset_global_maps(int k_new);
double* convertPDFtoCDF(double pdf[], double cdf[]);
double* copyConcentration(double src[], double dest[]);
double KStest(double empiricalCDF[], double theoreticalCDF[], int n);
double KStestPVal(double KS_stats, float precision);
void LoadFromFork(int k, int numSamples, GRAPH* G, double onedarray[], double* twodarray[], int mode);
double compareSynGraph(GRAPH *G, GRAPH *G_Syn, int numSamples, int k, double theoreticalPDF[], double theoreticalCDF[]);
int PickGraphletFromConcentration(int binaryNum[], double graphletCDF[], int k);
void stampFunction(GRAPH *G, int binaryNum[], int Varray[], int k);
void StampGraphletNBE(GRAPH *G, GRAPH *G_Syn, double graphletCDF[], int k, int k_small, double theoreticalPDF[], double theoreticalCDF[]);
int GenSynGraph(int k, int k_small, int numSamples, GRAPH *G, FILE *SynOutFile);

#endif