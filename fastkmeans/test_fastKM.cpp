#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
using namespace std;

#include "fast_kmeans.h"


char* in_data_file = "data";
char* out_cluster_file = "center";
char* out_assign_file = "assignment";


int main()
{
	int dataSize = 1000;
	int feat_dim = 128;
	int ncluster = 10;
	int i;

	//data and clusters
	vector< vector<double> > data(dataSize);
	vector< vector<double> > clusters;

	//reading data
	ifstream infile;
	infile.open(in_data_file);
	for (int j = 0; j < dataSize; j++)
	{
		data[j].resize(feat_dim);
		for (int dim = 0; dim < feat_dim; dim++)
		{
			infile>>data[j][dim];
		}
	}
	infile.close();

	vector<int> Assignment(dataSize);

	//kmeans
	kmeans(&data, &clusters, &Assignment, ncluster, 500, TRUE);
	
	
	//output cluster file and assignment file
	ofstream outfile;
	outfile.open(out_cluster_file);
	for (i = 0; i < ncluster; i++)
	{
		for (int dim = 0; dim < feat_dim; dim++)
		{
			outfile<<clusters[i][dim]<<" ";
		}
		outfile<<endl;
	}
	outfile.close();
	outfile.clear();


	outfile.open(out_assign_file);
	for (j = 0; j < dataSize; j++)
	{
		outfile<<Assignment[j]+1<<" ";
	}
	outfile.close();
	outfile.clear();

	getchar();
	return 0;
}