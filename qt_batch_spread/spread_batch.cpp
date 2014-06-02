#include "spread_batch.h"
#include <QMessageBox>
#include <QString>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QIODevice>
#include <QtCore/QTextCodec>

#include <QDateTime>
#include <QRegExp>

MyDlg::MyDlg()
{
	ui.setupUi(this);
    // 生成配置
    QObject::connect(ui.pushButton_writeconfig, SIGNAL(clicked()), this, SLOT(writespreadconfig()));
    // 读取配置
    QObject::connect(ui.pushButton_readconfig, SIGNAL(clicked()), this, SLOT(readspreadconfig()));
    // 更新程序
    QObject::connect(ui.pushButton_update, SIGNAL(clicked()), this, SLOT(updatespread()));
    // 运行程序
    QObject::connect(ui.pushButton_run, SIGNAL(clicked()), this, SLOT(runspread()));
    // 停止程序
    QObject::connect(ui.pushButton_kill, SIGNAL(clicked()), this, SLOT(killspread()));
    // 定时关机
    QObject::connect(ui.pushButton_shutdown, SIGNAL(clicked()), this, SLOT(shutdownspread()));
    // 关闭系统
    QObject::connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(closespread()));
    // 应用参数配置
    // QObject::connect(ui.pushButton_config_apply, SIGNAL(clicked()), this, SLOT(applyspread()));

    QObject::connect(ui.comboBox_onoff_tab1, SIGNAL(currentIndexChanged(int)), this, SLOT(onoff_tab(int)));
    QObject::connect(ui.comboBox_algo_tab1, SIGNAL(currentIndexChanged(int)), this, SLOT(algo_tab(int)));
    QObject::connect(ui.lineEdit_period_tab1, SIGNAL(textChanged(QString)), this, SLOT(period_tab(QString)));
    QObject::connect(ui.lineEdit_capacity_tab1, SIGNAL(textChanged(QString)), this, SLOT(capacity_tab(QString)));
    QObject::connect(ui.lineEdit_maxout_tab1, SIGNAL(textChanged(QString)), this, SLOT(maxout_tab(QString)));
    QObject::connect(ui.lineEdit_maxin_tab1, SIGNAL(textChanged(QString)), this, SLOT(maxin_tab(QString)));
    QObject::connect(ui.lineEdit_thresh_tab1, SIGNAL(textChanged(QString)), this, SLOT(thresh_tab(QString)));
    QObject::connect(ui.lineEdit_threshhigh_tab1, SIGNAL(textChanged(QString)), this, SLOT(threshhigh_tab(QString)));

    QObject::connect(ui.comboBox_onoff_tab2, SIGNAL(currentIndexChanged(int)), this, SLOT(onoff_tab(int)));
    QObject::connect(ui.comboBox_algo_tab2, SIGNAL(currentIndexChanged(int)), this, SLOT(algo_tab(int)));
    QObject::connect(ui.lineEdit_period_tab2, SIGNAL(textChanged(QString)), this, SLOT(period_tab(QString)));
    QObject::connect(ui.lineEdit_capacity_tab2, SIGNAL(textChanged(QString)), this, SLOT(capacity_tab(QString)));
    QObject::connect(ui.lineEdit_maxout_tab2, SIGNAL(textChanged(QString)), this, SLOT(maxout_tab(QString)));
    QObject::connect(ui.lineEdit_maxin_tab2, SIGNAL(textChanged(QString)), this, SLOT(maxin_tab(QString)));
    QObject::connect(ui.lineEdit_thresh_tab2, SIGNAL(textChanged(QString)), this, SLOT(thresh_tab(QString)));
    QObject::connect(ui.lineEdit_threshhigh_tab2, SIGNAL(textChanged(QString)), this, SLOT(threshhigh_tab(QString)));

    QObject::connect(ui.comboBox_onoff_tab3, SIGNAL(currentIndexChanged(int)), this, SLOT(onoff_tab(int)));
    QObject::connect(ui.comboBox_algo_tab3, SIGNAL(currentIndexChanged(int)), this, SLOT(algo_tab(int)));
    QObject::connect(ui.lineEdit_period_tab3, SIGNAL(textChanged(QString)), this, SLOT(period_tab(QString)));
    QObject::connect(ui.lineEdit_capacity_tab3, SIGNAL(textChanged(QString)), this, SLOT(capacity_tab(QString)));
    QObject::connect(ui.lineEdit_maxout_tab3, SIGNAL(textChanged(QString)), this, SLOT(maxout_tab(QString)));
    QObject::connect(ui.lineEdit_maxin_tab3, SIGNAL(textChanged(QString)), this, SLOT(maxin_tab(QString)));
    QObject::connect(ui.lineEdit_thresh_tab3, SIGNAL(textChanged(QString)), this, SLOT(thresh_tab(QString)));
    QObject::connect(ui.lineEdit_threshhigh_tab3, SIGNAL(textChanged(QString)), this, SLOT(threshhigh_tab(QString)));

//    killproc = new QProcess;
//    runproc = new QProcess;
//    updateproc = new QProcess;
//    shutdownproc = new QProcess;

    // 应该检验输入的
    if(ui.lineEdit_config_name->text() != ""){
        m_filename = ui.lineEdit_config_name->text();
    }else{
        m_filename = "spread_machine_config";
        ui.lineEdit_config_name->setText(m_filename);
    }
    readspreadconfig();
}

//void MyDlg::applyspread(){
//    // 应该检验输入的
//    if(ui.lineEdit_config_name->text() != ""){
//        m_filename = ui.lineEdit_config_name->text();
//    }else{
//        m_filename = "spread_machine_config";
//    }
//}

void MyDlg::writespreadconfig()
{
    QFile file(m_filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
        QMessageBox::warning(this, "warning", "can't open "+m_filename, QMessageBox::Yes);
		return;
    }

    QTextStream out(&file);
    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
    QString currenttime = time.toString("yyyy-MM-dd hh:mm:ss ddd"); //设置显示格式
    out << "###########################################################################" << endl;
    out << "##" << endl;
    out << "##  Automatically generate '" << m_filename << "'" << endl;
    out << "##" << endl;
    out << "##  Created by: Qt Creator" << endl;
    out << "##  Created At: " + currenttime << endl;
    out << "##  Author    : MJQ" << endl;
    out << "##" << endl;
    out << "###########################################################################" << endl;
    out << "##on ip\talgo\tperiod\tcapacity\tmaxout\tmaxin\tthresh\tthreshhigh" << endl;
    out << "###########################################################################" << endl;

    int startip = 11;
    int endip = 59;
    ui.textEdit_config->setText("");
    ui.textEdit_config->append("###########################################################################");
    ui.textEdit_config->append("##");
    ui.textEdit_config->append("##  Automatically generate '" + m_filename + "'");
    ui.textEdit_config->append("##");
    ui.textEdit_config->append("##  Created by: Qt Creator");
    ui.textEdit_config->append("##  Created At: " + currenttime);
    ui.textEdit_config->append("##  Author    : MJQ");
    ui.textEdit_config->append("##");
    ui.textEdit_config->append("###########################################################################");
    ui.textEdit_config->append("##on ip\talgo\tperiod\tcapacity\tmaxout\tmaxin\tthresh\tthreshhigh");
    ui.textEdit_config->append("###########################################################################");
    for(int i = startip; i<= endip; ++i){
        QComboBox *comboBox_onoff_ = ui.tabWidget->findChild<QComboBox *>("comboBox_onoff_" + QString::number(i));
        if(!comboBox_onoff_){
            continue;
        }
        if(comboBox_onoff_->currentText() == "off"){
            continue;
        }
        QLabel *label_ip_ = ui.tabWidget->findChild<QLabel *>("label_ip_" + QString::number(i));
        QComboBox *comboBox_algo_ = ui.tabWidget->findChild<QComboBox *>("comboBox_algo_" + QString::number(i));
        QLineEdit *lineEdit_period_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_period_" + QString::number(i));
        QLineEdit *lineEdit_capacity_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_capacity_" + QString::number(i));
        QLineEdit *lineEdit_maxout_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_maxout_" + QString::number(i));
        QLineEdit *lineEdit_maxin_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_maxin_" + QString::number(i));
        QLineEdit *lineEdit_thresh_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_thresh_" + QString::number(i));
        QLineEdit *lineEdit_threshhigh_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_threshhigh_" + QString::number(i));

        QString line = "";
        line += comboBox_onoff_->currentText() + " ";
        line += label_ip_->text() + "\t";
        line += comboBox_algo_->currentText() + "\t";
        line += lineEdit_period_->text() + "\t";
        line += lineEdit_capacity_->text() + "\t";
        line += lineEdit_maxout_->text() + "\t";
        line += lineEdit_maxin_->text() + "\t";
        line += lineEdit_thresh_->text() + "\t";
        line += lineEdit_threshhigh_->text() + "\t";
        out << line << endl;
        out.flush();

        ui.textEdit_config->append(line);
    }

    out.flush();
	file.close();
    ui.tabWidget->setCurrentIndex(3);
}

void MyDlg::readspreadconfig()
{
    QFile file(m_filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::warning(this, "warning", "config: "+ m_filename + " doesn't exist, you can create one.", QMessageBox::Yes);
        return;
    }
    QTextStream in(&file);
    QString line = in.readLine();
    ui.textEdit_config->setText("");
    while(!line.isNull()){
        ui.textEdit_config->append(line);
        if(line[0] == '#' || line[0] != 'o'){
            line = in.readLine();
            continue;
        }
        QStringList list = line.split(QRegExp("[ \t]"));
        for(int i=0;i<list.length(); ++i){
            QString ip = list[1];
            QComboBox *comboBox_onoff_ = ui.tabWidget->findChild<QComboBox *>("comboBox_onoff_" + ip);
            if(!comboBox_onoff_){
                break;
            }
//            QLabel *label_ip_ = ui.tabWidget->findChild<QLabel *>("label_ip_" + ip);
            QComboBox *comboBox_algo_ = ui.tabWidget->findChild<QComboBox *>("comboBox_algo_" + ip);
            QLineEdit *lineEdit_period_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_period_" + ip);
            QLineEdit *lineEdit_capacity_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_capacity_" + ip);
            QLineEdit *lineEdit_maxout_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_maxout_" + ip);
            QLineEdit *lineEdit_maxin_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_maxin_" + ip);
            QLineEdit *lineEdit_thresh_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_thresh_" + ip);
            QLineEdit *lineEdit_threshhigh_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_threshhigh_" + ip);

            comboBox_onoff_->setCurrentIndex(comboBox_onoff_->findText(list[0]));
//            label_ip_->setText(list[1]);
            comboBox_algo_->setCurrentIndex(comboBox_algo_->findText(list[2]));
            lineEdit_period_->setText(list[3]);
            lineEdit_capacity_->setText(list[4]);
            lineEdit_maxout_->setText(list[5]);
            lineEdit_maxin_->setText(list[6]);
            lineEdit_thresh_->setText(list[7]);
            lineEdit_threshhigh_->setText(list[8]);
        }
        line = in.readLine();
    }
}

void MyDlg::updatespread(){
    // 还是需要同步的，异步的无法获取是否执行完毕
    QProcess::execute(ui.lineEdit_update_command->text());
    QMessageBox::information(this, "update", "update successfully...", QMessageBox::Yes);
    // 这里是异步的
    //updateproc->start(ui.lineEdit_update_command->text());
}

// 运行程序按钮事件
void MyDlg::runspread()
{
    // 还是需要同步的，异步的无法获取是否执行完毕
    // 这里识别不了&
    QProcess::execute(ui.lineEdit_run_command->text());
    QMessageBox::information(this, "run", "run successfully...", QMessageBox::Yes);
    // 这里是异步的
    //runproc->start(ui.lineEdit_run_command->text());// 这里所异步的
}

// 定时关机按钮事件
void MyDlg::shutdownspread()
{
    // 还是需要同步的，异步的无法获取是否执行完毕
    // 这里识别不了&
    QProcess::execute(ui.lineEdit_shutdown_command->text());
    QMessageBox::information(this, "shutdown", "shutdown successfully...", QMessageBox::Yes);
    // 这里是异步的
    //shutdownproc->start(ui.lineEdit_shutdown_command->text());// 这里所异步的
}

void MyDlg::killspread()
{
    // 还是需要同步的，异步的无法获取是否执行完毕
    // 这里识别不了&
    QProcess::execute(ui.lineEdit_kill_command->text());
    QMessageBox::information(this, "kill", "kill successfully...", QMessageBox::Yes);
    // 这里是异步的
    //killproc->start(ui.lineEdit_kill_command->text());// 这里所异步的
//    if(killproc->exitCode() != 0){
//        QMessageBox::information(this, "sorry", "sorry, there is an error...", QMessageBox::Yes);
//    }
}

void MyDlg::closespread(){
    this->close();
}

void MyDlg::onoff_tab(int index){
    int startip, endip;
    QString name = sender()->objectName();
    if(name[name.length() - 1] == '1'){
        startip = 11;
        endip = 19;
    }else if(name[name.length() - 1] == '2'){
        startip = 21;
        endip = 29;
    }else if(name[name.length() - 1] == '3'){
        startip = 37;
        endip = 59;
    }else{
        startip = 11;
        endip = 19;
        QMessageBox::information(this, "sorry", "sorry, there is an error...", QMessageBox::Yes);
        this->close();
    }
    for(int i = startip; i<= endip; ++i){
        QComboBox *comboBox_onoff_ = ui.tabWidget->findChild<QComboBox *>("comboBox_onoff_" + QString::number(i));
        if(!comboBox_onoff_){
            continue;
        }
        comboBox_onoff_->setCurrentIndex(index);
    }
}

void MyDlg::algo_tab(int index){
    int startip, endip;
    QString name = sender()->objectName();
    if(name[name.length() - 1] == '1'){
        startip = 11;
        endip = 19;
    }else if(name[name.length() - 1] == '2'){
        startip = 21;
        endip = 29;
    }else if(name[name.length() - 1] == '3'){
        startip = 37;
        endip = 59;
    }else{
        startip = 11;
        endip = 19;
        QMessageBox::information(this, "sorry", "sorry, there is an error...", QMessageBox::Yes);
        this->close();
    }
    for(int i = startip; i<= endip; ++i){
        QComboBox *comboBox_algo_ = ui.tabWidget->findChild<QComboBox *>("comboBox_algo_" + QString::number(i));
        if(!comboBox_algo_){
            continue;
        }
        comboBox_algo_->setCurrentIndex(index);
    }
}

void MyDlg::period_tab(QString value){
    int startip, endip;
    QString name = sender()->objectName();
    if(name[name.length() - 1] == '1'){
        startip = 11;
        endip = 19;
    }else if(name[name.length() - 1] == '2'){
        startip = 21;
        endip = 29;
    }else if(name[name.length() - 1] == '3'){
        startip = 37;
        endip = 59;
    }else{
        startip = 11;
        endip = 19;
        QMessageBox::information(this, "sorry", "sorry, there is an error...", QMessageBox::Yes);
        this->close();
    }
    for(int i = startip; i<= endip; ++i){
        QLineEdit *lineEdit_period_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_period_" + QString::number(i));
        if(!lineEdit_period_){
            continue;
        }
        lineEdit_period_->setText(value);
    }
}

void MyDlg::capacity_tab(QString value){
    int startip, endip;
    QString name = sender()->objectName();
    if(name[name.length() - 1] == '1'){
        startip = 11;
        endip = 19;
    }else if(name[name.length() - 1] == '2'){
        startip = 21;
        endip = 29;
    }else if(name[name.length() - 1] == '3'){
        startip = 37;
        endip = 59;
    }else{
        startip = 11;
        endip = 19;
        QMessageBox::information(this, "sorry", "sorry, there is an error...", QMessageBox::Yes);
        this->close();
    }
    for(int i = startip; i<= endip; ++i){
        QLineEdit *lineEdit_capacity_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_capacity_" + QString::number(i));
        if(!lineEdit_capacity_){
            continue;
        }
        lineEdit_capacity_->setText(value);
    }
}

void MyDlg::maxout_tab(QString value){
    int startip, endip;
    QString name = sender()->objectName();
    if(name[name.length() - 1] == '1'){
        startip = 11;
        endip = 19;
    }else if(name[name.length() - 1] == '2'){
        startip = 21;
        endip = 29;
    }else if(name[name.length() - 1] == '3'){
        startip = 37;
        endip = 59;
    }else{
        startip = 11;
        endip = 19;
        QMessageBox::information(this, "sorry", "sorry, there is an error...", QMessageBox::Yes);
        this->close();
    }
    for(int i = startip; i<= endip; ++i){
        QLineEdit *lineEdit_maxout_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_maxout_" + QString::number(i));
        if(!lineEdit_maxout_){
            continue;
        }
        lineEdit_maxout_->setText(value);
    }
}

void MyDlg::maxin_tab(QString value){
    int startip, endip;
    QString name = sender()->objectName();
    if(name[name.length() - 1] == '1'){
        startip = 11;
        endip = 19;
    }else if(name[name.length() - 1] == '2'){
        startip = 21;
        endip = 29;
    }else if(name[name.length() - 1] == '3'){
        startip = 37;
        endip = 59;
    }else{
        startip = 11;
        endip = 19;
        QMessageBox::information(this, "sorry", "sorry, there is an error...", QMessageBox::Yes);
        this->close();
    }
    for(int i = startip; i<= endip; ++i){
        QLineEdit *lineEdit_maxin_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_maxin_" + QString::number(i));
        if(!lineEdit_maxin_){
            continue;
        }
        lineEdit_maxin_->setText(value);
    }
}

void MyDlg::thresh_tab(QString value){
    int startip, endip;
    QString name = sender()->objectName();
    if(name[name.length() - 1] == '1'){
        startip = 11;
        endip = 19;
    }else if(name[name.length() - 1] == '2'){
        startip = 21;
        endip = 29;
    }else if(name[name.length() - 1] == '3'){
        startip = 37;
        endip = 59;
    }else{
        startip = 11;
        endip = 19;
        QMessageBox::information(this, "sorry", "sorry, there is an error...", QMessageBox::Yes);
        this->close();
    }
    for(int i = startip; i<= endip; ++i){
        QLineEdit *lineEdit_thresh_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_thresh_" + QString::number(i));
        if(!lineEdit_thresh_){
            continue;
        }
        lineEdit_thresh_->setText(value);
    }
}

void MyDlg::threshhigh_tab(QString value){
    int startip, endip;
    QString name = sender()->objectName();
    if(name[name.length() - 1] == '1'){
        startip = 11;
        endip = 19;
    }else if(name[name.length() - 1] == '2'){
        startip = 21;
        endip = 29;
    }else if(name[name.length() - 1] == '3'){
        startip = 37;
        endip = 59;
    }else{
        startip = 11;
        endip = 19;
        QMessageBox::information(this, "sorry", "sorry, there is an error...", QMessageBox::Yes);
        this->close();
    }
    for(int i = startip; i<= endip; ++i){
        QLineEdit *lineEdit_threshhigh_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_threshhigh_" + QString::number(i));
        if(!lineEdit_threshhigh_){
            continue;
        }
        lineEdit_threshhigh_->setText(value);
    }
}
