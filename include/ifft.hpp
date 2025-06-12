#ifndef IFFT_HPP
#define IFFT_HPP
#include <iostream>
#include <opencv2/opencv.hpp>
#include <QString>

cv::Mat IFFT(cv::Mat Xk, int N);

cv::Mat IFFT2D(cv::Mat Xkv, int origin_rows, int origin_cols, QString type);

cv::Mat createGaussianLPF(cv::Size size, float sigma);

cv::Mat createIdealLPF(cv::Size size, float cutoffRadius);

double computeMSE(const cv::Mat& original, const cv::Mat& reconstructed);

double computePSNR(const cv::Mat& original, const cv::Mat& reconstructed);

#endif // IFFT_HPP