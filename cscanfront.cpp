#include "cscanfront.h"


#include <QProcess>
#include <cscanner.h>
#include <qstring.h>
#include <QImage>



CScanFront::CScanFront() {

}

std::vector<CScanner*> CScanFront::getDevices() {
    std::vector<CScanner*> devices;
    QProcess proc;

    // Объекты фабрик
    CBrotherScannerFactory* brotherFactory = new CBrotherScannerFactory();
    CDummyScannerFactory* dummyFactory = new CDummyScannerFactory();
    CHPScannerFactory* hpFactory = new CHPScannerFactory();

    // Запускаем поиск сканеров
    proc.start("scanimage", {"-f", "%d=>%v=>%m%n"});

    // Если поиск затянулся
    if (!proc.waitForFinished()) {
        delete brotherFactory;
        delete dummyFactory;
        delete hpFactory;
        return devices;
    }

    // Читаем инфу из стандартного вывода команды
    QString data = QString::fromUtf8(proc.readAllStandardOutput().constData());

    //
    if (data == "") {
        delete brotherFactory;
        delete dummyFactory;
        delete hpFactory;
        return devices;
    }

    QStringList lines;
    lines = data.split("\n");

  for (const QString &line : lines) {
        if (line == "") continue;
        QStringList elements = line.split("=>");
        if (elements[1] == "Hewlett-Packard") {
            devices.push_back(hpFactory->createScanner(elements[0],elements[1],elements[2]));
        } else if (elements[1] == "Brother") {
            devices.push_back(brotherFactory->createScanner(elements[0],elements[1],elements[2]));
        } else {
            devices.push_back(dummyFactory->createScanner(elements[0],elements[1],elements[2]));
        }
    }

      delete brotherFactory;
      delete dummyFactory;
      delete hpFactory;

    return devices;
}

QImage CScanFront::scanImage(CScanner* scanner) {
    QProcess proc;
    QImage image;

    QList args = scanner->getArgs();

    proc.start("scanimage", args);
    if ((!proc.waitForFinished(-1))||(proc.exitCode())) {
        QString error = QString::fromUtf8(proc.readAllStandardError().constData());
        throw std::runtime_error("Error:" + error.toStdString());
    }
    QByteArray data = proc.readAllStandardOutput();
    image = QImage::fromData(data);

    if (scanner->getCropNeeded()) {
        image = image.copy(scanner->getCropRect());
    }

    data.clear();
    args.clear();

    return image;
}
