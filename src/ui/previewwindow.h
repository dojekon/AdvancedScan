#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QImage>
#include <QRect>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include "../managers/profilemanager.h"

class PreviewImageWidget : public QLabel
{
    Q_OBJECT

public:
    explicit PreviewImageWidget(QWidget* parent = nullptr);
    
    void setImage(const QImage& image);
    QRect getSelectedArea() const { return selectedArea; }
    void setSelectedArea(const QRect& area);
    
signals:
    void areaSelected(const QRect& area);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QImage originalImage;
    QRect selectedArea;
    QPoint selectionStart;
    bool isSelecting;
    QRect scaledImageRect;
    
    QPoint mapToImage(const QPoint& widgetPos) const;
    QRect mapFromImage(const QRect& imageRect) const;
};

class PreviewWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewWindow(const ProfileManager::ScanProfile& profile, 
                          ScannerManager* scannerManager = nullptr, 
                          QWidget* parent = nullptr);
    ~PreviewWindow();

    QRect getSelectedArea() const;

signals:
    void areaSelected(const QRect& area);

private slots:
    void onScanClicked();
    void onAreaSelected(const QRect& area);
    void onApplyClicked();
    void onCancelClicked();

private:
    void setupUI();
    void updateButtons();
    
    ProfileManager::ScanProfile profile;
    ScannerManager* scannerManager;
    PreviewImageWidget* imageWidget;
    QPushButton* scanButton;
    QPushButton* applyButton;
    QPushButton* cancelButton;
    QLabel* statusLabel;
    QScrollArea* scrollArea;
    
    bool hasPreview;
    QRect selectedArea;
};

#endif // PREVIEWWINDOW_H
