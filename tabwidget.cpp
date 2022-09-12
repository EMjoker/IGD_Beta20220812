#include "tabwidget.h"
#include "ui_tabwidget.h"
//QT
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
//CV
#include <opencv.hpp>
#include <opencv_modules.hpp>
//STD
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>
#include <sys/stat.h>
#include <filesystem>

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::TabWidget)
{
    ui->setupUi(this);
    setSysIdx();
    m_pTimer = new QTimer(this);
    m_pTimer->setInterval(17);
    compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    ui->snpStopBtn->setEnabled(false);

    ui->snpCB1->setCurrentIndex(1);
    ui->snpCB2->setCurrentIndex(2);
    ui->snpSnapBtn->setEnabled(false);
    ui->snpLbl1->setScaledContents(true);
    ui->snpLbl2->setScaledContents(true);

    ui->fdLbl->setScaledContents(true);

    connect(m_pTimer, &QTimer::timeout, this, &TabWidget::snapPlay);

    //Snap
    connect(ui->snpPlaybtn, &QPushButton::clicked, this, [=](){
        timerStartRefresh();
//        if(m_pVideo1->get(cv::CAP_PROP_FRAME_HEIGHT) != 1080){
//            m_pVideo1->set(cv::CAP_PROP_FRAME_WIDTH, 1920);
//            m_pVideo1->set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
//            m_pVideo2->set(cv::CAP_PROP_FRAME_WIDTH, 1920);
//            m_pVideo2->set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
//        }
    });
    connect(ui->snpStopBtn, &QPushButton::clicked, this, &TabWidget::timerStopRefresh);
    connect(ui->snpSnapBtn, &QPushButton::clicked, this, &TabWidget::snapSnap);
    connect(ui->snpCBT, &QComboBox::currentIndexChanged, this, &TabWidget::roicountUpdater);

    //FD
    connect(ui->fdfBtn, &QPushButton::clicked, this, &TabWidget::loadFile);
    connect(ui->fdnBtn, &QPushButton::clicked, this, &TabWidget::loadNet);
    connect(ui->fddBtn, &QPushButton::clicked, this, &TabWidget::defectDetect);

    //RTD


    //Settings
    connect(ui->stgImgBtn, &QPushButton::clicked, this, &TabWidget::setImgDir);

}

TabWidget::~TabWidget()
{
    delete ui;
}

bool TabWidget::winOrLnx(){
#ifdef Q_OS_WIN
    return false;
#endif
#ifdef Q_OS_LINUX
    return true;
#endif
}

void TabWidget::setSysIdx(){
    if(winOrLnx())
        m_sysIdx = 2;
    else
        m_sysIdx = 1;
    return;
}

cv::Mat TabWidget::litPic(cv::Mat src, int cols, int rows){
    cv::cvtColor(src, src, cv::COLOR_BGR2GRAY);
    cv::resize(src, src, cv::Size(cols, rows));
    return src;
}

void TabWidget::snapPlay(){
    cv::Mat frame1, frame2;
    *m_pVideo1 >> frame1;
    *m_pVideo2 >> frame2;
    if (frame1.empty() || frame2.empty())
    {
        return;
    }
    cv::cvtColor(frame1, frame1, cv::COLOR_BGR2RGB);
    QImage disImage1 = QImage((const unsigned char*)(frame1.data),
                              frame1.cols,frame1.rows,
                              frame1.cols*frame1.channels(),
                              QImage::Format_RGB888);
    cv::cvtColor(frame2, frame2, cv::COLOR_BGR2RGB);
    QImage disImage2 = QImage((const unsigned char*)(frame2.data),
                              frame2.cols,frame2.rows,
                              frame2.cols*frame2.channels(),
                              QImage::Format_RGB888);
    ui->snpLbl1->setPixmap(QPixmap::fromImage(disImage1));
    ui->snpLbl2->setPixmap(QPixmap::fromImage(disImage2));
}

void TabWidget::snapSnap(){
    std::string* src_pic_name1 = new std::string;
    std::string* src_pic_name2 = new std::string;
    *src_pic_name1 = ui->stgImgLine->text().toStdString()
            + "/src/"
            + ui->snpCBT->currentText().toStdString()
            + "/left/"
            + std::to_string(ROI_Count)
            + ".png";
    *src_pic_name2 = ui->stgImgLine->text().toStdString()
            + "/src/"
            + ui->snpCBT->currentText().toStdString()
            + "/right/"
            + std::to_string(ROI_Count)
            + ".png";
    cv::Mat* frame1 = new cv::Mat;
    cv::Mat* frame2 = new cv::Mat;
    *m_pVideo1 >> *frame1;
    *m_pVideo2 >> *frame2;
    cv::imwrite(src_pic_name1->data(), *frame1, compression_params);
    cv::imwrite(src_pic_name2->data(), *frame2, compression_params);
    *frame1 = litPic(*frame1, 64, 64);
    *frame2 = litPic(*frame2, 64, 64);
    *src_pic_name1 = ui->stgImgLine->text().toStdString()
            + "/lit/"
            + ui->snpCBT->currentText().toStdString()
            + "/left/"
            + std::to_string(ROI_Count)
            + ".png";
    *src_pic_name2 = ui->stgImgLine->text().toStdString()
            + "/lit/"
            + ui->snpCBT->currentText().toStdString()
            + "/right/"
            + std::to_string(ROI_Count)
            + ".png";
    cv::imwrite(src_pic_name1->data(), *frame1, compression_params);
    cv::imwrite(src_pic_name2->data(), *frame2, compression_params);
    delete frame1;
    delete frame2;
    delete src_pic_name1;
    delete src_pic_name2;
    ++ROI_Count;
}

void TabWidget::timerStartRefresh(){
    m_pVideo1 = new cv::VideoCapture(m_sysIdx*(ui->snpCB1->currentIndex()));
    m_pVideo2 = new cv::VideoCapture(m_sysIdx*(ui->snpCB2->currentIndex()));
    m_pTimer->start();
    ui->snpPlaybtn->setEnabled(false);
    ui->snpStopBtn->setEnabled(true);
    ui->snpSnapBtn->setEnabled(true);
    ui->fdTab->setEnabled(false);
    ui->rtdTab->setEnabled(false);
}

void TabWidget::timerStopRefresh(){
    ui->snpPlaybtn->setEnabled(true);
    ui->snpStopBtn->setEnabled(false);
    ui->snpSnapBtn->setEnabled(false);
    ui->fdTab->setEnabled(true);
    ui->rtdTab->setEnabled(true);
    m_pTimer->stop();
    delete m_pVideo1;
    delete m_pVideo2;
}

bool TabWidget::fileExist(const std::string& name){
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

void TabWidget::roicountUpdater(){
    if(std::filesystem::exists(ui->stgImgLine->text().toStdString()
                               + "/src/"
                               + ui->snpCBT->currentText().toStdString()
                               + "/left")){
        auto dirIter = std::filesystem::directory_iterator(ui->stgImgLine->text().toStdString()
                                                           + "/src/"
                                                           + ui->snpCBT->currentText().toStdString()
                                                           + "/left");
        ROI_Count = 0;
        for (auto& entry : dirIter)
        {
            if (entry.is_regular_file())
            {
                ++ROI_Count;
            }
        }
    }
    else {
        this->setCurrentWidget(ui->stgTab);
    }
}

void TabWidget::setImgDir(){
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    "Set Saved Images Directory",
                                                    "./");
    ui->stgImgLine->setText(dir);
    if(ui->stgImgLine->text().isEmpty())
        ui->stgImgLine->setText("./pic");
    std::filesystem::create_directory(ui->stgImgLine->text().toStdString()
                                      + "/src");
    std::filesystem::create_directory(ui->stgImgLine->text().toStdString()
                                      + "/lit");
    for(int i = 0;i < ui->snpCBT->maxCount();++i){
        std::filesystem::create_directory(ui->stgImgLine->text().toStdString()
                                          + "/src/"
                                          + ui->snpCBT->itemText(i).toStdString());
        std::filesystem::create_directory(ui->stgImgLine->text().toStdString()
                                          + "/src/"
                                          + ui->snpCBT->itemText(i).toStdString()
                                          + "/left");
        std::filesystem::create_directory(ui->stgImgLine->text().toStdString()
                                          + "/src/"
                                          + ui->snpCBT->itemText(i).toStdString()
                                          + "/right");
        std::filesystem::create_directory(ui->stgImgLine->text().toStdString()
                                          + "/lit/"
                                          + ui->snpCBT->itemText(i).toStdString());
        std::filesystem::create_directory(ui->stgImgLine->text().toStdString()
                                          + "/lit/"
                                          + ui->snpCBT->itemText(i).toStdString()
                                          + "/left");
        std::filesystem::create_directory(ui->stgImgLine->text().toStdString()
                                          + "/lit/"
                                          + ui->snpCBT->itemText(i).toStdString()
                                          + "/right");
    }
}

void TabWidget::loadFile(){
    QString path = QFileDialog::getOpenFileName(this,
                                                "Open Images or Videos",
                                                "./",
                                                "Images(*.png *.jpg *.jpeg)");
    ui->fdfLE->setText(path);
    if(path.isEmpty()){
        qDebug() << "File: Load Nothing.";
        return;
    }
    m_pImg = new cv::Mat(cv::imread(path.toStdString(),cv::IMREAD_COLOR));
    if(m_pImg->empty()){
        qDebug() << "Image/Video File can't be initialized by cv::Mat.";
        return;
    }
    qDebug() << "Img loaded.";
    cv::cvtColor(*m_pImg, *m_pImg, cv::COLOR_BGR2RGB);
    QImage disImage1 = QImage((const unsigned char*)(m_pImg->data),
                              m_pImg->cols,m_pImg->rows,
                              m_pImg->cols*m_pImg->channels(),
                              QImage::Format_RGB888);
    ui->fdLbl->setPixmap(QPixmap::fromImage(disImage1));
}

void TabWidget::loadNet(){
    QString path = QFileDialog::getOpenFileName(this,
                                                "Open Net",
                                                "./",
                                                "TFNN(*.pb)");
    ui->fdnLE->setText(path);
    if(path.isEmpty()){
        qDebug() << "Net: Load Nothing.";
        return;
    }
    m_pNet = new cv::dnn::Net(cv::dnn::readNetFromTensorflow(path.toStdString()));
    if(m_pNet->empty()){
        qDebug() << "Net: Read Failed.";
        return;
    }
    qDebug() << "Net loaded.";
    std::vector<cv::String> netLayerName = m_pNet->getLayerNames();
    qDebug() << "Layer names: ";
    for(unsigned long i = 0;i < netLayerName.size();++i){
        qDebug() << netLayerName[i].data();
    }
    return;
}

void TabWidget::defectDetect(){
    if(!m_pImg->empty()){
        std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();
        cv::Mat preInput;
        cv::resize(*m_pImg, preInput, cv::Size(64, 64));
        cv::cvtColor(preInput, preInput, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(preInput, preInput);
        preInput.convertTo(preInput, CV_32FC1, 1.0/255.0);
        m_pNet->setInput(cv::dnn::blobFromImage(preInput));
        cv::Mat result = m_pNet->forward();
        std::chrono::steady_clock::time_point timeEnd = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> elapsed = timeEnd - timeStart;
        if(result.at<float>(0, 0) >= result.at<float>(0, 1) && result.at<float>(0, 0)>= 0.65){
            ui->fdRst->setText(QString(
                                   (std::string("Defect: ") +
                                    std::to_string(result.at<float>(0, 0)) +
                                    std::string("   Consumed Time: ") +
                                    std::to_string(elapsed.count()) +
                                    std::string(" ms"))
                                   .data()
                                   ));
        }
        else if(result.at<float>(0, 0) < result.at<float>(0, 1) && result.at<float>(0, 1)>= 0.65){
            ui->fdRst->setText(QString(
                                   (std::string("Perfect: ") +
                                    std::to_string(result.at<float>(0, 1)) +
                                    std::string("   Consumed Time: ") +
                                    std::to_string(elapsed.count()) +
                                    std::string(" ms"))
                                   .data()
                                   ));
        }
        else{
            ui->fdRst->setText(QString(
                                   (std::string("Not Sure...") +
                                    std::string("   Consumed Time: ") +
                                    std::to_string(elapsed.count()) +
                                    std::string(" ms"))
                                   .data()
                                   ));
        }
        return;
    }
    else{
        qDebug() << "Img Empty.";
    }
    return;
}
