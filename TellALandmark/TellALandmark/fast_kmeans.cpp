#include "fast_kmeans.h"

inline double centerdist(int i, int j, vector< vector<double> >& CentDist)
{
	if (i > j)
		return CentDist[i][j];
	else if (i < j)
		return CentDist[j][i];
	else
		return 0;
}



// change this for a different distance!!
double dist(vector<double>& featr1, vector<double>& featr2)
{
	//euclidean distance!
	int featr_dim = featr1.size();
	double result = 0;
	for (int dim = 0; dim < featr_dim; dim++)
	{
		result += (featr1[dim] - featr2[dim]) * (featr1[dim] - featr2[dim]);
	}
	result = sqrt(result);
	return result;
}



// change this for a different mean calculation
// NOTE: if your mean is NOT linear, only use the else block for calculating mean in kmeans()!!!!
// input data points, mask of points to be operated on
// output mean, memory already allocated by func user!
void mean(vector< vector<double> >* pData, vector<double>* pMean, vector<int>* pMask = NULL)
{
	int dataSize = (*pData).size();
	int featr_dim = (*pData)[0].size();
	int i;
	//initialize pMean with all zeros
	memset(&(*pMean)[0], 0, featr_dim*sizeof(double));
	if (pMask == NULL)
	{
		memset(&(*pMean)[0], 0, featr_dim*sizeof(double));
		for (i = 0; i < dataSize; i++)
		{
			for (int j = 0; j < featr_dim; j++)
			{
				(*pMean)[j] += (*pData)[i][j];
			}
		}
		for (int j = 0; j < featr_dim; j++)
		{
			(*pMean)[j] /= dataSize;
		}
	}
	else
	{
		int denominator = 0;
		for (i = 0; i < dataSize; i++)
		{
			if ((*pMask)[i])
			{
				for (int j = 0; j < featr_dim; j++)
				{
					(*pMean)[j] += (*pData)[i][j];
				}
				denominator++;
			}
		}
		for (int j = 0; j < featr_dim; j++)
		{
			(*pMean)[j] /= denominator;
		}
	}
}



void init_cluster(vector< vector<double> >* pClusters, vector< vector<double> >* pData, int ncluster, int dataSize, vector<int>* pMinCenter, vector<double>* pUpp_b, vector< vector<double> >* pLow_b, vector< vector<double> >* pCentDist)
{
	//allocate memory for the clusters
	int featr_dim = (*pData)[0].size();
	pClusters->resize(ncluster);
	for (int i = 0; i < ncluster; i++)
	{
		(*pClusters)[i].resize(featr_dim);
		memset(&((*pClusters)[i][0]), 0, featr_dim*sizeof(double));
	}
	//calculate the first cluster as mean(data)
	mean(pData, &(*pClusters)[0]);

	//now in the loop 1:ncluster
	for (int i = 0; i < ncluster; i++)
	{
		if (i != 0)
		{
			//find the idx of largest upp_b
			double max_dist = 0;
			int max_dist_idx = 0;
			for (int j = 0; j < dataSize; j++)
			{
				if (max_dist < (*pUpp_b)[j])
				{
					max_dist = (*pUpp_b)[j];
					max_dist_idx = j;
				}
			}
			//assign it to the i'th cluster
			copy((*pData)[max_dist_idx].begin(), (*pData)[max_dist_idx].end(), (*pClusters)[i].begin());
		}

		//calculate center distances with previous centers
		//0:i-1
		for (int k = 0; k < i; k++)
		{
			(*pCentDist)[i][k] = dist((*pClusters)[i], (*pClusters)[k]);
		}

		//find those points, whose identity is possible to change
		for (int j = 0; j < dataSize; j++)
		{
			if ( (*pUpp_b)[j] > centerdist(i,(*pMinCenter)[j],*pCentDist)*0.5 )
			{
				//need to calculate the exact dist between x(j) and center(i)
				(*pLow_b)[j][i] = dist((*pData)[j], (*pClusters)[i]);

				if ((*pLow_b)[j][i] < (*pUpp_b)[j])
				{
					//this time, it really needs changing identity
					(*pMinCenter)[j] = i;
					//add upping the upper-bound, also the dist with nearest center for now
					(*pUpp_b)[j] = (*pLow_b)[j][i];
				}
			}
		}

	}//end of the 1:ncluster loop
}






void kmeans(vector< vector<double> >* pData, vector< vector<double> >* pClusters, vector<int>* pAssignment, int ncluster, int maxi_iter, BOOL bVerbose)
{
	int dataSize = pData->size();
	int featr_dim = (*pData)[0].size();
	//check initialized clusters
	if (pClusters->size() != ncluster && pClusters->size() != 0)
	{
		cout<<"Inappropriate cluster input!\n";
		return;
	}

	//create memory for auxiliary variables
	//mincenter for every point
	vector<int>* pMinCenter;
	if (pAssignment==NULL)
	{
		pMinCenter = new vector<int>(dataSize);
	}
	else
	{
		pMinCenter = pAssignment;
	}
	//initialize as 0--the first element
	if (pMinCenter->size() != dataSize)
	{
		cout<<"Bad pAssignment: size not match dataSize!\n";
	}
	for (int i = 0; i < dataSize; i++)
	{
		(*pMinCenter)[i] = 0;
	}

	//upper bound for every point
	vector<double> upp_b(pData->size());
	//initialize the upp_b as all maximum
	for (int i = 0; i < dataSize; i++)
	{
		upp_b[i] = LARGE_VAL;
	}
	//lower bound matrix between point and center
	vector< vector<double> > lower_b(pData->size());
	for (int i = 0; i < pData->size(); i++)
	{
		lower_b[i].resize(ncluster);
		//set to 0s
		memset(&lower_b[i][0], 0, ncluster*sizeof(double));
	}
	//distance between cluster centers
	vector< vector<double> > centdist(ncluster);
	//when accessed, [i][j]: i > j!
	for (int i = 0; i < ncluster; i++)
	{
		//lower triangle
		centdist[i].resize(i);	//save A LOT OF memory!
	}
	//auxiliary array
	//NOTE: this is just the reverse of that in Matlab code
	//in Matlab it is 0 when it is TRUE
	vector<BOOL> recalculated(dataSize);
	//cluster population: used to speed up mean calculation
	vector<int> pop(ncluster);


	//decide if skip the first initialization step
	BOOL bSkipInit = FALSE;

	if (pClusters->size() == 0)
	{
		init_cluster(pClusters, pData, ncluster, dataSize, pMinCenter, &upp_b, &lower_b, &centdist);
		bSkipInit = TRUE;
	}


	int nchanged = dataSize;
	int iter = 0;
	// old min center--initialize with -1!!!!
	vector<int> OldMinCenter(dataSize);
	for (int j = 0; j < dataSize; j++)
	{
		OldMinCenter[j] = -1;
	}
	

	while ((nchanged > 0) && (iter < maxi_iter))
	{
		//verbose
		//telling about the iteration status
		if (bVerbose == TRUE)
		{
			cout<<nchanged<<" "<<iter<<endl;
		}

		if ((iter == 0) && (bSkipInit == FALSE))
		{
			//the first iteration and no skipping
			//go through each point and cluster
			//calculate upper/lower-bounds and nearest centers
			for (int i = 0; i < ncluster; i++)
			{
				for (int j = 0; j < dataSize; j++)
				{
					lower_b[j][i] = dist((*pData)[j], (*pClusters)[i]);
					if (lower_b[j][i] < upp_b[j])
					{
						upp_b[j] = lower_b[j][i];
						(*pMinCenter)[j] = i;
					}
				}//end of for j
			}//end of for i
		}
		else if ((bSkipInit == FALSE))
		{
			//nndist is 0.5*distance to the nearest center, for each center
			vector<double> nndist(ncluster);
			for (int i = 0; i < ncluster; i++)
			{
				double min_dist = LARGE_VAL;
				for (int j = 0; j < ncluster; j++)
				{
					if (i == j) continue;
					double tmp_dist = centerdist(i,j,centdist);
					if (min_dist > tmp_dist)
					{
						min_dist = tmp_dist;
					}
				}
				nndist[i] = min_dist * 0.5;
			}

			//find those possible to change
			vector<int> mobile;
			for (int j = 0; j < dataSize; j++)
			{
				if ( (upp_b)[j] > nndist[(*pMinCenter)[j]] )
				{
					mobile.push_back(j);		//points possible to change identity
				}
			}

			//now, recalculate distance and change identity if possible
			for (int i = 0; i < ncluster; i++)
			{
				for (int j = 0; j < mobile.size(); j++)
				{
					int data_idx = mobile[j];
					if ((*pMinCenter)[data_idx] == i)
						continue;

					//the same as d(c(x), c) in the paper
					double dist_cx_c = centerdist(i,(*pMinCenter)[data_idx],centdist);
					bool test = upp_b[data_idx] > 0.5*dist_cx_c &&
						upp_b[data_idx] > lower_b[data_idx][i];
					if (test)
					{
						//calculate the exact distances
						if (recalculated[data_idx])
						{
							recalculated[data_idx] = FALSE;
							//calculate the dist between the point and its curr center
							//and update upper-bound at the same time
							upp_b[data_idx] = dist((*pData)[data_idx], (*pClusters)[(*pMinCenter)[data_idx]]);
						}
						//test if need to compute d(x,c): x is the data_idx point, c is i'th center
						bool test_in = upp_b[data_idx] > 0.5*dist_cx_c ||
						upp_b[data_idx] > lower_b[data_idx][i];
						if (test_in)
						{
							//compute d(x,c) and update lower-bound at the same time
							lower_b[data_idx][i] = dist((*pData)[data_idx], (*pClusters)[i]);

							if (lower_b[data_idx][i] < upp_b[data_idx])
							{
								//change identity now
								(*pMinCenter)[data_idx] = i;
								//and update upper-bound
								upp_b[data_idx] = lower_b[data_idx][i];
							}
						}

					}
				}
			}
		}//end of if..elseif..


		//now, M-step, recalculating the centers
		vector< vector<double> > oldClusters(ncluster);
		for (int i = 0; i < ncluster; i++)
		{
			oldClusters[i].resize(featr_dim);
			copy( (*pClusters)[i].begin(), (*pClusters)[i].end(), oldClusters[i].begin() );
		}
		
		//find those changed cluster ids
		vector<int> cluster_change_status(ncluster);
		vector<int> changed_pts;
		memset(&cluster_change_status[0], 0, ncluster*sizeof(int));
		for (int j = 0; j < dataSize; j++)
		{
			if ((*pMinCenter)[j] != OldMinCenter[j])
			{
				cluster_change_status[(*pMinCenter)[j]] = 1;
				if (OldMinCenter[j] >= 0)
				{	//dangerous if not check! because initialized with -1!!!
					cluster_change_status[OldMinCenter[j]] = 1;
				}
				changed_pts.push_back(j);
			}
		}
		vector<int> changed_clusters;
		for (int i = 0; i < ncluster; i++)
		{
			if (cluster_change_status[i] != 0)
			{
				changed_clusters.push_back(i);
			}
		}
		cluster_change_status.clear();

		//linear mean
		//NOTE: if your mean is NOT linear, only use the else block!!!!
		if (changed_clusters.size() < ncluster/3 && iter > 0)
		{
			for (int i = 0; i < changed_clusters.size(); i++)
			{
				int cur_clst = changed_clusters[i];
				int plus = 0;
				int minus = 0;
				//flow = plus - minus
				vector<double> flow(featr_dim);
				memset(&flow[0], 0, featr_dim*sizeof(double));

				for (int j = 0; j < changed_pts.size(); j++)
				{
					int cur_pt = changed_pts[j];
					if ((*pMinCenter)[cur_pt] == cur_clst)
					{
						plus++;		//entering the current clst
						for (int dim = 0; dim < featr_dim; dim++)
						{
							flow[dim] += (*pData)[cur_pt][dim];
						}
					}
					if (OldMinCenter[cur_pt] == cur_clst)
					{
						minus++;		//leaving the current clst
						for (int dim = 0; dim < featr_dim; dim++)
						{
							flow[dim] -= (*pData)[cur_pt][dim];
						}
					}
				}

				int oldpop = pop[cur_clst];
				pop[cur_clst] = oldpop + plus - minus;
				
				if (pop[cur_clst] == 0)
					continue;

				//now, the new center
				for (int dim = 0; dim < featr_dim; dim++)
				{
					(*pClusters)[cur_clst][dim] = ( (*pClusters)[cur_clst][dim]*oldpop + flow[dim] ) / (double)pop[cur_clst];
				}
			}
		}
		else
		{
			for (int i = 0; i < changed_clusters.size(); i++)
			{
				int cur_clst = changed_clusters[i];
				vector<int> track(dataSize);
				pop[cur_clst] = 0;
				for (int j = 0; j < dataSize; j++)
				{
					if ((*pMinCenter)[j] == cur_clst)
					{
						track[j] = 1;
						pop[cur_clst]++;
					}
					else
					{
						track[j] = 0;
					}
				}
				if (pop[cur_clst] == 0)
					continue;

				mean(pData, &(*pClusters)[cur_clst], &track);
			}

		}//end of if linear mean

		//update lower-bounds and upper-bounds
		for (int i = 0; i < changed_clusters.size(); i++)
		{
			int cur_clst = changed_clusters[i];
			double offset = dist((*pClusters)[cur_clst], oldClusters[cur_clst]);
			if (offset < 1e-6)
				continue;
			for (int j = 0; j < dataSize; j++)
			{
				if ((*pMinCenter)[j] == cur_clst)
				{
					upp_b[j] = upp_b[j] + offset;
				}
				// for all pts! no track!
				lower_b[j][cur_clst] = max( (lower_b[j][cur_clst] - offset), 0 );
			}
		}

		//set recalculated as TRUE
		for (int j = 0; j < dataSize; j++)
		{
			recalculated[j] = TRUE;
		}
		//calculate distances between centers
		//reverse loop because centdist is a lower-triangle matrix
		for (int i = ncluster-1; i >= 1; i--)
		{
			for (int k = i-1; k >=0; k--)
			{
				centdist[i][k] = dist((*pClusters)[i], (*pClusters)[k]);
			}
		}


		//loop variables
		nchanged = changed_pts.size() + bSkipInit;
		iter++;
		bSkipInit = FALSE;
		for (int j = 0; j < dataSize; j++)
		{
			OldMinCenter[j] = (*pMinCenter)[j];
		}


	}//end of while loop

	if (pAssignment==NULL)
	{
		delete pMinCenter;
	}
	

}


