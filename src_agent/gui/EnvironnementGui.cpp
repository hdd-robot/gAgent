/*
 * EnvironnementGui.cpp
 */

#include "EnvironnementGui.hpp"

namespace gagent {

EnvironnementGui::EnvironnementGui()
{
    timer_ = new QTimer();
}

EnvironnementGui::~EnvironnementGui()
{
}

int EnvironnementGui::createWindow(int argc, char* argv[], bool gui, unsigned int timer_val)
{
    std::thread(&Environnement::readDataFromQueueMsg, env_ptr_).detach();

    app_    = new QApplication(argc, argv);
    widget_ = new QMainWindow;
    ui_     = new Ui_guiWindow();
    ui_->setupUi(widget_);
    scene_  = new QGraphicsScene();

    if (gui) {
        ui_->graphicsView->setGeometry(
            QRect(2, 2, env_ptr_->map_width + 5, env_ptr_->map_height + 5));
        ui_->graphicsView->setScene(scene_);
        widget_->show();
    }

    QObject::connect(timer_, SIGNAL(timeout()), this, SLOT(refresh()));
    timer_->start(timer_val);

    return app_->exec();
}

void EnvironnementGui::refresh()
{
    env_ptr_->event_loop();
    env_ptr_->make_agent();

    scene_->clear();

    for (size_t i = 0; i < env_ptr_->list_visual_agents.size(); i++) {
        gagent::VisualAgent* ag = env_ptr_->list_visual_agents[i];

        QColor color = Qt::black;
        QBrush brush = Qt::SolidPattern;

        if      (ag->color == "color0")      { color = Qt::color0; }
        else if (ag->color == "color1")      { color = Qt::color1; }
        else if (ag->color == "black")       { color = Qt::black; }
        else if (ag->color == "white")       { color = Qt::white; }
        else if (ag->color == "darkGray")    { color = Qt::darkGray; }
        else if (ag->color == "gray")        { color = Qt::gray; }
        else if (ag->color == "red")         { color = Qt::red; }
        else if (ag->color == "green")       { color = Qt::green; }
        else if (ag->color == "blue")        { color = Qt::blue; }
        else if (ag->color == "cyan")        { color = Qt::cyan; }
        else if (ag->color == "magenta")     { color = Qt::magenta; }
        else if (ag->color == "yellow")      { color = Qt::yellow; }
        else if (ag->color == "darkRed")     { color = Qt::darkRed; }
        else if (ag->color == "darkGreen")   { color = Qt::darkGreen; }
        else if (ag->color == "darkBlue")    { color = Qt::darkBlue; }
        else if (ag->color == "darkCyan")    { color = Qt::darkCyan; }
        else if (ag->color == "darkMagenta") { color = Qt::darkMagenta; }
        else if (ag->color == "darkYellow")  { color = Qt::darkYellow; }
        else if (ag->color == "transparent") { color = Qt::transparent; }

        if      (ag->pattern == "solid" )  { brush = Qt::SolidPattern; }
        else if (ag->pattern == "no"    )  { brush = Qt::NoBrush; }
        else if (ag->pattern == "dense1")  { brush = Qt::Dense1Pattern; }
        else if (ag->pattern == "dense2")  { brush = Qt::Dense2Pattern; }
        else if (ag->pattern == "dense3")  { brush = Qt::Dense3Pattern; }
        else if (ag->pattern == "dense4")  { brush = Qt::Dense4Pattern; }
        else if (ag->pattern == "dense5")  { brush = Qt::Dense5Pattern; }
        else if (ag->pattern == "dense6")  { brush = Qt::Dense6Pattern; }
        else if (ag->pattern == "dense7")  { brush = Qt::Dense7Pattern; }
        else if (ag->pattern == "horiz" )  { brush = Qt::HorPattern; }
        else if (ag->pattern == "verti" )  { brush = Qt::VerPattern; }
        else if (ag->pattern == "cross" )  { brush = Qt::CrossPattern; }
        else if (ag->pattern == "diag1" )  { brush = Qt::BDiagPattern; }
        else if (ag->pattern == "diag2" )  { brush = Qt::FDiagPattern; }
        else if (ag->pattern == "diag3" )  { brush = Qt::DiagCrossPattern; }
        else if (ag->pattern == "grad1" )  { brush = Qt::LinearGradientPattern; }
        else if (ag->pattern == "grad2" )  { brush = Qt::RadialGradientPattern; }
        else if (ag->pattern == "grad3" )  { brush = Qt::ConicalGradientPattern; }

        brush.setColor(color);

        if (ag->shape == "circle") {
            scene_->addEllipse(ag->pos_x, ag->pos_y, ag->size_x, ag->size_y, QPen(), brush);
        } else if (ag->shape == "rectangular") {
            QRect qrect(ag->pos_x, ag->pos_y, ag->size_x, ag->size_y);
            scene_->addRect(qrect, color, brush);
        }

        scene_->addRect(QRect(0, 0, env_ptr_->map_width, env_ptr_->map_height));
        ui_->graphicsView->viewport()->repaint();
    }
}

} // namespace gagent
