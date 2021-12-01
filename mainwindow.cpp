#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "timer.h"
#include "alarm.h"
#include "fileio.h"
#include "schedulecollection.h"

#include "snooze.h"

#include <QMessageBox>
#include <QCheckBox>
#include <QLabel>
#include <QCloseEvent>
#include <QTimeEdit>
#include <QTimer>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QSlider>
#include <QSystemTrayIcon>
#include <QListWidgetItem>
#include <QCalendarWidget>
#include <QToolTip>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    this->_supportsTray = QSystemTrayIcon::isSystemTrayAvailable();


    //Create / load Schedule
    _Schedules=new ScheduleCollection(this);
    _Schedules->LoadSchedules();
    PopulateListWidget();
    DisablePanelIfNoSelection();
    if(ui->listWidget->currentRow()==0)


    _isMilTime=FileIO::isMilTime();
    _WarnOnPm=FileIO::LoadWarnOnPm();
    _prevTimeWasMil=_isMilTime;
    displayTimeMode();

    //Call Time keeper
    TimeKeeper=new Timer(this,_Schedules);
    CurAlarm = &Alarm::GetInstance();
    TimeKeeper->StartTimer(CurAlarm);

    //Set Volume
    int Volume = FileIO::LoadVolume();
    ui->VolumeSlider->setValue(Volume<=0? 50:Volume);
    CurAlarm->SetVolume(ui->VolumeSlider->value());
    ui->listAlmBtn->button(QDialogButtonBox::Ok)->setText("&Add");
    ui->listAlmBtn->button(QDialogButtonBox::Cancel)->setText("&Remove");

    trayIcon=new QSystemTrayIcon(this);
    trayIconMenu=new QMenu(this);
    QAction *QAshow=new QAction("&Show",this);
    QAction *QAquit=new QAction("&Quit",this);

    trayIconMenu->addAction(QAshow);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(QAquit);
    trayIcon->setContextMenu(trayIconMenu);
    ChangeIconToDefault();
    trayIcon->show();

    ui->txtSoundPath->setText("");
    ui->CustEdit->setDate(QDate::currentDate());
    SetupClock();




    //set up slots
    connect(QAquit,SIGNAL(triggered()),this,SLOT(Quit()));
    connect(QAshow,SIGNAL(triggered()),this,SLOT(ToggleWindow()));
    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(ToggleWindow(QSystemTrayIcon::ActivationReason)));
    connect(ui->actionQuit,SIGNAL(triggered()),this,SLOT(Quit()));
    connect(ui->timeEdit,SIGNAL(editingFinished()),this,SLOT(SetTime()));
    connect(ui->listAlmBtn,SIGNAL(clicked(QAbstractButton*)),this,SLOT(AddRemoveAlarm(QAbstractButton*)));
    connect(ui->chkMon,SIGNAL(clicked(bool)),this,SLOT(ToggleMon(bool)));
    connect(ui->chkTues,SIGNAL(clicked(bool)),this,SLOT(ToggleTue(bool)));
    connect(ui->chkWed,SIGNAL(clicked(bool)),this,SLOT(ToggleWed(bool)));
    connect(ui->chkThurs,SIGNAL(clicked(bool)),this,SLOT(ToggleThur(bool)));
    connect(ui->chkFri,SIGNAL(clicked(bool)),this,SLOT(ToggleFri(bool)));
    connect(ui->chkSat,SIGNAL(clicked(bool)),this,SLOT(ToggleSat(bool)));
    connect(ui->chkSun,SIGNAL(clicked(bool)),this,SLOT(ToggleSun(bool)));
    connect(ui->chkCustom,SIGNAL(clicked(bool)),this,SLOT(ToggleCust(bool)));
    connect(ui->chkSounds,SIGNAL(clicked(bool)),this,SLOT(OpenDiaglog(bool)));
    connect(ui->TestBtn,SIGNAL(clicked()),this,SLOT(TestAlarm()));
    connect(ui->VolumeSlider,SIGNAL(valueChanged(int)),CurAlarm,SLOT(SetVolume(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    event->ignore();
    if(this->_supportsTray)
    {
        this->hide();
    }else{
        Quit();
    }
}

void MainWindow::SetupClock()
{
    //Set up clock display
    QTimer *CurrentTime=new QTimer(this);
    connect(CurrentTime,SIGNAL(timeout()),this,SLOT(timeCheck()));
    CurrentTime->start(500);
}

void MainWindow::ToggleWindow(QSystemTrayIcon::ActivationReason Reason)
{
    if(Reason==QSystemTrayIcon::DoubleClick || Reason==QSystemTrayIcon::Trigger)
    {
        ToggleWindow();
    }
}

void MainWindow::ToggleWindow()
{
    if(this->CurAlarm->isPlaying() && this->CurAlarm->isBastard==false)
    {
        this->CurAlarm->Stop();
    }
    ui->TestBtn->setText("Test");
    if(this->isHidden())
    {
        this->show();
    }else{
        this->hide();
    }
}

void MainWindow::SetTime()
{


    if(ui->listWidget->currentIndex().column()!=-1)
    {
        if(ui->timeEdit->time().hour()>12 && !_isMilTime && _WarnOnPm)
        {
            PMWarning();
        }
        Schedule *Active=this->_Schedules->GetSchedule(ui->listWidget->currentRow());
        Active->SetTime(ui->timeEdit->time());
        UpdateListWidget();
        this->_Schedules->Save();
    }
}

void MainWindow::SetCustomDate()
{
    if(ui->listWidget->currentIndex().column()!=-1)
    {
        Schedule *Active=this->_Schedules->GetSchedule(ui->listWidget->currentRow());
        Active->SetTime(ui->timeEdit->time());
       QDate CustomDate=ui->CustEdit->date();
        Active->SetCust(CustomDate);
        if(Active->isCustomEnabled())
            UpdateListWidget();
        this->_Schedules->Save();
    }
}



void MainWindow::ToggleMon(bool isEnabled)
{
    Schedule *Active=this->_Schedules->GetSchedule(ui->listWidget->currentRow());
    Active->setIsMonEnabled(isEnabled);
    UpdateListWidget();
}

void MainWindow::ToggleTue(bool isEnabled)
{
    Schedule *Active=this->_Schedules->GetSchedule(ui->listWidget->currentRow());
    Active->setIsTueEnabled(isEnabled);
    UpdateListWidget();
}
void MainWindow::ToggleWed(bool isEnabled)
{
    Schedule *Active=this->_Schedules->GetSchedule(ui->listWidget->currentRow());
    Active->setIsWedEnabled(isEnabled);
    UpdateListWidget();
}
void MainWindow::ToggleThur(bool isEnabled)
{
    Schedule *Active=this->_Schedules->GetSchedule(ui->listWidget->currentRow());
    Active->setIsThurEnabled(isEnabled);
    UpdateListWidget();
}
void MainWindow::ToggleFri(bool isEnabled)
{
    Schedule *Active=this->_Schedules->GetSchedule(ui->listWidget->currentRow());
    Active->setIsFriEnabled(isEnabled);
    UpdateListWidget();
}
void MainWindow::ToggleSat(bool isEnabled)
{
    Schedule *Active=this->_Schedules->GetSchedule(ui->listWidget->currentRow());
    Active->setIsSatEnabled(isEnabled);
    UpdateListWidget();
}
void MainWindow::ToggleSun(bool isEnabled)
{
    Schedule *Active=this->_Schedules->GetSchedule(ui->listWidget->currentRow());
    Active->setIsSunEnabled(isEnabled);
    UpdateListWidget();
}


void MainWindow::ToggleCust(bool isEnabled)
{
    Schedule *Active=this->_Schedules->GetSchedule(ui->listWidget->currentRow());
    Active->SetCustEnabled(isEnabled);
    UpdateListWidget();
}

void MainWindow::Quit()
{

        this->_Schedules->Save();
        FileIO::DelExtracted();
        FileIO::SaveVolume(ui->VolumeSlider->value());
        qApp->quit();

}

void MainWindow::AddRemoveAlarm(QAbstractButton *button)
{
    if(button->text()=="&Add")
    {
        Schedule *scheToAdd=new Schedule(this);
        this->_Schedules->AddSchedule(scheToAdd);
        ui->listWidget->addItem(scheToAdd->Name());
        ui->listWidget->setCurrentRow(ui->listWidget->count()-1);
    }
    else if(button->text()=="&Remove")
    {
        this->_lastDeletedIndex=ui->listWidget->currentRow();
        this->_Schedules->removeScheduleByIndex(ui->listWidget->currentRow());
        PopulateListWidget();
    }
    this->_Schedules->Save();
}


void MainWindow::timeCheck()
{
    UpdateClock();
    SnoozeMenuCheck();
    if(_isMilTime!=_prevTimeWasMil)
    {
        _prevTimeWasMil=_isMilTime;
        displayTimeMode();
    }
}

void MainWindow::UpdateClock()
{
    if(_isMilTime)
    {
        ui->Clock->setText(QTime::currentTime().toString("H:mm:ss"));
    }else{
        ui->Clock->setText(QTime::currentTime().toString("h:mm:ss ap"));
    }
}


void MainWindow::OpenDiaglog(bool isChecked)
{
    Schedule *Active=this->_Schedules->GetSchedule(ui->listWidget->currentRow());
    Active->SetCustomSoundEnabled(isChecked);
    if(isChecked)
    {
        QString AlarmLocation = QFileDialog::getOpenFileName(this,"Select Alarm File",QDir::homePath());
        Active->SetCustomSound(AlarmLocation);
        ui->txtSoundPath->setText(AlarmLocation);
    }else{
        ui->txtSoundPath->setText("");
    }
    this->_Schedules->Save();
}

void MainWindow::TestAlarm()
{
    if(ui->chkSounds->isChecked())
    {
        this->CurAlarm->SetCustomPath(ui->txtSoundPath->text());
        this->CurAlarm->Start(true);
    }else{
        this->CurAlarm->Start(false);
    }
    this->testrun=true;
}



void MainWindow::SnoozeMenuCheck()
{
    if((this->testrun==true && this->CurAlarm->isBastard==false) || (this->CurAlarm->isPlaying() && this->CurAlarm->canResume && this->CurAlarm->isBastard==false))
    {
        //Create Snooze Menu object
        snooze *snMenu=new snooze(this,CurAlarm);
        snMenu->show();
        if(this->_supportsTray && this->testrun==false)
        {
            this->hide();
        }
        this->CurAlarm->canResume=false;
        this->testrun=false;
    }
}



void MainWindow::PMWarning()
{
    QMessageBox::warning(this,"Time Verification","Your selected time is set for PM. If you intended to wake in the morning, now is your chance to fix it");
}




void MainWindow::displayTimeMode()
{
    if(_isMilTime)
    {
        ui->timeEdit->setDisplayFormat("H:mm:ss");
        ui->CustEdit->setDisplayFormat("d MMM yyyy");
    }else{
        ui->timeEdit->setDisplayFormat("h:mm:ss ap");
        ui->CustEdit->setDisplayFormat("d MMM yyyy");

    }
}

void MainWindow::PopulateListWidget()
{
    ui->listWidget->clear();
    Schedule *sche;
    foreach(sche,this->_Schedules->GetScheduleList())
    {
        ui->listWidget->addItem(sche->Name());
    }
    ui->listWidget->setCurrentRow(this->_lastDeletedIndex);
}

void MainWindow::DisablePanelIfNoSelection()
{
    if(ui->listWidget->currentRow()==-1)
    {
        ui->chkCustom->setEnabled(false);
        ui->chkFri->setEnabled(false);
        ui->chkMon->setEnabled(false);
        ui->chkSat->setEnabled(false);
        ui->chkSounds->setEnabled(false);
        ui->chkSun->setEnabled(false);
        ui->chkThurs->setEnabled(false);
        ui->chkTues->setEnabled(false);
        ui->chkWed->setEnabled(false);
        ui->CustEdit->setEnabled(false);
        ui->timeEdit->setEnabled(false);
        //ui->chkBastard->setEnabled(false);

        ui->chkCustom->setChecked(false);
        ui->chkFri->setChecked(false);
        ui->chkMon->setChecked(false);
        ui->chkSat->setChecked(false);
        ui->chkSounds->setChecked(false);
        ui->chkSun->setChecked(false);
        ui->chkThurs->setChecked(false);
        ui->chkTues->setChecked(false);
        ui->chkWed->setChecked(false);
        ui->txtSoundPath->setText("");

    }
    else
    {
        ui->chkCustom->setEnabled(true);
        ui->chkFri->setEnabled(true);
        ui->chkMon->setEnabled(true);
        ui->chkSat->setEnabled(true);
        ui->chkSounds->setEnabled(true);
        ui->chkSun->setEnabled(true);
        ui->chkThurs->setEnabled(true);
        ui->chkTues->setEnabled(true);
        ui->chkWed->setEnabled(true);
        ui->CustEdit->setEnabled(true);
        ui->timeEdit->setEnabled(true);
    }
}

void MainWindow::UpdateListWidget()
{
    int index=ui->listWidget->currentRow();
    PopulateListWidget();
    ui->listWidget->setCurrentRow(index);
    this->_Schedules->Save();
}

void MainWindow::SendTrayMessage(QString title, QString message)
{
    if(!FileIO::LoadSeenSolveText())
    {
        QIcon icon;
        if(FileIO::LoadisMono())
        {
            icon = QIcon(":/new/icons/mono.png");
        }else{
            icon = QIcon(":/new/icons/Clock.png");
        }
        trayIcon->showMessage(title,message,icon);
        FileIO::SaveSeenSolveText();
    }
}

void MainWindow::ChangeIconToDefault()
{
    trayIcon->setToolTip("QTalarm");
    if(FileIO::LoadisMono())
    {
        this->trayIcon->setIcon(QIcon(":/new/icons/mono.png"));
    }else{
        this->trayIcon->setIcon(QIcon(":/new/icons/Clock.png"));
    }
}


