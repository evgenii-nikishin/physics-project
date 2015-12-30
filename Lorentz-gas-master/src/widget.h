#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QImage>

class Model;

class Widget : public QWidget
{
	Q_OBJECT

public:
	Widget(Model *model, QWidget *parent);
	QImage getImage();

public slots:
	void animate();
	void setNumber(int);
	void setSide(int);
	void setSpeed(double);
	void setAtomR(double);
	void setElectronR(double);
	void setShowBins(bool);
	void setBinsNumber(int);
	void setBinIndex(int);
	void setDefaultDirection(double);
	void setDefaultRandom(bool);
	void setTrace(bool);
	void clear();

signals:
	void numberChanged(int);

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

private:
	QPainter painter;
	Model *model;
	int elapsed;

	QPoint vecBegin, vecEnd;
	QBrush vecBrush;

	qreal defDir;
	bool randomDefDir;
	bool showTrace;
};

#endif
