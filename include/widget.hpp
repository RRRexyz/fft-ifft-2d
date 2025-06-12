#ifndef WIDGET_HPP
#define WIDGET_HPP

#include <QWidget>
#include "ui_widget.h"
#include <opencv2/opencv.hpp>

class Widget : public QWidget
{
    public:
        explicit Widget(QWidget *parent = nullptr);
        ~Widget();
    private:
        Ui::Widget *ui;
        QPixmap recovered_image_pixmap;
        QPixmap lpf_recovered_image_pixmap;
        cv::Mat Xkv;
        cv::Mat gray_image;
        double recoveredMSE;
        double recoveredPSNR;
        double lpfMSE;
        double lpfPSNR;

        void on_enter_ok_clicked();
        void on_without_lpf_stateChanged(bool state);
        void on_with_sigma_slider_valueChanged(int value);
        void on_with_sigma_value_valueChanged(int value);
};


#endif