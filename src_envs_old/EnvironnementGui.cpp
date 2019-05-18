/*
 * EnvironnementGui.cpp
 *
 *  Created on: 13 juin 2018
 *      Author: ros
 */

#include "../src_envs_old/EnvironnementGui.h"

namespace gagent {

EnvironnementGui::EnvironnementGui(){
	this->timer = new QTimer();
}

EnvironnementGui::~EnvironnementGui() {

}

int EnvironnementGui::createWindow(int argc, char* argv[]) {
	app = new QApplication(argc, argv);
	this->widget = new QMainWindow;
	this->ui = new Ui_guiWindow();
	this->ui->setupUi(widget);
	this->scene = new QGraphicsScene();

	//this->scene->setSceneRect(0,0,0,0);

	this->ui->graphicsView->setGeometry(QRect(2, 2, env_ptr->map_width+5, env_ptr->map_height+5));



	this->ui->graphicsView->setScene(this->scene);


	QObject::connect(timer, SIGNAL(timeout()),this, SLOT(refresh()));

	timer->start(500);
	widget->show();

	return app->exec();

}

void EnvironnementGui::refresh() {

	env_ptr->make_agent();

	this->scene->clear();

	for (int i = 0; i < env_ptr->list_visual_agents.size(); i++) {
		 gagent::VisualAgent* agent_ptr = env_ptr->list_visual_agents[i];
		 this->scene->addRect(QRect(agent_ptr->pos_x, agent_ptr->pos_y, agent_ptr->size*10, agent_ptr->size*10));

		 std::cout<< "id=" << agent_ptr->id << " x = "<< agent_ptr->pos_x << " y = "<< agent_ptr->pos_x << " y = "<<  " size = "<< agent_ptr->size << std::endl;
	}


	this->scene->addRect(QRect(0,0,env_ptr->map_width,env_ptr->map_height));
	std::cout<< "Tick" << std::endl;
	ui->graphicsView->viewport()->repaint();
}

void EnvironnementGui::setColor(int r, int g, int b, int a) {

}

}
