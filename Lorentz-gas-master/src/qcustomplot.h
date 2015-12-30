/***************************************************************************
**                                                                        **
**  QCustomPlot, a simple to use, modern plotting widget for Qt           **
**  Copyright (C) 2011 Emanuel Eichhammer                                 **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: http://www.WorksLikeClockwork.com/                   **
**             Date: 19.11.11                                             **
****************************************************************************/

/*! \file */

#ifndef QCUSTOMPLOT_H
#define QCUSTOMPLOT_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QPixmap>
#include <QVector>
#include <QString>
#include <QPrinter>
#include <QDateTime>
#include <QMultiMap>
#include <QFlags>
#include <cmath>

class QCustomPlot;
class QCustomPlotRange;
class QCustomPlotAxis;
class QCustomPlotData;

/*! \typedef QCustomPlotDataMap
  Container for storing QCustomPlotData items in a sorted fashion. The key of the map
  is the key of the QCustomPlotData instance.
  \see QCustomPlotData, QCustomPlotGraph::setData
*/
typedef QMap<double,QCustomPlotData> QCustomPlotDataMap;

class QCustomPlotDataFetcher
{
public:
  //explicit QCustomPlotDataFetcher(QCustomPlot *parentplot);
  virtual void fetch(int inId, const QCustomPlotRange &inRange, int &outN, double *outKey, double *outValue) = 0;

};

class QCustomPlotData
{
public:
  QCustomPlotData();
  double key, value;
  double keyErrorPlus, keyErrorMinus;
  double valueErrorPlus, valueErrorMinus;
};

class QCustomPlotGraph
{
public:
  /*!
	Defines how the graph's line is represented visually in the plot. The line is drawn with the
	current pen of the graph (\ref setPen).
	\see setLineStyle
  */
  enum LineStyle {lsNone,       ///< data points are not connected with any lines (e.g. data only represented
								///< with symbols according to the scatter style, see \ref setScatterStyle)
				  lsLine,       ///< data points are connected by a straight line
				  lsStepLeft,   ///< line is drawn as steps where the step height is the value of the left data point
				  lsStepRight,  ///< line is drawn as steps where the step height is the value of the right data point
				  lsStepCenter, ///< line is drawn as steps where the step is in between two data points
				  lsImpulse     ///< data points are represented by a straight line parallel to the value axis, which ranges down/up to the key axis
				 };

  /*!
	This defines the visual appearance of the points, which are all drawn with the pen of the graph
	(\ref setPen). The sizes of these visualizations (with exception of \ref ssDot and \ref ssPixmap) can be
	set with \ref setScatterSize.
	\see setScatterStyle
  */
  enum ScatterStyle {ssNone,     ///< no scatter symbols are drawn (e.g. data only represented with lines, see \ref setLineStyle)
					 ssDot,      ///< a single pixel, \ref setScatterSize has no influence on its size.
					 ssCross,    ///< a cross (x)
					 ssPlus,     ///< a plus (+)
					 ssCircle,   ///< a circle which is not filled
					 ssDisc,     ///< a circle which is filled with the color of the graph's pen (not the brush!)
					 ssSquare,   ///< a square which is not filled
					 ssStar,     ///< a star with eight arms, i.e. a combination of cross and plus
					 ssTriangle, ///< an equilateral triangle which is not filled, standing on baseline
					 ssTriangleInverted, ///< an equilateral triangle which is not filled, standing on corner
					 ssCrossSquare,      ///< a square which is not filled, with a cross inside
					 ssPlusSquare,       ///< a square which is not filled, with a plus inside
					 ssCrossCircle,      ///< a circle which is not filled, with a cross inside
					 ssPlusCircle,       ///< a circle which is not filled, with a plus inside
					 ssPeace,    ///< a circle which is not filled, with one vertical and two downward diagonal lines
					 ssPixmap    ///< a custom pixmap specified by setScatterPixmap, centered on the data point coordinates. \ref setScatterSize has no influence on its size.
					};

  /*!
	Defines what kind of error bars are drawn for each data point
  */
  enum ErrorType {etNone,  ///< No error bars are shown
				  etKey,   ///< Error bars for the key dimension of the data point are shown
				  etValue, ///< Error bars for the value dimension of the data point are shown
				  etBoth   ///< Error bars for both key and value dimensions of the data point are shown
				 };

  explicit QCustomPlotGraph(QCustomPlotAxis *keyaxis, QCustomPlotAxis *valueaxis);
  ~QCustomPlotGraph();

  // getters:
  QString name() const { return mName; }
  QPen pen() const { return mPen; }
  QBrush brush() const { return mBrush; }
  bool visible() const { return mVisible; }
  QCustomPlotAxis *keyAxis() const { return mKeyAxis; }
  QCustomPlotAxis *valueAxis() const { return mValueAxis; }
  const QCustomPlotDataMap *data() const { return mData; }
  LineStyle lineStyle() const { return mLineStyle; }
  ScatterStyle scatterStyle() const { return mScatterStyle; }
  double scatterSize() const { return mScatterSize; }
  const QPixmap scatterPixmap() const { return mScatterPixmap; }
  ErrorType errorType() const { return mErrorType; }
  QPen errorPen() const { return mErrorPen; }
  double errorBarSize() const { return mErrorBarSize; }
  bool errorBarSkipSymbol() const { return mErrorBarSkipSymbol; }
  QCustomPlotGraph *channelFillGraph() const { return mChannelFillGraph; }

  // setters:
  void setName(const QString &str);
  void setPen(const QPen &p);
  void setBrush(const QBrush &b);
  void setVisible(bool on);
  void setKeyAxis(QCustomPlotAxis *keyaxis);
  void setValueAxis(QCustomPlotAxis *valueaxis);
  void setData(QCustomPlotDataMap *data, bool copy=false);
  void setData(const QVector<double> &key, const QVector<double> &value);
  void setDataKeyError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &keyError);
  void setDataKeyError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &keyErrorMinus, const QVector<double> &keyErrorPlus);
  void setDataValueError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &valueError);
  void setDataValueError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &valueErrorMinus, const QVector<double> &valueErrorPlus);
  void setDataBothError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &keyError, const QVector<double> &valueError);
  void setDataBothError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &keyErrorMinus, const QVector<double> &keyErrorPlus, const QVector<double> &valueErrorMinus, const QVector<double> &valueErrorPlus);
  void setLineStyle(LineStyle ls);
  void setScatterStyle(ScatterStyle ss);
  void setScatterSize(double size);
  void setScatterPixmap(const QPixmap &pixmap);
  void setErrorType(ErrorType errorType);
  void setErrorPen(const QPen &pen);
  void setErrorBarSize(double size);
  void setErrorBarSkipSymbol(bool enabled);
  void setChannelFillGraph(QCustomPlotGraph *targetGraph);

  // non-property methods:
  void addData(const QCustomPlotDataMap &dataMap);
  void addData(const QCustomPlotData &data);
  void addData(double key, double value);
  void addData(const QVector<double> &keys, const QVector<double> &values);
  void removeDataBefore(double key);
  void removeDataAfter(double key);
  void removeData(double fromKey, double toKey);
  void removeData(double key);
  void clearData();
  void rescaleAxes(bool onlyEnlarge=false, bool includeErrorBars=true);
  void rescaleKeyAxis(bool onlyEnlarge=false, bool includeErrorBars=true);
  void rescaleValueAxis(bool onlyEnlarge=false, bool includeErrorBars=true);

protected:
  QCustomPlot *mParentPlot;
  QCustomPlotAxis *mKeyAxis, *mValueAxis;
  QString mName;
  bool mVisible;
  QMap<double, QCustomPlotData> *mData;
  QPen mPen, mErrorPen;
  QBrush mBrush;
  LineStyle mLineStyle;
  ScatterStyle mScatterStyle;
  double mScatterSize;
  QPixmap mScatterPixmap;
  ErrorType mErrorType;
  double mErrorBarSize;
  bool mErrorBarSkipSymbol;
  QCustomPlotGraph *mChannelFillGraph;

  // main draw function, called by QCustomPlot::drawGraphs:
  void draw(QPainter *painter) const;

  // functions to generate plot data points in pixel coordinates:
  void getPlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const;
  // plot style specific functions to generate plot data, used by getPlotData:
  void getScatterPlotData(QVector<QCustomPlotData> *pointData) const;
  void getLinePlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const;
  void getStepLeftPlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const;
  void getStepRightPlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const;
  void getStepCenterPlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const;
  void getImpulsePlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const;

  // helper functions for drawing:
  void drawFill(QPainter *painter, QVector<QPointF> *lineData) const;
  void drawScatterPlot(QPainter *painter, QVector<QCustomPlotData> *pointData) const;
  void drawLinePlot(QPainter *painter, QVector<QPointF> *lineData) const;
  void drawImpulsePlot(QPainter *painter, QVector<QPointF> *lineData) const;
  void drawScatter(QPainter *painter, double x, double y, ScatterStyle style) const;
  void drawError(QPainter *painter, double x, double y, const QCustomPlotData &data) const;

  // draw function for representing this graph in QCustomPlotLegend:
  void drawLegendIcon(QPainter *painter, const QRect &rect) const;

  // helper functions:
  void getVisibleDataBounds(QCustomPlotDataMap::const_iterator &lower, QCustomPlotDataMap::const_iterator &upper, int &count) const;
  void addFillBasePoints(QVector<QPointF> *lineData) const;
  void removeFillBasePoints(QVector<QPointF> *lineData) const;
  QPointF lowerFillBasePoint(double lowerKey) const;
  QPointF upperFillBasePoint(double upperKey) const;
  const QPolygonF getChannelFillPolygon(const QVector<QPointF> *lineData) const;
  int findIndexBelowX(const QVector<QPointF> *data, double x) const;
  int findIndexAboveX(const QVector<QPointF> *data, double x) const;
  int findIndexBelowY(const QVector<QPointF> *data, double y) const;
  int findIndexAboveY(const QVector<QPointF> *data, double y) const;
  const QCustomPlotRange getKeyRange(bool &validRange, bool includeErrors=false, int restrictToSign=0) const;
  const QCustomPlotRange getValueRange(bool &validRange, bool includeErrors=false, int restrictToSign=0) const;

  friend class QCustomPlot;
  friend class QCustomPlotLegend;
};

class QCustomPlotRange
{
public:
  double lower, upper;
  QCustomPlotRange();
  QCustomPlotRange(double lower, double upper);
  double size() const;
  double center() const;
  void normalize();
  QCustomPlotRange sanitizedForLogScale() const;
  QCustomPlotRange sanitizedForLinScale() const;

  static bool validRange(double lower, double upper);
  static bool validRange(const QCustomPlotRange &range);
  static const double minRange; //1e-280;
  static const double maxRange; //1e280;
};

class QCustomPlotLegend : public QObject
{
  Q_OBJECT
public:
  /*!
	Defines where the legend is positioned inside the QCustomPlot axis rect.
  */
  enum PositionStyle {psManual,      ///< Position is not changed automatically. Set manually via \ref setPosition
					  psTopLeft,     ///< Legend is positioned in the top left corner of the axis rect with distance to the border corresponding to the currently set top and left margins
					  psTop,         ///< Legend is horizontally centered at the top of the axis rect with distance to the border corresponding to the currently set top margin
					  psTopRight,    ///< Legend is positioned in the top right corner of the axis rect with distance to the border corresponding to the currently set top and right margins
					  psRight,       ///< Legend is vertically centered at the right of the axis rect with distance to the border corresponding to the currently set right margin
					  psBottomRight, ///< Legend is positioned in the bottom right corner of the axis rect with distance to the border corresponding to the currently set bottom and right margins
					  psBottom,      ///< Legend is horizontally centered at the bottom of the axis rect with distance to the border corresponding to the currently set bottom margin
					  psBottomLeft,  ///< Legend is positioned in the bottom left corner of the axis rect with distance to the border corresponding to the currently set bottom and left margins
					  psLeft         ///< Legend is vertically centered at the left of the axis rect with distance to the border corresponding to the currently set left margin
					 };
  //Q_ENUMS(PositionStyle)

  explicit QCustomPlotLegend(QCustomPlot *parentPlot);
  ~QCustomPlotLegend();

  // getters:
  QPen borderPen() const { return mBorderPen; }
  QBrush brush() const { return mBrush; }
  QFont font() const { return mFont; }
  PositionStyle positionStyle() const { return mPositionStyle; }
  QPoint position() const { return mPosition; }
  bool autoSize() const { return mAutoSize; }
  QSize size() const { return mSize; }
  bool visible() const { return mVisible; }
  int paddingLeft() const { return mPaddingLeft; }
  int paddingRight() const { return mPaddingRight; }
  int paddingTop() const { return mPaddingTop; }
  int paddingBottom() const { return mPaddingBottom; }
  int marginLeft() const { return mMarginLeft; }
  int marginRight() const { return mMarginRight; }
  int marginTop() const { return mMarginTop; }
  int marginBottom() const { return mMarginBottom; }
  int itemSpacing() const { return mItemSpacing; }
  QSize iconSize() const { return mIconSize; }
  int iconTextPadding() const { return mIconTextPadding; }
  QPen iconBorderPen() const { return mIconBorderPen; }

  // setters:
  void setBorderPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setFont(const QFont &font);
  void setPositionStyle(PositionStyle legendPositionStyle);
  void setPosition(const QPoint &pixelPosition);
  void setAutoSize(bool on);
  void setSize(const QSize &size);
  void setSize(int width, int height);
  void setVisible(bool on);
  void setPaddingLeft(int padding);
  void setPaddingRight(int padding);
  void setPaddingTop(int padding);
  void setPaddingBottom(int padding);
  void setPadding(int left, int right, int top, int bottom);
  void setMarginLeft(int margin);
  void setMarginRight(int margin);
  void setMarginTop(int margin);
  void setMarginBottom(int margin);
  void setMargin(int left, int right, int top, int bottom);
  void setItemSpacing(int spacing);
  void setIconSize(const QSize &size);
  void setIconSize(int width, int height);
  void setIconTextPadding(int padding);
  void setIconBorderPen(const QPen &pen);

  // non-property methods:
  QCustomPlotGraph *graph(int i) const;
  int graphCount() const;
  bool hasGraph(QCustomPlotGraph *graph) const;
  void addGraph(QCustomPlotGraph *graph);
  void setGraphs(const QList<QCustomPlotGraph*> &graphList);
  void addGraphs(const QList<QCustomPlotGraph*> &graphList);
  bool removeGraph(QCustomPlotGraph *graph);
  void removeGraphs(const QList<QCustomPlotGraph*> &graphList);

  void reArrange();

protected:
  // simple properties with getters and setters:
  QPen mBorderPen, mIconBorderPen;
  QBrush mBrush;
  QFont mFont;
  QPoint mPosition;
  QSize mSize, mIconSize;
  PositionStyle mPositionStyle;
  bool mAutoSize, mVisible;
  int mPaddingLeft, mPaddingRight, mPaddingTop, mPaddingBottom;
  int mMarginLeft, mMarginRight, mMarginTop, mMarginBottom;
  int mItemSpacing, mIconTextPadding;

  // internal or not explicitly exposed properties:
  QCustomPlot *mParentPlot;
  QList<QCustomPlotGraph*> mGraphs;

  // introduced methods:
  virtual void draw(QPainter *painter);
  virtual void calculateAutoSize();
  virtual void calculateAutoPosition();

  friend class QCustomPlot;
};

class QCustomPlotAxis : public QObject
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QCustomPlotDataFetcher* dataFetcher READ dataFetcher WRITE setDataFetcher)
  Q_PROPERTY(AxisType axisType READ axisType WRITE setAxisType)
  Q_PROPERTY(ScaleType scaleType READ scaleType WRITE setScaleType)
  Q_PROPERTY(double scaleLogBase READ scaleLogBase WRITE setScaleLogBase)
  Q_PROPERTY(QRect axisRect READ axisRect WRITE setAxisRect)
  Q_PROPERTY(QCustomPlotRange range READ range WRITE setRange)
  Q_PROPERTY(bool visible READ visible WRITE setVisible)
  Q_PROPERTY(bool grid READ grid WRITE setGrid)
  Q_PROPERTY(bool subGrid READ subGrid WRITE setSubGrid)
  Q_PROPERTY(bool autoTicks READ autoTicks WRITE setAutoTicks)
  Q_PROPERTY(int autoTickCount READ autoTickCount WRITE setAutoTickCount)
  Q_PROPERTY(bool autoTickLabels READ autoTickLabels WRITE setAutoTickLabels)
  Q_PROPERTY(bool autoTickStep READ autoTickStep WRITE setAutoTickStep)
  Q_PROPERTY(bool autoSubTicks READ autoSubTicks WRITE setAutoSubTicks)
  Q_PROPERTY(bool ticks READ ticks WRITE setTicks)
  Q_PROPERTY(bool tickLabels READ tickLabels WRITE setTickLabels)
  Q_PROPERTY(int tickLabelPadding READ tickLabelPadding WRITE setTickLabelPadding)
  Q_PROPERTY(LabelType tickLabelType READ tickLabelType WRITE setTickLabelType)
  Q_PROPERTY(QFont tickLabelFont READ tickLabelFont WRITE setTickLabelFont)
  Q_PROPERTY(double tickLabelRotation READ tickLabelRotation WRITE setTickLabelRotation)
  Q_PROPERTY(QString dateTimeFormat READ dateTimeFormat WRITE setDateTimeFormat)
  Q_PROPERTY(QString numberFormat READ numberFormat WRITE setNumberFormat)
  Q_PROPERTY(double tickStep READ tickStep WRITE setTickStep)
  Q_PROPERTY(QVector<double>* tickVector READ tickVector WRITE setTickVector)
  Q_PROPERTY(QVector<QString>* tickVectorLabels READ tickVectorLabels WRITE setTickVectorLabels)
  Q_PROPERTY(int tickLength READ tickLength WRITE setTickLength)
  Q_PROPERTY(int subTickCount READ subTickCount WRITE setSubTickCount)
  Q_PROPERTY(int subTickLength READ subTickLength WRITE setSubTickLength)
  Q_PROPERTY(QPen basePen READ basePen WRITE setBasePen)
  Q_PROPERTY(QPen gridPen READ gridPen WRITE setGridPen)
  Q_PROPERTY(QPen subGridPen READ subGridPen WRITE setSubGridPen)
  Q_PROPERTY(QPen tickPen READ tickPen WRITE setTickPen)
  Q_PROPERTY(QPen subTickPen READ subTickPen WRITE setSubTickPen)
  Q_PROPERTY(QFont labelFont READ labelFont WRITE setLabelFont)
  Q_PROPERTY(QString label READ label WRITE setLabel)
  Q_PROPERTY(int labelPadding READ labelPadding WRITE setLabelPadding)
  /// \endcond
public:
  /*!
	Defines at which side of the axis rect the axis will appear. This also affects how the tick
	marks are drawn, on which side the labels are placed etc.
	\see setAxisType
  */
  enum AxisType {atLeft,  ///< Axis is vertical and on the left side of the axis rect of the parent QCustomPlot
				 atRight, ///< Axis is vertical and on the right side of the axis rect of the parent QCustomPlot
				 atTop,   ///< Axis is horizontal and on the top side of the axis rect of the parent QCustomPlot
				 atBottom ///< Axis is horizontal and on the bottom side of the axis rect of the parent QCustomPlot
				};
  /*!
	When automatic tick label generation is enabled (\ref setAutoTickLabels), defines how the
	numerical value (coordinate) of the tick position is translated into a string that will be
	drawn at the tick position.
	\see setTickLabelType
  */
  enum LabelType {ltNumber,  ///< Tick coordinate is regarded as normal number and will be displayed as such. (see \ref setNumberFormat)
				  ltDateTime ///< Tick coordinate is regarded as a date/time (seconds since 1970-01-01T00:00:00 UTC, see QDateTime::toTime_t) and will be displayed and formatted as such. (see \ref setDateTimeFormat)
				 };
  /*!
	Defines the scale of an axis.
	\see setScaleType
  */
  enum ScaleType {stLinear,     ///< Normal linear scaling
				  stLogarithmic ///< Logarithmic scaling with correspondingly transformed plots and (major) tick marks at every base power (see \ref setScaleLogBase).
				 };

  explicit QCustomPlotAxis(QCustomPlot *parentPlot, AxisType type);
  ~QCustomPlotAxis();

  // getters:
  QCustomPlotDataFetcher *dataFetcher() const { return mDataFetcher; }
  AxisType axisType() const { return mAxisType; }
  QRect axisRect() const { return mAxisRect; }
  ScaleType scaleType() const { return mScaleType; }
  double scaleLogBase() const { return mScaleLogBase; }
  const QCustomPlotRange range() const { return mRange; }
  bool visible() const { return mVisible; }
  bool grid() const { return mGrid; }
  bool subGrid() const { return mSubGrid; }
  bool autoTicks() const { return mAutoTicks; }
  int autoTickCount() const { return mAutoTickCount; }
  bool autoTickLabels() const { return mAutoTickLabels; }
  bool autoTickStep() const { return mAutoTickStep; }
  bool autoSubTicks() const { return mAutoSubTicks; }
  bool ticks() const { return mTicks; }
  bool tickLabels() const { return mTickLabels; }
  int tickLabelPadding() const { return mTickLabelPadding; }
  LabelType tickLabelType() const { return mTickLabelType; }
  QFont tickLabelFont() const { return mTickLabelFont; }
  double tickLabelRotation() const { return mTickLabelRotation; }
  QString dateTimeFormat() const { return mDateTimeFormat; }
  QString numberFormat() const;
  int numberPrecision() const { return mNumberPrecision; }
  double tickStep() const { return mTickStep; }
  QVector<double> *tickVector() const { return mTickVector; }
  QVector<QString> *tickVectorLabels() const { return mTickVectorLabels; }
  int tickLength() const { return mTickLength; }
  int subTickCount() const { return mSubTickCount; }
  int subTickLength() const { return mSubTickLength; }
  QPen basePen() const { return mBasePen; }
  QPen gridPen() const { return mGridPen; }
  QPen subGridPen() const { return mSubGridPen; }
  QPen tickPen() const { return mTickPen; }
  QPen subTickPen() const { return mSubTickPen; }
  QFont labelFont() const { return mLabelFont; }
  QString label() const { return mLabel; }
  int labelPadding() const { return mLabelPadding; }

  // setters:
  void setDataFetcher(QCustomPlotDataFetcher *fetcher);
  void setScaleType(ScaleType type);
  void setScaleLogBase(double base);
  void setRange(double lower, double upper);
  void setRange(double position, double size, Qt::AlignmentFlag alignment);
  void setRangeLower(double lower);
  void setRangeUpper(double upper);
  void setVisible(bool on);
  void setGrid(bool show);
  void setSubGrid(bool show);
  void setAutoTicks(bool on);
  void setAutoTickCount(int approximateCount);
  void setAutoTickLabels(bool on);
  void setAutoTickStep(bool on);
  void setAutoSubTicks(bool on);
  void setTicks(bool show);
  void setTickLabels(bool show);
  void setTickLabelPadding(int padding);
  void setTickLabelType(LabelType type);
  void setTickLabelFont(const QFont &font);
  void setTickLabelRotation(double degrees);
  void setDateTimeFormat(const QString &format);
  void setNumberFormat(const QString &formatCode);
  void setNumberPrecision(int precision);
  void setTickStep(double step);
  void setTickVector(QVector<double> *vec, bool copy=false);
  void setTickVectorLabels(QVector<QString> *vec, bool copy=false);
  void setTickLength(int length);
  void setSubTickCount(int count);
  void setSubTickLength(int length);
  void setBasePen(const QPen &pen);
  void setGridPen(const QPen &pen);
  void setSubGridPen(const QPen &pen);
  void setTickPen(const QPen &pen);
  void setSubTickPen(const QPen &pen);
  void setLabelFont(const QFont &font);
  void setLabel(const QString &str);
  void setLabelPadding(int padding);

  // non-property methods:
  void moveRange(double diff);
  void scaleRange(double factor, double center);
  void setScaleRatio(const QCustomPlotAxis *otherAxis, double ratio=1.0);
  double pixelToCoord(double value) const;
  double coordToPixel(double value) const;

public slots:
  // slot setters:
  void setRange(const QCustomPlotRange &range);

protected:
  // simple properties with getters and setters:
  QVector<double> *mTickVector;
  QVector<QString> *mTickVectorLabels;
  QCustomPlotDataFetcher *mDataFetcher;
  QCustomPlotRange mRange;
  QString mDateTimeFormat;
  QString mLabel;
  QRect mAxisRect;
  QPen mBasePen, mGridPen, mSubGridPen, mTickPen, mSubTickPen;
  QFont mTickLabelFont, mLabelFont;
  LabelType mTickLabelType;
  ScaleType mScaleType;
  AxisType mAxisType;
  double mTickStep;
  double mScaleLogBase, mScaleLogBaseLogInv;
  int mSubTickCount, mTickLength, mSubTickLength;
  int mAutoTickCount;
  int mTickLabelPadding, mLabelPadding;
  double mTickLabelRotation;
  bool mVisible, mGrid, mSubGrid, mTicks, mTickLabels, mAutoTicks, mAutoTickLabels, mAutoTickStep, mAutoSubTicks;

  // internal or not explicitly exposed properties:
  QCustomPlot *mParentPlot;
  QVector<double> *mSubTickVector;
  QChar mExponentialChar, mPositiveSignChar;
  int mNumberPrecision;
  char mNumberFormatChar;
  bool mNumberBeautifulPowers, mNumberMultiplyCross;

  // internal setters:
  void setAxisType(AxisType type);
  void setAxisRect(const QRect &rect);

  // introduced methods:
  virtual void generateTickVectors();
  virtual void generateAutoTicks();
  virtual int calculateAutoSubTickCount(double tickStep) const;
  virtual int calculateMargin() const;
  virtual void drawGrid(QPainter *painter);
  virtual void drawSubGrid(QPainter *painter);
  virtual void drawAxis(QPainter *painter);
  virtual void drawTickLabel(QPainter *painter, double position, const QString &text, QSize *tickLabelsSize);
  virtual void getMaxTickLabelSize(const QFont &font, const QString &text, QSize *tickLabelsSize) const;

  // basic non virtual helpers:
  void visibleTickBounds(int &lowIndex, int &highIndex) const;
  double baseLog(double value) const;
  double basePow(double value) const;

signals:
  void ticksRequest();
  void rangeChanged(const QCustomPlotRange &newRange);

  friend class QCustomPlot;
  friend class QCustomPlotGraph;
};

class QCustomPlot : public QWidget
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QString title READ title WRITE setTitle)
  Q_PROPERTY(QRect axisRect READ axisRect WRITE setAxisRect)
  Q_PROPERTY(int marginLeft READ marginLeft WRITE setMarginLeft)
  Q_PROPERTY(int marginRight READ marginRight WRITE setMarginRight)
  Q_PROPERTY(int marginTop READ marginTop WRITE setMarginTop)
  Q_PROPERTY(int marginBottom READ marginBottom WRITE setMarginBottom)
  Q_PROPERTY(int autoMargin READ autoMargin WRITE setAutoMargin)
  Q_PROPERTY(QColor color READ color WRITE setColor)
  Q_PROPERTY(Qt::Orientations rangeDrag READ rangeDrag WRITE setRangeDrag)
  Q_PROPERTY(Qt::Orientations rangeZoom READ rangeZoom WRITE setRangeZoom)
  /// \endcond
public:
  /*!
	Defines what elements of a plot will be drawn antialiased.

	\c AntialiasedElements is a flag of or-combined elements of this enum type.
	\see setAntialiasedElements, setAntialiasedElement
  */
  enum AntialiasedElement {aeAxes      = 0x01, ///< Axis base line and tick marks
						   aeGrid      = 0x02, ///< Grid lines
						   aeSubGrid   = 0x04, ///< Sub grid lines
						   aeGraphs    = 0x08, ///< Any lines of graphs (excluding error bars, see element \ref aeErrorBars)
						   aeScatters  = 0x10, ///< Scatter symbols of graphs (excluding scatter symbols of type \ref ssPixmap)
						   aeErrorBars = 0x20, ///< Error bars
						   aeFills     = 0x40  ///< Borders of fills under or between graphs
						  };
  Q_ENUMS(AntialiasedElement)
  Q_DECLARE_FLAGS(AntialiasedElements, AntialiasedElement)

  explicit QCustomPlot(QWidget *parent = 0);
  ~QCustomPlot();

  // getters:
  QString title() const { return mTitle; }
  QFont titleFont() const { return mTitleFont; }
  QRect axisRect() const { return mAxisRect; }
  int marginLeft() const { return mMarginLeft; }
  int marginRight() const { return mMarginRight; }
  int marginTop() const { return mMarginTop; }
  int marginBottom() const { return mMarginBottom; }
  bool autoMargin() const { return mAutoMargin; }
  QColor color() const { return mColor; }
  Qt::Orientations rangeDrag() const { return mRangeDrag; }
  Qt::Orientations rangeZoom() const { return mRangeZoom; }
  QCustomPlotAxis *rangeDragAxis(Qt::Orientation orientation);
  QCustomPlotAxis *rangeZoomAxis(Qt::Orientation orientation);
  double rangeZoomFactor(Qt::Orientation orientation);
  const AntialiasedElements antialiasedElements() const { return mAntialiasedElements; }
  bool autoAddGraphToLegend() const { return mAutoAddGraphToLegend; }

  // setters:
  void setTitle(const QString &title);
  void setTitleFont(const QFont &font);
  void setAxisRect(const QRect &arect);
  void setMarginLeft(int margin);
  void setMarginRight(int margin);
  void setMarginTop(int margin);
  void setMarginBottom(int margin);
  void setMargin(int left, int right, int top, int bottom);
  void setAutoMargin(bool enabled);
  void setColor(const QColor &color);
  void setRangeDrag(Qt::Orientations orientations);
  void setRangeZoom(Qt::Orientations orientations);
  void setRangeDragAxes(QCustomPlotAxis *horizontal, QCustomPlotAxis *vertical);
  void setRangeZoomAxes(QCustomPlotAxis *horizontal, QCustomPlotAxis *vertical);
  void setRangeZoomFactor(double horizontalFactor, double verticalFactor);
  void setRangeZoomFactor(double factor);
  void setAntialiasedElements(const AntialiasedElements &antialiasedElements);
  void setAntialiasedElement(AntialiasedElement antialiasedElement, bool enabled);
  void setAutoAddGraphToLegend(bool on);

  // non-property methods:
  QCustomPlotGraph *graph(int index);
  QCustomPlotGraph *graph();
  int addGraph(QCustomPlotAxis *keyAxis=0, QCustomPlotAxis *valueAxis=0);
  bool removeGraph(int index);
  int clearGraphs();
  int graphCount();
  void replot();
  void setupFullAxesBox();
  void savePdf(const QString &fileName, bool noCosmeticPen=false);
  //void saveSvg(const QString &fileName);
  void savePng(const QString &fileName, int width=0, int height=0);
  void savePngScaled(const QString &fileName, double scale, int width=0, int height=0);

  QCustomPlotAxis *xAxis, *yAxis, *xAxis2, *yAxis2;
  QCustomPlotLegend *legend;

protected:
  QPixmap buffer;
  QString mTitle;
  QFont mTitleFont;
  QRect mViewport;
  QRect mAxisRect;
  int mMarginLeft, mMarginRight, mMarginTop, mMarginBottom;
  bool mAutoMargin, mAutoAddGraphToLegend;
  QColor mColor;
  QList<QCustomPlotGraph*> mGraphs;
  Qt::Orientations mRangeDrag, mRangeZoom;
  QCustomPlotAxis *mRangeDragHorzAxis, *mRangeDragVertAxis, *mRangeZoomHorzAxis, *mRangeZoomVertAxis;
  double mRangeZoomFactorHorz, mRangeZoomFactorVert;
  bool mDragging;
  QPoint mDragStart;
  QCustomPlotRange mDragStartHorzRange, mDragStartVertRange;
  QFlags<AntialiasedElement> mAntialiasedElements;

  // reimplemented methods:
  virtual void paintEvent(QPaintEvent *event);
  virtual void resizeEvent(QResizeEvent *event);
  virtual void mouseDoubleClickEvent(QMouseEvent *event);
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent(QMouseEvent *event);
  virtual void wheelEvent(QWheelEvent *event);

  // introduced methods:
  virtual void draw(QPainter *painter);

  // helpers:
  void updateAxisRect();

signals:
  void mouseDoubleClick(QMouseEvent *event);
  void mousePress(QMouseEvent *event);
  void mouseMove(QMouseEvent *event);
  void mouseRelease(QMouseEvent *event);
  void mouseWheel(QWheelEvent *event);

public slots:

  friend class QCustomPlotLegend;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QCustomPlot::AntialiasedElements)

#endif // QCUSTOMPLOT_H
