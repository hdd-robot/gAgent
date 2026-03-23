/*
 * EnvironnementGui.hpp
 *
 * Composant Qt optionnel de visualisation.
 * Compile uniquement si BUILD_GUI=ON.
 */

#ifndef ENVS_ENVIRONNEMENTGUI_H_
#define ENVS_ENVIRONNEMENTGUI_H_

#include <QTimer>
#include <QObject>
#include <QtWidgets/QPushButton>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsRectItem>
#include <QtGui/QColor>
#include <iostream>

#include <gagent/env/Environnement.hpp>
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
