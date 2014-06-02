#include <QDialog>
#include <QProcess>
#include "ui_maxload_batch.h"


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
    void start_together_tab1(int);
    void blocksize_tab1(QString);
    void filenum_tab1(QString);
    void clientnum_tab1(QString);
    void cbuffer_tab1(QString);
    void sbuffer_tab1(QString);
private:
	Ui::mjqDialog ui;
    QString m_filename;
//    QProcess *killproc;
//    QProcess *updateproc;
//    QProcess *runproc;

};
