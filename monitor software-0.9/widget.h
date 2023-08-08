#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDateTime>
#include <QSystemTrayIcon>
#include <QAction>
#include <QTimer>
#include <QMenu>
#include "windows.h"
#include "nvml.h"
#include "info.h"
#include "OlsApi.h"
#include "QMovie"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    //系统托盘+定时器+动画----------------------------------
    QSystemTrayIcon *m_SystemTrayIcon;
    void closeEvent(QCloseEvent *event) override;//重写关闭事件
    void CreatMenu();
    void CreatAction();
    QAction *m_QActionShowMain;
    QAction *m_QActionExitApp;
    QAction *m_QActionShowInfo;
    QMenu *m_Menu;
    QTimer *time=new QTimer(this);
    Info *information=new Info();
    QMovie *gif=new QMovie(":/icon/ganyu.GIF");

    QString GetWare(QString cmd);



    //通信上传---------------------------------------------//
    QTcpSocket m_tcpSocket;
    int useageRam;
    QString useageGPU;
    QString CPUtemp;
    QString GPUtemp;
    //设备信息---------------------------------------------//

    QString cpuInf,cpuInfo;
    QString gpuInfo;
    char *gpunamee;
    void uploadInfo();

    QNetworkAccessManager *mNetAccessManager;
    //天气信息---------------------------------------------//
    QString city;
    int temptature;
    int pm25;
    QString type;
    void parseJson(QByteArray s);
    void getWeatherInfo(QString cityName);

    //CPU温度获取------------------------------------------//
    bool cpustate1=false;
    bool cpustate2=false;
    int cpuTemp[8];
    int coreNumber;
    int cputemp;
    DWORD initTemp;
    DWORD finalTemp;
    void CPUInit();//初始化

    //GPU温度获取-------------------------------------------//
    nvmlReturn_t result;
    nvmlDevice_t device;
    char gpuname[64];
    unsigned int gputemp;
    nvmlUtilization_t utilization;
    unsigned int gpuram;
    void GPUInit();//初始化


private slots:
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason);//还原槽函数
    void slotActionMain();
    void slotActionExitApp();
    void slotActionInfo();
    void slotGetInfo();
    void slotGetCPUTemp();
    void slotGetGPUTemp();
    void onReplied(QNetworkReply *reply);
    void uploadData();


    void on_startBtn_clicked();

private:
    Ui::Widget *ui;

};
#endif // WIDGET_H
