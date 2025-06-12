#include "fft.hpp"
#include <QDebug>
#include <vector>
#include <cmath>


#define PI  std::acos(-1.0)


/**
 * @brief 计算W(N,k)的值
 * @param N 序列长度
 * @param k 次方数
 * @return W(N,k)的值
 */
std::complex<double> W(int N, int k)
{
    std::complex<double> w(0, -2*PI/N*k);
    return std::exp(w);
}


/**
 * @brief 一维快速傅里叶变换(FFT)算法
 * @param xn 要进行变换的序列x(n)，格式为1xN的cv::Mat矩阵
 *           其中的元素类型为uchar(8位无符号整数)，这是因为灰度图像的灰度值类型为uchar
 * @param N 傅里叶变换的点数，必须是2的整数次方
 * @param type 输入序列x(n)的数据类型，支持的有uchar、int、complex（代表std::complex<double>）
 * @return 傅里叶变换的结果X(k)，格式为1xN的cv::Mat矩阵
 *          其中的元素类型为std::complex<double>（实部虚部都为双精度浮点数的复数）
 */
cv::Mat FFT(cv::Mat xn, int N, QString type)
{
    if(xn.rows != 1) throw std::invalid_argument("x(n) must be 1xN matrix");

    if(N < xn.cols) throw std::invalid_argument("N must be >= x(n) size");

    int M = std::log2(N); // 得到FFT级数

    // 将原序列x(n)补零扩充至2的整数次方个元素，并且将元素其转化为复数形式
    cv::Mat xn_expand = cv::Mat_<std::complex<double>>(1, N);        
    if(type == "uchar") // 输入序列x(n)的数据类型为uchar
    {
        for(int i=0; i<xn.size[1]; i++)
        {

            xn_expand.at<std::complex<double>>(0, i) = static_cast<double>(xn.at<uchar>(0, i));
        }
    }
    else if(type == "int") // 输入序列x(n)的数据类型为int
    {
        for(int i=0; i<xn.size[1]; i++)
        {
            xn_expand.at<std::complex<double>>(0, i) = static_cast<double>(xn.at<int>(0, i));
        }
    }
    else if(type == "complex") // 输入序列x(n)的数据类型为complex
    {
        for(int i=0; i<xn.size[1]; i++)
        {
            xn_expand.at<std::complex<double>>(0, i) = xn.at<std::complex<double>>(0, i);
        }
    }

    for(int i=xn.size[1]; i<N; i++)
    {
        xn_expand.at<std::complex<double>>(0, i) = std::complex<double>(0, 0);
    }

    // 创建一个向量，存储x(n)码位倒读后的索引顺序
    std::vector<int> xn_expand_reversed_order;
    for(int i=0; i<N; i++)
    {
        int n = i; // 原索引
        int new_n = 0; // 码位倒读后的索引
        for(int i=0; i<M; i++)
        {
            new_n += ((n & 1) << (M-1-i)) ;
            n >>= 1;
        }
        xn_expand_reversed_order.push_back(new_n);
    }

    for(int m=0; m<M; m++)  // 遍历蝶形图的每一级
    for(int l=0; l<N/(2<<m); l++)  // 遍历每一级的每一个“群”
    for(int i=0; i<(1<<m); i++)  // 遍历每一个“群”中的每一个蝶形结
    {
        int order1 = xn_expand_reversed_order[l*(2<<m) + i];
        int order2 = xn_expand_reversed_order[l*(2<<m) + i + (1<<m)];
        std::complex<double> tmp1 = xn_expand.at<std::complex<double>>(0, order1);
        std::complex<double> tmp2 = W(2<<m, -i)*xn_expand.at<std::complex<double>>(0, order2);
        xn_expand.at<std::complex<double>>(0, order1) = tmp1 + tmp2;
        xn_expand.at<std::complex<double>>(0, order2) = tmp1 - tmp2;
    }

    // 至此xn_expand中已经存储了FFT结果，但是按照码位倒读的顺序排列，需要将其恢复原顺序
    cv::Mat Xk = cv::Mat_<std::complex<double>>(1, N);
    for(int i=0; i<N; i++)
    {
        int origin_i = xn_expand_reversed_order[i];
        Xk.at<std::complex<double>>(0, i) = xn_expand.at<std::complex<double>>(0, origin_i);
    }
    return Xk;
}


/**
 * @brief 对二维傅里叶变换后的结果进行中心化，将低频分量移动到中心位置
 * @param complexImg 要进行中心化的二维频域复数矩阵
 */
void fftShift(cv::Mat& complexImg) 
{
    int cx = complexImg.cols / 2;
    int cy = complexImg.rows / 2;
    
    // 构造四个象限的矩阵视图
    cv::Mat q0(complexImg, cv::Rect(0, 0, cx, cy));
    cv::Mat q1(complexImg, cv::Rect(cx, 0, cx, cy));
    cv::Mat q2(complexImg, cv::Rect(0, cy, cx, cy));
    cv::Mat q3(complexImg, cv::Rect(cx, cy, cx, cy));
    
    // 交换象限
    cv::Mat tmp;
    q0.copyTo(tmp); q3.copyTo(q0); tmp.copyTo(q3);
    q1.copyTo(tmp); q2.copyTo(q1); tmp.copyTo(q2);
}


/**
 * @brief 二维快速傅里叶变换(FFT)算法
 * @param xnm 要进行变换的二维矩阵x(n,m)
 * @param type 输入二维矩阵x(n,m)的数据类型，支持的有uchar、int、complex（代表std::complex<double>）
 * @return 傅里叶变换的结果X(k,v)
 */
cv::Mat FFT2D(cv::Mat xnm, QString type)
{
    int N = xnm.size[0]; // FFT点数
    if(N < 1) throw std::invalid_argument("no image");
    int M = 0; // FFT级数
    // 如果N是2的整数次方，由对数得到M
    if((N & (N-1)) == 0) 
    {
        M = std::log2(N);
    }
    else // 如果N不是2的整数次方，则将点数扩大至大于N的最小的2的整数次方
    {
        int i = N; // 给定的FFT点数
        int times = 0; // 右移次数
        while(i > 1)
        {
            i >>= 1;
            times++;
        }
        M = times + 1; // 得到FFT级数
    }
    N = 1 << M; // 扩充后的FFT点数

    // 将原二维矩阵x(n,m)补零扩充至N*N个元素，并将元素转化为复数形式
    cv::Mat xnm_expand = cv::Mat_<std::complex<double>>(N, N);
    xnm_expand.setTo(0);
    if(type == "uchar") // 输入二维矩阵x(n,m)的数据类型为uchar
    {
        for(int i=0; i<xnm.size[0]; i++)
        for(int j=0; j<xnm.size[1]; j++)
        {
            xnm_expand.at<std::complex<double>>(i, j) = static_cast<double>(xnm.at<uchar>(i, j));
        }
    }
    else if(type == "int") // 输入二维矩阵x(n,m)的数据类型为int
    {
        for(int i=0; i<xnm.size[0]; i++)
        for(int j=0; j<xnm.size[1]; j++)
        {
            xnm_expand.at<std::complex<double>>(i, j) = static_cast<double>(xnm.at<int>(i, j));
        }
    }
    else if(type == "complex") // 输入二维矩阵x(n,m)的数据类型为complex
    {
        for(int i=0; i<xnm.size[0]; i++)
        for(int j=0; j<xnm.size[1]; j++)
        {
            xnm_expand.at<std::complex<double>>(i, j) = xnm.at<std::complex<double>>(i, j);
        }
    }

    cv::Mat Xkm = cv::Mat_<std::complex<double>>(N, N); // 定义矩阵存储第一级FFT结果
    for(int i=0; i<N; i++) // 遍历每一列
    {
        cv::Mat xm = xnm_expand.col(i).t();
        cv::Mat Xv = FFT(xm, N, "complex").t();
        for(int j=0; j<N; j++) // 遍历每一行，将Xv的值复制到Xkv中
        {
            Xkm.at<std::complex<double>>(j, i) = Xv.at<std::complex<double>>(j, 0);
        }
    }

    cv::Mat Xkv = cv::Mat_<std::complex<double>>(N, N); // 定义矩阵存储第二级FFT结果
    for(int i=0; i<N; i++) // 遍历每一行
    {
        cv::Mat xn = Xkm.row(i);
        cv::Mat Xk = FFT(xn, N, "complex");
        for(int j=0; j<N; j++) // 遍历每一列，将Xk的值复制到Xkv中
        {
            Xkv.at<std::complex<double>>(i, j) = Xk.at<std::complex<double>>(0, j);
        }
    }

    fftShift(Xkv); // 进行中心化

    return Xkv;
}