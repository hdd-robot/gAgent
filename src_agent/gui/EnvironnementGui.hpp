/*
 * EnvironnementGui.hpp
 *
 * Composant Qt optionnel de visualisation.
 * Compile uniquement si BUILD_GUI=ON.
 */

#ifndef ENVS_ENVIRONNEMENTGUI_H_
#define ENVS_ENVIRONNEMENTGUI_H_

#include <qt5/QtGui/QtGui>
#include <QTimer>
#include <QObject>
#include <qt5/QtWidgets/QPushButton>
#include <qt5/QtCore/QVariant>
#include <qt5/QtWidgets/QAction>
#include <qt5/QtWidgets/QApplication>
#include <qt5/QtWidgets/QButtonGroup>
#include <qt5/QtWidgets/QGraphicsView>
#include <qt5/QtWidgets/QHeaderView>
#include <qt5/QtWidgets/QMainWindow>
#include <qt5/QtWidgets/QMenu>
#include <qt5/QtWidgets/QMenuBar>
#include <qt5/QtWidgets/QStatusBar>
#include <qt5/QtWidgets/QWidget>
#include <qt5/QtWidgets/QGraphicsScene>
#include <qt5/QtWidgets/QGraphicsRectItem>
#include <qt5/QtCore/qtypetraits.h>
#include <qt5/QtGui/QColor>
#include <iostream>

#include "../Environnement.hpp"
#include "gAgentGui.hpp"

namespace gagent {

class Environnement;

class EnvironnementGui : public QObject {
    Q_OBJECT

public:
    EnvironnementGui();
    virtual ~EnvironnementGui();
    int  createWindow(int argc, char* argv[], bool gui, unsigned int timer_val);
    void setColor(int r, int g, int b, int a = 255);

    bool isGui() const          { return gui_; }
    void setGui(bool g = false) { gui_ = g; }
    void setEnvPtr(gagent::Environnement* ptr) { env_ptr_ = ptr; }

public slots:
    void refresh();

private:
    QMainWindow*       widget_  = nullptr;
    QApplication*      app_     = nullptr;
    QGraphicsScene*    scene_   = nullptr;
    Ui_guiWindow*      ui_      = nullptr;
    QTimer*            timer_   = nullptr;
    bool               gui_     = false;
    gagent::Environnement* env_ptr_ = nullptr;
};

} // namespace gagent

#endif /* ENVS_ENVIRONNEMENTGUI_H_ */
