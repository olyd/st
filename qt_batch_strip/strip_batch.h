#include <QDialog>
#include <QProcess>
#include "ui_strip_batch.h"


class MyDlg:public QDialog{
Q_OBJECT
public:
	MyDlg();
public slots:
    void writestripconfig();    // 1.生成配置
    void readstripconfig();     // 2.读取配置
    void updatestrip();         // 3.更新程序
    void runstrip();            // 4.开始运行
    void killstrip();           // 5.停止运行
    void shutdownstrip();       // 6.定时关机功能
    void closestrip();          // 7.关闭自动化测试系统

    // changed 事件
    void onoff_tab1(int);
    void p2p_tab1(int);
    void num_tab1(int);
    void salgo_tab1(int);
    void sbuffer_tab1(QString);
    void speriod_tab1(QString);
    void calgo_tab1(int);
    void cbuffer_tab1(QString);
    void cperiod_tab1(QString);
    void blocksize_tab1(QString);
private:
	Ui::mjqDialog ui;
    QString m_filename;
//    QProcess *killproc;
//    QProcess *updateproc;
//    QProcess *runproc;

};
