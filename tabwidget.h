#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QTimer>
#include <opencv.hpp>
#include <vector>
#include <QComboBox>

namespace Ui {
class TabWidget;
}

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit TabWidget(QWidget *parent = nullptr);
    ~TabWidget();

private slots:
    //Snap
    void snapPlay();
    void snapSnap();
    void timerStartRefresh();
    void timerStopRefresh();
    bool fileExist(const std::string& name);
    void roicountUpdater();
    //FD
    void loadFile();
    void loadNet();
    void defectDetect();
    //RTD

    //Settings
    void setImgDir();

    //Info&Help

private:
    Ui::TabWidget *ui;
    int ROI_Count = 0;
    QTimer *m_pTimer;
    cv::VideoCapture *m_pVideo1, *m_pVideo2;
    std::vector<int> compression_params;

    int m_sysIdx;

    cv::Mat* m_pImg;
    cv::VideoCapture* m_pVdo;
    cv::dnn::Net* m_pNet;
    void setSysIdx();
    bool winOrLnx();
    cv::Mat litPic(cv::Mat src, int cols, int rows);
};

#endif // TABWIDGET_H
