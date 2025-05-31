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
 * @brief 一维快速傅里叶变换(FFT)
 * @param xn 要进行变换的序列x(n)，格式为1xN的cv::Mat矩阵
 * @return 傅里叶变换的结果X(k)，格式为1xN的cv::Mat矩阵
 */
cv::Mat FFT(cv::Mat xn)
{
    if(xn.rows != 1) throw std::invalid_argument("x(n) must be 1xN matrix");
    int N = xn.size[1]; // FFT点数
    if(N < 1) throw std::invalid_argument("x(n) size must be > 0");
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
    // 将原序列x(n)补零扩充至2的整数次方个元素
    cv::Mat xn_expand = cv::Mat_<uchar>(1, N);
    for(int i=0; i<xn.size[1]; i++)
    {
        xn_expand.at<uchar>(0, i) = xn.at<uchar>(0, i);
    }
    for(int i=xn.size[1]; i<N; i++)
    {
        xn_expand.at<uchar>(0, i) = 0;
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


    cv::Mat Xk = cv::Mat_<std::complex<double>>(1, N);
    return Xk;
}
