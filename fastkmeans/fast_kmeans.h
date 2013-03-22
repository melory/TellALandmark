#ifndef _FAST_K_MEANS_H
#define _FAST_K_MEANS_H


#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string.h>
#include <memory.h>
#include <memory>
using namespace std;

#define LARGE_VAL 1e10
#define TRUE 1
#define FALSE 0

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

typedef long BOOL;

// input: data array(pData), num of clusters(ncluster), maximum iterations(maxi_iter), initial clusters(pClusters) (optional), verbose control(bVerbose)
// output: clusters(pClusters), point-wise assignment of each data to the clusters (pAssignment) (optional)	
// Implement it using efficient fast k-means!
// if pAssignment is NULL, it will not be calculated
// if pClusters->size() == 0, initial clusters will be assigned using furthest-first method
void kmeans(vector< vector<double> >* pData, vector< vector<double> >* pClusters, vector<int>* pAssignment, int ncluster, int maxi_iter, BOOL bVerbose = FALSE);


//refer to http://www-cse.ucsd.edu/~elkan/fastkmeans.html for the paper and its Matlab code


#endif