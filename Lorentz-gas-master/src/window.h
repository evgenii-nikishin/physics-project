#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QFileDialog>
#include <QVector>
#include <QIntegerForSize>

#include "model.h"
#include "widget.h"
#include "aboutdialog.h"
#include "qcustomplot.h"

static const int refresh_rate = 50;
static const int trace_length = 3000;

namespace Ui {
	class Window;
}

class Window : public QMainWindow
{
	Q_OBJECT

public:
	explicit Window(QWidget *parent = 0);

protected:
	void keyPressEvent(QKeyEvent *event);

protected slots:
	void replot();
	void saveShot();
	void togglePlay();
	void clearSettings();
	void updateTogglePlayButton();
	void updateBinsNumber(int);
    void setCurrentEnsembleElement(double);
    void setEnsembleSize(double);
	void trailMode(bool active);
    void setNumber(int);

private:
	Ui::Window *ui;

    int n_electrons;
    int ensemble_size;
    int current_ensemble_element;
    QVector<Model> model;

	QTimer *timer;
    QVector<Widget*> natives;
	QCustomPlot* plot;

	AboutDialog *aboutDialog;

	bool wasRunning;
};

#endif
