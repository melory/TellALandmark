/**
 * 如果之前已经对各个landmark图像的特征提取完毕（已经得到
 * IMAGE_FEATURE文件）
 * 可以用该例程来判断一张照片是属于哪一个landmark
 * 
 * 该程序需要读取之前生产的IMAGE_FEATURE文件(各类landmark的图像feature
 * 的特征)和CENTER文件（所有训练图像的keypoints的聚类中心），输入是一张图像
 * 的SIFT的所有keypoints文件.key（本例默认是keypoint有132维，最前面的
 * 四个数据是keypoints的方向等其他信息，若.key文件只有128维的keypoint
 * 数据，需要对程序稍微修改一下）
 *
 * Author: huangmeilong
 * Email: meilonghuang@zju.edu.cn
 * Date: 2012/1/11
 **/

#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>


using namespace std;

int main()
{
	char image_name[5][20] = {"caozhu", "maoxiang", "shengyilou", "tushuguan", "zhengmen"};
	double discard[4];
	char *out_cluster_file = "CENTER";  //聚类之后输出的中心
	ifstream infile;
	ofstream outfile;
	
	char name[50];
	double center[100][128];
	int image_feature[5][100];
	int feature[100];
	int one_key[128];

	cout<<"请输入图像keypoint信息对应的.key文件!"<<endl;
	for(int j = 0; j < 100; j++)
		feature[j] = 0;

	//scanf("%s", name);
	cin>>name;

	//先读入CENTER
	cout<<"开始读入CENTER文件..."<<endl;
	infile.open(out_cluster_file);
	if(!infile.is_open())
	{
		cout<<"打开文件错误!不存在'CENTER'文件!"<<endl;getchar();getchar();
		return 1;
	}
	for(int j = 0; j < 100; j ++)
		for(int k = 0; k < 128; k++)
		{
			infile>>center[j][k];
		}
		infile.close();

	//然后读入IMAGE_FEATURE
	cout<<"开始读入IMAGE_FEATURE文件..."<<endl;
	infile.open("IMAGE_FEATURES");
	if(!infile.is_open())
	{
		cout<<"打开文件错误!不存在'IMAGE_FEATURES'文件"<<endl;getchar();getchar();
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
		cout<<"指定的.key文件打开错误!请确定文件存在!"<<endl;getchar();getchar();
		return 1;
	}

	cout<<"开始计算每个keypoints是属于哪一类..."<<endl;
	double cur_mindist = 10.0e20;
	int cur_minnumber = 0;
	int counter = 0;
	float temp;
	while(!infile.eof())  //判断每一个keypoint是属于哪一个center里面的
	{
		//先要将前面的四个浮点数读走，若.key文件的内容仅仅
		//是128维的keypoint信息，即每一行仅仅是128个整数，则
		//需要将一下四行代码注释掉
		infile>>discard[0];
		infile>>discard[1];
		infile>>discard[2];
		infile>>discard[3];
		for(int j = 0; j < 128; j++)
			infile>>one_key[j];
		counter++;
		cout<<"完成了"<<counter<<"个keypoints..."<<endl;
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

	cout<<"开始计算这张图像描述的内容信息，判断出其拍摄地..."<<endl;
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
	cout<<"这张图像拍摄的是："<<image_name[cur_minnumber]<<"!"<<endl;

	getchar();getchar();
	return 0;
}