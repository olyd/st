#include <QDialog>
#include <QProcess>
#include "ui_spread_batch.h"


class MyDlg:public QDialog{
Q_OBJECT
public:
	MyDlg();
public slots:
    // 生成配置
    void writespreadconfig();
    void readspreadconfig();
    void updatespread();
    void runspread();
    void killspread();
    // 定时关机按钮事件
    void shutdownspread();
    void closespread();
    // void applyspread();

    // changed 事件
    void onoff_tab(int);
    void algo_tab(int);
    void period_tab(QString);
    void capacity_tab(QString);
    void maxout_tab(QString);
    void maxin_tab(QString);
    void thresh_tab(QString);
    void threshhigh_tab(QString);
private:
	Ui::mjqDialog ui;
    QString m_filename;
//    QProcess *killproc;
//    QProcess *updateproc;
//    QProcess *runproc;
//    QProcess *shutdownproc;
};
