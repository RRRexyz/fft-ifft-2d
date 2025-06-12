#include "ifft.hpp"
#include "fft.hpp"


/**
 * @brief 一维快速傅里叶逆变换(IFFT)算法
 * @param Xk 要进行逆变换的序列X(k)，格式为1xN的cv::Mat矩阵
 *           其中的元素类型为std::complex<double>（实部虚部都为双精度浮点数的复数）
 * @param N 傅里叶逆变换的点数，必须是2的整数次方
 * @param type 输入序列X(k)的数据类型，支持的有uchar、int、complex（代表std::complex<double>）
 * @return 傅里叶逆变换的结果x(n)，格式为1xN的cv::Mat矩阵
 *          其中的元素类型为std::complex<double>（实部虚部都为双精度浮点数的复数）
 */
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


/**
 * @brief 对二维傅里叶变换后进行过中心化的结果进行逆中心化，以进行IFFT运算
 * @param complexImg 要进行逆中心化的二维频域复数矩阵
 */
void fftInverseShift(cv::Mat& complexImg) 
{
    int cx = complexImg.cols / 2;
    int cy = complexImg.rows / 2;
    
    // 定义四个象限的ROI
    cv::Mat q0(complexImg, cv::Rect(0, 0, cx, cy));      // 左上
    cv::Mat q1(complexImg, cv::Rect(cx, 0, cx, cy));     // 右上
    cv::Mat q2(complexImg, cv::Rect(0, cy, cx, cy));     // 左下
    cv::Mat q3(complexImg, cv::Rect(cx, cy, cx, cy));    // 右下
    
    // 交换象限（与中心化操作完全相同）
    cv::Mat tmp;
    q0.copyTo(tmp); q3.copyTo(q0); tmp.copyTo(q3);  
    q1.copyTo(tmp); q2.copyTo(q1); tmp.copyTo(q2);  
}


/**
 * @brief 二维快速傅里叶逆变换(IFFT)算法
 * @param xnm 要进行变换的二维矩阵X(k,v)，
 *           其中的元素类型为std::complex<double>（实部虚部都为双精度浮点数的复数）
 * @param origin_rows 原二维矩阵x(n,m)的行数
 * @param origin_cols 原二维矩阵x(n,m)的列数
 * @param type 原二维矩阵x(n,m)的数据类型，支持的有uchar、int、complex（代表std::complex<double>）
 * @return 傅里叶逆变换的结果x(n,m)，大小将裁剪为与进行FFT时的原二维矩阵x(n,m)相同
 */
cv::Mat IFFT2D(cv::Mat Xkv, int origin_rows, int origin_cols, QString type)
{
    fftInverseShift(Xkv); // 逆中心化
    int N=Xkv.size[0];
    cv::Mat Xkv_expand = Xkv.clone();

    cv::Mat xnv = cv::Mat_<std::complex<double>>(N, N); // 定义矩阵存储第一级IFFT结果
    for(int i=0; i<N; i++) // 遍历每一列
    {
        cv::Mat Xv = Xkv_expand.col(i).t();
        cv::Mat xm = IFFT(Xv, N).t();
        for(int j=0; j<N; j++) // 遍历每一行，将Xv的值复制到Xkv中
        {
            xnv.at<std::complex<double>>(j, i) = xm.at<std::complex<double>>(j, 0);
        }
    }

    cv::Mat xnm = cv::Mat_<std::complex<double>>(N, N); // 定义矩阵存储第二级FFT结果
    for(int i=0; i<N; i++) // 遍历每一行
    {
        cv::Mat Xk = xnv.row(i);
        cv::Mat xn = IFFT(Xk, N);
        for(int j=0; j<N; j++) // 遍历每一列，将Xk的值复制到Xkv中
        {
            xnm.at<std::complex<double>>(i, j) = xn.at<std::complex<double>>(0, j);
        }
    }

    // 裁剪结果矩阵，去掉补零部分
    if(type == "uchar")
    {
        cv::Mat xnm_origin = cv::Mat_<uchar>(origin_rows, origin_cols);
        for(int i=0; i<origin_rows; i++)
        for(int j=0; j<origin_cols; j++)
        {
            xnm_origin.at<uchar>(i, j) = static_cast<uchar>(xnm.at<std::complex<double>>(i, j).real());
        }
        return xnm_origin;
    }
    else if(type == "int")
    {
        cv::Mat xnm_origin = cv::Mat_<int>(origin_rows, origin_cols);
        for(int i=0; i<origin_rows; i++)
        for(int j=0; j<origin_cols; j++)
        {
            xnm_origin.at<int>(i, j) = static_cast<int>(xnm.at<std::complex<double>>(i, j).real());
        }
        return xnm_origin;
    }
    else if(type == "complex")
    {
        cv::Mat xnm_origin = cv::Mat_<std::complex<double>>(origin_rows, origin_cols);
        for(int i=0; i<origin_rows; i++)
        for(int j=0; j<origin_cols; j++)
        {
            xnm_origin.at<std::complex<double>>(i, j) = xnm.at<std::complex<double>>(i, j);
        }
        return xnm_origin;
    }
    return xnm;
}


/**
 * @brief 生成高斯低通滤波器
 * @param size 滤波器尺寸
 * @param sigma 标准差
 * @return 高斯低通滤波器矩阵
 */
cv::Mat createGaussianLPF(cv::Size size, float sigma) 
{
    cv::Mat filter = cv::Mat_<std::complex<double>>(size);
    float d0 = 2 * sigma * sigma;  // 控制截止频率
    cv::Point center(size.width / 2, size.height / 2);

    for (int i = 0; i < size.height; i++) 
    {
        for (int j = 0; j < size.width; j++) 
        {
            float d = pow(i - center.y, 2) + pow(j - center.x, 2);
            filter.at<std::complex<double>>(i, j) = std::complex<double>(exp(-d / d0), 0);
        }
    }
    return filter;
}


/**
 * @brief 生成理想低通滤波器
 * @param size 滤波器尺寸
 * @param cutoffRadius 截止半径
 * @return 理想低通滤波器矩阵
 */
cv::Mat createIdealLPF(cv::Size size, float cutoffRadius) 
{
    cv::Mat filter = cv::Mat_<std::complex<double>>(size);
    cv::Point center(size.width / 2, size.height / 2);

    for (int i = 0; i < size.height; i++) {
        for (int j = 0; j < size.width; j++) {
            // 计算中心化的坐标
            float d = std::sqrt(pow(i - center.y, 2) + pow(j - center.x, 2));
            // 设置滤波器值
            if (d <= cutoffRadius) {
                filter.at<std::complex<double>>(i, j) = std::complex<double>(1, 0);  // 截止频率内的频率通过
            }
        }
    }
    return filter;
}


/**
 * @brief 计算两张图像的均方误差（MSE）
 * @param original 原图像
 * @param reconstructed 重建后的图像
 * @return MSE
 */
double computeMSE(const cv::Mat& original, const cv::Mat& reconstructed) 
{
    // 确保图像尺寸和类型一致
    CV_Assert(original.size() == reconstructed.size());
    CV_Assert(original.type() == CV_8U && reconstructed.type() == CV_8U);

    cv::Mat diff;
    cv::absdiff(original, reconstructed, diff);       // 计算绝对值差
    diff.convertTo(diff, CV_32F);                    // 转为浮点型
    diff = diff.mul(diff);                            // 平方

    return cv::mean(diff)[0];                         // 返回均值 (即MSE)
}


/**
 * @brief 计算两张图像的峰值信噪比（PSNR）
 * @param original 原图像
 * @param reconstructed 重建后的图像
 * @return PSNR
 */
double computePSNR(const cv::Mat& original, const cv::Mat& reconstructed) 
{
    const double MSE = computeMSE(original, reconstructed);
    if (MSE <= 1e-10) return 100;                    // 图像相同时返回高值

    const double MAX = 255.0;
    const double psnr = 10.0 * log10((MAX * MAX) / MSE);
    return psnr;
}
