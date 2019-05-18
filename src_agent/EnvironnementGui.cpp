/*
 * EnvironnementGui.cpp
 *
 *  Created on: 13 juin 2018
 *      Author: ros
 */

#include "EnvironnementGui.hpp"

namespace gagent {

EnvironnementGui::EnvironnementGui(){
	this->timer = new QTimer();
}

EnvironnementGui::~EnvironnementGui() {

}

int EnvironnementGui::createWindow(int argc, char* argv[],bool gui,unsigned int timer_val) {

	std::thread (&Environnement::readDataFromQueueMsg, this->env_ptr).detach();

	app = new QApplication(argc, argv);
	this->widget = new QMainWindow;
	this->ui = new Ui_guiWindow();
	this->ui->setupUi(widget);
	this->scene = new QGraphicsScene();

	if(gui){
		this->ui->graphicsView->setGeometry(QRect(2, 2, env_ptr->map_width+5, env_ptr->map_height+5));
		this->ui->graphicsView->setScene(this->scene);
		widget->show();
	}


	QObject::connect(timer, SIGNAL(timeout()),this, SLOT(refresh()));
	timer->start(timer_val);

	return app->exec();

}

void EnvironnementGui::refresh() {


	//std::cout << "EnvironnementGui::refresh ==== " << std::endl;

	env_ptr->event_loop();
	env_ptr->make_agent();

	this->scene->clear();

	for (int i = 0; i < env_ptr->list_visual_agents.size(); i++) {
		gagent::VisualAgent* agent_ptr = env_ptr->list_visual_agents[i];

		QColor color = Qt::black;
		QBrush brush = Qt::SolidPattern;

		// set color
		     if(agent_ptr->color == "color0")	{color = Qt::color0;}
		else if(agent_ptr->color == "color1")	{color = Qt::color1;}
		else if(agent_ptr->color == "black")	{color = Qt::black;}
		else if(agent_ptr->color == "white")	{color = Qt::white;}
		else if(agent_ptr->color == "darkGray")	{color = Qt::darkGray;}
		else if(agent_ptr->color == "gray")		{color = Qt::gray;}
		else if(agent_ptr->color == "red")		{color = Qt::red;}
		else if(agent_ptr->color == "green")	{color = Qt::green;}
		else if(agent_ptr->color == "blue")		{color = Qt::blue;}
		else if(agent_ptr->color == "cyan")		{color = Qt::cyan;}
		else if(agent_ptr->color == "magenta")	{color = Qt::magenta;}
		else if(agent_ptr->color == "yellow")	{color = Qt::yellow;}
		else if(agent_ptr->color == "darkRed")	{color = Qt::darkRed;}
		else if(agent_ptr->color == "darkGreen"){color = Qt::darkGreen;}
		else if(agent_ptr->color == "darkBlue")	{color = Qt::darkBlue;}
		else if(agent_ptr->color == "darkCyan")	{color = Qt::darkCyan;}
		else if(agent_ptr->color == "darkMagenta"){color = Qt::darkMagenta;}
		else if(agent_ptr->color == "darkYellow"){color = Qt::darkYellow;}
		else if(agent_ptr->color == "transparent"){color = Qt::transparent;}

		//set pattern
		     if(agent_ptr->pattern == "solid" ) { brush = Qt::SolidPattern;    }
		else if(agent_ptr->pattern == "no"    ) { brush = Qt::NoBrush;   	   }
		else if(agent_ptr->pattern == "dense1") { brush = Qt::Dense1Pattern;   }
		else if(agent_ptr->pattern == "dense2") { brush = Qt::Dense2Pattern;   }
		else if(agent_ptr->pattern == "dense3") { brush = Qt::Dense3Pattern;   }
		else if(agent_ptr->pattern == "dense4") { brush = Qt::Dense4Pattern;   }
		else if(agent_ptr->pattern == "dense5") { brush = Qt::Dense5Pattern;   }
		else if(agent_ptr->pattern == "dense6") { brush = Qt::Dense6Pattern;   }
		else if(agent_ptr->pattern == "dense7") { brush = Qt::Dense7Pattern;   }
		else if(agent_ptr->pattern == "horiz" ) { brush = Qt::HorPattern;      }
		else if(agent_ptr->pattern == "verti" ) { brush = Qt::VerPattern;      }
		else if(agent_ptr->pattern == "cross" ) { brush = Qt::CrossPattern;    }
		else if(agent_ptr->pattern == "diag1" ) { brush = Qt::BDiagPattern;    }
		else if(agent_ptr->pattern == "diag2" ) { brush = Qt::FDiagPattern;    }
		else if(agent_ptr->pattern == "diag3" ) { brush = Qt::DiagCrossPattern;}
		else if(agent_ptr->pattern == "grad1" ) { brush = Qt::LinearGradientPattern;}
		else if(agent_ptr->pattern == "grad2" ) { brush = Qt::RadialGradientPattern;}
		else if(agent_ptr->pattern == "grad3" ) { brush = Qt::ConicalGradientPattern;}

		brush.setColor(color);

		// draw shape

//		std::cout << "HDD == pos_x:" << agent_ptr->pos_x << " - pos_y : "
//				<< agent_ptr->pos_y << " - size_x : " << agent_ptr->size_x
//				<< " - size_y : " << agent_ptr->size_y << " - sahpe: " << agent_ptr->shape
//				<< " - color: " << agent_ptr->color
//				<< std::endl;

		if (agent_ptr->shape == "circle") {
			this->scene->addEllipse(agent_ptr->pos_x, agent_ptr->pos_y, agent_ptr->size_x, agent_ptr->size_y,QPen(),brush);
		}

		else if (agent_ptr->shape == "rectangular") {
			QRect qrect = QRect(agent_ptr->pos_x, agent_ptr->pos_y,agent_ptr->size_x, agent_ptr->size_y);
			this->scene->addRect(qrect, color, brush);

		}

		else if (agent_ptr->shape == "triangle") {

		}

		else if (agent_ptr->shape == "polygon") {

		}

		this->scene->addRect( QRect(0, 0, env_ptr->map_width, env_ptr->map_height));
		ui->graphicsView->viewport()->repaint();

	}

}



} // end namespace
