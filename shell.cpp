#include "shell.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <QProcess>
#include <QDebug>

Shell::Shell() {

}

QString Shell::Execute(const QString cmd) {
    QProcess proc;
    proc.start("" + cmd);
    proc.waitForFinished(-1);
    QString strout = proc.readAllStandardOutput();
    QString stderrr = proc.readAllStandardError();

            /*
    if (!proc->waitForStarted())
        qWarning() << "Cannot start process";

    int waitTime = 6000;
    if (!proc->waitForFinished(waitTime))
        qWarning() << "Timeout...";*/

    return strout;
}
