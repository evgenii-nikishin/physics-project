#include <QtGui>
#include "window.h"
#include "widget.h"
#include "model.h"
#include <iostream>
using namespace std;

static const int w = 400;
static const int h = 400;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
	showTrace = false;
	elapsed = 0;
	setFixedSize(w, h);

	vecBegin = QPoint(-1, -1);
	vecBrush = QBrush(Qt::green);
}

void Widget::animate()
{
    for (int i = 0; i < models.size(); i++)
        models[i]->step(refresh_rate);
	repaint();
}

void Widget::setTrace(bool set)
{
	showTrace = set;
	repaint();
}

void Widget::paintEvent(QPaintEvent *event)
{
    painter.begin(this);
	painter.setRenderHint(QPainter::Antialiasing);

    models[current_model]->paint(&painter, event);
	if (vecBegin.x() >= 0) {
		painter.setBrush(vecBrush);
		painter.drawLine(vecBegin, vecEnd);
	}

	if (showTrace) {
		int sum = 0;
		int step = refresh_rate;
		int length = trace_length/refresh_rate;

        models[current_model]->save();
        models[current_model]->setPaintTraceOnly(true);
		for (int i = 1; i <= length; i++) {
			sum += step;
            models[current_model]->step(step);
            models[current_model]->paint(&painter, event);
		}
        models[current_model]->setPaintTraceOnly(false);
        models[current_model]->load();
	}

	painter.end();
}

void Widget::mousePressEvent(QMouseEvent *event)
{
	vecBegin = event->pos();
	vecEnd = event->pos();
	repaint();
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
	vecEnd = event->pos();
	repaint();
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
	qreal angle;
	qreal longEnough = 3;
	if ((vecBegin - vecEnd).manhattanLength() >= longEnough)
		// The direction is set by user
		angle = (qreal)atan2(vecEnd.y()-vecBegin.y(), vecEnd.x()-vecBegin.x());
	else
		// The direction is default
		if (randomDefDir)
			angle = (2*M_PI / 360) * (rand() % 360);
		else
			angle = (2*M_PI / 360) * (defDir - 90);
    models[current_model]->add(vecBegin.x(), vecBegin.y(), angle);
	vecBegin = QPoint(-1, -1);
	repaint();
    numberChanged(models[current_model]->getNumber());
}

QImage Widget::getImage()
{
	QPixmap pixmap(this->size());
	render(&pixmap);
	return pixmap.toImage();
}

void Widget::addModel() {
    Model *model;
    if (models.size() > 0)
        model = new Model(*models.back());
    else {
        model = new Model();
        model->setDim(w, h);
    }
    models.push_back(model);
}

void Widget::removeModel() {
    delete models.back();
    models.pop_back();
}

void Widget::setEnsembleSize(int size)
{
    while (size > models.size())
        addModel();
    while (size < models.size())
        removeModel();
}

void Widget::setCurrentModel(int idx)
{
    current_model = idx;
}

Model* Widget::getCurrentModel()
{
    return models[current_model];
}

Model* Widget::getModel(int idx)
{
    return models[idx];
}

void Widget::setNumber(int num)
{
    for (int i = 0; i < models.size(); i++)
        models[i]->setNumber(num);
	repaint();
}

void Widget::setSide(int val)
{
    for (int i = 0; i < models.size(); i++)
        models[i]->setSide(val);
	repaint();
}

void Widget::setAtomR(double val)
{
    for (int i = 0; i < models.size(); i++)
        models[i]->setAtomR((qreal)val);
	repaint();
}

void Widget::setElectronR(double val)
{
    for (int i = 0; i < models.size(); i++)
        models[i]->setElectronR((qreal)val);
	repaint();
}

void Widget::setSpeed(double val)
{
    for (int i = 0; i < models.size(); i++)
        models[i]->setSpeed(val);
	repaint();
}

void Widget::setDefaultDirection(double dir)
{
	defDir = dir;
}

void Widget::setDefaultRandom(bool isRandom)
{
	randomDefDir = isRandom;
}

void Widget::clear()
{
    for (int i = 0; i < models.size(); i++)
        models[i]->clear();
}



Widget::~Widget() {

}
