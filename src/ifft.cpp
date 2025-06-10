#include "ifft.hpp"
#include "fft.hpp"
#include <QString>


cv::Mat IFFT(cv::Mat Xk, int N)
{
    if(Xk.rows != 1) throw std::invalid_argument("X(k) must be 1xN matrix");

    if(N < Xk.cols) throw std::invalid_argument("N must be >= X(k) size");

    int M = std::log2(N); // 得到FFT级数

    // 将原序列X(k)补零扩充至2的整数次方个元素
    cv::Mat Xk_expand = cv::Mat_<std::complex<double>>(1, N);        
    for(int i=0; i<Xk.size[1]; i++)
    {
        Xk_expand.at<std::complex<double>>(0, i) = Xk.at<std::complex<double>>(0, i);
    }
    for(int i=Xk.size[1]; i<N; i++)
    {
        Xk_expand.at<std::complex<double>>(0, i) = std::complex<double>(0, 0);
    }

    // 创建一个向量，存储X(k)码位倒读后的索引顺序
    std::vector<int> Xk_expand_reversed_order;
    for(int i=0; i<N; i++)
    {
        int n = i; // 原索引
        int new_n = 0; // 码位倒读后的索引
        for(int i=0; i<M; i++)
        {
            new_n += ((n & 1) << (M-1-i)) ;
            n >>= 1;
        }
        Xk_expand_reversed_order.push_back(new_n);
    }

    for(int m=0; m<M; m++)  // 遍历蝶形图的每一级
    for(int l=0; l<N/(2<<m); l++)  // 遍历每一级的每一个“群”
    for(int i=0; i<(1<<m); i++)  // 遍历每一个“群”中的每一个蝶形结
    {
        int order1 = Xk_expand_reversed_order[l*(2<<m) + i];
        int order2 = Xk_expand_reversed_order[l*(2<<m) + i + (1<<m)];
        std::complex<double> tmp1 = Xk_expand.at<std::complex<double>>(0, order1);
        std::complex<double> tmp2 = W(2<<m, i)*Xk_expand.at<std::complex<double>>(0, order2);
        Xk_expand.at<std::complex<double>>(0, order1) = tmp1 + tmp2;
        Xk_expand.at<std::complex<double>>(0, order2) = tmp1 - tmp2;
    }

    // 至此Xk_expand中已经存储了FFT结果，但是按照码位倒读的顺序排列，需要将其恢复原顺序
    // 并且计算IFFT时还得乘上一个系数1/N
    cv::Mat xn = cv::Mat_<std::complex<double>>(1, N);
    for(int i=0; i<N; i++)
    {
        int origin_i = Xk_expand_reversed_order[i];
        xn.at<std::complex<double>>(0, i) = Xk_expand.at<std::complex<double>>(0, origin_i);
    }
    xn = xn/N;
    return xn;
}