#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <windows.h>

#include <QMainWindow>

//---------------------------------------------------------------------------//

namespace Ui
{
class MainWindow;
}

//---------------------------------------------------------------------------//

struct VersionInfo;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public:
    void addConsoleText(LPCWSTR text);
    void addListItem(REFCLSID clsid, LPCWSTR  name, LPCWSTR  description, LPCWSTR  copyright, VersionInfo* vi);
    void removeListItem();
    void clearList();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    Ui::MainWindow* ui;
    struct Impl;
    Impl* pimpl;
};

//---------------------------------------------------------------------------//

#endif // MAINWINDOW_H
