#ifndef CONFIGUREWINDOW_H
#define CONFIGUREWINDOW_H

#include <QMainWindow>

namespace Ui {
class ConfigureWindow;
}

class ConfigureWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit ConfigureWindow(QWidget *parent = 0);
    ~ConfigureWindow();
    
private:
    Ui::ConfigureWindow *ui;
};

#endif // CONFIGUREWINDOW_H
