#ifndef CSCANFRONT_H
#define CSCANFRONT_H

#include <QImage>
#include <QStringList>
#include <cscanner.h>

class CScanFront {
public:
    CScanFront();
    static std::vector<CScanner*> getDevices();
    static QImage scanImage(CScanner* scanner);
};

#endif // CSCANFRONT_H
