/*
 * EnvironnementGui.h
 *
 *  Created on: 13 juin 2018
 *      Author: ros
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

#include "../src_envs_old/Environnement.h"
#include "../src_envs_old/gAgentGui.h"

namespace gagent {

class Environnement;

class EnvironnementGui : public QObject{
	Q_OBJECT

public:
	EnvironnementGui();
	virtual ~EnvironnementGui();
	int createWindow(int argc, char* argv[],unsigned int);
	void setColor(int r, int g, int b, int a = 255);

	bool isGui() const {
		return gui;
	}

	void setGui(bool gui = false) {
		this->gui = gui;
	}

	void setEnvPtr(gagent::Environnement* _env_ptr) {
		env_ptr = _env_ptr;
	}

public slots:
	void refresh();
private:
	QMainWindow *widget;
	QApplication *app;
	QGraphicsScene *scene;
	Ui_guiWindow *ui;
	QTimer *timer;
	bool gui = false;

	gagent::Environnement* env_ptr;
};

} /* namespace gAgent */

#endif /* ENVS_ENVIRONNEMENTGUI_H_ */
