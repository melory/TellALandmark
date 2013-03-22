/**
 *
 * Email: imsrch@melory.me
 * Date: 2012/1/11
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>

#include "fast_kmeans.h"

using namespace std;

int main()
{
	FILE *ALL_KPS, *KPS_INFO, *KEY;
	char *fname1 = "ALL_KPS";  //存放所有的keypoints
	char *fname2 = "KPS_INFO"; //存放keypoints的相关信息
	int N = 50; //每类照片的最大数目
	char image_name[5][20] = {"caozhu", "maoxiang", "shengyilou", "tushuguan", "zhengmen"};
	int image_number[5];
	int image_feature[5][100]; //image_featurep[0][34]的意思是
								//所有caozhu的照片提取出来的keypoints，
								//有234个是属于第34个聚类中心
	char fname_key[20];
	int number, i, j, k, start, end = 0, _start[5], _end[5];
	double discard[4];


	if(NULL == (ALL_KPS = fopen(fname1, "w")))
	{
		printf("openning file 'ALL_KPS' failed!");
		return 1;
	}
	if(NULL == (KPS_INFO = fopen(fname2, "w")))
	{
		printf("openning file 'KPS_INFO' failed!");
		return 1;
	}

 /* 下面一段程序读入所有图片对应的keypoints文件(*.key)， 将所有这些文件的内容写到一个
 * ALL_KPS文件中，每个keypoint为一行。
 * 
 * 之前提取的keypoints都是128维的证整数，但是也包含了每个keypoint的方向等信息，
 * 所以*.key里面的每一个keypoint（每一行）有128 + 4 = 132个数值，前4个位方向等
 * 信息， 后面的128个整数才是各个keypoint。提取出来的只有每个后面的128个整数。
 *
 * 将所有的keypoints都集中地提取出来，还需要记录每个keypoint是属于哪一张图片，
 * 将这些信息写在一个KP_INFO的文件中，描述的格式为：
 *	  	照片的类别 在ALL_KPS中起始行数 在ALL_KPS中终止行数
 * 就是说将每一类的照片的keypoints集中地放在一起，一个ALL_KPS文件的内容可能是
 * 如下这样的：
 *		caozhu 1 7812
 *		tushuguang 7813 12331
 *		zhengmen 12332 18905
 *		maoxiang 18906 24966
 **/

	printf("提取所有的keypoints!\n");
	for(i = 0; i< 5; i++)  //依次地读取每类图像的key文件
	{
		image_number[i] = 0;
		start = end + 1;
		for(j = 0; j <= N; j++) //每类图像的数目
		{
			sprintf(fname_key, "%s (%d).key", image_name[i], j); //读取每图像对应的key文件
			printf("%s\n", fname_key);
			if(NULL != (KEY = fopen(fname_key, "r")))
			{
				image_number[i]++;
				while(EOF != fscanf(KEY, "%lf%lf%lf%lf", &discard[0], &discard[1], &discard[2], &discard[3]))
				{
					for(k = 1; k <= 128; k++)
					{
						if(EOF != fscanf(KEY, "%d", &number))
							fprintf(ALL_KPS, "%d ", number);
					}
					fprintf(ALL_KPS, "\n");
					end++;
				}
				fclose(KEY);
			}
			else
				printf("openning file '%s' failed!\n", fname_key);
		}
		if(start > end){
			printf("satrt > end on image: %s", image_name[i]);
			return 1;
		}
		fprintf(KPS_INFO, "%s %d %d\n", image_name[i], start, end);
		_start[i] = start;
		_end[i] = end;
	}	
	fclose(ALL_KPS);
	fclose(KPS_INFO);

	printf("准备进行聚类!\n");
	//进行聚类操作
	//int dataSize = 1000;
	int dataSize;  //聚类点的个数
	int feat_dim = 128;  //聚类点的维数
	int ncluster = 100;  //k-means中的k
	//char *in_data_file = "data";
	char *in_data_file = fname1; //输入文件为所有的keypoints
	char *out_cluster_file = "CENTER";  //聚类之后输出的中心
	char *out_assign_file = "ASSIGNMENT";  //各个点属于哪一类的信息，比如
											//说ASSIGNMENT文件第i个数字为j
											//表示第i个点聚类之后应该属于第
											//j类
	dataSize = end;
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

	printf("开始进行聚类!\n");
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



	printf("获取每一类照片的feature特性!\n");
	//获取每一类照片的feature特性
	//统计每一类照片具有怎样的feature，在所有image的keypoint进行k-means聚类之后，得到
	//k个center，统计每一类照片有多少keypoints落在每一个center上
	infile.open(out_assign_file);  //读入'ASSIGNMENT'文件
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 100; j++)
			image_feature[i][j] = 0;

		for (int j = _start[i]; j <= _end[i]; j++)
		{
			infile>>number;
			image_feature[i][number]++;
		}
		for (int j = 0; j < 100; j++)  
			if(image_number[i] > 0)
				image_feature[i][j] = image_feature[i][j] / image_number[i];
			else
			{
				printf("%s的图片张数为0！\n", image_name[i]);
				//return 1;
			}
	}
	infile.close();
	//将结果输出到文件中
	outfile.open("IMAGE_FEATURES");
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			outfile<<image_feature[i][j]<<" ";
		}
		outfile<<endl;
	}
	outfile.close();
	outfile.clear();



	//各个地标景点图像的训练和特征提取工作已经完成
	//各景点图片的特征信息提出出来保存在'IMAGE_FEATURE'文件中
	//可以选自退出程序或者来预测一张图像的位置！
	printf("各个地标景点图像的训练和特征提取工作已经完成\n");
	printf("各景点图片的特征信息提出出来保存在'IMAGE_FEATURE'文件中\n");
	printf("可以选自退出程序或者来预测一张图像的位置\n");
	printf("退出程序请按0\n预测景点图像请按1\n");
	scanf("%d", &number);
	if(number == 0) return 0;
	if(number != 0 && number != 1)
	{
		printf("输入错误，程序退出!\n");
		return 1;
	}
	if(number == 1)
	{
		printf("请输入图像keypoint信息对应的.key文件!\n");
		char name[50];
		double center[100][128];
		int image_feature[5][100];
		int feature[100];
		int one_key[128];

		for(int j = 0; j < 100; j++)
			feature[j] = 0;

		scanf("%s", name);

		//先读入CENTER
		printf("开始读入CENTER文件...\n");
		infile.open(out_cluster_file);
		if(!infile.is_open())
		{
			printf("指定的文件打开错误!\n");
			return 1;
		}
		for(int j = 0; j < 100; j ++)
			for(int k = 0; k < 128; k++)
			{
				infile>>center[j][k];
			}
			infile.close();

		//然后读入IMAGE_FEATURE
		printf("开始读入IMAGE_FEATURE文件...\n");
		infile.open("IMAGE_FEATURES");
		if(!infile.is_open())
		{
			printf("指定的文件打开错误!\n");
			return 1;
		}
		for(int j = 0; j < 5; j ++)
			for(int k = 0; k < 100; k++)
			{
				infile>>image_feature[j][k];
			}
			infile.close();

		infile.open(name);
		if(!infile.is_open())
		{
			printf("指定的文件打开错误!\n");getchar();
			return 1;
		}

		printf("开始计算每个keypoints是属于哪一类...\n");
		double cur_mindist = 10.0e20;
		int cur_minnumber = 0;
		int counter = 0;
		float temp;
		while(!infile.eof())  //判断每一个keypoint是属于哪一个center里面的
		{
			//先要将前面的四个浮点数读走
			infile>>discard[0];
			infile>>discard[1];
			infile>>discard[2];
			infile>>discard[3];
			for(int j = 0; j < 128; j++)
				infile>>one_key[j];
			printf("完成了%d个keypoints...\n", ++counter);
			double dist;
			for(int l = 0; l < 100; l++)
			{
				dist = 0.0;
				for (int k = 0; k < 128; k++)
					dist += (one_key[k] - center[l][k]) * (one_key[k] - center[l][k]);
				if(dist < cur_mindist)
				{
					cur_mindist = dist;
					cur_minnumber = l;  //l不是1
				}
			}

			feature[cur_minnumber]++;
		}
		infile.close();

		//计算出这张图像的feature之后，在与image_feature相比，判断与哪一类图像距离最近
		//从而判断它属于哪一类

		printf("开始计算这张图像描述的内容信息，判断出其拍摄地...\n");
		double dist;
		cur_mindist = 10.0e20;
		cur_minnumber = 0;
		for(int j = 0; j < 5; j++)
		{
			dist = 0.0;
			for(int k = 0; k < 100; k++)
				dist += (feature[k] - image_feature[j][k]) * (feature[k] - image_feature[j][k]);
			if(dist < cur_mindist)
			{
				cur_mindist = dist;
				cur_minnumber = j; 
			}
		}
		printf("这张图像拍摄的是： %s!\n", image_name[cur_minnumber]);
	}

	getchar();getchar();
	return 0;
}