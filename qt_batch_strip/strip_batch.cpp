#include "strip_batch.h"
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

    // 1.生成配置
    QObject::connect(ui.pushButton_writeconfig, SIGNAL(clicked()), this, SLOT(writestripconfig()));
    // 2.读取配置
    QObject::connect(ui.pushButton_readconfig, SIGNAL(clicked()), this, SLOT(readstripconfig()));
    // 3.更新程序
    QObject::connect(ui.pushButton_update, SIGNAL(clicked()), this, SLOT(updatestrip()));
    // 4.开始运行
    QObject::connect(ui.pushButton_run, SIGNAL(clicked()), this, SLOT(runstrip()));
    // 5.停止运行
    QObject::connect(ui.pushButton_kill, SIGNAL(clicked()), this, SLOT(killstrip()));
    // 6.定时关机功能
    QObject::connect(ui.pushButton_shutdown, SIGNAL(clicked()), this, SLOT(shutdownstrip()));
    // 7.关闭自动化测试系统
    QObject::connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(closestrip()));

    QObject::connect(ui.comboBox_onoff_tab1, SIGNAL(currentIndexChanged(int)), this, SLOT(onoff_tab1(int)));
    QObject::connect(ui.comboBox_p2p_tab1, SIGNAL(currentIndexChanged(int)), this, SLOT(p2p_tab1(int)));
    QObject::connect(ui.comboBox_num_tab1, SIGNAL(currentIndexChanged(int)), this, SLOT(num_tab1(int)));
    QObject::connect(ui.comboBox_salgo_tab1, SIGNAL(currentIndexChanged(int)), this, SLOT(salgo_tab1(int)));
    QObject::connect(ui.lineEdit_sbuffer_tab1, SIGNAL(textChanged(QString)), this, SLOT(sbuffer_tab1(QString)));
    QObject::connect(ui.lineEdit_speriod_tab1, SIGNAL(textChanged(QString)), this, SLOT(speriod_tab1(QString)));
    QObject::connect(ui.comboBox_calgo_tab1, SIGNAL(currentIndexChanged(int)), this, SLOT(calgo_tab1(int)));
    QObject::connect(ui.lineEdit_cbuffer_tab1, SIGNAL(textChanged(QString)), this, SLOT(cbuffer_tab1(QString)));
    QObject::connect(ui.lineEdit_cperiod_tab1, SIGNAL(textChanged(QString)), this, SLOT(cperiod_tab1(QString)));
    QObject::connect(ui.lineEdit_blocksize_tab1, SIGNAL(textChanged(QString)), this, SLOT(blocksize_tab1(QString)));

    QObject::connect(ui.comboBox_onoff_tab2, SIGNAL(currentIndexChanged(int)), this, SLOT(onoff_tab1(int)));
    QObject::connect(ui.comboBox_p2p_tab2, SIGNAL(currentIndexChanged(int)), this, SLOT(p2p_tab1(int)));
    QObject::connect(ui.comboBox_num_tab2, SIGNAL(currentIndexChanged(int)), this, SLOT(num_tab1(int)));
    QObject::connect(ui.comboBox_salgo_tab2, SIGNAL(currentIndexChanged(int)), this, SLOT(salgo_tab1(int)));
    QObject::connect(ui.lineEdit_sbuffer_tab2, SIGNAL(textChanged(QString)), this, SLOT(sbuffer_tab1(QString)));
    QObject::connect(ui.lineEdit_speriod_tab2, SIGNAL(textChanged(QString)), this, SLOT(speriod_tab1(QString)));
    QObject::connect(ui.comboBox_calgo_tab2, SIGNAL(currentIndexChanged(int)), this, SLOT(calgo_tab1(int)));
    QObject::connect(ui.lineEdit_cbuffer_tab2, SIGNAL(textChanged(QString)), this, SLOT(cbuffer_tab1(QString)));
    QObject::connect(ui.lineEdit_cperiod_tab2, SIGNAL(textChanged(QString)), this, SLOT(cperiod_tab1(QString)));
    QObject::connect(ui.lineEdit_blocksize_tab2, SIGNAL(textChanged(QString)), this, SLOT(blocksize_tab1(QString)));

    QObject::connect(ui.comboBox_onoff_tab3, SIGNAL(currentIndexChanged(int)), this, SLOT(onoff_tab1(int)));
    QObject::connect(ui.comboBox_p2p_tab3, SIGNAL(currentIndexChanged(int)), this, SLOT(p2p_tab1(int)));
    QObject::connect(ui.comboBox_num_tab3, SIGNAL(currentIndexChanged(int)), this, SLOT(num_tab1(int)));
    QObject::connect(ui.comboBox_salgo_tab3, SIGNAL(currentIndexChanged(int)), this, SLOT(salgo_tab1(int)));
    QObject::connect(ui.lineEdit_sbuffer_tab3, SIGNAL(textChanged(QString)), this, SLOT(sbuffer_tab1(QString)));
    QObject::connect(ui.lineEdit_speriod_tab3, SIGNAL(textChanged(QString)), this, SLOT(speriod_tab1(QString)));
    QObject::connect(ui.comboBox_calgo_tab3, SIGNAL(currentIndexChanged(int)), this, SLOT(calgo_tab1(int)));
    QObject::connect(ui.lineEdit_cbuffer_tab3, SIGNAL(textChanged(QString)), this, SLOT(cbuffer_tab1(QString)));
    QObject::connect(ui.lineEdit_cperiod_tab3, SIGNAL(textChanged(QString)), this, SLOT(cperiod_tab1(QString)));
    QObject::connect(ui.lineEdit_blocksize_tab3, SIGNAL(textChanged(QString)), this, SLOT(blocksize_tab1(QString)));

//    killproc = new QProcess;
//    runproc = new QProcess;
//    updateproc = new QProcess;

    if(ui.lineEdit_config_name->text() != ""){
        m_filename = ui.lineEdit_config_name->text();
    }else{
        m_filename = "strip_machine_config";
        ui.lineEdit_config_name->setText(m_filename);
    }

    readstripconfig();
}


void MyDlg::writestripconfig()
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
    out << "##on ip\tp2p\tnum\tsalgo\tsbuffer\tcalgo\tcbuffer\tcperiod\tblocksize" << endl;
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
    ui.textEdit_config->append("##on ip\tp2p\tnum\tsalgo\tsbuffer\tcalgo\tcbuffer\tcperiod\tblocksize");
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
        QComboBox *comboBox_p2p_ = ui.tabWidget->findChild<QComboBox *>("comboBox_p2p_" + QString::number(i));
        QComboBox *comboBox_num_ = ui.tabWidget->findChild<QComboBox *>("comboBox_num_" + QString::number(i));
        QComboBox *comboBox_salgo_ = ui.tabWidget->findChild<QComboBox *>("comboBox_salgo_" + QString::number(i));

        QLineEdit *lineEdit_sbuffer_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_sbuffer_" + QString::number(i));
        QComboBox *comboBox_calgo_ = ui.tabWidget->findChild<QComboBox *>("comboBox_calgo_" + QString::number(i));
        QLineEdit *lineEdit_cbuffer_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_cbuffer_" + QString::number(i));
        QLineEdit *lineEdit_cperiod_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_cperiod_" + QString::number(i));
        QLineEdit *lineEdit_blocksize_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_blocksize_" + QString::number(i));

        // on off
        QString line = "";
        line += comboBox_onoff_->currentText() + " ";
        // ip
        line += label_ip_->text() + "\t";
        // p2p: true false
        line += comboBox_p2p_->currentText() + "\t";
        // test num: 0 1 2 3
        line += comboBox_num_->currentText() + "\t";
        // server algo: fifos lrus lfus...
        line += comboBox_salgo_->currentText() + "\t";
        // server buffer size: 1000
        line += lineEdit_sbuffer_->text() + "\t";
        // server algo period:40
        // line += lineEdit_speriod_->text() + "\t";
        // client algo: fifo lru lfu dw...
        line += comboBox_calgo_->currentText() + "\t";
        // client buffer size: 20
        line += lineEdit_cbuffer_->text() + "\t";
        // client algo period: 40
        line += lineEdit_cperiod_->text() + "\t";
        // block size:10
        line += lineEdit_blocksize_->text() + "\t";
        out << line << endl;
        out.flush();

        ui.textEdit_config->append(line);
    }

    out.flush();
	file.close();
    ui.tabWidget->setCurrentIndex(3);
}

void MyDlg::readstripconfig()
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
//          QLabel *label_ip_ = ui.tabWidget->findChild<QLabel *>("label_ip_" + ip);
            QComboBox *comboBox_p2p_ = ui.tabWidget->findChild<QComboBox *>("comboBox_p2p_" + ip);
            QComboBox *comboBox_num_ = ui.tabWidget->findChild<QComboBox *>("comboBox_num_" + ip);
            QComboBox *comboBox_salgo_ = ui.tabWidget->findChild<QComboBox *>("comboBox_salgo_" + ip);
            QLineEdit *lineEdit_sbuffer_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_sbuffer_" + ip);
            QComboBox *comboBox_calgo_ = ui.tabWidget->findChild<QComboBox *>("comboBox_calgo_" + ip);
            QLineEdit *lineEdit_cbuffer_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_cbuffer_" + ip);
            QLineEdit *lineEdit_cperiod_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_cperiod_" + ip);
            QLineEdit *lineEdit_blocksize_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_blocksize_" + ip);

            comboBox_onoff_->setCurrentIndex(comboBox_onoff_->findText(list[0]));
            comboBox_p2p_->setCurrentIndex(comboBox_p2p_->findText(list[2]));
            comboBox_num_->setCurrentIndex(comboBox_num_->findText(list[3]));
            comboBox_salgo_->setCurrentIndex(comboBox_salgo_->findText(list[4]));
            lineEdit_sbuffer_->setText(list[5]);
            comboBox_calgo_->setCurrentIndex(comboBox_calgo_->findText(list[6]));
            lineEdit_cbuffer_->setText(list[7]);
            lineEdit_cperiod_->setText(list[8]);
            lineEdit_blocksize_->setText(list[9]);
        }
        line = in.readLine();
    }
}

void MyDlg::updatestrip(){
    QProcess::execute(ui.lineEdit_update_command->text());// 这里识别不了&
    QMessageBox::information(this, "update", "update successfully...", QMessageBox::Yes);
    // 这里是异步的
    // updateproc->start(ui.lineEdit_update_command->text());
}

void MyDlg::runstrip()
{
    QProcess::execute(ui.lineEdit_run_command->text());// 这里识别不了&
    QMessageBox::information(this, "run", "run successfully...", QMessageBox::Yes);
    // 这里是异步的
    // runproc->start(ui.lineEdit_run_command->text());
}

void MyDlg::killstrip()
{
    QProcess::execute(ui.lineEdit_kill_command->text());// 这里识别不了&
    QMessageBox::information(this, "kill", "kill successfully...", QMessageBox::Yes);
    // 这里是异步的
    // killproc->start(ui.lineEdit_kill_command->text());
}

void MyDlg::shutdownstrip()
{
    QProcess::execute(ui.lineEdit_shutdown_command->text());// 这里识别不了&
    QMessageBox::information(this, "shutdown", "shutdown successfully...", QMessageBox::Yes);
    // 这里是异步的
    // runproc->start(ui.lineEdit_shutdown_command->text());
}

void MyDlg::closestrip(){
    this->close();
}

void MyDlg::onoff_tab1(int index){
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

void MyDlg::p2p_tab1(int index){
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
        QComboBox *comboBox_p2p_ = ui.tabWidget->findChild<QComboBox *>("comboBox_p2p_" + QString::number(i));
        if(!comboBox_p2p_){
            continue;
        }
        comboBox_p2p_->setCurrentIndex(index);
    }
}

void MyDlg::num_tab1(int index){
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
        QComboBox *comboBox_num_ = ui.tabWidget->findChild<QComboBox *>("comboBox_num_" + QString::number(i));
        if(!comboBox_num_){
            continue;
        }
        comboBox_num_->setCurrentIndex(index);
    }
}

void MyDlg::salgo_tab1(int index){
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
        QComboBox *comboBox_salgo_ = ui.tabWidget->findChild<QComboBox *>("comboBox_salgo_" + QString::number(i));
        if(!comboBox_salgo_){
            continue;
        }
        comboBox_salgo_->setCurrentIndex(index);
    }
}

void MyDlg::sbuffer_tab1(QString value){
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
        QLineEdit *lineEdit_sbuffer_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_sbuffer_" + QString::number(i));
        if(!lineEdit_sbuffer_){
            continue;
        }
        lineEdit_sbuffer_->setText(value);
    }
}

void MyDlg::speriod_tab1(QString value){
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
        QLineEdit *lineEdit_speriod_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_speriod_" + QString::number(i));
        if(!lineEdit_speriod_){
            continue;
        }
        lineEdit_speriod_->setText(value);
    }
}

void MyDlg::calgo_tab1(int index){
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
        QComboBox *comboBox_calgo_ = ui.tabWidget->findChild<QComboBox *>("comboBox_calgo_" + QString::number(i));
        if(!comboBox_calgo_){
            continue;
        }
        comboBox_calgo_->setCurrentIndex(index);
    }
}

void MyDlg::cbuffer_tab1(QString value){
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
        QLineEdit *lineEdit_cbuffer_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_cbuffer_" + QString::number(i));
        if(!lineEdit_cbuffer_){
            continue;
        }
        lineEdit_cbuffer_->setText(value);
    }
}

void MyDlg::cperiod_tab1(QString value){
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
        QLineEdit *lineEdit_cperiod_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_cperiod_" + QString::number(i));
        if(!lineEdit_cperiod_){
            continue;
        }
        lineEdit_cperiod_->setText(value);
    }
}

void MyDlg::blocksize_tab1(QString value){
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
        QLineEdit *lineEdit_blocksize_ = ui.tabWidget->findChild<QLineEdit *>("lineEdit_blocksize_" + QString::number(i));
        if(!lineEdit_blocksize_){
            continue;
        }
        lineEdit_blocksize_->setText(value);
    }
}
