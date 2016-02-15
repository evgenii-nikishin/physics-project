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
    Widget(QWidget *parent);
	QImage getImage();
    virtual ~Widget();

public slots:
	void animate();
	void setNumber(int);
	void setSide(int);
	void setSpeed(double);
	void setAtomR(double);
    void setElectronR(double);
	void setDefaultDirection(double);

    void addModel();
    void removeModel();
    void setEnsembleSize(int);


    void setCurrentModel(int idx);
    Model* getCurrentModel();
    Model* getModel(int idx);

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

public:
    QVector<Model*> models;

private:
    QPainter painter;
	int elapsed;
    int current_model;

	QPoint vecBegin, vecEnd;
	QBrush vecBrush;

	qreal defDir;
	bool randomDefDir;
	bool showTrace;
};

#endif
