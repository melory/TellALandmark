TellALandmark
=============

输入一张图片，判断这个图片是拍摄于哪个标志性的景点

主要算法：
1.	收集各个景点的图片（比如jpg图像，每个景点500张），然后将所有这些图片的SIFT的keypoints提取出来；

2.	然后将所有的这些图像的SIFT keypoints进行聚类，根据参考文献，可以使用k-means进行聚类，k取值为100~500之间，然后得到100~500个聚类中心（center）；

3.	得到的所有的keypoints其实是属于某一个景点的，而聚类之后每一个keypoint又是可以划分到这k个center中的一个，这样就可以统计每一个景点在每一个center上有多少个keypoints，例如选取玉泉的5个标志性的建筑（maoxiang，caozhulou，damen，tushuguan,…）来实验，k-means的k取值200，则可以使用如下形式的数组来统计这些信息;
maoxiang[200], caozhulou[200], tushuguan[200],…，假设maoxiang[45]=1236，则表示所有maoxiang图片得到的SIFT keypoints中有1236个SIFT keypoints是属于center45的；

4.	得到了每个标志性景点图像的keypoints分布情况之后，对于一张输入的待判定图像，首先提取出它的SIFT keypoints，然后计算出它每一个SIFT keypoints是属于哪一个center（直接通过距离计算，计算每一个SIFT keypoint与哪一个center的距离最小），这样就可以得到这张待测试图像的SIFT keypoints的分布情况，也可以使用类似的数组来描述：
Image[200]，比如image[45]=219，则表示这张图像的所有SIFT keypoints里面，有219个是属于center45的；

5.	计算这张图像是属于哪一个标志性景点，还是简单的根据距离来计算，依次计算dist(landmarki[200], image[200])，即计算待测试图像的SIFT keypoint分布向量（image[200]）和之前得到的每个景点的SIFT keypoint分布向量（maoxiang[200]，tushuguan[200]，damen[200]等等），找出dist(landmarki[200], image[200])最小的i，那么这张待测图像就属于landmarki。

实验和编程实现的过程：

1.	首先将各个景点的jpg的图像（训练图像）转换成灰度的pgm图像（有些sift程序处理需要pgm格式），可以使用IMAGEMAGICK来进行处理，使用简单的命令就可以完成图片的批转换：
mogrify -format pgm *.jpg

2.  sift算法。Sift算法现在已经很成熟，国外有很多有名的project做包括sift在内的各种图像处理算法，所以完全不需要也不必要再自己实现sift算法来用在该大作业中。Sift算法比较有名的是University of California的Andrea Vedaldi教授的一个c++实现的sift算法siftpp（其实也是被VLFeat支持的），具体参见http://www.vlfeat.org/~vedaldi/code/siftpp.html。而VLFeat（http://www.vlfeat.org/）库中也有很好的sift的实现。
本实验中选取的是siftpp的实现；

3.  聚类算法。聚类算法一般都是非常需要cpu和内存的，本实验中参考了ICML'03中的一个快速k-means(http://www-cse.ucsd.edu/~elkan/fastkmeans.html),它是一个matlab code的快速k-means，本次实验中将其改写成了对应的c++实现（可能在效率上没有仔细考虑）；

4.  本实验的主要代码（除去pgm转换和SIFT keypoints的提取是在linux下完成的）是在windows环境下的VS2008中实现的，可能在linux环境中运行需要将程序进行少部分的改动；

5.  TellALandmark中是一个完整的实现过程（需要读取所有图像的keypoint的.key文件），包括对所有keypoints进行聚类，计算每个景点的keypoints的分布情况和计算待测试图像的keypoints分布情况，最后判断待测图像是属于哪一个landmark。完整的运行整个程序可能需要很长时间（根据训练图像的多少和每张图像的keypoints的多少）；

6.  TestTellALandmark是一个测试一张输入图像的程序，该程序直接使用已经提取出的各个景点的keypoints的分布数据（使用TellALandmark提取），直接输入一个待测试图像的SIFT keypoint文件（.key文件）即可。该程序提取的是玉泉的5个景点，分别是曹光彪主楼、玉泉大门、图书馆、毛主席像、以及生仪楼，但是训练的数据集很小，每个景点只选取了10张图像，聚类也没有等到完全收敛就停止了（当时选取的原始图像过大，提取的SIFT keypoints过多，每一张有2W+歌keypoints，当时从时间上考虑，所以没有等到完全收敛就强制停止了k-means算法的迭代，建议进行训练的每一张图像的长和宽选取为500pix左右，这样每一张图像的keypoints不会过多，大概2k左右），所以有些照片的测试结果可能与实际情况有差别，若要获得更准确的实验结果，建议重新运行TellALandmark（即重新选取更多的训练图像，重新进行训练）；

7.  本实验由于时间仓促，代码是几个部分合在一起的，代码没有进行很详细的调整，里面既有c代码也有c++代码，代码风格和规范上做的很差；

8.  本实验重新选取了更大的数据集（分别从互联网上爬取了埃菲尔铁塔、自由女神像、进门大桥、凯旋门、比萨斜塔、上海东方之珠和美国银行中心大厦各200张图像）重做实验，结果还没有出来；

9.  该实验所用的代码仅作为计算机视觉的提交作业，不经允许不能上传到互联网上！作者可能会进行后续工作。

