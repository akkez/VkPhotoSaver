#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>

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
    QString token;
    void getAlbums();
    
private slots:
    void on_initAuthButton_clicked();
    void finishedGetAlbumsSlot(QNetworkReply* reply);
    void on_authButton_clicked();
    void on_radioButton1_clicked();
    void on_radioButton2_clicked();
    void on_chooseAlbumBotton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
