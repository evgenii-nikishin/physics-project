#include <QtGui>
#include <iostream>
#include <algorithm>
#include <numeric>
using namespace std;

#include "widget.h"
#include "window.h"
#include "ui_window.h"
bool equillibrium = false;


Window::Window(QWidget *parent)
	: QMainWindow(parent),
	ui(new Ui::Window)
{
	ui->setupUi(this);
//  setStyleSheet("background-color: magenta");
	setWindowTitle(tr("Physics"));

    current_ensemble_element = 0;
    ensemble_size = 0;

    connect(ui->ensemble_current_element_field, SIGNAL(valueChanged(double)), this, SLOT(setCurrentEnsembleElement(double)));
    connect(ui->ensemble_size_field, SIGNAL(valueChanged(double)), this, SLOT(setEnsembleSize(double)));
    ui->ensemble_size_field->setValue(10);

	QIcon appIcon;
	appIcon.addFile(":/resources/app.png", QSize(16,16));
	appIcon.addFile(":/resources/app32.png", QSize(32,32));
	setWindowIcon(appIcon);

	aboutDialog = new AboutDialog(this);
	connect(ui->aboutButton, SIGNAL(clicked()), aboutDialog, SLOT(show()));

	plot = new QCustomPlot(this);
	ui->plotLayout->addWidget(plot);


    natives[current_ensemble_element]->setVisible(true);
	wasRunning = false;

	connect(ui->togglePlayButton, SIGNAL(clicked()), this, SLOT(togglePlay()));
	connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearSettings()));
	connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(saveShot()));
    connect(ui->trailModeCheckBox, SIGNAL(toggled(bool)), this, SLOT(trailMode(bool)));


    plot->xAxis->setRange(0, 1000);
    plot->yAxis->setRange(0, 1);

    connect(natives[current_ensemble_element], SIGNAL(numberChanged(int)), ui->numberBox, SLOT(setValue(int)));

	trailMode(ui->trailModeCheckBox->checkState());
	updateTogglePlayButton();

    setMinimumHeight(700);
	adjustSize();
}

void Window::setNumber(int newNumber) {
    n_electrons = newNumber;
}

void Window::setCurrentEnsembleElement(double new_element) {
    natives[current_ensemble_element]->setVisible(false);
    current_ensemble_element = new_element;
    natives[current_ensemble_element]->setVisible(true);

}

void Window::setEnsembleSize(double new_size) {
    timer = new QTimer(this);
    timer->setInterval(refresh_rate);

    if (new_size > ensemble_size) {
        for (int i = 0; i < new_size - ensemble_size; i++) {
            model.push_back(Model());
            natives.push_back(0);
        }
    }

    for (int i = ensemble_size; i < new_size; i++) {
        natives[i] = new Widget(&model[i], this);
        ui->nativeLayout->addWidget(natives[i], 0, 0);
    }
    for (int i = ensemble_size; i < new_size; i++) {
        connect(timer, SIGNAL(timeout()), natives[i], SLOT(animate()));
        connect(timer, SIGNAL(timeout()), this, SLOT(replot()));
        natives[i]->setVisible(false);
    }

    for (int i = ensemble_size; i < new_size; i++) {
        Widget*& native = natives[i];
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


    }
    ensemble_size = new_size;
    if (current_ensemble_element >= new_size) {
        ui->ensemble_current_element_field->setValue(new_size - 1);
    }
    connect(ui->numberBox, SIGNAL(valueChanged(int)), this, SLOT(setNumber(int)));
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
    static const int n = 10;
    TVec temp = fmap((TVal (*)(TVal))std::fabs, slice(v1, v1.size() - n, v1.size()) - slice(v2, v2.size() - n, v2.size()));
    return std::accumulate(temp.begin(), temp.end(), 0.0, std::plus<TVal>()) / n < treshold;
}

void Window::replot()
{
    double eps = 0.01;

    static double equillibrium_time = -1.0f;

	plot->clearGraphs();
    /*
    for (int i = 0; i < ensemble_size; i++) {
        if (i != current_ensemble_element)
            natives[i]->animate();
    }
    */
    QVector<qreal> x, x_cur;
    QVector<qreal> y, y_avg, y_cur;
    if (ui->plotPressureButton->isChecked())
	{
        x = model[current_ensemble_element].getTime();
        y = model[current_ensemble_element].getImpulses();
		for (int i = 0; i < x.size(); i++)
			y[i] = y[i] / x[i];

        y_avg.resize(y.size());
        for (int i = 0; i < y_avg.size(); i++)
            y_avg[i] = 0;
        for (int i = 0; i < ensemble_size; i++) {
            x_cur = model[i].getTime();
            y_cur = model[i].getImpulses();
            for (int j = 0; j < y_avg.size() && j < y_cur.size() && j < x_cur.size(); j++) {
                y_avg[j] += (y_cur[j] / x_cur[j]) / ensemble_size;
            }
            /*
            cerr << "y_cur" << endl;
            for (int j = 0; j < y_cur.size(); j++)
                cerr << y_cur[j] << ' ';
            cerr << endl;
            cerr << "y_avg" << endl;
            for (int j = 0; j < y_avg.size(); j++)
                cerr << y_avg[j] << ' ';
            cerr << endl;
            */

        }
		plot->yAxis->setLabel("pressure");
    }


	plot->xAxis->setLabel("t");

    plot->addGraph();
    plot->graph(0)->setData(x, y);
    plot->graph(0)->setPen(QPen(QColor(255, 0, 0)));
    plot->addGraph();
    plot->graph(1)->setData(x, y_avg);
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

    if (stop_criteria(y, y_avg, 0.03*n_electrons) && y.size() > 10 && !equillibrium) {
        equillibrium = true;
        equillibrium_time = x[x.size() - 10];
        ui->equilib->setText("Равновесие достигнуто при t=" + QString::number(equillibrium_time) + "c");
        togglePlay();
    }

	qreal xmax = x.last();
	qreal xmin = x.first();
	plot->xAxis->setRange(xmin, xmax);

	qreal ymax = -100500.0;
	qreal ymin = 100500.0;
	for (int i = 0; i < y.size(); i++) {
        if (y_avg[i] > ymax)
            ymax = y_avg[i];
        if (y_avg[i] < ymin)
            ymin = y_avg[i];
	}
	qreal gap = (ymax-ymin)*0.05;

    if (ui->plotPressureButton->isChecked())
		plot->yAxis->setRange(ymin-gap, ymax+gap);
	plot->replot();
}

void Window::saveShot()
{
	QString filename = QFileDialog::getSaveFileName(this, "Save Shot", QDir::currentPath(), "PNG Images (*.png)");
    natives[current_ensemble_element]->getImage().save(filename);
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

void Window::updateBinsNumber(int num)
{

}

void Window::clearSettings()
{
	ui->trailModeCheckBox->setChecked(false);
	ui->numberBox->setValue(0);
	timer->stop();
	updateTogglePlayButton();
    for (int i = 0; i < ensemble_size; i++)
        model[i].clear();
	plot->clearGraphs();
    equillibrium = false;
    ui->equilib->setText("Равновесие не достигнуто");
	plot->xAxis->setRange(0, 1000);
	plot->yAxis->setRange(0, 1);
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
        for (int i = 0; i < ensemble_size; i++)
            natives[i]->setTrace(true);
	}
	else {
        natives[current_ensemble_element]->setTrace(false);
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
