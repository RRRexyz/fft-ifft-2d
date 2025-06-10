#ifndef FFT_HPP
#define FFT_HPP
#include <iostream>
#include <opencv2/opencv.hpp>
#include <QString>

std::complex<double> W(int N, int k);

cv::Mat FFT(cv::Mat xn, int N, QString type);

cv::Mat FFT2D(cv::Mat xnm, QString type);

#endif // FFT_HPP