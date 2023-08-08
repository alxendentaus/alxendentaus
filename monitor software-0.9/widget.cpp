#include "widget.h"
#include "QMenu"
#include "QTimer"
#include "QDateTime"
#include "QPixmap"
#include "info.h"
#include "OlsApi.h"
#include "nvml.h"
#include "ui_widget.h"
#include <QTcpSocket>
#include "QSystemTrayIcon"
#include "QHostAddress"
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "QMovie"
#include "QDebug"


//实现功能 查询CPU温度  内存使用率   换成64位的程序


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //设置动图
    ui->label->setMovie(gif);
    gif->start();

    //连接服务器
    m_tcpSocket.connectToHost(QHostAddress("192.168.31.252"),3333);
    if (m_tcpSocket.waitForConnected()) {
        qDebug() << "Connected to connect to ESP32.";
    }

    CPUInit();
    GPUInit();
    cpuInfo=GetWare("wmic cpu get name");

    //初始化完成 发送一次设备信息
    uploadInfo();
    //启动抓取数据功能
    connect(time,SIGNAL(timeout()),this,SLOT(slotGetInfo()));
    connect(time,SIGNAL(timeout()),this,SLOT(slotGetCPUTemp()));
    connect(time,SIGNAL(timeout()),this,SLOT(slotGetGPUTemp()));
    connect(time,SIGNAL(timeout()),this,SLOT(uploadData()));
    time->start(2000);
}

Widget::~Widget()
{
    delete ui;
    delete m_Menu;
    delete information;
    delete gif;
    delete time;
}
//调用Windows WMIC指令 获取硬件信息 此方法仅适用于Windows环境
QString Widget::GetWare(QString cmd)
{
    QProcess p;
    p.start(cmd);
    p.waitForFinished();
    QString result = QString::fromLocal8Bit(p.readAllStandardOutput());
    QStringList list1 = cmd.split(" ");
    result = result.remove(list1.last(), Qt::CaseInsensitive);
    result = result.replace("\r", "");
    result = result.replace("\n", "");
    result = result.simplified();
    QStringList list2=result.split(" ");
    cpuInf=list2[2]+list2[4];
    return cpuInf;
}

//关闭事件
void Widget::closeEvent(QCloseEvent *event)
{
    m_SystemTrayIcon = new QSystemTrayIcon();
    m_SystemTrayIcon->setIcon(QIcon(":/icon/YS.jpg"));     // 托盘时显示的图片
    m_SystemTrayIcon->setToolTip("snatching");                                             // 鼠标在托盘图片时的提示
    //给QSystemTrayIcon添加槽函数
    connect(m_SystemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason)));
    //建立托盘操作的菜单
    CreatAction();
    CreatMenu();
    m_SystemTrayIcon->show();   // 显示图片图标
    this->hide();
}
//创建托盘菜单
void Widget::CreatMenu()
{
    m_Menu = new QMenu(this);//新增菜单项---显示主界面
    m_Menu->addAction(m_QActionShowInfo);
    m_Menu->addSeparator();
    m_Menu->addAction(m_QActionExitApp); //新增菜单项---退出程序
    m_SystemTrayIcon->setContextMenu(m_Menu);//把QMenu赋给QSystemTrayIcon对象

}
//添加托盘活动
void Widget::CreatAction()
{
    m_QActionShowInfo = new QAction("Info",this);//信息
    connect(m_QActionShowInfo,SIGNAL(triggered()),this,SLOT(slotActionInfo()));
    m_QActionExitApp = new QAction("Exit",this);//退出
    connect(m_QActionExitApp, SIGNAL(triggered()), this, SLOT(slotActionExitApp()));
}

void Widget::uploadInfo()
{
    QString str1=cpuInfo+","+gpuInfo;
    qDebug()<<"SSSSS"<<str1;
    m_tcpSocket.write(str1.toUtf8());
    m_tcpSocket.waitForBytesWritten();

}

//还原状态
void Widget::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
        {
        case QSystemTrayIcon::Trigger:
            //单击托盘图标
            break;
        case QSystemTrayIcon::DoubleClick:
            //双击托盘图标
            //双击后显示主程序窗口
            this->show();
            m_SystemTrayIcon->hide();
            break;
        default:
            break;
        }
}
//主页面
void Widget::slotActionMain()
{
    this->show();
    m_SystemTrayIcon->hide();
}
//退出
void Widget::slotActionExitApp()
{
    exit(0);
}
//信息
void Widget::slotActionInfo()
{
     information->show();
}

//获取硬件信息
void Widget::slotGetInfo()
{
    //获取当前时间
    //   QDateTime DateTime=QDateTime::currentDateTime();
    //   qDebug() <<"当前时间:" << DateTime;
    //获取CPU使用率
    //获取内存使用率
   MEMORYSTATUSEX statex;
   statex.dwLength = sizeof (statex);
   GlobalMemoryStatusEx (&statex);
   useageRam=int(statex.dwMemoryLoad);
   qDebug() <<"内存使用率:" << useageRam;
   //获取GPU使用率
   result = nvmlDeviceGetUtilizationRates(device, &utilization);
   if (result != NVML_SUCCESS) {
               qDebug() << "Failed to get GPU utilization.";
           }
   gpuram=int(utilization.gpu);
   qDebug()<< "GPU使用率:" << gpuram << "%";
}
//CPU初始化 必须初始化 否则温度会出错
void Widget::CPUInit()
{
    cpustate1=InitializeOls();
    cpustate2=IsCpuid();
    if (!cpustate1) {qDebug() << "Failed to initialize WinRing0.";}
    if (!cpustate2) {qDebug() << "CPU does not support CPUID.";}
}

//获取CPU温度
void Widget::slotGetCPUTemp()
{
    DWORD dwStatus;
    DWORD dwCpuTemp1,dwCpuTemp2;
    Rdmsr(0x1A2, &dwCpuTemp1, &dwStatus);
    initTemp=dwCpuTemp1;
    initTemp &=0xff0000;
    initTemp = initTemp >> 16;
    Rdmsr(0x19C, &dwCpuTemp2, &dwStatus);
    finalTemp =dwCpuTemp2;
    finalTemp&=0xff0000; // 提取温度值
    finalTemp = finalTemp >> 16;
    cputemp=(int)(initTemp-finalTemp);
    qDebug() << "CPU Temperature: " << cputemp << "°C";
    // 清理WinRing0
    //DeinitializeOls();
}

//初始化GPU
void Widget::GPUInit()
{

    unsigned int deviceCount;
    nvmlReturn_t result;

    result = nvmlInit();
    if (result != NVML_SUCCESS) {
        qDebug() << "Failed to initialize NVML.";// 处理初始化错误
    }
    result = nvmlDeviceGetCount(&deviceCount);
    if (result != NVML_SUCCESS) {
        qDebug() << "Failed to get device count: " << nvmlErrorString(result) <<endl;
        nvmlShutdown();
    }
    result = nvmlDeviceGetHandleByIndex(0, &device); // Assuming there is only one GPU, index 0
    if (result != NVML_SUCCESS) {
        qDebug() << "Failed to get GPU handle.";
        nvmlShutdown();
    }
    result = nvmlDeviceGetName(device, gpuname, 64);
    if (result != NVML_SUCCESS) {
        qDebug() << "Failed to get device name: " << nvmlErrorString(result) <<endl;
        nvmlShutdown();
        //gpunamee=gpuname;
    }
    qDebug()<< "GPU Name: " << gpuname <<endl;
    //qDebug()<< "GPU Name: " << gpunamee  <<endl;
}

//获取GPU温度
void Widget::slotGetGPUTemp()
{
    //GPUInit();
    result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &gputemp);
    if (result != NVML_SUCCESS) {
        qDebug()<< "Failed to get temperature: " << nvmlErrorString(result) <<endl;
        nvmlShutdown();
    }
    qDebug()<< "GPU Temperature: " << gputemp << "°C" <<endl;
}

void Widget::onReplied(QNetworkReply *reply)
{
    int statusCode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(statusCode!=200 || reply->error()!=QNetworkReply::NoError)
    {
        //QMessageBox::warning(this,"天气","请求数据失败",QMessageBox::Ok);
    }
    else
    {
        QByteArray byteArray=reply->readAll();
        parseJson(byteArray);// 分析Json数据
    }
    reply->deleteLater();//延时释放,防止堆区的接收数据泄漏
}
//上传抓取的数据
void Widget::uploadData()
{
    //数据格式 CPU使用率 CPU温度cputemp GPU使用率gpuram GPU温度gputemp 内存使用率useageRam
    QString str2;
    str2=QString::number(cputemp)+","+QString::number(gpuram)+","+QString::number(gputemp)+","+QString::number(useageRam);
    m_tcpSocket.write(str2.toUtf8());
    m_tcpSocket.waitForBytesWritten();
}
//获取天气
void Widget::getWeatherInfo(QString cityName)
{

}

//解析json
void Widget::parseJson(QByteArray s)
{

}

void Widget::on_startBtn_clicked()
{

}
