#include <QtGui>
#include "window.h"
#include "widget.h"
#include "model.h"

static const int w = 400;
static const int h = 400;

Widget::Widget(Model *model, QWidget *parent)
	: QWidget(parent), model(model)
{
	showTrace = false;
	elapsed = 0;
	setFixedSize(w, h);
	model->setDim(w, h);
	vecBegin = QPoint(-1, -1);
	vecBrush = QBrush(Qt::green);
}

void Widget::animate()
{
	model->step(refresh_rate);
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

	model->paint(&painter, event);
	if (vecBegin.x() >= 0) {
		painter.setBrush(vecBrush);
		painter.drawLine(vecBegin, vecEnd);
	}

	if (showTrace) {
		int sum = 0;
		int step = refresh_rate;
		int length = trace_length/refresh_rate;

		model->save();
		model->setPaintTraceOnly(true);
		for (int i = 1; i <= length; i++) {
			sum += step;
			model->step(step);
			model->paint(&painter, event);
		}
		model->setPaintTraceOnly(false);
		model->load();
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
	model->add(vecBegin.x(), vecBegin.y(), angle);
	vecBegin = QPoint(-1, -1);
	repaint();
	numberChanged(model->getNumber());
}

QImage Widget::getImage()
{
	QPixmap pixmap(this->size());
	render(&pixmap);
	return pixmap.toImage();
}

void Widget::setNumber(int num)
{
	model->setNumber(num);
	repaint();
}

void Widget::setSide(int val)
{
	model->setSide(val);
	repaint();
}

void Widget::setAtomR(double val)
{
	model->setAtomR((qreal)val);
	repaint();
}

void Widget::setElectronR(double val)
{
	model->setElectronR((qreal)val);
	repaint();
}

void Widget::setSpeed(double val)
{
	model->setSpeed(val);
	repaint();
}

void Widget::setShowBins(bool val)
{
	model->setShowBins(val);
	repaint();
}

void Widget::setBinsNumber(int val)
{
	model->setBinsNumber(val);
	repaint();
}

void Widget::setBinIndex(int val)
{
	model->setBinIndex(val-1);
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
	model->clear();
}
