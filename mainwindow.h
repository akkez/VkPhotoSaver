#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>
#include <QFile>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool chooseScreen = false;
    bool authScreen = true;
    bool downloadScreen = false;

    int offset;
    int currentPhoto;
    QString token;
    QString albumId;
    QString ownerId;
    QString dirname;
    QVector<QString> photoUrls;

    void getAlbums();
    void getPhotoList();
    void goToChooseScreen();
    void goToDownloadScreen();
    void downloadNextPhoto();
    QString getLocalPhotoFilename(QString photoUrl);
    
private slots:
    void on_initAuthButton_clicked();
    void albumListReceivedSlot(QNetworkReply* reply);
    void photoListReceivedSlot(QNetworkReply* reply);
    void on_authButton_clicked();
    void on_radioButton1_clicked();
    void on_radioButton2_clicked();
    void on_chooseAlbumBotton_clicked();
    void photoDownloadFinishedSlot(QNetworkReply* reply);
    void photoDownloadProgressSlot(qint64 received, qint64 total);

    void on_retryButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
