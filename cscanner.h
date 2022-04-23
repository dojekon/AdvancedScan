#ifndef CSCANNER_H
#define CSCANNER_H

#include <QList>
#include <QRect>
#include <QString>
#include <QStringList>

class CScanner {
public:
    virtual QString getDevice() = 0;
    virtual QString getVendor() = 0;
    virtual QString getModel() = 0;

    virtual QStringList getSupportedColorModes() = 0;
    virtual QStringList getSupportedResolutions() = 0;
    virtual QStringList getSupportedScanAreas() = 0;
    virtual int getDefaultColorIndex() = 0;
    virtual int getDefaultResolutionIndex() = 0;
    virtual int getDefaultScanAreaIndex() = 0;
    virtual QString getExtension() = 0;
    virtual bool getCropNeeded() = 0;
    virtual QRect getCropRect() = 0;
    virtual void setSelectedColor(int index) = 0;
    virtual void setSelectedResolution(int index) = 0;
    virtual void setSelectedScanArea(int index) = 0;
    virtual QList<QString> getArgs() = 0;

    virtual ~CScanner() {}
protected:
    QString device;
    QString vendor;
    QString model;

    QString color;
    QString format;
    QString extension;
    QString resolution;
    QString pageFormat;
    QString scanArea;
    bool cropNeeded;
    QRect cropRect;

    QStringList colors;
    QStringList dpis;
    QStringList pageFormats;
    QStringList scanAreas;
    QStringList formats;
    QStringList extensions;
    int defaultColorIndex;
    int defaultResolutionIndex;
    int defaultScanAreaIndex;
};

class CDummyScanner: public CScanner {
public:
    CDummyScanner(QString device, QString vendor, QString model);

    QString getDevice();
    QString getVendor();
    QString getModel();

    QStringList getSupportedColorModes();
    QStringList getSupportedResolutions();
    QStringList getSupportedScanAreas();
    int getDefaultColorIndex();
    int getDefaultResolutionIndex();
    int getDefaultScanAreaIndex();
    QString getExtension();
    bool getCropNeeded();
    QRect getCropRect();
    void setSelectedColor(int index);
    void setSelectedResolution(int index);
    void setSelectedScanArea(int index);
    QList<QString> getArgs();
};

class CHPScanner: public CScanner {
public:
    CHPScanner(QString device, QString vendor, QString model);
    QString getDevice();
    QString getVendor();
    QString getModel();

    QStringList getSupportedColorModes();
    QStringList getSupportedResolutions();
    QStringList getSupportedScanAreas();
    int getDefaultColorIndex();
    int getDefaultResolutionIndex();
    int getDefaultScanAreaIndex();
    QString getExtension();
    bool getCropNeeded();
    QRect getCropRect();
    void setSelectedColor(int index);
    void setSelectedResolution(int index);
    void setSelectedScanArea(int index);
    QList<QString> getArgs();
};

class CBrotherScanner: public CScanner {
public:
    CBrotherScanner(QString device, QString vendor, QString model);
    QString getDevice();
    QString getVendor();
    QString getModel();

    QStringList getSupportedColorModes();
    QStringList getSupportedResolutions();
    QStringList getSupportedScanAreas();
    int getDefaultColorIndex();
    int getDefaultResolutionIndex();
    int getDefaultScanAreaIndex();
    QString getExtension();
    bool getCropNeeded();
    QRect getCropRect();
    void setSelectedColor(int index);
    void setSelectedResolution(int index);
    void setSelectedScanArea(int index);
    QList<QString> getArgs();
};

class Factory {
public:
    virtual CScanner* createScanner(QString device, QString vendor, QString model) = 0;
    virtual ~Factory() {}
};

class CDummyScannerFactory : public Factory {
public:
    CScanner* createScanner(QString device, QString vendor, QString model) {
        return new CDummyScanner(device, vendor, model);
    }
};

class CHPScannerFactory : public Factory {
public:
    CScanner* createScanner(QString device, QString vendor, QString model) {
        return new CHPScanner(device, vendor, model);
    }
};

class CBrotherScannerFactory : public Factory {
public:
    CScanner* createScanner(QString device, QString vendor, QString model) {
        return new CBrotherScanner(device, vendor, model);
    }
};

#endif // CSCANNER_H
