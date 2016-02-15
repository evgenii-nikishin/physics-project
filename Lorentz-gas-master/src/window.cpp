#include <QtGui>
#include <iostream>
#include <algorithm>
#include <numeric>
using namespace std;

#include "widget.h"
#include "window.h"
#include "ui_window.h"

bool equillibrium = false;
const int equillibrium_detection_len = 20;

Window::Window(QWidget *parent)
	: QMainWindow(parent),
	ui(new Ui::Window)
{
    ui->setupUi(this);
	setWindowTitle(tr("Physics"));

    timer = new QTimer(this);
    timer->setInterval(refresh_rate);

    native = new Widget(this);

    connect(ui->ensemble_current_element_field, SIGNAL(valueChanged(double)), this, SLOT(setCurrentEnsembleElement(double)));
    connect(ui->ensemble_size_field, SIGNAL(valueChanged(double)), this, SLOT(setEnsembleSize(double)));
    ui->ensemble_size_field->setValue(10);
    ui->numberBox->setValue(0);

    connect(timer, SIGNAL(timeout()), native, SLOT(animate()));
    connect(timer, SIGNAL(timeout()), this, SLOT(replot()));

    connect(ui->numberBox, SIGNAL(valueChanged(int)), this, SLOT(setNumber(int)));
    connect(native, SIGNAL(numberChanged(int)), ui->numberBox, SLOT(setValue(int)));

    connect(ui->numberBox, SIGNAL(valueChanged(int)), native, SLOT(setNumber(int)));
    connect(ui->sideBox, SIGNAL(valueChanged(int)), native, SLOT(setSide(int)));
    connect(ui->atomRadBox, SIGNAL(valueChanged(double)), native, SLOT(setAtomR(double)));
    connect(ui->electronRadBox, SIGNAL(valueChanged(double)), native, SLOT(setElectronR(double)));
    connect(ui->speedBox, SIGNAL(valueChanged(double)), native, SLOT(setSpeed(double)));
    connect(ui->defDirBox, SIGNAL(valueChanged(double)), native, SLOT(setDefaultDirection(double)));
    connect(ui->randomDefDirBox, SIGNAL(toggled(bool)), native, SLOT(setDefaultRandom(bool)));

    native->setNumber(ui->numberBox->value());
    native->setSide(ui->sideBox->value());
    native->setAtomR(ui->atomRadBox->value());
    native->setElectronR(ui->electronRadBox->value());
    native->setSpeed(ui->speedBox->value());
    native->setDefaultDirection(ui->defDirBox->value());
    native->setDefaultRandom(ui->randomDefDirBox->checkState());

    ui->nativeLayout->addWidget(native, 0, 0);

	QIcon appIcon;
	appIcon.addFile(":/resources/app.png", QSize(16,16));
	appIcon.addFile(":/resources/app32.png", QSize(32,32));
	setWindowIcon(appIcon);

	aboutDialog = new AboutDialog(this);
	connect(ui->aboutButton, SIGNAL(clicked()), aboutDialog, SLOT(show()));

	plot = new QCustomPlot(this);
	ui->plotLayout->addWidget(plot);

	wasRunning = false;

	connect(ui->togglePlayButton, SIGNAL(clicked()), this, SLOT(togglePlay()));
	connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearSettings()));
	connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(saveShot()));
    connect(ui->trailModeCheckBox, SIGNAL(toggled(bool)), this, SLOT(trailMode(bool)));


    plot->xAxis->setRange(0, 1000);
    plot->yAxis->setRange(0, 1);

	trailMode(ui->trailModeCheckBox->checkState());
	updateTogglePlayButton();

    setMinimumHeight(700);
	adjustSize();
}

void Window::setNumber(int newNumber) {
    n_electrons = newNumber;
}

void Window::setCurrentEnsembleElement(double new_element) {
    native->setCurrentModel(new_element);
}

void Window::setEnsembleSize(double new_size) {
    int saved_n_electrons = n_electrons;
    clearSettings();
    ui->numberBox->setValue(saved_n_electrons);


    ui->ensemble_current_element_field->setMaximum(new_size - 1);
    ui->togglePlayButton->setText(tr("Play"));

    if (ui->ensemble_current_element_field->value() >= new_size) {
        ui->ensemble_current_element_field->setValue(new_size - 1);
    }
    native->setEnsembleSize(int(new_size));
    if (plot != NULL) {
        plot->clearGraphs();
        plot->replot();
    }
}

template <typename TVec>
TVec operator-(const TVec& v1, const TVec& v2) {
    TVec result;
    for (int i = 0; i < v1.size() && v2.size(); i++)
        result.push_back(v1[i] - v2[i]);
    return result;
}

template <typename TVec>
TVec slice(const TVec& vec, int begin, int end) {
    TVec result;
    copy(vec.begin() + max(0, begin), vec.begin() + end, back_inserter(result));
    return result;
}

template <typename TVec>
TVec averaged(const TVec& vec) {
    static const int n = 1000000;
    TVec result;
    for (int i = 0; i < vec.size(); i++) {
        double temp = 0.f;
        for (int j = -i; j <= 0; j++) {
            temp += vec[max(0, i + j)];
        }
        temp /= (i + 1);
        result.push_back(temp);
    }
    return result;
}

template <typename TVec, typename F>
TVec fmap(F f, const TVec& vec) {
    TVec result;
    for (int i = 0; i < vec.size(); i++) {
        result.push_back(f(vec[i]));
    }
    return result;
}

template <typename TVec, typename F>
typename TVec::value_type ffold(F f, const TVec& vec) {
    typename TVec::value_type result = vec[0];
    for (int i = 1; i < vec.size(); i++) {
        result = f(result, vec[i]);
    }
    return result;
}

template <typename TVec>
bool stop_criteria(const TVec& v1, const TVec& v2, double treshold) {
    typedef double TVal;
    static const int n = equillibrium_detection_len;
    TVec temp = fmap((TVal (*)(TVal))std::fabs, slice(v1, v1.size() - n, v1.size()) - slice(v2, v2.size() - n, v2.size()));
    return std::accumulate(temp.begin(), temp.end(), 0.0, std::plus<TVal>()) / n < treshold;
}

void Window::replot()
{
    static double equillibrium_time = -1.0f;

	plot->clearGraphs();
    QVector<qreal> x, x_cur;
    QVector<qreal> y, y_avg, y_cur;

    x = native->getCurrentModel()->getTime();
    y = native->getCurrentModel()->getImpulses();

    for (int i = 0; i < x.size(); i++) {
        y[i] = y[i] / x[i];
    }

    y_avg.resize(y.size());
    for (int i = 0; i < y_avg.size(); i++)
        y_avg[i] = 0;

    for (int i = 0; i < native->models.size(); i++) {
        x_cur = native->getModel(i)->getTime();
        y_cur = native->getModel(i)->getImpulses();
        for (int j = 0; j < y_avg.size() && j < y_cur.size() && j < x_cur.size(); j++) {
            y_avg[j] += (y_cur[j] / x_cur[j]) / native->models.size();
        }

    }
    plot->yAxis->setLabel("pressure");


    plot->xAxis->setLabel("t");

    plot->addGraph();
    plot->graph(0)->setData(x, y);
    plot->graph(0)->setPen(QPen(QColor(255, 0, 0)));

    plot->addGraph();
    plot->graph(1)->setData(x, y_avg);
    plot->graph(1)->setPen(QPen(QColor(0, 0, 255)));

    plot->addGraph();
    plot->graph(2)->setData(x, averaged(y));
    plot->graph(2)->setPen(QPen(QColor(0, 255, 0)));

    if (equillibrium) {
        plot->dumpObjectInfo();
        plot->addGraph();
        plot->graph(3)->setPen(QPen(QColor(0, 0, 0)));
        double min_y = ffold(std::min<double>, y);
        double max_y = ffold(std::max<double>, y_avg);
        QVector<double> vline_x, vline_y;
        vline_x.push_back(equillibrium_time);
        vline_x.push_back(equillibrium_time);
        vline_y.push_back(min_y);
        vline_y.push_back(max_y);
        plot->graph(3)->setData(vline_x, vline_y);
    }



    if (stop_criteria(y, y_avg, 0.03*n_electrons) && y.size() > equillibrium_detection_len && !equillibrium) {
        equillibrium = true;
        equillibrium_time = x[x.size() - equillibrium_detection_len];
        ui->equilib->setText(QString::fromWCharArray(L"Равновесие достигнуто при t=") + QString::number(equillibrium_time) + "c");
        togglePlay();
    }


	qreal xmax = x.last();
	qreal xmin = x.first();
	plot->xAxis->setRange(xmin, xmax);

	qreal ymax = -100500.0;
	qreal ymin = 100500.0;
	for (int i = 0; i < y.size(); i++) {
        if (y[i] > ymax)
            ymax = y[i];
        if (y[i] < ymin)
            ymin = y[i];
	}
	qreal gap = (ymax-ymin)*0.05;

    plot->yAxis->setRange(ymin-gap, ymax+gap);
	plot->replot();
}

void Window::saveShot()
{
	QString filename = QFileDialog::getSaveFileName(this, "Save Shot", QDir::currentPath(), "PNG Images (*.png)");
    native->getImage().save(filename);
}

void Window::togglePlay()
{
	if (timer->isActive())
		timer->stop();
	else
		timer->start();
	updateTogglePlayButton();
}

void Window::updateTogglePlayButton()
{
	if (timer->isActive())
		ui->togglePlayButton->setText(tr("Pause"));
	else
		ui->togglePlayButton->setText(tr("Play"));
}

void Window::clearSettings()
{
	ui->trailModeCheckBox->setChecked(false);
	ui->numberBox->setValue(0);
	timer->stop();
    updateTogglePlayButton();
    native->clear();
    if (plot != NULL) {
        plot->clearGraphs();
        plot->xAxis->setRange(0, 1000);
        plot->yAxis->setRange(0, 1);
        plot->replot();
    }
    equillibrium = false;
    ui->equilib->setText(QString::fromWCharArray(L"Равновесие не достигнуто"));

}

void Window::trailMode(bool active)
{
	if (active) {
		if (timer->isActive())
			wasRunning = true;
		else
			wasRunning = false;
		timer->stop();
        ui->togglePlayButton->setEnabled(false);
        native->setTrace(true);
	}
	else {
        native->setTrace(false);
		if (wasRunning)
			timer->start();
		ui->togglePlayButton->setEnabled(true);
	}
}

void Window::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Escape)
		close();
	else
		QWidget::keyPressEvent(e);
}
