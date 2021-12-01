#include "alarm.h"
#include "fileio.h"
#include <QString>
#include <QDir>


Alarm::Alarm(QObject *parent) :
    QObject(parent)
{
    this->_DefaultPath=QDir::tempPath()+"/QTalarm.ogg";
    this->_isPlaying=false;
    this->_Pause=new QTimer(this);
    this->canResume=true;

    connect(this->_Pause,SIGNAL(timeout()),this,SLOT(Resume()));
}

void Alarm::Start(bool useCustom)
{
    if(useCustom)
    {
        this->UsingCustomPath=true;
    }else{
        FileIO::ExtractAudio();
        this->UsingCustomPath=false;
    }
    this->_isPlaying=true;
}

void Alarm::Stop()
{
    this->_Pause->start(60000);
    this->_isPlaying=false;
}



bool Alarm::isPlaying()
{
    return this->_isPlaying;
}

void Alarm::SetCustomPath(QString CustPath)
{
    this->_CustPath=CustPath;
}

void Alarm::Resume()
{
    this->canResume=true;
    this->_Pause->stop();
}


void Alarm::SetVolume(int Volume)
{
    FileIO::SaveVolume(Volume);
}

