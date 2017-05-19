#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "client.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Client* client;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_btnconnect_clicked();

    void on_btnjoin_clicked();

    void on_btndisconnect_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
