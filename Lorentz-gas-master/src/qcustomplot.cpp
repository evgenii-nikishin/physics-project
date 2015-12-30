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
****************************************************************************/

/*! \mainpage %QCustomPlot Documentation
 
  Below is a brief overview of the classes and their relations. If you are new to QCustomPlot and
  just want to start using it, it's recommended to look at the examples/tutorials at
 
  http://www.WorksLikeClockWork.com/index.php/components/qt-plotting-widget
 
  This documentation is especially helpful when the basic concept of how to use %QCustomPlot is
  clear and you wish to learn more about specific functionality.
 
  \section classoverview Classes and their relations
  
  The central widget which displays the graphs and axes on its surface is QCustomPlot. Graphs and
  axes are themselves managed by QCustomPlotAxis and QCustomPlotGraph. For normal usage however,
  the axis and graph classes don't need to be instantiated by you, since you interface with them
  via the QCustomPlot instance. By default, %QCustomPlot has four axes \a yAxis, \a xAxis, \a
  yAxis2, \a xAxis2 (left, bottom, right, top). Graphs are created by calling QCustomPlot::addGraph
  and can be accessed via QCustomPlot::graph.
  
  The range an axis shows is handled by the simple QCustomPlotRange class. Plottable data is
  assigned to a graph via its QCustomPlotGraph::setData functions. The data is stored internally as
  a ::QCustomPlotDataMap which is a QMap mapping a key of type double to the actual data storage
  container QCustomPlotData. Assigning data by creating own QCustomPlotData instances, filling them
  with data points, and passing them to %QCustomPlotGraph is possible but not necessary. More user
  friendly functions exist that accept for example QVector<double> parameters as x, y, errors, etc.
  
  Every %QCustomPlot owns a QCustomPlotLegend (as \a legend). That's a small window inside the plot
  which lists the graphs with an icon of the graph line/symbol and a description. The Description
  is retrieved from the QCustomPlotGraph::name property. Graphs displayed in the legend can be
  added and removed via QCustomPlotLegend::addGraph and QCustomPlotLegend::removeGraph. By default,
  adding a graph to %QCustomPlot automatically adds it to the legend, too. (This behaviour can be
  modified with the QCustomPlot::autoAddGraphToLegend property.)
  
  \section performancetweaks Performance Tweaks
  
  Although QCustomPlot is quite fast, some features (transparent fills and antialiasing) cause a
  significant slow down. Here are some thoughts on how to increase performance:
  
  Most performance gets lost in the drawing functions, specifically the drawing of graphs. For
  maximum performance, consider the following (in no particular order):
  \li avoid any kind of alpha (transparency),
  especially in fills \li avoid any kind of antialiasing, especially in graph lines
  (QCustomPlot::setAntialiasedElements)
  \li avoid repeatedly setting the complete data set with setData. Use addData instead, if most
  data points stay unchanged, e.g. in a running measurement.
  \li set the \a copy parameter of the setData functions to false, so only pointers get
  transferred.
  \li on X11 (linux), avoid the (slow) native drawing system, use raster by supplying
  "-graphicssystem raster" as command line argument
  \li on all operating systems, use OpenGL hardware acceleration by supplying "-graphicssystem
  opengl" as command line argument. If OpenGL is available, this will slightly decrease the quality
  of antialiasing, but extremely increase performance especially with alpha (transparent) fills and
  a large QCustomPlot drawing surface. Note however, that the maximum frame rate might be
  constrained by the vertical sync frequency of your monitor (VSync can be disabled in the graphics
  card driver configuration). So for simple plots (potential framerate far above 60 frames per
  second), OpenGL acceleration might achieve \a lower frame rates than the other graphics systems,
  because they are not capped at the VSync frequency.
*/

#include "qcustomplot.h"

// ================================================================================
// =================== QCustomPlotData
// ================================================================================

/*! \class QCustomPlotData
  A class holding the data of one single data point.
  \see QCustomPlotDataMap
*/

/*!
  Constructs an empty data point with key, value and all errors set to zero.
*/
QCustomPlotData::QCustomPlotData() :
  key(0),
  value(0),
  keyErrorPlus(0),
  keyErrorMinus(0),
  valueErrorPlus(0),
  valueErrorMinus(0)
{
}

// ================================================================================
// =================== QCustomPlotGraph
// ================================================================================

/*! \class QCustomPlotGraph
  A class representing a graph in a plot. Usually QCustomPlot creates it internally via
  QCustomPlot::addGraph and the resulting instance is accessed via QCustomPlot::graph.
  To plot data, assign it with the \ref setData or \ref addData functions.
  
  The appearance of the graph is mainly determined by the line style, scatter style, brush and pen
  of the graph (\ref setLineStyle, \ref setScatterStyle, \ref setBrush, \ref setPen).
  \see QCustomPlot::addGraph, QCustomPlot::graph, QCustomPlotLegend::addGraph
*/

/*!
  Constructs a graph which uses \a keyaxis as its key axis ("x") and \a valueaxis as its value
  axis ("y").
  To create a graph inside a plot, rather use the QCustomPlot::addGraph function.
  \see setkeyAxis, setValueAxis
*/
QCustomPlotGraph::QCustomPlotGraph(QCustomPlotAxis *keyaxis, QCustomPlotAxis *valueaxis)
{
  mKeyAxis = keyaxis;
  mValueAxis = valueaxis;
  mParentPlot = keyaxis->mParentPlot;
  mData = new QCustomPlotDataMap;
  mVisible = true;
  mPen.setColor(Qt::blue);
  mPen.setStyle(Qt::SolidLine);
  mErrorPen.setColor(Qt::black);
  mBrush.setColor(Qt::blue);
  mBrush.setStyle(Qt::NoBrush);
  mName = "";
  mLineStyle = lsLine;
  mScatterStyle = ssNone;
  mScatterSize = 6;
  mErrorType = etNone;
  mErrorBarSize = 4;
  mErrorBarSkipSymbol = true;
  mChannelFillGraph = NULL;
}

QCustomPlotGraph::~QCustomPlotGraph()
{
  delete mData;
}

/*!
   The name is the textual representation of this graph as it is displayed in the QCustomPlotLegend of the
   parent QCustomPlot. It may contain any utf-8 characters, including newlines.
*/
void QCustomPlotGraph::setName(const QString &str)
{
  mName = str;
}

/*!
  The pen is used to draw lines (e.g. when line style isn't \ref lsNone) and scatter points (e.g. when scatter style isn't \ref ssNone).
  \see setBrush
*/
void QCustomPlotGraph::setPen(const QPen &p)
{
  mPen = p;
}

/*!
  If the brush is not Qt::noBrush, a graph fill will be drawn.
 
  QCustomPlot knows two types of fills: Normal graph fills towards the zero-value-line parallel to
  the key axis of the graph, and fills between two graphs, called channel fills. To enable a fill,
  just set a brush \a b which is neither Qt::noBrush nor fully transparent. By default, a normal fill
  towards the zero-value-line will be drawn. To set up a channel fill between this graph and
  another one, call \ref setChannelFillGraph with the other graph as parameter.

  The Fill can be a for example a color or a texture, see the usage of QBrush.
  \see setPen
*/
void QCustomPlotGraph::setBrush(const QBrush &b)
{
  mBrush = b;
}

/*!
  If the graphs visibility is set to false, it won't be drawn to the plot surface.
*/
void QCustomPlotGraph::setVisible(bool on)
{
  mVisible = on;
}

/*!
  The key axis of a graph can be set to any axis of a QCustomPlot, as long as it is orthogonal to the graph's value axis.
  The typical mathematical choice is to use the x-axis (QCustomPlot::xAxis) as key axis and the y-axis (QCustomPlot::yAxis) as value axis.
  \see setValueAxis
*/
void QCustomPlotGraph::setKeyAxis(QCustomPlotAxis *keyaxis)
{
  mKeyAxis = keyaxis;
}

/*!
  The value axis of a graph can be set to any axis of a QCustomPlot, as long as it is orthogonal to the graph's key axis.
  The typical mathematical choice is to use the x-axis (QCustomPlot::xAxis) as key axis and the y-axis (QCustomPlot::yAxis) as value axis.
  \see setKeyAxis
*/
void QCustomPlotGraph::setValueAxis(QCustomPlotAxis *valueaxis)
{
  mValueAxis = valueaxis;
}

/*!
  Replaces the current data with the provided \a data.
  
  \param data a number of data points organized in a QCustomPlotDataMap which will replace the currently set data
  \param copy if true, data points in \a data will only be copied. if false, the graph takes ownership of the
  passed QCustomPlotDataMap and replaces the internal data pointer with it. This is significantly faster than copying for
  large datasets.
*/
void QCustomPlotGraph::setData(QCustomPlotDataMap *data, bool copy)
{
  if (copy)
  {
    *mData = *data;
  } else
  {
    delete mData;
    mData = data;
  }
}

/*!
  Replaces the current data with the provided points in \a key and \a value pairs.
  The provided vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.
*/
void QCustomPlotGraph::setData(const QVector<double> &key, const QVector<double> &value)
{
  mData->clear();
  int n = key.size();
  n = qMin(n, value.size());
  QCustomPlotData newData;
  for (int i=0; i<n; ++i)
  {
    newData.key = key[i];
    newData.value = value[i];
    mData->insertMulti(key[i], newData);
  }
}

/*!
  Replaces the current data with the provided points in \a key and \a value pairs. Additionally the
  symmetrical value error of the data points are set to the values in \a valueError.
  For error bars to show appropriately, see \ref setErrorType.
  The provided vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.
*/
void QCustomPlotGraph::setDataValueError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &valueError)
{
  mData->clear();
  int n = key.size();
  n = qMin(n, value.size());
  n = qMin(n, valueError.size());
  QCustomPlotData newData;
  for (int i=0; i<n; ++i)
  {
    newData.key = key[i];
    newData.value = value[i];
    newData.valueErrorMinus = valueError[i];
    newData.valueErrorPlus = valueError[i];
    mData->insertMulti(key[i], newData);
  }
}

/*!
  Replaces the current data with the provided points in \a key and \a value pairs. Additionally the
  negative value error of the data points are set to the values in \a valueErrorMinus, the positive
  value error to \a valueErrorPlus.
  For error bars to show appropriately, see \ref setErrorType.
  The provided vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.
*/
void QCustomPlotGraph::setDataValueError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &valueErrorMinus, const QVector<double> &valueErrorPlus)
{
  mData->clear();
  int n = key.size();
  n = qMin(n, value.size());
  n = qMin(n, valueErrorMinus.size());
  n = qMin(n, valueErrorPlus.size());
  QCustomPlotData newData;
  for (int i=0; i<n; ++i)
  {
    newData.key = key[i];
    newData.value = value[i];
    newData.valueErrorMinus = valueErrorMinus[i];
    newData.valueErrorPlus = valueErrorPlus[i];
    mData->insertMulti(key[i], newData);
  }
}

/*!
  Replaces the current data with the provided points in \a key and \a value pairs. Additionally the
  symmetrical key error of the data points are set to the values in \a keyError.
  For error bars to show appropriately, see \ref setErrorType.
  The provided vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.
*/
void QCustomPlotGraph::setDataKeyError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &keyError)
{
  mData->clear();
  int n = key.size();
  n = qMin(n, value.size());
  n = qMin(n, keyError.size());
  QCustomPlotData newData;
  for (int i=0; i<n; ++i)
  {
    newData.key = key[i];
    newData.value = value[i];
    newData.keyErrorMinus = keyError[i];
    newData.keyErrorPlus = keyError[i];
    mData->insertMulti(key[i], newData);
  }
}

/*!
  Replaces the current data with the provided points in \a key and \a value pairs. Additionally the
  negative key error of the data points are set to the values in \a keyErrorMinus, the positive
  key error to \a keyErrorPlus.
  For error bars to show appropriately, see \ref setErrorType.
  The provided vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.
*/
void QCustomPlotGraph::setDataKeyError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &keyErrorMinus, const QVector<double> &keyErrorPlus)
{
  mData->clear();
  int n = key.size();
  n = qMin(n, value.size());
  n = qMin(n, keyErrorMinus.size());
  n = qMin(n, keyErrorPlus.size());
  QCustomPlotData newData;
  for (int i=0; i<n; ++i)
  {
    newData.key = key[i];
    newData.value = value[i];
    newData.keyErrorMinus = keyErrorMinus[i];
    newData.keyErrorPlus = keyErrorPlus[i];
    mData->insertMulti(key[i], newData);
  }
}

/*!
  Replaces the current data with the provided points in \a key and \a value pairs. Additionally the
  symmetrical key and value errors of the data points are set to the values in \a keyError and \a valueError.
  For error bars to show appropriately, see \ref setErrorType.
  The provided vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.
*/
void QCustomPlotGraph::setDataBothError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &keyError, const QVector<double> &valueError)
{
  mData->clear();
  int n = key.size();
  n = qMin(n, value.size());
  n = qMin(n, valueError.size());
  n = qMin(n, keyError.size());
  QCustomPlotData newData;
  for (int i=0; i<n; ++i)
  {
    newData.key = key[i];
    newData.value = value[i];
    newData.keyErrorMinus = keyError[i];
    newData.keyErrorPlus = keyError[i];
    newData.valueErrorMinus = valueError[i];
    newData.valueErrorPlus = valueError[i];
    mData->insertMulti(key[i], newData);
  }
}

/*!
  Replaces the current data with the provided points in \a key and \a value pairs. Additionally the
  negative key and value errors of the data points are set to the values in \a keyErrorMinus and \a valueErrorMinus. The positive
  key and value errors are set to the values in \a keyErrorPlus \a valueErrorPlus.
  For error bars to show appropriately, see \ref setErrorType.
  The provided vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.
*/
void QCustomPlotGraph::setDataBothError(const QVector<double> &key, const QVector<double> &value, const QVector<double> &keyErrorMinus, const QVector<double> &keyErrorPlus, const QVector<double> &valueErrorMinus, const QVector<double> &valueErrorPlus)
{
  mData->clear();
  int n = key.size();
  n = qMin(n, value.size());
  n = qMin(n, valueErrorMinus.size());
  n = qMin(n, valueErrorPlus.size());
  n = qMin(n, keyErrorMinus.size());
  n = qMin(n, keyErrorPlus.size());
  QCustomPlotData newData;
  for (int i=0; i<n; ++i)
  {
    newData.key = key[i];
    newData.value = value[i];
    newData.keyErrorMinus = keyErrorMinus[i];
    newData.keyErrorPlus = keyErrorPlus[i];
    newData.valueErrorMinus = valueErrorMinus[i];
    newData.valueErrorPlus = valueErrorPlus[i];
    mData->insertMulti(key[i], newData);
  }
}

/*!
  Adds the provided data points in \a dataMap to the current data in the graph.
  \see removeData
*/
void QCustomPlotGraph::addData(const QCustomPlotDataMap &dataMap)
{
  mData->unite(dataMap);
}

/*!
  Adds the provided single data point in \a data to the current data in the graph.
  \see removeData
*/
void QCustomPlotGraph::addData(const QCustomPlotData &data)
{
  mData->insertMulti(data.key, data);
}

/*!
  Adds the provided single data point as \a key and \a value pair to the current data in the graph.
  \see removeData
*/
void QCustomPlotGraph::addData(double key, double value)
{
  QCustomPlotData newData;
  newData.key = key;
  newData.value = value;
  mData->insertMulti(newData.key, newData);
}

/*!
  Adds the provided data points as \a key and \a value pairs to the current data in the graph.
  \see removeData
*/
void QCustomPlotGraph::addData(const QVector<double> &keys, const QVector<double> &values)
{
  int n = qMin(keys.size(), values.size());
  QCustomPlotData newData;
  for (int i=0; i<n; ++i)
  {
    newData.key = keys.at(i);
    newData.value = values.at(i);
    mData->insertMulti(newData.key, newData);
  }
}

/*!
  Removes all data points with keys smaller than \a key from the graph.
  \see addData
*/
void QCustomPlotGraph::removeDataBefore(double key)
{
  QCustomPlotDataMap::iterator it = mData->begin();
  while (it != mData->end() && it.key() < key)
    it = mData->erase(it);
}

/*!
  Removes all data points with keys greater than \a key from the graph.
  \see addData, clearData
*/
void QCustomPlotGraph::removeDataAfter(double key)
{
  if (mData->isEmpty()) return;
  QCustomPlotDataMap::iterator it = mData->upperBound(key);
  while (it != mData->end())
    it = mData->erase(it);
}

/*!
  Removes all data points with keys between \a fromKey and \a toKey from the graph.
  if \a fromKey is greater or equal to \a toKey, the function does nothing. To remove
  a single data point with known key, use \ref removeData(double key).
  \see addData, clearData
*/
void QCustomPlotGraph::removeData(double fromKey, double toKey)
{
  if (fromKey >= toKey || mData->isEmpty()) return;
  QCustomPlotDataMap::iterator it = mData->upperBound(fromKey);
  QCustomPlotDataMap::iterator itEnd = mData->upperBound(toKey);
  while (it != itEnd)
    it = mData->erase(it);
}

/*!
  Removes a single data point at \a key. If the position is not known with absolute precision,
  consider using \ref removeData(double fromKey, double toKey) with a small fuzziness interval around
  the suspected position, depeding on the precision with which the key is known.
  \see addData, clearData
*/
void QCustomPlotGraph::removeData(double key)
{
  mData->remove(key);
}

/*!
  Removes all data points.
  \see removeData, removeDataAfter, removeDataBefore
*/
void QCustomPlotGraph::clearData()
{
  mData->clear();
}

/*!
  Sets how the single data points are connected in the plot or how they are represented visually
  apart from the scatter symbol. For scatter-only plots, set \a ls to \ref lsNone and \ref
  setScatterStyle to the desired scatter style.
  \see setScatterStyle
*/
void QCustomPlotGraph::setLineStyle(LineStyle ls)
{
  mLineStyle = ls;
}

/*! 
  Sets the visual appearance of single data points in the plot. If set to \ref ssNone, no scatter points
  are drawn (e.g. for line-only-plots with appropriate line style).
  \see ScatterStyle, setLineStyle
*/
void QCustomPlotGraph::setScatterStyle(ScatterStyle ss)
{
  mScatterStyle = ss;
}

/*! 
  This defines how big (in pixels) single scatters are drawn, if scatter style (\ref
  setScatterStyle) isn't \ref ssNone, \ref ssDot or \ref ssPixmap. Floating point values are
  allowed for fine grained control over optical appearance with antialiased painting.
  \see ScatterStyle
*/
void QCustomPlotGraph::setScatterSize(double size)
{
  mScatterSize = size;
}

/*! 
  If the scatter style (\ref setScatterStyle) is set to ssPixmap, this function defines the QPixmap
  that will be drawn centered on the data point coordinate.
  \see ScatterStyle
*/
void QCustomPlotGraph::setScatterPixmap(const QPixmap &pixmap)
{
  mScatterPixmap = pixmap;
}

/*! 
  \see ErrorType
*/
void QCustomPlotGraph::setErrorType(ErrorType errorType)
{
  mErrorType = errorType;
}

/*!
  Sets the pen with which the error bars will be drawn.
  \see setErrorBarSize, setErrorType
*/
void QCustomPlotGraph::setErrorPen(const QPen &pen)
{
  mErrorPen = pen;
}

/*! 
  Sets the width of the handles at both ends of an error bar in pixels.
*/
void QCustomPlotGraph::setErrorBarSize(double size)
{
  mErrorBarSize = size;
}

/*! 
  If \a enabled is set to true, the error bar will not be drawn as a solid line under the scatter symbol but
  leave some free space around the symbol.
  
  This feature uses the current scatter size (\ref setScatterSize) to determine the size of the
  area to leave blank. So when drawing Pixmaps as scatter points (\ref ssPixmap), the scatter size
  must be set manually to a value corresponding to the size of the Pixmap, if the error bars should
  leave gaps to its boundaries.
*/
void QCustomPlotGraph::setErrorBarSkipSymbol(bool enabled)
{
  mErrorBarSkipSymbol = enabled;
}

/*! 
  Sets the target graph for filling the area between this graph and \a targetGraph with the current
  brush (\ref setBrush).
  
  When \a targetGraph is set to 0, a normal graph fill will be produced. This means, when the brush
  is not Qt::noBrush or fully transparent, a fill all the way to the zero-value-line parallel to
  the key axis of this graph will be drawn. To disable any filling, set the brush to Qt::NoBrush.
  \see setBrush
*/
void QCustomPlotGraph::setChannelFillGraph(QCustomPlotGraph *targetGraph)
{
  // prevent setting channel target to this graph itself:
  if (targetGraph == this)
  {
    qDebug() << "Attempt to set channel fill to self";
    mChannelFillGraph = 0;
    return;
  }
  // prevent setting channel target to a graph not in the plot:
  if (targetGraph && targetGraph->mParentPlot != mParentPlot)
  {
    qDebug() << "Attempt to set channel fill to graph not in same plot";
    mChannelFillGraph = 0;
    return;
  }
  
  mChannelFillGraph = targetGraph;
}

/*! 
  Rescales the key and value axes associated with this graph to contain all data points, so the whole graph
  is visible. If the scaling of an axis is logarithmic, rescaleAxes will make sure not to rescale to an
  illegal range i.e. a range containing different signs and/or zero. Instead it will stay in the current sign
  domain and ignore all points of the graph that lie outside of that domain.
  
  \param onlyEnlarge makes sure the ranges are only expanded, never reduced, so it's possible to
  show multiple graphs in their entirety by multiple calls to rescaleAxes where the first call has
  onlyEnlarge set to false (the default), and all subsequent set to true.
  \param includeErrorBars makes sure not only the data points themselves but also their error bars (if present)
  are in visible range
*/
void QCustomPlotGraph::rescaleAxes(bool onlyEnlarge, bool includeErrorBars)
{
  rescaleKeyAxis(onlyEnlarge, includeErrorBars);
  rescaleValueAxis(onlyEnlarge, includeErrorBars);
}

/*! 
  Rescales only the key axis, else same functionality as rescaleAxes
  \see rescaleAxes
*/
void QCustomPlotGraph::rescaleKeyAxis(bool onlyEnlarge, bool includeErrorBars)
{
  if (mData->isEmpty()) return;

  int restrictToSign = 0;
  if (mKeyAxis->mScaleType == QCustomPlotAxis::stLogarithmic)
    restrictToSign = (mKeyAxis->mRange.upper < 0 ? -1 : 1);
  
  bool validRange;
  QCustomPlotRange newRange = getKeyRange(validRange, includeErrorBars, restrictToSign);
  
  if (validRange)
  {
    if (onlyEnlarge)
    {
      if (mKeyAxis->mRange.lower < newRange.lower)
        newRange.lower = mKeyAxis->mRange.lower;
      if (mKeyAxis->mRange.upper > newRange.upper)
        newRange.upper = mKeyAxis->mRange.upper;
    }
    mKeyAxis->setRange(newRange);
  }
}

/*! 
  Rescales only the value axis, else same functionality as rescaleAxes
  \see rescaleAxes
*/
void QCustomPlotGraph::rescaleValueAxis(bool onlyEnlarge, bool includeErrorBars)
{
  if (mData->isEmpty()) return;

  int restrictToSign = 0;
  if (mValueAxis->mScaleType == QCustomPlotAxis::stLogarithmic)
    restrictToSign = (mValueAxis->mRange.upper < 0 ? -1 : 1);
  
  bool validRange;
  QCustomPlotRange newRange = getValueRange(validRange, includeErrorBars, restrictToSign);
  
  if (validRange)
  {
    if (onlyEnlarge)
    {
      if (mValueAxis->mRange.lower < newRange.lower)
        newRange.lower = mValueAxis->mRange.lower;
      if (mValueAxis->mRange.upper > newRange.upper)
        newRange.upper = mValueAxis->mRange.upper;
    }
    mValueAxis->setRange(newRange);
  }
}

/*! 
  \internal
  called by QCustomPlot::draw on every graph in the plot. Sets basic painter settings, gathers
  the required point/line data for the current scatter and line style and then
  forks into the specific drawing functions, to draw the graph with \a painter.
*/
void QCustomPlotGraph::draw(QPainter *painter) const
{
  if (!mVisible) return;
  if (mKeyAxis->range().size() <= 0) return;
  if (mData->isEmpty()) return;
  if (mLineStyle == lsNone && mScatterStyle == ssNone) return;
  
  painter->save();
  painter->setClipRect(mKeyAxis->axisRect().united(mValueAxis->axisRect()));
  
  // allocate line and (if necessary) point vectors:
  QVector<QPointF> *lineData = new QVector<QPointF>;
  QVector<QCustomPlotData> *pointData = NULL;
  if (mScatterStyle != ssNone)
    pointData = new QVector<QCustomPlotData>;
  
  // fill vectors with data appropriate to plot style:
  getPlotData(lineData, pointData);

  // draw fill of graph:
  drawFill(painter, lineData);
  
  // draw line:
  if (mLineStyle == lsImpulse)
    drawImpulsePlot(painter, lineData);
  else if (mLineStyle != lsNone)
    drawLinePlot(painter, lineData); // also step plots can be drawn as a line plot
  
  // draw scatters:
  if (pointData)
    drawScatterPlot(painter, pointData);
  
  // free allocated line and point vectors:
  delete lineData;
  if (pointData)
    delete pointData;
  
  painter->restore();
}

/*! 
  \internal
  This function branches out to the line style specific "get(...)PlotData" functions, according to the
  line style of the graph.
  \param lineData will be filled with raw points that will be drawn with the according draw functions, e.g. \ref drawLinePlot and \ref drawImpulsePlot.
  These aren't necessarily the original data points, since for step plots for example, additional points are needed for drawing lines that make up steps.
  If the line style of the graph is \ref lsNone, the \a lineData vector will be left untouched.
  \param pointData will be filled with the original data points so \ref drawScatterPlot can draw the scatter symbols accordingly. If no scatters need to be
  drawn, i.e. scatter style is \ref ssNone, pass 0 as \a pointData, and this step will be skipped.
  \see getScatterPlotData, getLinePlotData, getStepLeftPlotData, getStepRightPlotData, getStepCenterPlotData, getImpulsePlotData
*/
void QCustomPlotGraph::getPlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const
{
  switch(mLineStyle)
  {
  case lsNone: getScatterPlotData(pointData); break;
  case lsLine: getLinePlotData(lineData, pointData); break;
  case lsStepLeft: getStepLeftPlotData(lineData, pointData); break;
  case lsStepRight: getStepRightPlotData(lineData, pointData); break;
  case lsStepCenter: getStepCenterPlotData(lineData, pointData); break;
  case lsImpulse: getImpulsePlotData(lineData, pointData); break;
  }
}

/*! 
  \internal
  If line style is \ref lsNone and scatter style is not \ref ssNone, this function serves at providing the
  visible data points in \a pointData, so the \ref drawScatterPlot function can draw the scatter points
  accordingly.
  
  If line style is not \ref lsNone, this function is not called and the data for the scatter points
  are (if needed) calculated inside the corresponding other "get(...)PlotData" functions.
  \see drawScatterPlot
*/
void QCustomPlotGraph::getScatterPlotData(QVector<QCustomPlotData> *pointData) const
{
  if (!pointData) return;
  
  // get visible data range:
  QCustomPlotDataMap::const_iterator lower, upper;
  int dataCount;
  getVisibleDataBounds(lower, upper, dataCount);
  // prepare vectors:
  if (pointData)
    pointData->resize(dataCount);

  // position data points:
  QMap<double, QCustomPlotData>::const_iterator it = lower;
  QCustomPlotDataMap::const_iterator upperEnd = upper+1;
  int i = 0;
  if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft || mKeyAxis->axisType() == QCustomPlotAxis::atRight)
  {
    while (it != upperEnd)
    {
      (*pointData)[i] = it.value();
      ++i;
      ++it;
    }
  } else // key axis is bottom or top
  {
    while (it != upperEnd)
    {
      (*pointData)[i] = it.value();
      ++i;
      ++it;
    }
  }
}

/*! 
  \internal
  Places the raw data points needed for a normal linearly connected plot in \a lineData.

  As for all plot data retrieval functions, \a pointData just contains all unaltered data (scatter)
  points that are visible, for drawing scatter points, if necessary. If drawing scatter points is
  disabled (i.e. scatter style \ref ssNone), pass 0 as \a pointData, and the function will skip
  filling the vector.
  \see drawLinePlot
*/
void QCustomPlotGraph::getLinePlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const
{
  // get visible data range:
  QCustomPlotDataMap::const_iterator lower, upper;
  int dataCount;
  getVisibleDataBounds(lower, upper, dataCount);
  // prepare vectors:
  if (lineData)
  { 
    // added 2 to reserve memory for lower/upper fill base points that might be needed for fill
    lineData->reserve(dataCount+2);
    lineData->resize(dataCount);
  }
  if (pointData)
    pointData->resize(dataCount);

  // position data points:
  QMap<double, QCustomPlotData>::const_iterator it = lower;
  QCustomPlotDataMap::const_iterator upperEnd = upper+1;
  int i = 0;
  if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft || mKeyAxis->axisType() == QCustomPlotAxis::atRight)
  {
    while (it != upperEnd)
    {
      if (pointData)
        (*pointData)[i] = it.value();
      (*lineData)[i].setX(mValueAxis->coordToPixel(it.value().value));
      (*lineData)[i].setY(mKeyAxis->coordToPixel(it.key()));
      ++i;
      ++it;
    }
  } else // key axis is bottom or top
  {
    while (it != upperEnd)
    {
      if (pointData)
        (*pointData)[i] = it.value();
      (*lineData)[i].setX(mKeyAxis->coordToPixel(it.key()));
      (*lineData)[i].setY(mValueAxis->coordToPixel(it.value().value));
      ++i;
      ++it;
    }
  }
}

/*! 
  \internal
  Places the raw data points needed for a step plot with left oriented steps in \a lineData.

  As for all plot data retrieval functions, \a pointData just contains all unaltered data (scatter)
  points that are visible, for drawing scatter points, if necessary. If drawing scatter points is
  disabled (i.e. scatter style \ref ssNone), pass 0 as \a pointData, and the function will skip
  filling the vector.
  \see drawLinePlot
*/
void QCustomPlotGraph::getStepLeftPlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const
{
  // get visible data range:
  QCustomPlotDataMap::const_iterator lower, upper;
  int dataCount;
  getVisibleDataBounds(lower, upper, dataCount);
  // prepare vectors:
  if (lineData)
  {
    // added 2 to reserve memory for lower/upper fill base points that might be needed for fill
    // multiplied by 2 because step plot needs two polyline points per one actual data point
    lineData->reserve(dataCount*2+2);
    lineData->resize(dataCount*2);
  }
  if (pointData)
    pointData->resize(dataCount);
  
  // position data points:
  QMap<double, QCustomPlotData>::const_iterator it = lower;
  QCustomPlotDataMap::const_iterator upperEnd = upper+1;
  int i = 0;
  int ipoint = 0;
  if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft || mKeyAxis->axisType() == QCustomPlotAxis::atRight)
  {
    double lastValue = mValueAxis->coordToPixel(it.value().value);
    double key;
    while (it != upperEnd)
    {
      if (pointData)
      {
        (*pointData)[ipoint] = it.value();
        ++ipoint;
      }
      key = mKeyAxis->coordToPixel(it.key());
      (*lineData)[i].setX(lastValue);
      (*lineData)[i].setY(key);
      ++i;
      lastValue = mValueAxis->coordToPixel(it.value().value);
      (*lineData)[i].setX(lastValue);
      (*lineData)[i].setY(key);
      ++i;
      ++it;
    }
  } else // key axis is bottom or top
  {
    double lastValue = mValueAxis->coordToPixel(it.value().value);
    double key;
    while (it != upperEnd)
    {
      if (pointData)
      {
        (*pointData)[ipoint] = it.value();
        ++ipoint;
      }
      key = mKeyAxis->coordToPixel(it.key());
      (*lineData)[i].setX(key);
      (*lineData)[i].setY(lastValue);
      ++i;
      lastValue = mValueAxis->coordToPixel(it.value().value);
      (*lineData)[i].setX(key);
      (*lineData)[i].setY(lastValue);
      ++i;
      ++it;
    }
  }
}

/*! 
  \internal
  Places the raw data points needed for a step plot with right oriented steps in \a lineData.

  As for all plot data retrieval functions, \a pointData just contains all unaltered data (scatter)
  points that are visible, for drawing scatter points, if necessary. If drawing scatter points is
  disabled (i.e. scatter style \ref ssNone), pass 0 as \a pointData, and the function will skip
  filling the vector.
  \see drawLinePlot
*/
void QCustomPlotGraph::getStepRightPlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const
{
  // get visible data range:
  QCustomPlotDataMap::const_iterator lower, upper;
  int dataCount;
  getVisibleDataBounds(lower, upper, dataCount);
  // prepare vectors:
  if (lineData)
  {
    // added 2 to reserve memory for lower/upper fill base points that might be needed for fill
    // multiplied by 2 because step plot needs two polyline points per one actual data point
    lineData->reserve(dataCount*2+2);
    lineData->resize(dataCount*2);
  }
  if (pointData)
    pointData->resize(dataCount);
  
  // position points:
  QMap<double, QCustomPlotData>::const_iterator it = lower;
  QCustomPlotDataMap::const_iterator upperEnd = upper+1;
  int i = 0;
  int ipoint = 0;
  if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft || mKeyAxis->axisType() == QCustomPlotAxis::atRight)
  {
    double lastKey = mKeyAxis->coordToPixel(it.key());
    double value;
    while (it != upperEnd)
    {
      if (pointData)
      {
        (*pointData)[ipoint] = it.value();
        ++ipoint;
      }
      value = mValueAxis->coordToPixel(it.value().value);
      (*lineData)[i].setX(value);
      (*lineData)[i].setY(lastKey);
      ++i;
      lastKey = mKeyAxis->coordToPixel(it.key());
      (*lineData)[i].setX(value);
      (*lineData)[i].setY(lastKey);
      ++i;
      ++it;
    }
  } else // key axis is bottom or top
  {
    double lastKey = mKeyAxis->coordToPixel(it.key());
    double value;
    while (it != upperEnd)
    {
      if (pointData)
      {
        (*pointData)[ipoint] = it.value();
        ++ipoint;
      }
      value = mValueAxis->coordToPixel(it.value().value);
      (*lineData)[i].setX(lastKey);
      (*lineData)[i].setY(value);
      ++i;
      lastKey = mKeyAxis->coordToPixel(it.key());
      (*lineData)[i].setX(lastKey);
      (*lineData)[i].setY(value);
      ++i;
      ++it;
    }
  }
}

/*! 
  \internal
  Places the raw data points needed for a step plot with centered steps in \a lineData.

  As for all plot data retrieval functions, \a pointData just contains all unaltered data (scatter)
  points that are visible, for drawing scatter points, if necessary. If drawing scatter points is
  disabled (i.e. scatter style \ref ssNone), pass 0 as \a pointData, and the function will skip
  filling the vector.
  \see drawLinePlot
*/
void QCustomPlotGraph::getStepCenterPlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const
{
  // get visible data range:
  QCustomPlotDataMap::const_iterator lower, upper;
  int dataCount;
  getVisibleDataBounds(lower, upper, dataCount);
  // prepare vectors:
  if (lineData)
  {
    // added 2 to reserve memory for lower/upper fill base points that might be needed for base fill
    // multiplied by 2 because step plot needs two polyline points per one actual data point
    lineData->reserve(dataCount*2+2);
    lineData->resize(dataCount*2);
  }
  if (pointData)
    pointData->resize(dataCount);
  
  // position points:
  QCustomPlotDataMap::const_iterator it = lower;
  QCustomPlotDataMap::const_iterator upperEnd = upper+1;
  int i = 0;
  int ipoint = 0;
  if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft || mKeyAxis->axisType() == QCustomPlotAxis::atRight)
  {
    double lastKey = mKeyAxis->coordToPixel(it.key());
    double lastValue = mValueAxis->coordToPixel(it.value().value);
    double key;
    if (pointData)
    {
      (*pointData)[ipoint] = it.value();
      ++ipoint;
    }
    (*lineData)[i].setX(lastValue);
    (*lineData)[i].setY(lastKey);
    ++it;
    ++i;
    while (it != upperEnd)
    {
      if (pointData)
      {
        (*pointData)[ipoint] = it.value();
        ++ipoint;
      }
      key = (mKeyAxis->coordToPixel(it.key())-lastKey)*0.5 + lastKey;
      (*lineData)[i].setX(lastValue);
      (*lineData)[i].setY(key);
      ++i;
      lastValue = mValueAxis->coordToPixel(it.value().value);
      lastKey = mKeyAxis->coordToPixel(it.key());
      (*lineData)[i].setX(lastValue);
      (*lineData)[i].setY(key);
      ++it;
      ++i;
    }
    (*lineData)[i].setX(lastValue);
    (*lineData)[i].setY(lastKey);
  } else // key axis is bottom or top
  {
    double lastKey = mKeyAxis->coordToPixel(it.key());
    double lastValue = mValueAxis->coordToPixel(it.value().value);
    double key;
    if (pointData)
    {
      (*pointData)[ipoint] = it.value();
      ++ipoint;
    }
    (*lineData)[i].setX(lastKey);
    (*lineData)[i].setY(lastValue);
    ++it;
    ++i;
    while (it != upperEnd)
    {
      if (pointData)
      {
        (*pointData)[ipoint] = it.value();
        ++ipoint;
      }
      key = (mKeyAxis->coordToPixel(it.key())-lastKey)*0.5 + lastKey;
      (*lineData)[i].setX(key);
      (*lineData)[i].setY(lastValue);
      ++i;
      lastValue = mValueAxis->coordToPixel(it.value().value);
      lastKey = mKeyAxis->coordToPixel(it.key());
      (*lineData)[i].setX(key);
      (*lineData)[i].setY(lastValue);
      ++it;
      ++i;
    }
    (*lineData)[i].setX(lastKey);
    (*lineData)[i].setY(lastValue);
  }
}

/*! 
  \internal
  Places the raw data points needed for an impulse plot in \a lineData.

  As for all plot data retrieval functions, \a pointData just contains all unaltered data (scatter)
  points that are visible, for drawing scatter points, if necessary. If drawing scatter points is
  disabled (i.e. scatter style \ref ssNone), pass 0 as \a pointData, and the function will skip
  filling the vector.
  \see drawImpulsePlot
*/
void QCustomPlotGraph::getImpulsePlotData(QVector<QPointF> *lineData, QVector<QCustomPlotData> *pointData) const
{
  // get visible data range:
  QCustomPlotDataMap::const_iterator lower, upper;
  int dataCount;
  getVisibleDataBounds(lower, upper, dataCount);
  // prepare vectors:
  if (lineData)
  {
    // no need to reserve 2 extra points, because there is no fill for impulse plot
    lineData->resize(dataCount*2);
  }
  if (pointData)
    pointData->resize(dataCount);
  
  // position data points:
  QMap<double, QCustomPlotData>::const_iterator it = lower;
  QCustomPlotDataMap::const_iterator upperEnd = upper+1;
  int i = 0;
  int ipoint = 0;
  if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft || mKeyAxis->axisType() == QCustomPlotAxis::atRight)
  {
    double zeroPointX = mValueAxis->coordToPixel(0);
    double key;
    while (it != upperEnd)
    {
      if (pointData)
      {
        (*pointData)[ipoint] = it.value();
        ++ipoint;
      }
      key = mKeyAxis->coordToPixel(it.key());
      (*lineData)[i].setX(zeroPointX);
      (*lineData)[i].setY(key);
      ++i;
      (*lineData)[i].setX(mValueAxis->coordToPixel(it.value().value));
      (*lineData)[i].setY(key);
      ++i;
      ++it;
    }
  } else // key axis is bottom or top
  {
    double zeroPointY = mValueAxis->coordToPixel(0);
    double key;
    while (it != upperEnd)
    {
      if (pointData)
      {
        (*pointData)[ipoint] = it.value();
        ++ipoint;
      }
      key = mKeyAxis->coordToPixel(it.key());
      (*lineData)[i].setX(key);
      (*lineData)[i].setY(zeroPointY);
      ++i;
      (*lineData)[i].setX(key);
      (*lineData)[i].setY(mValueAxis->coordToPixel(it.value().value));
      ++i;
      ++it;
    }
  }
}

/*! 
  \internal
  Draws the fill of the graph with the specified brush. If the fill is a normal "base" fill, i.e.
  under the graph toward the zero-value-line, only the \a lineData is required (and two extra points
  at the zero-value-line, which are added by \ref addFillBasePoints and removed by \ref removeFillBasePoints
  after the fill drawing is done).
  
  If the fill is a channel fill between this graph and another graph (mChannelFillGraph), the more complex
  polygon is calculated with the \ref getChannelFillPolygon function.
  \see drawLinePlot
*/
void QCustomPlotGraph::drawFill(QPainter *painter, QVector<QPointF> *lineData) const
{
  if (mLineStyle == lsImpulse) return; // fill doesn't make sense for impulse plot
  if (mBrush.style() == Qt::NoBrush || mBrush.color().alpha() == 0) return;
  
  painter->setRenderHint(QPainter::Antialiasing, mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeFills));
  if (!mChannelFillGraph)
  {
    // draw base fill under graph, fill goes all the way to the zero-value-line:
    addFillBasePoints(lineData);
    painter->setPen(Qt::NoPen);
    painter->setBrush(mBrush);
    painter->drawPolygon(QPolygonF(*lineData));
    removeFillBasePoints(lineData);
  } else
  {
    // draw channel fill between this graph and mChannelFillGraph:
    painter->setPen(Qt::NoPen);
    painter->setBrush(mBrush);
    painter->drawPolygon(getChannelFillPolygon(lineData));
  }
}

/*! 
  \internal
  Draws scatter symbols at every data point passed in \a pointData. scatter symbols are independent of
  the line style and are always drawn if scatter style is not \ref ssNone. Hence, the \a pointData vector
  is outputted by all "get(...)PlotData" functions, together with the (line style dependent) line data.
  \see drawLinePlot, drawImpulsePlot
*/
void QCustomPlotGraph::drawScatterPlot(QPainter *painter, QVector<QCustomPlotData> *pointData) const
{
  /* TODO: make this more efficient
     In total, we calculate the coordToPixel transformation for the .key and .value
     three times (for polygon line, for error and for scatter). Maybe provide the
     polygon data here and make this function aware of at which increments the
     data points are positioned in the polygon (depends on line style!).
  */
  // draw error bars:
  if (mErrorType != etNone)
  {
    painter->setRenderHint(QPainter::Antialiasing, mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeErrorBars));
    painter->setPen(mErrorPen);
    if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft || mKeyAxis->axisType() == QCustomPlotAxis::atRight)
    {
      for (int i=0; i<pointData->size(); ++i)
        drawError(painter, mValueAxis->coordToPixel(pointData->at(i).value), mKeyAxis->coordToPixel(pointData->at(i).key), pointData->at(i));
    } else
    {
      for (int i=0; i<pointData->size(); ++i)
        drawError(painter, mKeyAxis->coordToPixel(pointData->at(i).key), mValueAxis->coordToPixel(pointData->at(i).value), pointData->at(i));
    }
  }
  
  // draw scatter point symbols:
  painter->setRenderHint(QPainter::Antialiasing, mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeScatters));
  painter->setPen(mPen);
  painter->setBrush(mBrush);
  if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft || mKeyAxis->axisType() == QCustomPlotAxis::atRight)
  {
    for (int i=0; i<pointData->size(); ++i)
      drawScatter(painter, mValueAxis->coordToPixel(pointData->at(i).value), mKeyAxis->coordToPixel(pointData->at(i).key), mScatterStyle);
  } else
  {
    for (int i=0; i<pointData->size(); ++i)
      drawScatter(painter, mKeyAxis->coordToPixel(pointData->at(i).key), mValueAxis->coordToPixel(pointData->at(i).value), mScatterStyle);
  }
}

/*! 
  \internal
  Draws line graphs from the provided data. It connects all points in \a lineData, which
  was created by one of the "get(...)PlotData" functions for line styles that require simple line
  connections between the point vector they create. These are for example \ref getLinePlotData, \ref
  getStepLeftPlotData, \ref getStepRightPlotData and \ref getStepCenterPlotData.
  \see drawScatterPlot, drawImpulsePlot
*/
void QCustomPlotGraph::drawLinePlot(QPainter *painter, QVector<QPointF> *lineData) const
{
  // draw line of graph:
  if (mPen.style() != Qt::NoPen && mPen.color().alpha() != 0)
  {
    painter->setRenderHint(QPainter::Antialiasing, mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeGraphs));
    painter->setPen(mPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(QPolygonF(*lineData));
  }
}

/*! 
  \internal
  Draws impulses graphs from the provided data, i.e. it connects all line pairs in \a lineData, which was
  created by \ref getImpulsePlotData.
  \see drawScatterPlot, drawLinePlot
*/
void QCustomPlotGraph::drawImpulsePlot(QPainter *painter, QVector<QPointF> *lineData) const
{
  // draw impulses:
  if (mPen.style() != Qt::NoPen && mPen.color().alpha() != 0)
  {
    painter->setRenderHint(QPainter::Antialiasing, mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeGraphs));
    painter->setPen(mPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLines(*lineData);
  }
}

/*! 
  \internal
  Called by the scatter plot drawing function (\ref drawScatterPlot) to draw single point representations at the
  pixel positions \a x and \a y in the scatter style \a style.
  
  \warning this function changes the brush of the painter for scatter styles \ref ssCircle, \ref ssDisc and \ref ssSquare
  in order to draw clear (ssSquare, ssCircle) and filled (ssDisc) shapes.
*/
void QCustomPlotGraph::drawScatter(QPainter *painter, double x, double y, ScatterStyle style) const
{
  // If you change this correction, make sure pdf exported scatters are properly centered in error bars!
  // There seems to be some kind of discrepancy for different paint devices here.
  if (style == ssCross || style == ssPlus)
  {
    x = x-0.7; // paint system correction, else, we don't get pixel exact matches (Qt problem)
    y = y-0.7; // paint system correction, else, we don't get pixel exact matches (Qt problem)
  }
  
  double w = mScatterSize/2.0;
  if (style == ssDot)
  {
    painter->drawPoint(QPointF(x, y));
  } else if (style == ssCross)
  {
    painter->drawLine(QLineF(x-w, y-w, x+w, y+w));
    painter->drawLine(QLineF(x-w, y+w, x+w, y-w));
  } else if (style == ssPlus)
  {
    painter->drawLine(QLineF(x-w, y, x+w, y));
    painter->drawLine(QLineF(x, y+w, x, y-w));
  } else if (style == ssCircle)
  {
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(x-w,y-w,mScatterSize,mScatterSize);
  } else if (style == ssDisc)
  {
    painter->setBrush(QBrush(painter->pen().color()));
    painter->drawEllipse(QPointF(x,y), w, w);
  } else if (style == ssSquare)
  {
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(x-w,y-w,mScatterSize,mScatterSize);
  } else if (style == ssStar)
  {
    painter->drawLine(QLineF(x-w, y, x+w, y));
    painter->drawLine(QLineF(x, y+w, x, y-w));
    painter->drawLine(QLineF(x-w*0.707, y-w*0.707, x+w*0.707, y+w*0.707));
    painter->drawLine(QLineF(x-w*0.707, y+w*0.707, x+w*0.707, y-w*0.707));
  } else if (style == ssTriangle)
  {
    painter->drawLine(QLineF(x-w, y+0.755*w, x+w, y+0.755*w));
    painter->drawLine(QLineF(x+w, y+0.755*w, x, y-0.977*w));
    painter->drawLine(QLineF(x, y-0.977*w, x-w, y+0.755*w));
  } else if (style == ssTriangleInverted)
  {
    painter->drawLine(QLineF(x-w, y-0.755*w, x+w, y-0.755*w));
    painter->drawLine(QLineF(x+w, y-0.755*w, x, y+0.977*w));
    painter->drawLine(QLineF(x, y+0.977*w, x-w, y-0.755*w));
  } else if (style == ssCrossSquare)
  {
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(QLineF(x-w, y-w, x+w*0.95, y+w*0.95));
    painter->drawLine(QLineF(x-w, y+w*0.95, x+w*0.95, y-w));
    painter->drawRect(x-w,y-w,mScatterSize,mScatterSize);
  } else if (style == ssPlusSquare)
  {
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(QLineF(x-w, y, x+w*0.95, y));
    painter->drawLine(QLineF(x, y+w, x, y-w));
    painter->drawRect(x-w,y-w,mScatterSize,mScatterSize);
  } else if (style == ssCrossCircle)
  {
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(QLineF(x-w*0.707, y-w*0.707, x+w*0.67, y+w*0.67));
    painter->drawLine(QLineF(x-w*0.707, y+w*0.67, x+w*0.67, y-w*0.707));
    painter->drawEllipse(x-w,y-w,mScatterSize,mScatterSize);
  } else if (style == ssPlusCircle)
  {
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(QLineF(x-w, y, x+w, y));
    painter->drawLine(QLineF(x, y+w, x, y-w));
    painter->drawEllipse(x-w,y-w,mScatterSize,mScatterSize);
  } else if (style == ssPeace)
  {
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(QLineF(x, y-w, x, y+w));
    painter->drawLine(QLineF(x, y, x-w*0.707, y+w*0.707));
    painter->drawLine(QLineF(x, y, x+w*0.707, y+w*0.707));
    painter->drawEllipse(x-w,y-w,mScatterSize,mScatterSize);
  } else if (style == ssPixmap)
  {
    painter->drawPixmap(x-mScatterPixmap.width()*0.5, y-mScatterPixmap.height()*0.5, mScatterPixmap);
    // if something in here is changed, adapt ssPixmap scatter style case in drawLegendIcon(), too
  }
}

/*! 
  \internal
  called by the scatter drawing function (\ref drawScatterPlot) to draw the error bars on one data
  point. \a x and \a y pixel positions of the data point are passed since they are already known in
  pixel coordinates in the drawing function, so we save some extra coordToPixel transforms here. \a
  data is therefore only used for the errors, not key and value.
*/
void QCustomPlotGraph::drawError(QPainter *painter, double x, double y, const QCustomPlotData &data) const
{
  double a, b; // positions of error bar bounds in pixels
  double barWidthHalf = mErrorBarSize*0.5;
  double skipSymbolMargin = mScatterSize*0.75; // pixels left blank per side, when mErrorBarSkipSymbol is true
  
  if (!mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeErrorBars))
  {
    x = x-0.9; // paint system correction, else, we don't get pixel exact matches (Qt problem)
    y = y-0.9; // paint system correction, else, we don't get pixel exact matches (Qt problem)
  }
  
  if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft || mKeyAxis->axisType() == QCustomPlotAxis::atRight)
  {
    // draw key error vertically and value error horizontally
    if (mErrorType == etKey || mErrorType == etBoth)
    {
      a = mKeyAxis->coordToPixel(data.key-data.keyErrorMinus);
      b = mKeyAxis->coordToPixel(data.key+data.keyErrorPlus);
      // draw spine:
      if (mErrorBarSkipSymbol)
      {
        if (a-y > skipSymbolMargin) // don't draw spine if error is so small it's within skipSymbolmargin
          painter->drawLine(QLineF(x, a, x, y+skipSymbolMargin));
        if (y-b > skipSymbolMargin) 
          painter->drawLine(QLineF(x, y-skipSymbolMargin, x, b));
      } else
        painter->drawLine(QLineF(x, a, x, b));
      // draw handles:
      painter->drawLine(QLineF(x-barWidthHalf, a, x+barWidthHalf, a));
      painter->drawLine(QLineF(x-barWidthHalf, b, x+barWidthHalf, b));
    }
    if (mErrorType == etValue || mErrorType == etBoth)
    {
      a = mValueAxis->coordToPixel(data.value-data.valueErrorMinus);
      b = mValueAxis->coordToPixel(data.value+data.valueErrorPlus);
      // draw spine:
      if (mErrorBarSkipSymbol)
      {
        if (x-a > skipSymbolMargin) // don't draw spine if error is so small it's within skipSymbolmargin
          painter->drawLine(QLineF(a, y, x-skipSymbolMargin, y));
        if (b-x > skipSymbolMargin)
          painter->drawLine(QLineF(x+skipSymbolMargin, y, b, y));
      } else
        painter->drawLine(QLineF(a, y, b, y));
      // draw handles:
      painter->drawLine(QLineF(a, y-barWidthHalf, a, y+barWidthHalf));
      painter->drawLine(QLineF(b, y-barWidthHalf, b, y+barWidthHalf));
    }
  } else
  {
    // draw value error vertically and key error horizontally
    if (mErrorType == etKey || mErrorType == etBoth)
    {
      a = mKeyAxis->coordToPixel(data.key-data.keyErrorMinus);
      b = mKeyAxis->coordToPixel(data.key+data.keyErrorPlus);
      // draw spine:
      if (mErrorBarSkipSymbol)
      {
        if (x-a > skipSymbolMargin) // don't draw spine if error is so small it's within skipSymbolmargin
          painter->drawLine(QLineF(a, y, x-skipSymbolMargin, y));
        if (b-x > skipSymbolMargin)
          painter->drawLine(QLineF(x+skipSymbolMargin, y, b, y));
      } else
        painter->drawLine(QLineF(a, y, b, y));
      // draw handles:
      painter->drawLine(QLineF(a, y-barWidthHalf, a, y+barWidthHalf));
      painter->drawLine(QLineF(b, y-barWidthHalf, b, y+barWidthHalf));
    }
    if (mErrorType == etValue || mErrorType == etBoth)
    {
      a = mValueAxis->coordToPixel(data.value-data.valueErrorMinus);
      b = mValueAxis->coordToPixel(data.value+data.valueErrorPlus);
      // draw spine:
      if (mErrorBarSkipSymbol)
      {
        if (a-y > skipSymbolMargin) // don't draw spine if error is so small it's within skipSymbolmargin
          painter->drawLine(QLineF(x, a, x, y+skipSymbolMargin));
        if (y-b > skipSymbolMargin)
          painter->drawLine(QLineF(x, y-skipSymbolMargin, x, b));
      } else
        painter->drawLine(QLineF(x, a, x, b));
      // draw handles:
      painter->drawLine(QLineF(x-barWidthHalf, a, x+barWidthHalf, a));
      painter->drawLine(QLineF(x-barWidthHalf, b, x+barWidthHalf, b));
    }
  }
}

/*! 
  \internal
  called by QCustomPlotLegend::draw to create a graphical representation of this graph next to the
  graph name.
  
  \param rect is the rect, in which the representation is drawn.
*/
void QCustomPlotGraph::drawLegendIcon(QPainter *painter, const QRect &rect) const
{
  painter->save();
  painter->setClipRect(rect);
  // draw fill:
  if (mBrush.style() != Qt::NoBrush)
  {
    painter->setRenderHint(QPainter::Antialiasing, mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeGraphs));
    painter->fillRect(rect.left(), rect.top()+rect.height()/2.0, rect.width(), rect.height()/3.0, mBrush);
  }
  
  // draw line vertically centered:
  if (mLineStyle != lsNone)
  {
    painter->setRenderHint(QPainter::Antialiasing, mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeGraphs));
    painter->setPen(mPen);
    painter->drawLine(rect.left(), rect.top()+rect.height()/2.0, rect.right()+5, rect.top()+rect.height()/2.0); // +5 on x2 else last segment is missing from dashed/dotted pens
  }
  
  // draw scatter symbol:
  if (mScatterStyle != ssNone)
  {
    if (mScatterStyle == ssPixmap && (mScatterPixmap.size().width() > rect.width() || mScatterPixmap.size().height() > rect.height()))
    {
      // handle pixmap scatters that are larger than legend icon rect separately.
      // We resize them and draw them manually, instead of calling drawScatter:
      QSize newSize = mScatterPixmap.size();
      newSize.scale(rect.size(), Qt::KeepAspectRatio);
      QRect targetRect;
      targetRect.setSize(newSize);
      targetRect.moveCenter(rect.center());
      bool smoothBackup = painter->testRenderHint(QPainter::SmoothPixmapTransform);
      painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
      painter->drawPixmap(targetRect, mScatterPixmap);
      painter->setRenderHint(QPainter::SmoothPixmapTransform, smoothBackup);
    } else // mScatterStype != ssPixmap
    {
      painter->setPen(mPen);
      painter->setRenderHint(QPainter::Antialiasing, mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeScatters));
      drawScatter(painter, rect.center().x()+1, rect.center().y()+1, mScatterStyle);
    }
  }
  
  painter->restore();
}

/*! 
  \internal
  called by the specific plot data generating functions "get(...)PlotData" to determine
  which data range is visible, so only that needs to be processed.
  
  \param[out] lower returns an iterator to the lowest data point that needs to be taken into account
  when plotting. Note that in order to get a clean plot all the way to the edge of the axes, \a lower
  may still be outside the visible range.
  \param[out] upper returns an iterator to the highest data point. Same as before, \a upper may also
  lie outside of the visible range.
  \param[out] count number of data points that need plotting, i.e. points between \a lower and \a upper,
  including them. This is useful for allocating the array of QPointFs in the specific drawing functions.
*/
void QCustomPlotGraph::getVisibleDataBounds(QCustomPlotDataMap::const_iterator &lower, QCustomPlotDataMap::const_iterator &upper, int &count) const
{
  // get visible data range as QMap iterators
  QCustomPlotDataMap::const_iterator lbound = mData->lowerBound(mKeyAxis->range().lower);
  QCustomPlotDataMap::const_iterator ubound = mData->upperBound(mKeyAxis->range().upper)-1;
  bool lowoutlier = lbound != mData->constBegin(); // indicates whether there exist points below axis range
  bool highoutlier = ubound+1 != mData->constEnd(); // indicates whether there exist points above axis range
  lower = (lowoutlier ? lbound-1 : lbound); // data pointrange that will be actually drawn
  upper = (highoutlier ? ubound+1 : ubound); // data pointrange that will be actually drawn
  
  // count number of points in range lower to upper (including them), so we can allocate array for them in draw functions:
  QCustomPlotDataMap::const_iterator it = lower;
  count = 1;
  while (it != upper)
  {
    ++it;
    ++count;
  }
}

/*! 
  \internal
  The line data vector generated by e.g. getLinePlotData contains only the line
  that connects the data points. If the graph needs to be filled, two additional points
  need to be added at the value-zero-line in the lower and upper key positions, the graph
  reaches. This function calculates these points and adds them to the end of \a lineData.
  Since the fill is typically drawn before the line stroke, these added points need to
  be removed again after the fill is done, with the removeFillBasePoints function.
  
  The expanding of \a lineData by two points will not cause unnecessary memory reallocations,
  because the data vector generation functions (getLinePlotData etc.) reserve two extra points
  when they allocate memory for \a lineData.
*/
void QCustomPlotGraph::addFillBasePoints(QVector<QPointF> *lineData) const
{
  // append points that close the polygon fill at the key axis:
  if (mKeyAxis->mAxisType == QCustomPlotAxis::atLeft || mKeyAxis->mAxisType == QCustomPlotAxis::atRight)
  {
    *lineData << upperFillBasePoint(lineData->last().y());
    *lineData << lowerFillBasePoint(lineData->first().y());
  } else
  {
    *lineData << upperFillBasePoint(lineData->last().x());
    *lineData << lowerFillBasePoint(lineData->first().x());
  }
}

/*! 
  \internal
  removes the two points from \a lineData that were added by addFillBasePoints.
*/
void QCustomPlotGraph::removeFillBasePoints(QVector<QPointF> *lineData) const
{
  lineData->remove(lineData->size()-2, 2);
}

/*! 
  \internal
  called by addFillBasePoints to conveniently assign the point which closes the fill
  polygon on the lower side of the zero-value-line parallel to the key axis.
  The logarithmic axis scale case is a bit special, since the zero-value-line in pixel coordinates
  is in positive or negative infinity. So this case is handled separately by just closing the
  fill polygon on the axis which lies in the direction towards the zero value.
  
  \param lowerKey pixel position of the lower key of the point. Depending on whether the key axis
  is horizontal or vertical, \a lowerKey will end up as the x or y value of the returned point,
  respectively.
  \see upperFillBasePoint, addFillBasePoints
*/
QPointF QCustomPlotGraph::lowerFillBasePoint(double lowerKey) const
{
  QPointF point;
  if (mValueAxis->mScaleType == QCustomPlotAxis::stLinear)
  {
    if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft)
    {
      point.setX(mValueAxis->coordToPixel(0));
      point.setY(lowerKey);
    } else if (mKeyAxis->axisType() == QCustomPlotAxis::atRight)
    {
      point.setX(mValueAxis->coordToPixel(0));
      point.setY(lowerKey);
    } else if (mKeyAxis->axisType() == QCustomPlotAxis::atTop)
    {
      point.setX(lowerKey);
      point.setY(mValueAxis->coordToPixel(0));
    } else if (mKeyAxis->axisType() == QCustomPlotAxis::atBottom)
    {
      point.setX(lowerKey);
      point.setY(mValueAxis->coordToPixel(0));
    }
  } else // mValueAxis->mScaleType == QCustomPlotAxis::stLogarithmic
  {
    // In logarithmic scaling we can't just draw to value zero so we just fill all the way
    // to the axis which is in the direction towards zero
    if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft || mKeyAxis->axisType() == QCustomPlotAxis::atRight)
    {
      if (mValueAxis->mRange.upper < 0) // if range is negative, zero is on opposite side of key axis
        point.setX(mKeyAxis->mAxisRect.left()+mKeyAxis->mAxisRect.width());
      else
        point.setX(mKeyAxis->mAxisRect.left());
      point.setY(lowerKey);
    } else if (mKeyAxis->axisType() == QCustomPlotAxis::atTop || mKeyAxis->axisType() == QCustomPlotAxis::atBottom)
    {
      point.setX(lowerKey);
      if (mValueAxis->mRange.upper < 0) // if range is negative, zero is on opposite side of key axis
        point.setY(mKeyAxis->mAxisRect.top());
      else
        point.setY(mKeyAxis->mAxisRect.top()+mKeyAxis->mAxisRect.height());
    }
  }
  return point;
}

/*! 
  \internal
  called by addFillBasePoints to conveniently assign the point which closes the fill
  polygon on the upper side of the zero-value-line parallel to the key axis. The logarithmic axis
  scale case is a bit special, since the zero-value-line in pixel coordinates is in positive or
  negative infinity. So this case is handled separately by just closing the fill polygon on the
  axis which lies in the direction towards the zero value.
  
  \param upperKey pixel position of the upper key of the point. Depending on whether the key axis
  is horizontal or vertical, \a upperKey will end up as the x or y value of the returned point,
  respectively.
  \see lowerFillBasePoint, addFillBasePoints
*/
QPointF QCustomPlotGraph::upperFillBasePoint(double upperKey) const
{
  QPointF point;
  if (mValueAxis->mScaleType == QCustomPlotAxis::stLinear)
  {
    if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft)
    {
      point.setX(mValueAxis->coordToPixel(0));
      point.setY(upperKey);
    } else if (mKeyAxis->axisType() == QCustomPlotAxis::atRight)
    {
      point.setX(mValueAxis->coordToPixel(0));
      point.setY(upperKey);
    } else if (mKeyAxis->axisType() == QCustomPlotAxis::atTop)
    {
      point.setX(upperKey);
      point.setY(mValueAxis->coordToPixel(0));
    } else if (mKeyAxis->axisType() == QCustomPlotAxis::atBottom)
    {
      point.setX(upperKey);
      point.setY(mValueAxis->coordToPixel(0));
    }
  } else // mValueAxis->mScaleType == QCustomPlotAxis::stLogarithmic
  {
    // In logarithmic scaling we can't just draw to value 0 so we just fill all the way
    // to the axis which is in the direction towards 0
    if (mKeyAxis->axisType() == QCustomPlotAxis::atLeft || mKeyAxis->axisType() == QCustomPlotAxis::atRight)
    {
      if (mValueAxis->mRange.upper < 0) // if range is negative, zero is on opposite side of key axis
        point.setX(mKeyAxis->mAxisRect.left()+mKeyAxis->mAxisRect.width());
      else
        point.setX(mKeyAxis->mAxisRect.left());
      point.setY(upperKey);
    } else if (mKeyAxis->axisType() == QCustomPlotAxis::atTop || mKeyAxis->axisType() == QCustomPlotAxis::atBottom)
    {
      point.setX(upperKey);
      if (mValueAxis->mRange.upper < 0) // if range is negative, zero is on opposite side of key axis
        point.setY(mKeyAxis->mAxisRect.top());
      else
        point.setY(mKeyAxis->mAxisRect.top()+mKeyAxis->mAxisRect.height());
    }
  }
  return point;
}

/*!
  \internal
  Generates the polygon needed for drawing channel fills between this graph (data passed via \a
  lineData) and the graph specified by mChannelFillGraph (data generated by calling its \ref
  getPlotData function). May return an empty polygon if the key ranges have no overlap or fill
  target graph and this graph don't have same orientation (i.e. both key axes horizontal or both
  key axes vertical). For increased performance (due to implicit sharing), keep the returned QPolygonF
  const.
*/
const QPolygonF QCustomPlotGraph::getChannelFillPolygon(const QVector<QPointF> *lineData) const
{
  if ((mChannelFillGraph->mKeyAxis->mAxisType == QCustomPlotAxis::atLeft || mChannelFillGraph->mKeyAxis->mAxisType == QCustomPlotAxis::atRight) &&
      (mKeyAxis->mAxisType == QCustomPlotAxis::atBottom || mKeyAxis->mAxisType == QCustomPlotAxis::atTop))
    return QPolygonF(); // don't have same axis orientation, can't fill that
  if ((mChannelFillGraph->mKeyAxis->mAxisType == QCustomPlotAxis::atTop || mChannelFillGraph->mKeyAxis->mAxisType == QCustomPlotAxis::atBottom) &&
      (mKeyAxis->mAxisType == QCustomPlotAxis::atLeft || mKeyAxis->mAxisType ==QCustomPlotAxis:: atRight))
    return QPolygonF(); // don't have same axis orientation, can't fill that
  
  if (lineData->isEmpty()) return QPolygonF();
  QVector<QPointF> otherData;
  mChannelFillGraph->getPlotData(&otherData, NULL);
  if (otherData.isEmpty()) return QPolygonF();
  QVector<QPointF> thisData;
  thisData.reserve(lineData->size()+otherData.size()); // because we will join both vectors at end of this function
  for (int i=0; i<lineData->size(); ++i) // don't use the vector<<(vector),  it squeezes internally, which ruins the performance tuning with reserve()
    thisData << lineData->at(i);
  
  // pointers to be able to swap them, depending which data range needs cropping:
  QVector<QPointF> *staticData = &thisData;
  QVector<QPointF> *croppedData = &otherData;
  
  // crop both vectors to ranges in which the keys overlap (which coord is key, depends on axisType):
  if (mKeyAxis->mAxisType == QCustomPlotAxis::atBottom || mKeyAxis->mAxisType == QCustomPlotAxis::atTop)
  {
    // x is key
    // crop lower bound:
    if (staticData->first().x() < croppedData->first().x()) // other one must be cropped
      qSwap(staticData, croppedData);
    int lowBound = findIndexBelowX(croppedData, staticData->first().x());
    if (lowBound == -1) return QPolygonF(); // key ranges have no overlap
    croppedData->remove(0, lowBound);
    // set lowest point of cropped data to fit exactly key position of first static data
    // point via linear interpolation:
    if (croppedData->size() < 2) return QPolygonF(); // need at least two points for interpolation
    double slope;
    if (croppedData->at(1).x()-croppedData->at(0).x() != 0)
      slope = (croppedData->at(1).y()-croppedData->at(0).y())/(croppedData->at(1).x()-croppedData->at(0).x());
    else
      slope = 0;
    (*croppedData)[0].setY(croppedData->at(0).y()+slope*(staticData->first().x()-croppedData->at(0).x()));
    (*croppedData)[0].setX(staticData->first().x());
    
    // crop upper bound:
    if (staticData->last().x() > croppedData->last().x()) // other one must be cropped
      qSwap(staticData, croppedData);
    int highBound = findIndexAboveX(croppedData, staticData->last().x());
    if (highBound == -1) return QPolygonF(); // key ranges have no overlap
    croppedData->remove(highBound+1, croppedData->size()-(highBound+1));
    // set highest point of cropped data to fit exactly key position of last static data
    // point via linear interpolation:
    if (croppedData->size() < 2) return QPolygonF(); // need at least two points for interpolation
    int li = croppedData->size()-1; // last index
    if (croppedData->at(li).x()-croppedData->at(li-1).x() != 0)
      slope = (croppedData->at(li).y()-croppedData->at(li-1).y())/(croppedData->at(li).x()-croppedData->at(li-1).x());
    else
      slope = 0;
    (*croppedData)[li].setY(croppedData->at(li-1).y()+slope*(staticData->last().x()-croppedData->at(li-1).x()));
    (*croppedData)[li].setX(staticData->last().x());
  } else
  {
    // y is key
    // similar to "x is key" but switched x,y. Further, lower/upper meaning is inverted compared to x,
    // because in pixel coordinates, y goes from top to bottom, not bottom to top like data.
    // crop lower bound:
    if (staticData->first().y() > croppedData->first().y()) // other one must be cropped
      qSwap(staticData, croppedData);
    int lowBound = findIndexAboveY(croppedData, staticData->first().y());
    if (lowBound == -1) return QPolygonF(); // key ranges have no overlap
    croppedData->remove(0, lowBound);
    // set lowest point of cropped data to fit exactly key position of first static data
    // point via linear interpolation:
    if (croppedData->size() < 2) return QPolygonF(); // need at least two points for interpolation
    double slope;
    if (croppedData->at(1).y()-croppedData->at(0).y() != 0) // avoid division by zero in step plots
      slope = (croppedData->at(1).x()-croppedData->at(0).x())/(croppedData->at(1).y()-croppedData->at(0).y());
    else
      slope = 0;
    (*croppedData)[0].setX(croppedData->at(0).x()+slope*(staticData->first().y()-croppedData->at(0).y()));
    (*croppedData)[0].setY(staticData->first().y());
    
    // crop upper bound:
    if (staticData->last().y() < croppedData->last().y()) // other one must be cropped
      qSwap(staticData, croppedData);
    int highBound = findIndexBelowY(croppedData, staticData->last().y());
    if (highBound == -1) return QPolygonF(); // key ranges have no overlap
    croppedData->remove(highBound+1, croppedData->size()-(highBound+1));
    // set highest point of cropped data to fit exactly key position of last static data
    // point via linear interpolation:
    if (croppedData->size() < 2) return QPolygonF(); // need at least two points for interpolation
    int li = croppedData->size()-1; // last index
    if (croppedData->at(li).y()-croppedData->at(li-1).y() != 0) // avoid division by zero in step plots
      slope = (croppedData->at(li).x()-croppedData->at(li-1).x())/(croppedData->at(li).y()-croppedData->at(li-1).y());
    else
      slope = 0;
    (*croppedData)[li].setX(croppedData->at(li-1).x()+slope*(staticData->last().y()-croppedData->at(li-1).y()));
    (*croppedData)[li].setY(staticData->last().y());
  }
  
  // return joined:
  for (int i=otherData.size()-1; i>=0; --i) // insert reversed, otherwise the polygon will be twisted
    thisData << otherData.at(i);
  return QPolygonF(thisData);
}

/*! 
  \internal
  Finds the smallest index of \a data, whose points x value is just above \a x.
  Assumes x values in \a data points are ordered ascending, as is the case
  when plotting with horizontal key axis.
  Used to calculate the channel fill polygon, see \ref getChannelFillPolygon.
*/
int QCustomPlotGraph::findIndexAboveX(const QVector<QPointF> *data, double x) const
{
  for (int i=data->size()-1; i>=0; --i)
  {
    if (data->at(i).x() < x)
    {
      if (i<data->size()-1)
        return i+1;
      else
        return data->size()-1;
    }
  }
  return -1;
}

/*! 
  \internal
  Finds the greatest index of \a data, whose points x value is just below \a x.
  Assumes x values in \a data points are ordered ascending, as is the case
  when plotting with horizontal key axis.
  Used to calculate the channel fill polygon, see \ref getChannelFillPolygon.
*/
int QCustomPlotGraph::findIndexBelowX(const QVector<QPointF> *data, double x) const
{
  for (int i=0; i<data->size(); ++i)
  {
    if (data->at(i).x() > x)
    {
      if (i>0)
        return i-1;
      else
        return 0;
    }
  }
  return -1;
}

/*! 
  \internal
  Finds the smallest index of \a data, whose points y value is just above \a y.
  Assumes y values in \a data points are ordered descending, as is the case
  when plotting with vertical key axis.
  Used to calculate the channel fill polygon, see \ref getChannelFillPolygon.
*/
int QCustomPlotGraph::findIndexAboveY(const QVector<QPointF> *data, double y) const
{
  for (int i=0; i<data->size(); ++i)
  {
    if (data->at(i).y() < y)
    {
      if (i>0)
        return i-1;
      else
        return 0;
    }
  }
  return -1;
}

/*! 
  \internal
  Finds the greatest index of \a data, whose points y value is just below \a y.
  Assumes y values in \a data points are ordered descending, as is the case
  when plotting with vertical key axis.
  Used to calculate the channel fill polygon, see \ref getChannelFillPolygon.
*/
int QCustomPlotGraph::findIndexBelowY(const QVector<QPointF> *data, double y) const
{
  for (int i=data->size()-1; i>=0; --i)
  {
    if (data->at(i).y() > y)
    {
      if (i<data->size()-1)
        return i+1;
      else
        return data->size()-1;
    }
  }
  return -1;
}

/*! 
  \internal
  called by \ref rescaleAxes functions to get the full data key bounds. For logarithmic plots, one
  can set \a restrictToSign to either -1 or 1 in order to restrict the returned range to that sign
  domain. When only negative range is wanted, set \a restrictToSign to -1 and all positive points
  will be ignored for range calculation. For no restriction, just set \a restrictToSign to 0
  (default).
  
  \param[out] validRange is an output parameter that indicates whether a proper range could be found or not.
  If this is false, you shouldn't use the returned range.
  \see rescaleAxes, getValueRange
*/
const QCustomPlotRange QCustomPlotGraph::getKeyRange(bool &validRange, bool includeErrors, int restrictToSign) const
{
  QCustomPlotRange range;
  bool haveLower = false;
  bool haveUpper = false;
  
  double current, currentErrorMinus, currentErrorPlus;
  
  if (restrictToSign == 0) // range may be anywhere
  {
    QCustomPlotDataMap::const_iterator it = mData->constBegin();
    while (it != mData->constEnd())
    {
      current = it.value().key;
      currentErrorMinus = (includeErrors ? it.value().keyErrorMinus : 0);
      currentErrorPlus = (includeErrors ? it.value().keyErrorPlus : 0);
      if (current-currentErrorMinus < range.lower || !haveLower)
      {
        range.lower = current-currentErrorMinus;
        haveLower = true;
      }
      if (current+currentErrorPlus > range.upper || !haveUpper)
      {
        range.upper = current+currentErrorPlus;
        haveUpper = true;
      }
      it++;
    }
  } else if (restrictToSign < 0) // range may only be in the negative sign domain
  {
    QCustomPlotDataMap::const_iterator it = mData->constBegin();
    while (it != mData->constEnd())
    {
      current = it.value().key;
      currentErrorMinus = (includeErrors ? it.value().keyErrorMinus : 0);
      currentErrorPlus = (includeErrors ? it.value().keyErrorPlus : 0);
      if ((current-currentErrorMinus < range.lower || !haveLower) && current-currentErrorMinus < 0)
      {
        range.lower = current-currentErrorMinus;
        haveLower = true;
      }
      if ((current+currentErrorPlus > range.upper || !haveUpper) && current+currentErrorPlus < 0)
      {
        range.upper = current+currentErrorPlus;
        haveUpper = true;
      }
      if (includeErrors) // in case point is in valid sign domain but errobars stretch beyond it, we still want to geht that point.
      {
        if ((current < range.lower || !haveLower) && current < 0)
        {
          range.lower = current;
          haveLower = true;
        }
        if ((current > range.upper || !haveUpper) && current < 0)
        {
          range.upper = current;
          haveUpper = true;
        }
      }
      it++;
    }
  } else if (restrictToSign > 0) // range may only be in the positive sign domain
  {
    QCustomPlotDataMap::const_iterator it = mData->constBegin();
    while (it != mData->constEnd())
    {
      current = it.value().key;
      currentErrorMinus = (includeErrors ? it.value().keyErrorMinus : 0);
      currentErrorPlus = (includeErrors ? it.value().keyErrorPlus : 0);
      if ((current-currentErrorMinus < range.lower || !haveLower) && current-currentErrorMinus > 0)
      {
        range.lower = current-currentErrorMinus;
        haveLower = true;
      }
      if ((current+currentErrorPlus > range.upper || !haveUpper) && current+currentErrorPlus > 0)
      {
        range.upper = current+currentErrorPlus;
        haveUpper = true;
      }
      if (includeErrors) // in case point is in valid sign domain but errobars stretch beyond it, we still want to get that point.
      {
        if ((current < range.lower || !haveLower) && current > 0)
        {
          range.lower = current;
          haveLower = true;
        }
        if ((current > range.upper || !haveUpper) && current > 0)
        {
          range.upper = current;
          haveUpper = true;
        }
      }
      it++;
    }
  }
  
  validRange = haveLower && haveUpper;
  return range;
}

/*! 
  \internal
  called by rescaleAxes functions to get the full data value bounds. For logarithmic plots,
  one can set \a restrictToSign to either -1 or 1 in order to restrict te returned range
  to that sign domain. E.g. when only negative range is wanted, set \a restrictToSign to -1 and
  all positive points will be ignored for range calculation.
  For no restriction, just set \a restrictToSign to 0 (default).
  \a validRange is an output parameter that indicates whether a proper range could be found or not.
  If this is false, you shouldn't use the returned range (e.g. no points).
  \see rescaleAxes, getKeyRange
*/
const QCustomPlotRange QCustomPlotGraph::getValueRange(bool &validRange, bool includeErrors, int restrictToSign) const
{
  QCustomPlotRange range;
  bool haveLower = false;
  bool haveUpper = false;
  
  double current, currentErrorMinus, currentErrorPlus;
  
  if (restrictToSign == 0) // range may be anywhere
  {
    QCustomPlotDataMap::const_iterator it = mData->constBegin();
    while (it != mData->constEnd())
    {
      current = it.value().value;
      currentErrorMinus = (includeErrors ? it.value().valueErrorMinus : 0);
      currentErrorPlus = (includeErrors ? it.value().valueErrorPlus : 0);
      if (current-currentErrorMinus < range.lower || !haveLower)
      {
        range.lower = current-currentErrorMinus;
        haveLower = true;
      }
      if (current+currentErrorPlus > range.upper || !haveUpper)
      {
        range.upper = current+currentErrorPlus;
        haveUpper = true;
      }
      it++;
    }
  } else if (restrictToSign < 0) // range may only be in the negative sign domain
  {
    QCustomPlotDataMap::const_iterator it = mData->constBegin();
    while (it != mData->constEnd())
    {
      current = it.value().value;
      currentErrorMinus = (includeErrors ? it.value().valueErrorMinus : 0);
      currentErrorPlus = (includeErrors ? it.value().valueErrorPlus : 0);
      if ((current-currentErrorMinus < range.lower || !haveLower) && current-currentErrorMinus < 0)
      {
        range.lower = current-currentErrorMinus;
        haveLower = true;
      }
      if ((current+currentErrorPlus > range.upper || !haveUpper) && current+currentErrorPlus < 0)
      {
        range.upper = current+currentErrorPlus;
        haveUpper = true;
      }
      if (includeErrors) // in case point is in valid sign domain but errobars stretch beyond it, we still want to geht that point.
      {
        if ((current < range.lower || !haveLower) && current < 0)
        {
          range.lower = current;
          haveLower = true;
        }
        if ((current > range.upper || !haveUpper) && current < 0)
        {
          range.upper = current;
          haveUpper = true;
        }
      }
      it++;
    }
  } else if (restrictToSign > 0) // range may only be in the positive sign domain
  {
    QCustomPlotDataMap::const_iterator it = mData->constBegin();
    while (it != mData->constEnd())
    {
      current = it.value().value;
      currentErrorMinus = (includeErrors ? it.value().valueErrorMinus : 0);
      currentErrorPlus = (includeErrors ? it.value().valueErrorPlus : 0);
      if ((current-currentErrorMinus < range.lower || !haveLower) && current-currentErrorMinus > 0)
      {
        range.lower = current-currentErrorMinus;
        haveLower = true;
      }
      if ((current+currentErrorPlus > range.upper || !haveUpper) && current+currentErrorPlus > 0)
      {
        range.upper = current+currentErrorPlus;
        haveUpper = true;
      }
      if (includeErrors) // in case point is in valid sign domain but errobars stretch beyond it, we still want to geht that point.
      {
        if ((current < range.lower || !haveLower) && current > 0)
        {
          range.lower = current;
          haveLower = true;
        }
        if ((current > range.upper || !haveUpper) && current > 0)
        {
          range.upper = current;
          haveUpper = true;
        }
      }
      it++;
    }
  }
  
  validRange = haveLower && haveUpper;
  return range;
}


// ================================================================================
// =================== QCustomPlotRange
// ================================================================================
/*! \class QCustomPlotRange
  Contains a \a lower and \a upper double value which represent the range an axis is encompassing.
*/

/*! 
  Minimum range size (\a upper - \a lower) the range changing functions will accept. Smaller
  intervals would cause errors due to the 11-bit exponent of double precision numbers,
  corresponding to a minimum magnitude of roughly 1e-308.
  \see validRange, maxRange
*/
const double QCustomPlotRange::minRange = 1e-280;

/*! 
  Maximum values (negative and positive) the range will accept in range-changing functions.
  Larger absolute values would cause errors due to the 11-bit exponent of double precision numbers,
  corresponding to a maximum magnitude of roughly 1e308.
  Since the number of planck-volumes in the entire visible universe is only ~1e183, this should
  be enough.
  \see validRange, minRange
*/
const double QCustomPlotRange::maxRange = 1e250;

QCustomPlotRange::QCustomPlotRange()
{
  this->lower = 0;
  this->upper = 0;
}

QCustomPlotRange::QCustomPlotRange(double lower, double upper)
{
  this->lower = lower;
  this->upper = upper;
}

/*! 
  Returns the size of the range, i.e. \a upper-\a lower
*/
double QCustomPlotRange::size() const
{
  return upper-lower;
}

/*! 
  Returns the center of the range, i.e. (\a upper-\a lower)*0.5 + \a lower
*/
double QCustomPlotRange::center() const
{
  return (upper-lower)*0.5 + lower;
}

/*! 
  Makes sure \a lower is numerically smaller than \a upper. If this is not the case, the values
  are swapped.
*/
void QCustomPlotRange::normalize()
{
  if (lower > upper)
    qSwap(lower, upper);
}

/*! 
  Returns a sanitized version of the range. Sanitized means for logarithmic scales, that
  the range won't span the positive and negative sign domain, i.e. contain zero. Further
  \a lower will always be numerically smaller (or equal) to \a upper.
  
  If the original range does span positive and negative sign domains or contains zero,
  the returned range will try to approximate the original range as good as possible.
  If the positive interval of the original range is wider than the negative interval, the
  returned range will only contain the positive interval, with lower bound set to \a rangeFac or
  \a rangeFac *\a upper, whichever is closer to zero. Same procedure is used if the negative interval
  is wider than the positive interval, this time by changing the \a upper bound.
*/
QCustomPlotRange QCustomPlotRange::sanitizedForLogScale() const
{
  double rangeFac = 1e-3;
  QCustomPlotRange sanitizedRange(lower, upper);
  sanitizedRange.normalize();
  // can't have range spanning negative and positive values in log plot, so change range to fix it
  //if (qFuzzyCompare(sanitizedRange.lower+1, 1) && !qFuzzyCompare(sanitizedRange.upper+1, 1))
  if (sanitizedRange.lower == 0.0 && sanitizedRange.upper != 0.0)
  {
    // case lower is 0
    if (rangeFac < sanitizedRange.upper*rangeFac)
      sanitizedRange.lower = rangeFac;
    else
      sanitizedRange.lower = sanitizedRange.upper*rangeFac;
  } //else if (!qFuzzyCompare(lower+1, 1) && qFuzzyCompare(upper+1, 1))
  else if (sanitizedRange.lower != 0.0 && sanitizedRange.upper == 0.0)
  {
    // case upper is 0
    if (-rangeFac > sanitizedRange.lower*rangeFac)
      sanitizedRange.upper = -rangeFac;
    else
      sanitizedRange.upper = sanitizedRange.lower*rangeFac;
  } else if (sanitizedRange.lower < 0 && sanitizedRange.upper > 0)
  {
    // find out whether negative or positive interval is wider to decide which sign domain will be chosen
    if (-sanitizedRange.lower > sanitizedRange.upper)
    {
      // negative is wider, do same as in case upper is 0
      if (-rangeFac > sanitizedRange.lower*rangeFac)
        sanitizedRange.upper = -rangeFac;
      else
        sanitizedRange.upper = sanitizedRange.lower*rangeFac;
    } else
    {
      // positive is wider, do same as in case lower is 0
      if (rangeFac < sanitizedRange.upper*rangeFac)
        sanitizedRange.lower = rangeFac;
      else
        sanitizedRange.lower = sanitizedRange.upper*rangeFac;
    }
  }
  // due to normalization, case lower>0 && upper<0 should never occur, because that implies upper<lower
  return sanitizedRange;
}

/*! 
  Returns a sanitized version of the range. Sanitized means for linear scales, that
  \a lower will always be numerically smaller (or equal) to \a upper.
*/
QCustomPlotRange QCustomPlotRange::sanitizedForLinScale() const
{
  QCustomPlotRange sanitizedRange(lower, upper);
  sanitizedRange.normalize();
  return sanitizedRange;
}

/*! 
  Checks, whether the specified range is within valid bounds, which are defined
  as QCustomPlotRange::maxRange and QCustomPlotRange::minRange.
  A valid range means:
  \li range bounds within -maxRange and maxRange
  \li range size above minRange
  \li range size below maxRange
*/
bool QCustomPlotRange::validRange(double lower, double upper)
{
  /*
  return (lower > -maxRange &&
          upper < maxRange &&
          fabs(lower-upper) > minRange &&
          (lower < -minRange || lower > minRange) &&
          (upper < -minRange || upper > minRange));
          */
  return (lower > -maxRange &&
          upper < maxRange &&
          fabs(lower-upper) > minRange &&
          fabs(lower-upper) < maxRange);
}

/*! 
  Checks, whether the specified range is within valid bounds, which are defined
  as QCustomPlotRange::maxRange and QCustomPlotRange::minRange.
  A valid range means:
  \li range bounds within -maxRange and maxRange
  \li range size above minRange
  \li range size below maxRange
*/
bool QCustomPlotRange::validRange(const QCustomPlotRange &range)
{
  /*
  return (range.lower > -maxRange &&
          range.upper < maxRange &&
          fabs(range.lower-range.upper) > minRange &&
          fabs(range.lower-range.upper) < maxRange &&
          (range.lower < -minRange || range.lower > minRange) &&
          (range.upper < -minRange || range.upper > minRange));
          */
  return (range.lower > -maxRange &&
          range.upper < maxRange &&
          fabs(range.lower-range.upper) > minRange &&
          fabs(range.lower-range.upper) < maxRange);
}


// ================================================================================
// =================== QCustomPlotLegend
// ================================================================================

/*! \class QCustomPlotLegend
  Manages a legend inside a QCustomPlot. Doesn't need to be instantiated
  externally, rather access QCustomPlot::legend
*/

/*!
  Constructs a new QCustomPlotLegend instance with \a parentPlot as the containing plot and default
  values. Under normal usage, QCustomPlotLegend needn't be instantiated outside of QCustomPlot.
  Access QCustomPlot::legend to modify the legend (set to invisible by default, see \ref
  setVisible).
*/
QCustomPlotLegend::QCustomPlotLegend(QCustomPlot *parentPlot)
{
  mParentPlot = parentPlot;
  setVisible(true);
  setBorderPen(QPen(Qt::black));
  setIconBorderPen(Qt::NoPen);
  setBrush(QBrush(Qt::white));
  setFont(parentPlot->font());
  setPositionStyle(psTopRight);
  setSize(200,100);
  setAutoSize(true);
  
  setMargin(12, 12, 12, 12);
  setPadding(8, 8, 3, 3);
  setIconSize(32,18);
  setItemSpacing(3);
  setIconTextPadding(7);
}

QCustomPlotLegend::~QCustomPlotLegend()
{
  
}

/*!
  Sets the pen, the border of the entire legend is drawn with.
*/
void QCustomPlotLegend::setBorderPen(const QPen &pen)
{
  mBorderPen = pen;
}

/*!
  Sets the brush of the legend background.
*/
void QCustomPlotLegend::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

/*!
  Sets the font used for the legend text describing the graphs, i.e. their QCustomPlotGraph::name()
  properties.
*/
void QCustomPlotLegend::setFont(const QFont &font)
{
  mFont = font;
}

/*!
  Sets the position style of the legend. If the \a legendPositionStyle is not \ref psManual, the
  position is found automatically depending on the specific \a legendPositionStyle and the
  legend margins. If \a legendPositionStyle is \ref psManual, the exact pixel position of the
  legend must be specified via \ref setPosition. Margins have no effect in that case.
  \see setMargin
*/
void QCustomPlotLegend::setPositionStyle(PositionStyle legendPositionStyle)
{
  mPositionStyle = legendPositionStyle;
}

/*!
  Sets the exact pixel Position of the legend inside the QCustomPlot widget, if \ref
  setPositionStyle is set to \ref psManual. Margins have no effect in that case.
*/
void QCustomPlotLegend::setPosition(const QPoint &pixelPosition)
{
  mPosition = pixelPosition;
}

/*!
  Sets whether the size of the legend should be calculated automatically to fit all the content
  (plus padding), or whether the size must be specified manually with \ref setSize.
*/
void QCustomPlotLegend::setAutoSize(bool on)
{
  mAutoSize = on;
}

/*!
  Sets the size of the legend. Setting the size manually with this function only has an effect, if
  \ref setAutoSize is set to false.
*/
void QCustomPlotLegend::setSize(const QSize &size)
{
  mSize = size;
}

/*!
  Sets the size of the legend. Setting the size manually with this function only has an effect, if
  \ref setAutoSize is set to false.
*/
void QCustomPlotLegend::setSize(int width, int height)
{
  mSize.setWidth(width);
  mSize.setHeight(height);
}

/*!
  Sets the visibility of the legend.
*/
void QCustomPlotLegend::setVisible(bool on)
{
  mVisible = on;
}

/*!
  Sets the left padding of the legend. Padding is the space by what the legend box is made larger
  than minimally needed for the content to fit. I.e. it's the space left blank on each side inside
  the legend.
*/
void QCustomPlotLegend::setPaddingLeft(int padding)
{
  mPaddingLeft = padding;
}

/*!
  Sets the right padding of the legend. Padding is the space by what the legend box is made larger
  than minimally needed for the content to fit. I.e. it's the space left blank on each side inside
  the legend.
*/
void QCustomPlotLegend::setPaddingRight(int padding)
{
  mPaddingRight = padding;
}

/*!
  Sets the top padding of the legend. Padding is the space by what the legend box is made larger
  than minimally needed for the content to fit. I.e. it's the space left blank on each side inside
  the legend.
*/
void QCustomPlotLegend::setPaddingTop(int padding)
{
  mPaddingTop = padding;
}

/*!
  Sets the bottom padding of the legend. Padding is the space by what the legend box is made larger
  than minimally needed for the content to fit. I.e. it's the space left blank on each side inside
  the legend.
*/
void QCustomPlotLegend::setPaddingBottom(int padding)
{
  mPaddingBottom = padding;
}

/*!
  Sets the padding of the legend. Padding is the space by what the legend box is made larger than
  minimally needed for the content to fit. I.e. it's the space left blank on each side inside the
  legend.
*/
void QCustomPlotLegend::setPadding(int left, int right, int top, int bottom)
{
  mPaddingLeft = left;
  mPaddingRight = right;
  mPaddingTop = top;
  mPaddingBottom = bottom;
}

/*!
  Sets the left margin of the legend. Margins are the distances the legend will keep to the axis
  rect, when \ref setPositionStyle is not \ref psManual.
*/
void QCustomPlotLegend::setMarginLeft(int margin)
{
  mMarginLeft = margin;
}

/*!
  Sets the right margin of the legend. Margins are the distances the legend will keep to the axis
  rect, when \ref setPositionStyle is not \ref psManual.
*/
void QCustomPlotLegend::setMarginRight(int margin)
{
  mMarginRight = margin;
}

/*!
  Sets the top margin of the legend. Margins are the distances the legend will keep to the axis
  rect, when \ref setPositionStyle is not \ref psManual.
*/
void QCustomPlotLegend::setMarginTop(int margin)
{
  mMarginTop = margin;
}

/*!
  Sets the bottom margin of the legend. Margins are the distances the legend will keep to the axis
  rect, when \ref setPositionStyle is not \ref psManual.
*/
void QCustomPlotLegend::setMarginBottom(int margin)
{
  mMarginBottom = margin;
}

/*!
  Sets the margin of the legend. Margins are the distances the legend will keep to the axis rect,
  when \ref setPositionStyle is not \ref psManual.
*/
void QCustomPlotLegend::setMargin(int left, int right, int top, int bottom)
{
  mMarginLeft = left;
  mMarginRight = right;
  mMarginTop = top;
  mMarginBottom = bottom;
}

/*!
  Sets the vertical space between two graph entries in the legend.
  \see setIconTextPadding, setMargin
*/
void QCustomPlotLegend::setItemSpacing(int spacing)
{
  mItemSpacing = spacing;
}

/*!
  Sets the size of the legend icons representing the visual appearance of the graph.
*/
void QCustomPlotLegend::setIconSize(const QSize &size)
{
  mIconSize = size;
}

/*!
  Sets the size of the legend icons representing the visual appearance of the graph.
*/
void QCustomPlotLegend::setIconSize(int width, int height)
{
  mIconSize.setWidth(width);
  mIconSize.setHeight(height);
}

/*!
  Sets the horizontal space between the icon and the text of a graph entry in the legend.
  \see setItemSpacing, setMargin
*/
void QCustomPlotLegend::setIconTextPadding(int padding)
{
  mIconTextPadding = padding;
}

/*!
  Sets the pen, the border of each single graph icon is drawn with.
*/
void QCustomPlotLegend::setIconBorderPen(const QPen &pen)
{
  mIconBorderPen = pen;
}

/*!
  Returns the graph with index \a i. This index is legend-internal and has no connection
  to the graph index in QCustomPlot::graph().
  \see graphCount
*/
QCustomPlotGraph *QCustomPlotLegend::graph(int i) const
{
  if (i > -1 && i < mGraphs.size())
    return mGraphs[i];
  else
    return NULL;
}

/*!
  Returns the number of graphs currently in the legend.
  \see graph
*/
int QCustomPlotLegend::graphCount() const
{
  return mGraphs.size();
}

/*!
  Returns whether the legend contains \a graph.
*/
bool QCustomPlotLegend::hasGraph(QCustomPlotGraph *graph) const
{
  return mGraphs.contains(graph);
}

/*!
  Adds \a graph to the legend, if it's not present already.
*/
void QCustomPlotLegend::addGraph(QCustomPlotGraph *graph)
{
  // only add graphs that aren't in the legend already:
  if (!mGraphs.contains(graph))
    mGraphs.append(graph);
}

/*!
  Sets the graphs in \a graphList as the graphs of the legend. No duplicate checking is performed.
*/
void QCustomPlotLegend::setGraphs(const QList<QCustomPlotGraph*> &graphList)
{
  mGraphs.clear();
  mGraphs.append(graphList);
}

/*!
  Adds the graphs in \a graphList to the legend. Graphs that are in the legend already
  are skipped.
*/
void QCustomPlotLegend::addGraphs(const QList<QCustomPlotGraph*> &graphList)
{
  // only add graphs that aren't in the legend already:
  for (int i=0; i<graphList.count(); ++i)
  {
    if (!mGraphs.contains(graphList.at(i)))
      mGraphs.append(graphList.at(i));
  }
}

/*!
  Removes \a graph from the legend. Returns true, if successfull.
*/
bool QCustomPlotLegend::removeGraph(QCustomPlotGraph *graph)
{
  return mGraphs.removeOne(graph);
}

/*!
  Removes all graphs in \a graphList from the legend.
*/
void QCustomPlotLegend::removeGraphs(const QList<QCustomPlotGraph*> &graphList)
{
  for (int i=0; i<graphList.count(); ++i)
  {
    if (!mGraphs.contains(graphList.at(i)))
      mGraphs.removeOne(graphList.at(i));
  }
}

/*!
  If \a setAutoSize is true, the size needed to fit all legend contents is calculated and applied.
  Finally, the automatic positioning of the legend is performed, depending on the \ref
  setPositionStyle setting.
*/
void  QCustomPlotLegend::reArrange()
{
  if (mAutoSize)
  {
    calculateAutoSize();
  }
  calculateAutoPosition();
}

/*!
  \internal
  Draws the legend with the provided \a painter.
*/
void QCustomPlotLegend::draw(QPainter *painter)
{
  if (!mVisible) return;
  painter->save();
  
  painter->setBrush(mBrush);
  painter->setPen(mBorderPen);
  painter->setFont(mFont);
  // draw background rect:
  painter->drawRect(QRect(mPosition, mSize));
  painter->setClipRect(QRect(mPosition, mSize).adjusted(1, 1, 0, 0));
  
  painter->setPen(QPen());
  painter->setBrush(Qt::NoBrush);
  // draw legend items:
  int currentTop = mPosition.y()+mPaddingTop;
  int yOffset = 0;
  for (int i=0; i<mGraphs.size(); ++i)
  {
    QRect textRect = painter->fontMetrics().boundingRect(0, 0, 0, mIconSize.height(), Qt::TextDontClip, mGraphs.at(i)->name());
    QRect iconRect(QPoint(mPosition.x()+mPaddingLeft, currentTop), mIconSize);
    // line height smaller than icon height, offset text downward so first line is vertically centered with icon:
    yOffset = 0;
    if (painter->fontMetrics().height() < mIconSize.height())
      yOffset = (mIconSize.height()-painter->fontMetrics().height())/2.0;
    else // icon height smaller than line height, center icon with first line:
      iconRect.translate(0, (painter->fontMetrics().height()-mIconSize.height())/2.0);
    if (mIconBorderPen.style() != Qt::NoPen)
    {
      painter->setPen(mIconBorderPen);
      painter->drawRect(iconRect);
      painter->setPen(QPen());
    }
    mGraphs.at(i)->drawLegendIcon(painter, iconRect);
    painter->drawText(mPosition.x()+mPaddingLeft+mIconSize.width()+mIconTextPadding, currentTop+yOffset, 0, mIconSize.height(), Qt::TextDontClip, mGraphs.at(i)->name());
    currentTop += qMax(textRect.height()+yOffset, mIconSize.height())+mItemSpacing;
  }
  
  painter->restore();
}

/*!
  \internal
  Goes through similar steps as \ref draw and calculates the width and height needed to
  fit all text lines and padding etc. in the legend. The new calculated size is then applied to the
  mSize of this QCustomPlotLegend.
*/
void QCustomPlotLegend::calculateAutoSize()
{
  QFontMetrics fontMetrics(mFont);
  int width = mPaddingLeft+mIconSize.width()+mPaddingRight;
  int currentTop = mPaddingTop;
  int yOffset = 0;
  // width and height of legend items:
  for (int i=0; i<mGraphs.size(); ++i)
  {
    QRect textRect = fontMetrics.boundingRect(0, 0, 0, mIconSize.height(), Qt::TextDontClip, mGraphs.at(i)->name());
    if (width < mPaddingLeft+mIconSize.width()+mIconTextPadding+textRect.width()+mPaddingRight)
      width = mPaddingLeft+mIconSize.width()+mIconTextPadding+textRect.width()+mPaddingRight;
    // line height smaller than icon height, offset text downward so first line is vertically centered with icon:
    yOffset = 0;
    if (fontMetrics.height() < mIconSize.height())
      yOffset = (mIconSize.height()-fontMetrics.height())/2.0;
    currentTop += qMax(textRect.height()+yOffset, mIconSize.height())+mItemSpacing;
  }
  if (mGraphs.size() > 0)
    currentTop -= mItemSpacing;
  currentTop += mPaddingBottom;
  
  mSize.setWidth(width);
  mSize.setHeight(currentTop);
}

/*!
  \internal
  Sets the position dependant on the \ref setPositionStyle setting and the margins.
*/
void QCustomPlotLegend::calculateAutoPosition()
{
  if (mPositionStyle == psTopLeft)
  {
    mPosition = mParentPlot->mAxisRect.topLeft() + QPoint(mMarginLeft, mMarginTop);
  } else if (mPositionStyle == psTop)
  {
    mPosition = mParentPlot->mAxisRect.topLeft() + QPoint(mParentPlot->mAxisRect.width()/2.0-mSize.width()/2.0, mMarginTop);
  } else if (mPositionStyle == psTopRight)
  {
    mPosition = mParentPlot->mAxisRect.topRight() + QPoint(-mMarginRight-mSize.width(), mMarginTop);
  } else if (mPositionStyle == psRight)
  {
    mPosition = mParentPlot->mAxisRect.topRight() + QPoint(-mMarginRight-mSize.width(), mParentPlot->mAxisRect.height()/2.0-mSize.height()/2.0);
  } else if (mPositionStyle == psBottomRight)
  {
    mPosition = mParentPlot->mAxisRect.bottomRight() + QPoint(-mMarginRight-mSize.width(), -mMarginBottom-mSize.height());
  } else if (mPositionStyle == psBottom)
  {
    mPosition = mParentPlot->mAxisRect.bottomLeft() + QPoint(mParentPlot->mAxisRect.width()/2.0-mSize.width()/2.0, -mMarginBottom-mSize.height());
  } else if (mPositionStyle == psBottomLeft)
  {
    mPosition = mParentPlot->mAxisRect.bottomLeft() + QPoint(mMarginLeft, -mMarginBottom-mSize.height());
  } else if (mPositionStyle == psLeft)
  {
    mPosition = mParentPlot->mAxisRect.topLeft() + QPoint(mMarginLeft, mParentPlot->mAxisRect.height()/2.0-mSize.height()/2.0);
  }
}


// ================================================================================
// =================== QCustomPlotAxis
// ================================================================================

/*! \class QCustomPlotAxis
  Manages a single axis inside a QCustomPlot. Usually doesn't need to be
  instantiated externally. Access %QCustomPlot's axes via QCustomPlot::xAxis (bottom),
  QCustomPlot::yAxis (left), QCustomPlot::xAxis2 (top) and QCustomPlot::yAxis2 (right).
*/

/*!
  Constructs an Axis instance of Type \a type inside \a parentPlot.
*/
QCustomPlotAxis::QCustomPlotAxis(QCustomPlot *parentPlot, AxisType type)
{
  mParentPlot = parentPlot;
  mTickVector = new QVector<double>;
  mSubTickVector = new QVector<double>;
  mTickVectorLabels = new QVector<QString>;
  setDataFetcher(NULL);
  setAxisType(type);
  setAxisRect(parentPlot->axisRect()); 
  setScaleType(stLinear);
  setScaleLogBase(10);
  
  setVisible(true);
  setRange(0, 5);
  
  setTicks(true);
  setTickStep(1);
  setAutoTickCount(6);
  setAutoTicks(true);
  setAutoTickLabels(true);
  setAutoTickStep(true);
  setTickLabelFont(parentPlot->font());
  setTickLength(5);
  setTickPen(QPen(Qt::black));
  setTickLabels(true);
  setTickLabelType(ltNumber);
  setTickLabelRotation(0);
  setDateTimeFormat("hh:mm:ss\ndd.MM.yy");
  setNumberFormat("gbd");
  setNumberPrecision(6);
  setLabel("");
  setLabelFont(parentPlot->font());
  
  setAutoSubTicks(true);
  setSubTickCount(4);
  setSubTickLength(2);
  setSubTickPen(QPen(Qt::black));
  
  QPen gpen;
  gpen.setColor(QColor(200,200,200));
  gpen.setStyle(Qt::DotLine);
  setGridPen(gpen);
  setGrid(true);
  QPen subgpen;
  subgpen.setColor(QColor(220,220,220));
  subgpen.setStyle(Qt::DotLine);
  setSubGridPen(subgpen);
  setSubGrid(false);
  setBasePen(QPen(Qt::black));

  if (type == atTop)
  {
    setTickLabelPadding(3);
    setLabelPadding(3);
  } else if (type == atRight)
  {
    setTickLabelPadding(7);
    setLabelPadding(12);
  } else if (type == atBottom)
  {
    setTickLabelPadding(3);
    setLabelPadding(3);
  } else if (type == atLeft)
  {
    setTickLabelPadding(5);
    setLabelPadding(10);
  }
}

QCustomPlotAxis::~QCustomPlotAxis()
{
  delete mTickVector;
  delete mTickVectorLabels;
  delete mSubTickVector;
}

/*!
  Returns the number Format.
  \see setNumberFormat
*/
QString QCustomPlotAxis::numberFormat() const
{
  QString result;
  result.append(mNumberFormatChar);
  if (mNumberBeautifulPowers)
  {
    result.append("b");
    if (mNumberMultiplyCross)
      result.append("c");
  }
  return result;
}

/*!
  Sets a subclassed QCustomPlotDataFetcher to achieve a more direct connection to the internal
  plotting mechanism. QCustomPlot calls functions of QCustomPlotDataFetcher to get information
  about what is to be displayed at the current axis ranges.
  \todo This feature isn't implemented
  yet, in fact, it's concept isn't even fully developed. However, different setData, addData and
  removeData functions of QCustomPlotGraph seem to give a sufficient interface, even for real time
  purposes.
  See the "Performance Tweaks" section on the main documentation page for tips how to increase
  performance.
*/
void QCustomPlotAxis::setDataFetcher(QCustomPlotDataFetcher *fetcher)
{
  mDataFetcher = fetcher;
}

/*!
  \internal
  Sets the axis type. Together with the current axis rect (see \ref setAxisRect), this determines
  the orientation and position of the axis. Depending on \a type, ticks, tick labels, and label are
  drawn on corresponding sides of the axis base line.
*/
void QCustomPlotAxis::setAxisType(AxisType type)
{
  mAxisType = type;
}

/*!
  \internal
  Sets the axis rect. The axis uses this rect to position itself within the plot,
  together with the information of its type (\ref setAxisType). Theoretically it's possible to give
  a plot's axes different axis rects (e.g. for gaps between them), however, they are currently all
  synchronized by the QCustomPlot::setAxisRect function.
*/
void QCustomPlotAxis::setAxisRect(const QRect &rect)
{
  mAxisRect = rect;
}

/*!
  Sets whether the axis uses a linear scale or a logarithmic scale. If \a type is set to \ref
  stLogarithmic, the logarithm base can be set with \ref setScaleLogBase. In logarithmic axis
  scaling, major tick marks appear at all powers of the logarithm base. Properties like tick step
  (\ret setTickStep) don't apply in logarithmic scaling. If you wish a decimal base but less major
  ticks, consider choosing a logarithm base of 100, 1000 or even higher.
  
  If \a type is \ref stLogarithmic and the number format (\ref setNumberFormat) uses the 'b' option
  (beautifully typeset decimal powers), the display usually is "1 [multiplication sign] 10
  [superscript] n", which looks unnatural for logarithmic scaling (the "1 [multiplication sign]"
  part). To only display the decimal power, set the number precision to zero with
  \ref setNumberPrecision.
*/
void QCustomPlotAxis::setScaleType(ScaleType type)
{
  mScaleType = type;
  if (mScaleType == stLogarithmic)
    mRange = mRange.sanitizedForLogScale();
}

/*!
  If \ref setScaleType is set to \ref stLogarithmic, \a base will be the logarithm base of the
  scaling. In logarithmic axis scaling, major tick marks appear at all powers of \a base.
  
  Properties like tick step (\ref setTickStep) don't apply in logarithmic scaling. If you wish a decimal base but
  less major ticks, consider choosing \a base 100, 1000 or even higher.
*/
void QCustomPlotAxis::setScaleLogBase(double base)
{
  if (base > 1)
  {
    mScaleLogBase = base;
    mScaleLogBaseLogInv = 1.0/log(mScaleLogBase); // buffer for faster baseLog() calculation
  } else
    qDebug() << "Invalid logarithmic scale base:" << base;
}

/*!
  Sets the range of the axis.
  
  This slot may be connected with the \ref rangeChanged signal of another axis in order for the
  ranges to be synchronized (in one direction).
*/
void QCustomPlotAxis::setRange(const QCustomPlotRange &range)
{
  if (!QCustomPlotRange::validRange(range)) return;
  if (mScaleType == stLogarithmic)
  {
    mRange = range.sanitizedForLogScale();
  } else
  {
    mRange = range.sanitizedForLinScale();
  }
  emit rangeChanged(mRange);
}

/*!
  Sets the lower and upper bound of the axis range.
*/
void QCustomPlotAxis::setRange(double lower, double upper)
{
  if (!QCustomPlotRange::validRange(lower, upper)) return;
  mRange.lower = lower;
  mRange.upper = upper;
  if (mScaleType == stLogarithmic)
  {
    mRange = mRange.sanitizedForLogScale();
  } else
  {
    mRange = mRange.sanitizedForLinScale();
  }
  emit rangeChanged(mRange);
}

/*!
  Sets the range of the axis.

  \param position the \a position coordinate indicates together with the \a alignment parameter, where
  the new range will be positioned.
  \param size defines the size (upper-lower) of the new axis range.
  \param alignment determines how \a position is to be interpreted.\n
  If \a alignment is Qt::AlignLeft, \a position will be the lower bound of the range.\n
  If \a alignment is Qt::AlignRight, \a position will be the upper bound of the range.\n
  If \a alignment is Qt::AlignCenter, the new range will be centered around \a position.\n
  Any other values for \a alignment will default to Qt::AlignCenter.
*/
void QCustomPlotAxis::setRange(double position, double size, Qt::AlignmentFlag alignment)
{
  if (alignment == Qt::AlignLeft)
    setRange(position, position+size);
  else if (alignment == Qt::AlignRight)
    setRange(position-size, position);
  else // alignment == Qt::AlignCenter
    setRange(position-size/2.0, position+size/2.0);
}

/*!
  Sets the lower bound of the axis range, independently of the upper bound.
  \see setRange
*/
void QCustomPlotAxis::setRangeLower(double lower)
{
  if (!QCustomPlotRange::validRange(lower, 0)) return;
  mRange.lower = lower;
  if (mScaleType == stLogarithmic)
  {
    mRange = mRange.sanitizedForLogScale();
  } else
  {
    mRange = mRange.sanitizedForLinScale();
  }
  emit rangeChanged(mRange);
}

/*!
  Sets the upper bound of the axis range, independently of the lower bound.
  \see setRange
*/
void QCustomPlotAxis::setRangeUpper(double upper)
{
  if (!QCustomPlotRange::validRange(0, upper)) return;
  mRange.upper = upper;
  if (mScaleType == stLogarithmic)
  {
    mRange = mRange.sanitizedForLogScale();
  } else
  {
    mRange = mRange.sanitizedForLinScale();
  }
  emit rangeChanged(mRange);
}

/*!
  Sets whether the axis (including all its decoration, i.e. labels and grid lines) is visible.
  An invisible axis doesn't mean a non functional axis. Graphs attached to an invisible axis can still
  be plotted/moved/zoomed normally as if the axis was visible.
*/
void QCustomPlotAxis::setVisible(bool on)
{
  mVisible = on;
}

/*!
  Sets whether the grid lines are visible.
  \see setSubGrid, setGridPen
*/
void QCustomPlotAxis::setGrid(bool show)
{
  mGrid = show;
}

/*!
  Sets whether the sub grid lines are visible.
  \see setGrid, setSubGridPen
*/
void QCustomPlotAxis::setSubGrid(bool show)
{
  mSubGrid = show;
}

/*!
  Sets whether the tick positions should be calculated automatically (either from an automatically
  generated tick step or a tick step provided manually via \ref setTickStep, see \ref setAutoTickStep).
  
  If \a on is set to false, you must provide the tick positions manually via \ref setTickVector.
  For these manual ticks you may let QCustomPlotAxis generate the appropriate labels automatically
  by setting/leaving \ref setAutoTickLabels true. If you also wish to control the displayed labels
  manually, set \ref setAutoTickLabels to false and provide the label strings with \ref setTickVectorLabels.
*/
void QCustomPlotAxis::setAutoTicks(bool on)
{
  mAutoTicks = on;
}

/*!
  When \ref setAutoTickStep is true, \a approximateCount determines how many ticks should be generated
  in the visible range approximately.
*/
void QCustomPlotAxis::setAutoTickCount(int approximateCount)
{
  mAutoTickCount = approximateCount;
}

/*!
  Sets whether the tick labels are generated automatically depending on the tick label type
  (\ref ltNumber or \ltDateTime).
  
  If \a on is set to false, you should provide the tick labels via \ref setTickVectorLabels.
  (This is usually used in a combination with \ref setAutoTicks set to false for complete control over
   tick positions and labels, e.g. when the ticks should be at multiples of pi and show n*pi as tick labels.)
*/
void QCustomPlotAxis::setAutoTickLabels(bool on)
{
  mAutoTickLabels = on;
}

/*!
  Sets whether the tick step, i.e. the interval between two (major) ticks, is calculated
  automatically. If \a on is set to true, the axis finds a tick step that is reasonable for human
  readable plots. This means the tick step mantissa is chosen such that it's either a multiple of
  two or ends in 0.5. The number of ticks the algorithm aims for within the visible range can be
  set with \ref setAutoTickCount. It's not guaranteed that this number of ticks is met exactly, but
  approximately within a tolerance of two or three.
  
  If \a on is set to false, you may set the tick step manually with \ref setTickStep.
*/
void QCustomPlotAxis::setAutoTickStep(bool on)
{
  mAutoTickStep = on;
}

/*!
  Sets whether then number of sub ticks in one tick interval is determined automatically.
  This works, as long as the tick step mantissa is a multiple of 0.5 (which it is, when
  \ref setAutoTickStep is enabled).\n
  When \a on is set to false, you may set the sub tick count with \ref setSubTickCount manually.
*/
void QCustomPlotAxis::setAutoSubTicks(bool on)
{
  mAutoSubTicks = on;
}

/*!
  Sets whether tick marks are displayed. Setting \a show to false does not imply, that tick labels
  are invisible, too. To achieve that, see \ref setTickLabels.
*/
void QCustomPlotAxis::setTicks(bool show)
{
  mTicks = show;
}

/*!
  Sets whether tick labels are displayed.
*/
void QCustomPlotAxis::setTickLabels(bool show)
{
  mTickLabels = show;
}

/*!
  Sets the distance between the axis base line and the tick labels.
*/
void QCustomPlotAxis::setTickLabelPadding(int padding)
{
  mTickLabelPadding = padding;
}

/*!
  Sets whether the tick labels display numbers or dates/times.\n
  If \a type is set to \ref ltNumber, the format specifications of \ref setNumberFormat apply.\n
  If \a type is set to \ref ltDateTime, the format specifications of \ref setDateTimeFormat apply.\n
  In QCustomPlot, date/time coordinates are double numbers representing the seconds since 1970-01-01T00:00:00 UTC.
  This format can be retrieved from QDateTime objects with the QDateTime::toTime_t() function. Since this
  only gives a resolution of one second, there is also the QDateTime::toMSecsSinceEpoch() function which
  returns the timespan described above in milliseconds. Divide its return value by 1000.0 to get a value with
  the format needed for date/time plotting, this time with a resolution of one millisecond.
*/
void QCustomPlotAxis::setTickLabelType(LabelType type)
{
  mTickLabelType = type;
}

/*!
  Sets the font of the tick labels, i.e. the numbers drawn next to tick marks.
*/
void QCustomPlotAxis::setTickLabelFont(const QFont &font)
{
  mTickLabelFont = font;
}

/*!
  Sets the rotation of the tick labels, i.e. the numbers drawn next to tick marks. If \a degrees
  is zero, the labels are drawn normally. Else, the tick labels are drawn rotated by \a degrees
  clockwise. The specified angle is bound to values from -90 to 90 degrees.
*/
void QCustomPlotAxis::setTickLabelRotation(double degrees)
{
  mTickLabelRotation = qBound(-90.0, degrees, 90.0);
}

/*!
  Sets the format in which dates and times are displayed as tick labels, if \ref setTickLabelType is \ref ltDateTime.
  for details about the \a format string, see the documentation of QDateTime::toString().
  Newlines can be inserted with "\n".
*/
void QCustomPlotAxis::setDateTimeFormat(const QString &format)
{
  mDateTimeFormat = format;
}

/*!
  Sets the number format for the numbers drawn as tick labels (if tick label type is \ref
  ltNumber). This \a formatCode is an extended version of the format code used e.g. by
  QString::number() and QLocale::toString(). For reference about that, see the "Argument Formats"
  section in the detailed description of the QString class. \a formatCode is a string of one, two
  or three characters. The first character is identical to the normal format code used by Qt. In
  short, this means: 'e'/'E' scientific format, 'f' fixed format, 'g'/'G' scientific or fixed,
  whichever is shorter.
  
  The second and third characters are optional and specific to QCustomPlot:\n
  If the first char was 'e' or 'g', numbers are/might be displayed in the scientific format, e.g.
  "5.5e9", which is ugly in a plot. So when the second char of \a formatCode is set to 'b' (for
  "beautiful"), those exponential numbers are formatted in a more natural way, i.e. "5.5
  [multiplication sign] 10 [superscript] 9". By default, the multiplication sign is a centered dot.
  If instead a cross should be shown (as is usual in the USA), the third char of \a formatCode can
  be set to 'c'. The inserted multiplication signs are the UTF-8 characters 215 (0xD7) for the
  cross and 183 (0xB7) for the dot.
  
  If the scale type (\ref setScaleType) is \ref stLogarithmic and the \a formatCode uses the 'b'
  option (beautifully typeset decimal powers), the display usually is "1 [multiplication sign] 10
  [superscript] n", which looks unnatural for logarithmic scaling (the "1 [multiplication sign]"
  part). To only display the decimal power, set the number precision to zero with \ref
  setNumberPrecision.
  
  Examples for \a formatCode:
  \li \c g normal format code behaviour. If number is small, fixed format is used, if number is large,
  normal scientific format is used
  \li \c gb If number is small, fixed format is used, if number is large, scientific format is used with
  beautifully typeset decimal powers and a dot as multiplication sign
  \li \c ebc All numbers are in scientific format with beautifully typeset decimal power and a cross as
  multiplication sign
  \li \c fb illegal format code, since fixed format doesn't support (or need) beautifully typeset decimal
  powers. Format code will be reduced to 'f'.
  \li \c hello illegal format code, since first char is not 'e', 'E', 'f', 'g' or 'G'. Current format
  code will not be changed.
*/
void QCustomPlotAxis::setNumberFormat(const QString &formatCode)
{
  if (formatCode.length() < 1) return;
  
  // interpret first char as number format char:
  QString allowedFormatChars = "eEfgG";
  if (allowedFormatChars.contains(formatCode.at(0)))
  {
    mNumberFormatChar = formatCode.at(0).toLatin1();
  } else
  {
    qDebug() << "Invalid number format code:" << formatCode;
    return;
  }
  if (formatCode.length() < 2)
  {
    mNumberBeautifulPowers = false;
    mNumberMultiplyCross = false;
    return;
  }
  
  // interpret second char as indicator for beautiful decimal powers:
  if (formatCode.at(1) == 'b' && (mNumberFormatChar == 'e' || mNumberFormatChar == 'g'))
  {
    mNumberBeautifulPowers = true;
  } else
  {
    qDebug() << "Invalid number format code:" << formatCode;
    return;
  }
  if (formatCode.length() < 3)
  {
    mNumberMultiplyCross = false;
    return;
  }
  
  // interpret third char as indicator for dot or cross multiplication symbol:
  if (formatCode.at(2) == 'c')
  {
    mNumberMultiplyCross = true;
  } else if (formatCode.at(2) == 'd')
  {
    mNumberMultiplyCross = false;
  } else
  {
    qDebug() << "Invalid number format code:" << formatCode;
    return;
  }
}

/*!
  Sets the precision of the numbers drawn as tick labels. See QLocale::toString(double i, char f,
  int prec) for details. The effect of precisions are most notably for number Format starting with
  'e', see \ref setNumberFormat

  If the scale type (\ref setScaleType) is \ref stLogarithmic and the number format (\ref
  setNumberFormat) uses the 'b' format code (beautifully typeset decimal powers), the display
  usually is "1 [multiplication sign] 10 [superscript] n", which looks unnatural for logarithmic
  scaling (the "1 [multiplication sign]" part). To only display the decimal power, set \a precision
  to zero.
*/
void QCustomPlotAxis::setNumberPrecision(int precision)
{
  mNumberPrecision = precision;
}

/*!
  If \ref setAutoTickStep is set to false, use this function to set the tick step manually.
  The tick step is the interval between (major) ticks, in plot coordinates.
  \see setSubTickCount
*/
void QCustomPlotAxis::setTickStep(double step)
{
  mTickStep = step;
}

/*!
  If you want full control over what ticks (and possibly labels) the axes show, this function is
  used to set the coordinates at which ticks will appear.\ref setAutoTicks must be disabled, else
  the provided tick vector will be overwritten with automatically generated tick coordinates. The
  labels of the ticks can either be generated automatically when \ref setAutoTickLabels is left
  enabled, or be set manually with \ref setTickVectorLabels, when setAutoTickLabels is disabled.
  
  \param copy if this is set to true, the provided \a vec is copied to the internal tick vector. In
  this case, QCustomPlotAxis does not take ownership of \a vec. If it's set to false, the internal
  tick vector is deleted and replaced by \a vec, QCustomPlotAxis takes ownership of the vector
  memory \a vec points to. The latter might give slight performance benefits.
  \see setTickVectorLabels
*/
void QCustomPlotAxis::setTickVector(QVector<double> *vec, bool copy)
{
  if (copy)
  {
    *mTickVector = *vec;
  } else
  {
    delete mTickVector;
    mTickVector = vec;
  }
}

/*!
  If you want full control over what ticks and labels the axes show, this function
  is used to set a number of QStrings that will be displayed at the tick positions
  which you need to provide with \ref setTickVector. These two vectors should have
  the same size.
  (Note that you need to disable \ref setAutoTicks and \ref setAutoTickLabels first.)
  
  \param copy if this is set to true, the provided \a vec is copied to the internal label vector.
  In this case QCustomPlotAxis does not take ownership of \a vec. If it's set to false, the
  internal label vector is deleted and replaced by \a vec, QCustomPlotAxis takes ownership of the
  vector memory \a vec points to. The latter might give slight performance benefits.
  \see
  setTickVector
*/
void QCustomPlotAxis::setTickVectorLabels(QVector<QString> *vec, bool copy)
{
  if (copy)
  {
    *mTickVectorLabels = *vec;
  } else
  {
    delete mTickVectorLabels;
    mTickVectorLabels = vec;
  }
}

/*!
  Sets the length of the ticks in pixels.
  \see setSubTickLength
*/
void QCustomPlotAxis::setTickLength(int length)
{
  mTickLength = length;
}

/*!
  Sets the number of sub ticks in one (major) tick step. A sub tick count of three for example,
  divides the tick intervals in four sub intervals.
  
  By default, the number of sub ticks is chosen automatically in a reasonable manner as long as
  the mantissa of the tick step is a multiple of 0.5 (which it is, when \ref setAutoTickStep is enabled).
  If you want to disable automatic sub ticks and use this function to set the count manually, see
  \ref setAutoSubTicks.
*/
void QCustomPlotAxis::setSubTickCount(int count)
{
  mSubTickCount = count;
}

/*!
  Sets the length of the sub ticks in pixels.
  \see setTickLength
*/
void QCustomPlotAxis::setSubTickLength(int length)
{
  mSubTickLength = length;
}

/*!
  Sets the pen, the axis base line itself is drawn with.
*/
void QCustomPlotAxis::setBasePen(const QPen &pen)
{
  mBasePen = pen;
}

/*!
  Sets the pen, grid lines are drawn with.
  \see setSubGridPen
*/
void QCustomPlotAxis::setGridPen(const QPen &pen)
{
  mGridPen = pen;
}

/*!
  Sets the pen, the sub grid lines are drawn with.
  (By default, subgrid drawing needs to be enabled first with \ref setSubGrid.)
  \see setGridPen
*/
void QCustomPlotAxis::setSubGridPen(const QPen &pen)
{
  mSubGridPen = pen;
}

/*!
  Sets the pen, tick marks will be drawn with.
  \see setTickLength
*/
void QCustomPlotAxis::setTickPen(const QPen &pen)
{
  mTickPen = pen;
}

/*!
  Sets the pen, subtick marks will be drawn with.
  \see setSubTickCount, setSubTickLength
*/
void QCustomPlotAxis::setSubTickPen(const QPen &pen)
{
  mSubTickPen = pen;
}

/*!
  Sets the font of the axis label.
*/
void QCustomPlotAxis::setLabelFont(const QFont &font)
{
  mLabelFont = font;
}

/*!
  Sets the axis label that will be shown below/above or next to the axis, depending on its orientation.
*/
void QCustomPlotAxis::setLabel(const QString &str)
{
  mLabel = str;
}

/*!
  Sets the distance between the tick labels and the axis label.
  \see setTickLabelPadding
*/
void QCustomPlotAxis::setLabelPadding(int padding)
{
  mLabelPadding = padding;
}

/*!
  If the scale type (\ref setScaleType) is \ref stLinear, \a diff is added to the lower and upper bounds of the range. The range is
  simply moved by \a diff.
  If the scale type is \ref stLogarithmic, the range bounds are multiplied by \a diff. This corresponds to an
  apparent "linear" move in logarithmic scaling by a distance of log(diff).
*/
void QCustomPlotAxis::moveRange(double diff)
{
  if (mScaleType == stLinear)
  {
    mRange.lower += diff;
    mRange.upper += diff;
  } else // mScaleType == stLogarithmic
  {
    mRange.lower *= diff;
    mRange.upper *= diff;
  }
  emit rangeChanged(mRange);
}

/*!
  Scales the range of this axis by \a factor around the coordinate \a center. For example, if \a
  factor is 2.0, \a center is 1.0, then the axis range will double its size, and the point at
  coordinate 1.0 won't have changed it's position in the QCustomPlot widget (i.e. coordinates
  around 1.0 will have moved symmetrically closer to 1.0).
*/
void QCustomPlotAxis::scaleRange(double factor, double center)
{
  
  if (mScaleType == stLinear)
  {
    QCustomPlotRange newRange;
    newRange.lower = (mRange.lower-center)*factor + center;
    newRange.upper = (mRange.upper-center)*factor + center;
    if (QCustomPlotRange::validRange(newRange))
      mRange = newRange.sanitizedForLinScale();
  } else // mScaleType == stLogarithmic
  {
    if ((mRange.upper < 0 && center < 0) || (mRange.upper > 0 && center > 0)) // make sure center has same sign as range
    {
      QCustomPlotRange newRange;
      newRange.lower = pow(mRange.lower/center, factor)*center;
      newRange.upper = pow(mRange.upper/center, factor)*center;
      if (QCustomPlotRange::validRange(newRange))
        mRange = newRange.sanitizedForLogScale();
    } else
      qDebug() << "center of scaling operation doesn't lie in same logarithmic sign domain as range:" << center;
  }
  emit rangeChanged(mRange);
}

/*!
  Sets the range of this axis to have a certain scale \a ratio to \a otherAxis. For example, if \a
  ratio is 1, this axis is the \a yAxis and \otherAxis is \a xAxis, graphs plotted with those axes
  will appear in a 1:1 ratio, independent of the aspect ratio the axis rect has. This is an
  operation that changes the range of this axis once, it doesn't fix the scale ratio indefinitely.
  Consequently calling this function in the constructor won't have the desired effect, since the
  widget's dimensions aren't defined yet, and a resizeEvent will follow.
*/
void QCustomPlotAxis::setScaleRatio(const QCustomPlotAxis *otherAxis, double ratio)
{
  int otherPixelSize, ownPixelSize;
  
  if (otherAxis->mAxisType == atTop || otherAxis->mAxisType == atBottom)
    otherPixelSize = otherAxis->mAxisRect.width();
  else
    otherPixelSize = otherAxis->mAxisRect.height();
  
  if (mAxisType == atTop || mAxisType == atBottom)
    ownPixelSize = mAxisRect.width();
  else
    ownPixelSize = mAxisRect.height();
  
  double newRangeSize = ratio*otherAxis->mRange.size()*ownPixelSize/(double)otherPixelSize;
  setRange(range().center(), newRangeSize, Qt::AlignCenter);
}

/*!
  \internal
  Transforms \a value (in pixel coordinates of the QCustomPlot widget) to axis coordinates.
*/
double QCustomPlotAxis::pixelToCoord(double value) const
{
  if (mAxisType == atTop || mAxisType == atBottom)
  {
    if (mScaleType == stLinear)
      return (value-mAxisRect.left())/(double)mAxisRect.width()*mRange.size()+mRange.lower;
    else // mScaleType == stLogarithmic
      return pow(mRange.upper/mRange.lower, (value-mAxisRect.left())/(double)mAxisRect.width())*mRange.lower;
  } else // mAxisType == atLeft || mAxisType == atRight
  {
    if (mScaleType == stLinear)
      return (mAxisRect.bottom()-value)/(double)mAxisRect.height()*mRange.size()+mRange.lower;
    else // mScaleType == stLogarithmic
      return pow(mRange.upper/mRange.lower, (mAxisRect.bottom()-value)/(double)mAxisRect.height())*mRange.lower;
  }
}

/*!
  \internal
  Transforms \a value (in coordinates of the axis) to pixel coordinates of the QCustomPlot widget.
*/
double QCustomPlotAxis::coordToPixel(double value) const
{
  if (mAxisType == atTop || mAxisType == atBottom)
  {
    if (mScaleType == stLinear)
      return (value-mRange.lower)/mRange.size()*mAxisRect.width()+mAxisRect.left();
    else // mScaleType == stLogarithmic
    {
      if (value >= 0 && mRange.upper < 0) // invalid value for logarithmic scale, just draw it outside visible range
        return mAxisRect.left()+mAxisRect.width()+100;
      else if (value <= 0 && mRange.upper > 0) // invalid value for logarithmic scale, just draw it outside visible range
        return mAxisRect.left()-100;
      else
        return baseLog(value/mRange.lower)/baseLog(mRange.upper/mRange.lower)*mAxisRect.width()+mAxisRect.left();
    }
  } else // mAxisType == atLeft || mAxisType == atRight
  {
    if (mScaleType == stLinear)
      return mAxisRect.bottom()-(value-mRange.lower)/mRange.size()*mAxisRect.height();
    else // mScaleType == stLogarithmic
    {     
      if (value >= 0 && mRange.upper < 0) // invalid value for logarithmic scale, just draw it outside visible range
        return mAxisRect.top()-100;
      else if (value <= 0 && mRange.upper > 0) // invalid value for logarithmic scale, just draw it outside visible range
        return mAxisRect.top()+mAxisRect.height()+100;
      else
        return mAxisRect.bottom()-baseLog(value/mRange.lower)/baseLog(mRange.upper/mRange.lower)*mAxisRect.height();
    }
  }
}

/*!
  \internal
  This function is called before the grid and axis is drawn, in order to prepare the tick
  vector, sub tick vector and tick label vector. If \ref setAutoTicks is set to true, appropriate
  tick value are determined automatically via \ref generateAutoTicks. If it's set to false, the
  signal ticksRequest is emitted, which can be used to provide external tick positions. Then the
  sub tick vectors and tick label vectors are created.
*/
void QCustomPlotAxis::generateTickVectors()
{
  if ((!mTicks && !mTickLabels && !mGrid) || mRange.size() <= 0) return;
  
  // fill tick vectors, either by auto generating or by notifying user to fill the vectors himself
  if (mAutoTicks)
  {
    generateAutoTicks();
  } else
  {
    emit ticksRequest();
  }
  
  if (mTickVector->isEmpty())
  {
    mSubTickVector->clear();
    return;
  }
  
  // generate subticks between ticks:
  mSubTickVector->resize((mTickVector->size()-1)*mSubTickCount);
  if (mSubTickCount > 0)
  {
    double subTickStep = 0;
    double subTickPosition = 0;
    int subTickIndex = 0;
    bool done = false;
    for (int i=1; i<mTickVector->size(); ++i)
    {
      subTickStep = (mTickVector->at(i)-mTickVector->at(i-1))/(double)(mSubTickCount+1);
      for (int k=1; k<=mSubTickCount; ++k)
      {
        subTickPosition = mTickVector->at(i-1) + k*subTickStep;
        if (subTickPosition < mRange.lower)
          continue;
        if (subTickPosition > mRange.upper)
        {
          done = true;
          break;
        }
        (*mSubTickVector)[subTickIndex] = subTickPosition;
        subTickIndex++;
      }
      if (done) break;
    }
    mSubTickVector->resize(subTickIndex);
  }

  // generate tick labels according to tick positions:
  mExponentialChar = mParentPlot->locale().exponential();   // will be needed when drawing the numbers generated here, in drawTickLabel()
  mPositiveSignChar = mParentPlot->locale().positiveSign(); // will be needed when drawing the numbers generated here, in drawTickLabel()
  if (mAutoTickLabels)
  {
    int vecsize = mTickVector->size();
    mTickVectorLabels->resize(vecsize);
    if (mTickLabelType == ltNumber)
    {
      for (int i=0; i<vecsize; ++i)
        (*mTickVectorLabels)[i] = mParentPlot->locale().toString(mTickVector->at(i), mNumberFormatChar, mNumberPrecision);
    } else if (mTickLabelType == ltDateTime)
    {
      for (int i=0; i<vecsize; ++i)
        (*mTickVectorLabels)[i] = mParentPlot->locale().toString(QDateTime::fromTime_t(mTickVector->at(i)), mDateTimeFormat);
    }
  } else // mAutoTickLabels == false
  {
    if (mAutoTicks) // ticks generated automatically, but not ticklabels, so emit ticksRequest here for labels
    {
      emit ticksRequest();
    }
    // make sure provided tick label vector has correct (minimal) length:
    if (mTickVectorLabels->size() < mTickVector->size())
      mTickVectorLabels->resize(mTickVector->size());
  }
}

/*!
  \internal
  If \ref setAutoTicks is set to true, this function is called by \ref generateTickVectors to
  generate reasonable tick positions (and subtick count). The algorithm tries to create approximately
  <tt>mAutoTickCount</tt> ticks (set via \ref setAutoTickCount), taking into account, that tick
  mantissas that are divisable by two or end in .5 are nice to look at and practical in linear
  scales. If the scale is logarithmic, one tick is generated at every power of the current
  logarithm base, set via \ref setScaleLogBase.
*/
void QCustomPlotAxis::generateAutoTicks()
{
  if (mScaleType == stLinear)
  {
    if (mAutoTickStep)
    {
      // Generate tick positions according to linear scaling:
      mTickStep = mRange.size()/(double)mAutoTickCount; // mAutoTickCount ticks on average
      double magnitudeFactor = pow(10, (int)floor(log10(mTickStep))); // get magnitude factor e.g. 0.01, 1, 10, 1000 etc.
      double tickStepMantissa = mTickStep/magnitudeFactor;
      if (tickStepMantissa < 5)
      {
        // round digit after decimal point to 0.5
        mTickStep = (int)(tickStepMantissa*2)/2.0*magnitudeFactor;
      } else
      {
        // round to first digit in multiples of 2
        mTickStep = (int)((tickStepMantissa/10.0)*5)/5.0*10*magnitudeFactor;
      }
    }
    if (mAutoSubTicks)
      mSubTickCount = calculateAutoSubTickCount(mTickStep);
    // Generate tick positions according to mTickStep:
    int firstStep = floor(mRange.lower/mTickStep);
    int lastStep = ceil(mRange.upper/mTickStep);
    int tickcount = lastStep-firstStep+1;
    if (tickcount < 0) tickcount = 0;
    mTickVector->resize(tickcount);
    for (int i=0; i<tickcount; ++i)
    {
      (*mTickVector)[i] = (firstStep+i)*mTickStep;
    }
  } else // mScaleType == stLogarithmic
  {
    // Generate tick positions according to logbase scaling:
    if (mRange.lower > 0 && mRange.upper > 0) // positive range
    {
      double lowerMag = basePow((int)floor(baseLog(mRange.lower)));
      double currentMag = lowerMag;
      mTickVector->clear();
      mTickVector->append(currentMag);
      while (currentMag < mRange.upper && currentMag > 0) // currentMag might be zero for ranges ~1e-300, just cancel in that case
      {
        currentMag *= mScaleLogBase;
        mTickVector->append(currentMag);
      }
    } else if (mRange.lower < 0 && mRange.upper < 0) // negative range
    {
      double lowerMag = -basePow((int)ceil(baseLog(-mRange.lower)));
      double currentMag = lowerMag;
      mTickVector->clear();
      mTickVector->append(currentMag);
      while (currentMag < mRange.upper && currentMag < 0) // currentMag might be zero for ranges ~1e-300, just cancel in that case
      {
        currentMag /= mScaleLogBase;
        mTickVector->append(currentMag);
      }
    } else // invalid range for logarithmic scale, because lower and upper have different sign
    {
      mTickVector->clear();
      qDebug() << "Invalid range for logarithmic plot: " << mRange.lower << "-" << mRange.upper;
    }
  }
}

/*!
  \internal
  Called by generateAutoTicks when \ref setAutoSubTicks is set to true. Depending on the
  \a tickStep between two major ticks on the axis, a different number of sub ticks is appropriate.
  For Example taking 4 sub ticks for a \a tickStep of 1 makes more sense than taking 5 sub ticks,
  because this corresponds to a sub tick step of 0.2, instead of the less intuitive 0.16666. Note
  that a subtick count of 4 means dividing the major tick step into 5 sections.
  
  This is implemented by a hand made lookup for integer tick steps as well as fractional tick steps with
  a fractional part of (approximately) 0.5.
  If a tick step is different (i.e. has no fractional part close to 0.5), the currently set
  sub tick count (\ref setSubTickCount) is returned.
*/
int QCustomPlotAxis::calculateAutoSubTickCount(double tickStep) const
{
  int result = mSubTickCount; // default to current setting, if no proper value can be found
  
  // get mantissa of tickstep:
  double magnitudeFactor = pow(10, (int)floor(log10(tickStep))); // get magnitude factor e.g. 0.01, 1, 10, 1000 etc.
  double tickStepMantissa = tickStep/magnitudeFactor;
  
  // separate integer and fractional part of mantissa:
  double epsilon = 0.01;
  double intPartf;
  int intPart;
  double fracPart = modf(tickStepMantissa, &intPartf);
  intPart = intPartf;
  
  // handle cases with (almost) integer mantissa:
  if (fracPart < epsilon || 1.0-fracPart < epsilon)
  {
    if (1.0-fracPart < epsilon)
      intPart++;
    switch (intPart)
    {
    case 1: result = 4; break; // 1.0 -> 0.2 substep
    case 2: result = 3; break; // 2.0 -> 0.5 substep
    case 3: result = 2; break; // 3.0 -> 1.0 substep
    case 4: result = 3; break; // 4.0 -> 1.0 substep
    case 5: result = 4; break; // 5.0 -> 1.0 substep
    case 6: result = 2; break; // 6.0 -> 2.0 substep
    case 7: result = 6; break; // 7.0 -> 1.0 substep
    case 8: result = 3; break; // 8.0 -> 2.0 substep
    case 9: result = 2; break; // 9.0 -> 3.0 substep
    }
  } else
  {
    // handle cases with significantly fractional mantissa:
    if (qAbs(fracPart-0.5) < epsilon) // *.5 mantissa
    {
      switch (intPart)
      {
      case 1: result = 2; break; // 1.5 -> 0.5 substep
      case 2: result = 4; break; // 2.5 -> 0.5 substep
      case 3: result = 4; break; // 3.5 -> 0.7 substep
      case 4: result = 2; break; // 4.5 -> 1.5 substep
      case 5: result = 4; break; // 5.5 -> 1.1 substep (won't occur with autoTickStep from here on)
      case 6: result = 4; break; // 6.5 -> 1.3 substep
      case 7: result = 2; break; // 7.5 -> 2.5 substep
      case 8: result = 4; break; // 8.5 -> 1.7 substep
      case 9: result = 4; break; // 9.5 -> 1.9 substep
      }
    }
    // if mantissa fraction isnt 0.0 or 0.5, don't bother finding good sub tick marks, leave default
  }
  
  return result;
}

/*!
  \internal
  The function to draw grid lines belonging to ticks of this axis, spanning over the complete
  axis rect. Called by QCustomPlot::draw for each axis.
*/
void QCustomPlotAxis::drawGrid(QPainter *painter)
{
  if (!mVisible || !mGrid) return;
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeGrid));
  int lowTick, highTick;
  visibleTickBounds(lowTick, highTick);
  int t; // helper variable, mostly result of coordinate-to-pixel transforms
  painter->setPen(mGridPen);
  if (mAxisType == atTop || mAxisType == atBottom)
  {
    for (int i=lowTick; i <= highTick; ++i)
    {
      t = coordToPixel(mTickVector->at(i)); // x
      painter->drawLine(t, mAxisRect.bottom(), t, mAxisRect.top());
    }
  } else
  {
    for (int i=lowTick; i <= highTick; ++i)
    {
      t = coordToPixel(mTickVector->at(i)); // y
      painter->drawLine(mAxisRect.left(), t, mAxisRect.right(), t);
    }
  }
  painter->restore();
}

/*!
  \internal
  The function to draw subgrid lines belonging to subticks of this axis, spanning over the complete
  axis rect. Called by QCustomPlot::draw for each axis.
*/
void QCustomPlotAxis::drawSubGrid(QPainter *painter)
{
  if (!mVisible || !mSubGrid || !mGrid) return;
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeSubGrid));
  
  int t; // helper variable, mostly result of coordinate-to-pixel transforms
  painter->setPen(mSubGridPen);
  if (mAxisType == atTop || mAxisType == atBottom)
  {
    for (int i=0; i<mSubTickVector->size(); ++i)
    {
      t = coordToPixel(mSubTickVector->at(i)); // x
      painter->drawLine(t, mAxisRect.bottom(), t, mAxisRect.top());
    }
  } else
  {
    for (int i=0; i<mSubTickVector->size(); ++i)
    {
      t = coordToPixel(mSubTickVector->at(i)); // y
      painter->drawLine(mAxisRect.left(), t, mAxisRect.right(), t);
    }
  }
  painter->restore();
}

/*!
  \internal
  The main draw function of an axis, called by QCustomPlot::draw for each axis. Draws
  axis baseline, major ticks, subticks, tick labels and axis label.
*/
void QCustomPlotAxis::drawAxis(QPainter *painter)
{
  if (!mVisible) return;
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing, mParentPlot->antialiasedElements().testFlag(QCustomPlot::aeAxes));
  QPoint origin;
  if (mAxisType == atLeft)
    origin = mAxisRect.bottomLeft();
  else if (mAxisType == atRight)
    origin = mAxisRect.bottomRight();
  else if (mAxisType == atTop)
    origin = mAxisRect.topLeft();
  else if (mAxisType == atBottom)
    origin = mAxisRect.bottomLeft();
  
  int xCor = 0, yCor = 0; // paint system correction, for pixel exact matches (affects baselines and ticks of top/right axes)
  if (mAxisType == atTop)
    yCor = -1;
  else if (mAxisType == atRight)
    xCor = 1;
  
  int lowTick, highTick;
  visibleTickBounds(lowTick, highTick);
  
  int t; // helper variable, mostly result of coordinate-to-pixel transforms

  /* draw axes */
  // baselines:
  painter->setPen(mBasePen);
  if (mAxisType == atTop || mAxisType == atBottom)
    painter->drawLine(origin+QPoint(xCor, yCor), origin+QPoint(mAxisRect.width()+xCor, yCor));
  else
    painter->drawLine(origin+QPoint(xCor, yCor), origin+QPoint(xCor, -mAxisRect.height()+yCor));
  
  // ticks:
  if (mTicks)
  {
    painter->setPen(mTickPen);
    int actualTickLength = mTickLength; // will be inverted for top and right axes, so ticks are drawn inwards
    if (mAxisType == atTop || mAxisType == atRight)
      actualTickLength *= -1;
    if (mAxisType == atTop || mAxisType == atBottom)
    {
      for (int i=lowTick; i <= highTick; ++i)
      {
        t = coordToPixel(mTickVector->at(i)); // x
        painter->drawLine(t+xCor, origin.y()+yCor, t+xCor, origin.y()-actualTickLength+yCor);
      }
    } else
    {
      for (int i=lowTick; i <= highTick; ++i)
      {
        t = coordToPixel(mTickVector->at(i)); // y
        painter->drawLine(origin.x()+xCor, t+yCor, origin.x()+actualTickLength+xCor, t+yCor);
      }
    }
  }
  
  // subticks:
  if (mTicks && mSubTickCount > 0)
  {
    painter->setPen(mSubTickPen);
    int actualSubTickLength = mSubTickLength; // will be inverted for top and right axes, so ticks are drawn inwards
    if (mAxisType == atTop || mAxisType == atRight)
      actualSubTickLength *= -1;
    if (mAxisType == atTop || mAxisType == atBottom)
    {
      for (int i=0; i<mSubTickVector->size(); ++i) // no need to check bounds because subticks are always only created inside current mRange
      {
        t = coordToPixel(mSubTickVector->at(i));
        painter->drawLine(t+xCor, origin.y()+yCor, t+xCor, origin.y()-actualSubTickLength+yCor);
      }
    } else
    {
      for (int i=0; i<mSubTickVector->size(); ++i)
      {
        t = coordToPixel(mSubTickVector->at(i));
        painter->drawLine(origin.x()+xCor, t+yCor, origin.x()+actualSubTickLength+xCor, t+yCor);
      }
    }
  }
  
  // tick labels:
  QSize tickLabelsSize; // size of largest tick label, for offset calculation of axis label
  if (mTickLabels)
  {
    painter->setFont(mTickLabelFont);
    for (int i=lowTick; i <= highTick; ++i)
    {
      t = coordToPixel(mTickVector->at(i));
      drawTickLabel(painter, t, mTickVectorLabels->at(i), &tickLabelsSize);
    }
  }

  // axis label:
  if (!mLabel.isEmpty())
  {
    painter->setFont(mLabelFont);
    QRect bounds;
    bounds = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, mLabel);
    if (mAxisType == atLeft)
    {
      painter->translate((origin.x()-tickLabelsSize.width()-bounds.height()-mLabelPadding), origin.y());
      painter->rotate(-90);
      painter->drawText(0, 0, mAxisRect.height(), bounds.height(), Qt::TextDontClip | Qt::AlignCenter, mLabel);
      painter->resetTransform();
    }
    else if (mAxisType == atRight)
    {
      painter->translate((origin.x()+tickLabelsSize.width()+bounds.height()+mLabelPadding), origin.y()-mAxisRect.height());
      painter->rotate(90);
      painter->drawText(0, 0, mAxisRect.height(), bounds.height(), Qt::TextDontClip | Qt::AlignCenter, mLabel);
      painter->resetTransform();
    }
    else if (mAxisType == atTop)
      painter->drawText(origin.x(), origin.y()-tickLabelsSize.height()-bounds.height()-mLabelPadding, mAxisRect.width(), bounds.height(), Qt::TextDontClip | Qt::AlignCenter, mLabel);
    else if (mAxisType == atBottom)
      painter->drawText(origin.x(), origin.y()+tickLabelsSize.height()+mLabelPadding, mAxisRect.width(), bounds.height(), Qt::TextDontClip | Qt::AlignCenter, mLabel);
  }

  painter->restore();
}

/*!
  \internal
  Draws a single tick label with the provided \a painter. The tick label is always bound
  to an axis in one direction. The position in the other direction is passed in the \a position
  parameter. Hence for the bottom axis, \a position would indicate the horizontal pixel position (not
  coordinate!), at which the label should be drawn.
  
  In order to draw the axis label after all the tick labels in a position, that doesn't overlap with
  the tick labels, we need to know the largest tick label size. This is done by passing
  a \a tickLabelsSize to all \ref drawTickLabel calls during the process of drawing all tick labels of one axis.
  \a tickLabelSize is only expanded, if the drawn label exceeds the value \a tickLabelsSize
  currently holds.
  
  This function is also responsible for turning ugly exponential numbers "5.5e9" into a more beautifully
  typeset format "5.5 [multiplication sign] 10 [superscript] 9".
  This feature is controlled with \ref setNumberFormat.
*/
void QCustomPlotAxis::drawTickLabel(QPainter *painter, double position, const QString &text, QSize *tickLabelsSize)
{
  // warning: if you change anything here, also adapt getMaxTickLabelSize() accordingly!
  
  // determine whether beautiful decimal powers should be used
  bool useBeautifulPowers = false;
  int ePos;
  if (mAutoTickLabels && mNumberBeautifulPowers && mTickLabelType == ltNumber)
  {
    ePos = text.indexOf('e');
    if (ePos > -1)
      useBeautifulPowers = true;
  }
  
  // calculate bounding rects and do string preparation for beautiful decimal powers:
  QRect bounds, baseBounds, expBounds;
  QString basePart, expPart;
  QFont expFont;
  if (useBeautifulPowers)
  {
    // split string parts for part of number/symbol that will be drawn normally and part that will be drawn as exponent:
    basePart = text.left(ePos);
    // in log scaling, we want to turn "1*10^n" into "10^n", else add multiplication sign and decimal base:
    if (mScaleType == stLogarithmic && basePart == "1")
      basePart = "10";
    else
      basePart += (mNumberMultiplyCross ? QString(QChar(215)) : QString(QChar(183))) + "10";
    expPart = text.mid(ePos+1);
    // clip "+" and leading zeros off expPart:
    while (expPart.at(1) == '0' && expPart.length() > 2) // length > 2 so we leave one zero when numberFormatChar is 'e'
      expPart.remove(1, 1);
    if (expPart.at(0) == mPositiveSignChar)
      expPart.remove(0, 1);
    // prepare smaller font for exponent:
    expFont = painter->font();
    expFont.setPointSize(expFont.pointSize()*0.75);
    // calculate bounding rects of base part, exponent part and total one:
    baseBounds = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, basePart);
    QFontMetrics expFontMetrics(expFont);
    expBounds = expFontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip, expPart);
    bounds = baseBounds.adjusted(0, 0, expBounds.width(), 0);
  } else // useBeautifulPowers == false
  {
    bounds = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignHCenter, text);
  }
  
  // if rotated tick labels, transform bounding rect, too:
  QRect rotatedBounds = bounds;
  if (!qFuzzyCompare(mTickLabelRotation+1.0, 1.0))
  {
    QTransform transform;
    transform.rotate(mTickLabelRotation);
    rotatedBounds = transform.mapRect(bounds);
  }
  // expand passed tickLabelsSize if current tick label is larger:
  if (rotatedBounds.width() > tickLabelsSize->width()) 
    tickLabelsSize->setWidth(rotatedBounds.width());
  if (rotatedBounds.height() > tickLabelsSize->height())
    tickLabelsSize->setHeight(rotatedBounds.height());
  
  // calculate coordinates:
  bool doRotation = !qFuzzyCompare(mTickLabelRotation+1.0, 1.0);
  double angle = mTickLabelRotation/180.0*M_PI;
  int x = 0, y = 0;
  if (mAxisType == atLeft)
  {
    if (doRotation)
    {
      if (mTickLabelRotation > 0)
      {
        x = mAxisRect.left()-cos(angle)*bounds.width()-mTickLabelPadding;
        y = position-sin(angle)*bounds.width()-cos(angle)*bounds.height()/2.0;
      } else
      {
        x = mAxisRect.left()-cos(-angle)*bounds.width()-sin(-angle)*bounds.height()-mTickLabelPadding;
        y = position+sin(-angle)*bounds.width()-cos(-angle)*bounds.height()/2.0;
      }
    } else
    {
      x = mAxisRect.left()-bounds.width()-mTickLabelPadding;
      y = position-bounds.height()/2.0;
    }
  } else if (mAxisType == atRight)
  {
    if (doRotation)
    {
      if (mTickLabelRotation > 0)
      {
        x = mAxisRect.right()+sin(angle)*bounds.height()+mTickLabelPadding;
        y = position-cos(angle)*bounds.height()/2.0;
      } else
      {
        x = mAxisRect.right()+mTickLabelPadding;
        y = position-cos(-angle)*bounds.height()/2.0;
      }
    } else
    {
      x = mAxisRect.right()+mTickLabelPadding;
      y = position-bounds.height()/2.0;
    }
  } else if (mAxisType == atTop)
  {
    if (doRotation)
    {
      if (mTickLabelRotation > 0)
      {
        x = position-cos(angle)*bounds.width()+sin(angle)*bounds.height()/2.0;
        y = mAxisRect.top()-sin(angle)*bounds.width()-cos(angle)*bounds.height()-mTickLabelPadding;
      } else
      {
        x = position-sin(-angle)*bounds.height()/2.0;
        y = mAxisRect.top()-cos(-angle)*bounds.height()-mTickLabelPadding;
      }
    } else
    {
      x = position-bounds.width()/2.0;
      y = mAxisRect.top()-bounds.height()-mTickLabelPadding;
    }
  } else if (mAxisType == atBottom)
  {
    if (doRotation)
    {
      if (mTickLabelRotation > 0)
      {
        x = position+sin(angle)*bounds.height()/2.0;
        y = mAxisRect.bottom()+mTickLabelPadding;
      } else
      {
        x = position-cos(-angle)*bounds.width()-sin(-angle)*bounds.height()/2.0;
        y = mAxisRect.bottom()+sin(-angle)*bounds.width()+mTickLabelPadding;
      }
    } else
    {
      x = position-bounds.width()/2.0;
      y = mAxisRect.bottom()+mTickLabelPadding;
    }
  }
  
  // transform painter to position/rotation:
  painter->translate(x, y);
  if (doRotation)
    painter->rotate(mTickLabelRotation);
  
  // draw text:
  if (useBeautifulPowers)
  {
    // draw base:
    painter->drawText(0, 0, 0, 0, Qt::TextDontClip, basePart);
    // draw exponent:
    painter->setFont(expFont);
    painter->drawText(baseBounds.width()+1, 0, expBounds.width(), expBounds.height(), Qt::TextDontClip,  expPart);
    painter->setFont(mTickLabelFont);
  } else // useBeautifulPowers == false
  {
    painter->drawText(0, 0, bounds.width(), bounds.height(), Qt::TextDontClip | Qt::AlignHCenter, text);
  }
  
  // reset rotation/translation transform:
  painter->setTransform(QTransform());
}

/*!
  \internal
  Simulates the steps done by \ref drawTickLabel by calculating bounding boxes of the text
  label to be drawn, depending on number format etc.
  Since we only want the largest tick label for the margin calculation, the passed \a tickLabelsSize
  isn't overridden with the calculated label size, but it's only expanded, if it's
  currently set to a smaller width/height.
*/
void QCustomPlotAxis::getMaxTickLabelSize(const QFont &font, const QString &text,  QSize *tickLabelsSize) const
{
  // This function does the same as drawTickLabel but omits the actual drawing
  // changes involve creating extra QFontMetrics instances for font, since painter->fontMetrics() isn't available
  
  // determine whether beautiful powers should be used
  bool useBeautifulPowers = false;
  int ePos;
  if (mAutoTickLabels && mNumberBeautifulPowers && mTickLabelType == ltNumber)
  {
    ePos = text.indexOf(mExponentialChar);
    if (ePos > -1)
      useBeautifulPowers = true;
  }
  
  // calculate and draw text, depending on whether beautiful powers are applicable or not:
  QRect bounds, baseBounds, expBounds;
  QString basePart, expPart;
  QFont expFont;
  if (useBeautifulPowers)
  {
    // split string parts for part of number/symbol that will be drawn normally and part that will be drawn as exponent:
    basePart = text.left(ePos);
    // in log scaling, we want to turn "1*10^n" into "10^n", else add multiplication sign and decimal base:
    if (mScaleType == stLogarithmic && basePart == "1")
      basePart = "10";
    else
      basePart += (mNumberMultiplyCross ? QString(QChar(215)) : QString(QChar(183))) + "10";
    expPart = text.mid(ePos+1);
    // clip "+" and leading zeros off expPart:
    while (expPart.at(1) == '0' && expPart.length() > 2) // length > 2 so we leave one zero when numberFormatChar is 'e'
      expPart.remove(1, 1);
    if (expPart.at(0) == mPositiveSignChar)
      expPart.remove(0, 1);
    // prepare smaller font for exponent:
    expFont = font;
    expFont.setPointSize(expFont.pointSize()*0.75);
    // calculate bounding rects of base part, exponent part and total one:
    QFontMetrics baseFontMetrics(font);
    baseBounds = baseFontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip, basePart);
    QFontMetrics expFontMetrics(expFont);
    expBounds = expFontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip, expPart);
    bounds = baseBounds.adjusted(0, 0, expBounds.width(), 0); 
  } else // useBeautifulPowers == false
  {
    QFontMetrics fontMetrics(font);
    bounds = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignHCenter, text);
  }
  
  // if rotated tick labels, transform bounding rect, too:
  QRect rotatedBounds = bounds;
  if (!qFuzzyCompare(mTickLabelRotation+1.0, 1.0))
  {
    QTransform transform;
    transform.rotate(mTickLabelRotation);
    rotatedBounds = transform.mapRect(bounds);
  }
  // expand passed tickLabelsSize if current tick label is larger:
  if (rotatedBounds.width() > tickLabelsSize->width()) 
    tickLabelsSize->setWidth(rotatedBounds.width());
  if (rotatedBounds.height() > tickLabelsSize->height())
    tickLabelsSize->setHeight(rotatedBounds.height());
}

/*!
  \internal
  Returns via \a lowIndex and \a highIndex, which ticks in the current tick vector
  are visible in the current range. The return values are indices of the
  tick vector, not the tick values themselves.
  The actual use of this function is when we have an externally provided tick vector, which
  might exceed far beyond the currently displayed range, and would cause unnecessary calculations
  e.g. of subticks.
*/
void QCustomPlotAxis::visibleTickBounds(int &lowIndex, int &highIndex) const
{
  lowIndex = 0;
  highIndex = -1;
  // make sure only ticks that are in visible range are returned
  for (int i=0; i < mTickVector->size(); ++i)
  {
    lowIndex = i;
    if (mTickVector->at(i) >= mRange.lower) break;
  }
  for (int i=mTickVector->size()-1; i >= 0; --i)
  {
    highIndex = i;
    if (mTickVector->at(i) <= mRange.upper) break;
  }
}

/*!
  \internal
  A log function with the base mScaleLogBase, used mostly for coordinate transforms in
  logarithmic scales with arbitrary log base. Uses the buffered mScaleLogBaseLogInv for faster
  calculation. This is set to <tt>1.0/log(mScaleLogBase)</tt> in \ref setScaleLogBase.
  \see basePow, setScaleLogBase, setScaleType
*/
double QCustomPlotAxis::baseLog(double value) const
{
  return log(value)*mScaleLogBaseLogInv;
}

/*!
  \internal
  A power function with the base mScaleLogBase, used mostly for coordinate transforms in
  logarithmic scales with arbitrary log base.
  \see baseLog, setScaleLogBase, setScaleType
*/
double QCustomPlotAxis::basePow(double value) const
{
  return pow(mScaleLogBase, value);
}

/*!
  \internal
  Simulates the steps of \ref drawAxis by calculating all appearing text bounding boxes. From this
  information, the appropriate margin for this axis is determined, so nothing is drawn beyond the
  widget border in the actual \ref drawAxis function (if \ref QCustomPlot::setAutoMargin is set to
  true).
  The margin consists of: tick label padding, tick label size, label padding, label size.
  The return value is the calculated margin for this axis. Thus, an axis with axis type
  \ref atLeft will return an appropriate left margin, \ref atBottom will return an appropriate
  bottom margin and so forth.
  \warning if anything is changed in this function, make sure it's synchronized with
  the actual drawing function \ref drawAxis.
*/
int QCustomPlotAxis::calculateMargin() const
{
  // run through similar steps as QCustomPlotAxis::drawAxis, and caluclate margin needed to fit axis and its labels
  int margin = 0;
  
  if (mVisible)
  {
    int lowTick, highTick;
    visibleTickBounds(lowTick, highTick);
    
    // calculate size of tick labels:
    QSize tickLabelsSize;
    if (mTickLabels)
    {
      for (int i=lowTick; i <= highTick; ++i)
      {
        getMaxTickLabelSize(mTickLabelFont, mTickVectorLabels->at(i), &tickLabelsSize);
      }
      if (mAxisType == atLeft || mAxisType == atRight)
        margin += tickLabelsSize.width() + mTickLabelPadding;
      else
        margin += tickLabelsSize.height() + mTickLabelPadding;
    }
    
    // calculate size of axis label (only height needed, because left/right labels are rotated by 90 degrees):
    if (!mLabel.isEmpty())
    {
      QFontMetrics fontMetrics(mLabelFont);
      QRect bounds;
      bounds = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignHCenter | Qt::AlignVCenter, mLabel);
      margin += bounds.height() + mLabelPadding;
    }
  }
  
  if (margin < 15) // need a bit of margin if no axis text is shown at all (i.e. only baseline and tick lines, or no axis at all)
    margin = 15;
  return margin;
}


// ================================================================================
// =================== QCustomPlot
// ================================================================================

/*! \class QCustomPlot
  The central class which is also the QWidget which displays the plot and interacts with the user.
  
  For tutorials on how to use QCustomPlot, see the website\n
  http://www.WorksLikeClockWork.com/index.php/components/qt-plotting-widget
*/

/*!
  Constructs a QCustomPlot and sets reasonable default values.
  Four axes are created at the bottom, left, top and right sides (xAxis, yAxis, xAxis2, yAxis2),
  however, only the bottom and left axes are set to be visible.
  The legend is also set to be invisible initially.
*/
QCustomPlot::QCustomPlot(QWidget *parent) :
    QWidget(parent)
{  
  setMouseTracking(true);
  QLocale currentLocale = locale();
  currentLocale.setNumberOptions(QLocale::OmitGroupSeparator);
  setLocale(currentLocale);
  
  buffer = QPixmap(size());
  mViewport = rect();
  mDragging = false;
  QFont titleFont;
  titleFont.setPointSize(14);
  titleFont.setBold(true);
  setTitleFont(titleFont);
  setTitle("");
  setColor(Qt::white);
  setAntialiasedElements(aeGraphs | aeScatters);
  legend = new QCustomPlotLegend(this);
  legend->setVisible(false);
  setAutoAddGraphToLegend(true);
  xAxis = new QCustomPlotAxis(this, QCustomPlotAxis::atBottom);
  yAxis = new QCustomPlotAxis(this, QCustomPlotAxis::atLeft);
  xAxis2 = new QCustomPlotAxis(this, QCustomPlotAxis::atTop);
  yAxis2 = new QCustomPlotAxis(this, QCustomPlotAxis::atRight);
  xAxis2->setGrid(false);
  yAxis2->setGrid(false);
  xAxis2->setVisible(false);
  yAxis2->setVisible(false);
  
  setRangeDragAxes(xAxis, yAxis);
  setRangeZoomAxes(xAxis, yAxis);
  setRangeDrag(0);
  setRangeZoom(0);
  setRangeZoomFactor(0.85);
  
  setMargin(0, 0, 0, 0);
  setAutoMargin(true);
  replot();
}

QCustomPlot::~QCustomPlot()
{
  delete xAxis;
  delete yAxis;
  delete xAxis2;
  delete yAxis2;
}

/*!
  Returns the range drag axis of the \a orientation provided
  \see setRangeDragAxes
*/
QCustomPlotAxis *QCustomPlot::rangeDragAxis(Qt::Orientation orientation)
{
  return (orientation == Qt::Horizontal ? mRangeDragHorzAxis : mRangeDragVertAxis);
}

/*!
  Returns the range zoom axis of the \a orientation provided
  \see setRangeZoomAxes
*/
QCustomPlotAxis *QCustomPlot::rangeZoomAxis(Qt::Orientation orientation)
{
  return (orientation == Qt::Horizontal ? mRangeZoomHorzAxis : mRangeZoomVertAxis);
}

/*!
  Returns the range zoom factor of the \a orientation provided
  \see setRangeZoomFactor
*/
double QCustomPlot::rangeZoomFactor(Qt::Orientation orientation)
{
  return (orientation == Qt::Horizontal ? mRangeZoomFactorHorz : mRangeZoomFactorVert);
}

/*!
  Sets the plot title which will be drawn centered at the top of the widget.
  The title position is not dependant on the actual position of the axes. However, if
  \ref setAutoMargin is set to true, the top margin will be adjusted appropriately,
  so the top axis labels/tick labels will not overlap with the title.
*/
void QCustomPlot::setTitle(const QString &title)
{
  mTitle = title;
}

/*!
   Sets the font of the plot title
   \see setTitle
*/
void QCustomPlot::setTitleFont(const QFont &font)
{
  mTitleFont = font;
}

/*!
  An alternative way to set the margins, by directly setting the wanted axis rect. The rect
  will be translated into appropriate margin values.
  \warning setting the axis rect with this function does not guarantee that the axis rect
  will stay like this indefinitely. In QCustomPlot, margins are the fixed values (if \ref
  setAutoMargin is false). Consequently, the axis rect is automatically changed when the
  widget size changes, but the margins (distances between axis rect edges and widget/viewport
  rect edges) stay the same.
  \see setMargin
*/
void QCustomPlot::setAxisRect(const QRect &arect)
{
  mMarginLeft = arect.left()-mViewport.left();
  mMarginRight = mViewport.right()-arect.right();
  mMarginTop = arect.top()-mViewport.top();
  mMarginBottom = mViewport.bottom()-arect.bottom();
  updateAxisRect();
}

/*!
  Sets the left margin manually. Will only have effect, if \ref setAutoMargin is set to false.
  see \ref setMargin for an explanation of what margins mean in QCustomPlot.
*/
void QCustomPlot::setMarginLeft(int margin)
{
  mMarginLeft = margin;
  updateAxisRect();
}

/*!
  Sets the right margin manually. Will only have effect, if \ref setAutoMargin is set to false.
  see \ref setMargin for an explanation of what margins mean in QCustomPlot.
*/
void QCustomPlot::setMarginRight(int margin)
{
  mMarginRight = margin;
  updateAxisRect();
}

/*!
  Sets the top margin manually. Will only have effect, if \ref setAutoMargin is set to false.
  see \ref setMargin for an explanation of what margins mean in QCustomPlot.
*/
void QCustomPlot::setMarginTop(int margin)
{
  mMarginTop = margin;
  updateAxisRect();
}

/*!
  Sets the bottom margin manually. Will only have effect, if \ref setAutoMargin is set to false.
  see \ref setMargin for an explanation of what margins mean in QCustomPlot.
*/
void QCustomPlot::setMarginBottom(int margin)
{
  mMarginBottom = margin;
  updateAxisRect();
}

/*!
  Sets the margins manually. Will only have effect, if \ref setAutoMargin is set to false.
  The margins are the distances in pixels between the axes box and the viewport box.
  The viewport box normally is the entire QCustomPlot widget or the entire image, if
  using one of the export functions. Positive margin values always mean the axes box
  is shrinked, going inward from the sides of the viewport box.
*/
void QCustomPlot::setMargin(int left, int right, int top, int bottom)
{
  mMarginLeft = left;
  mMarginRight = right;
  mMarginTop = top;
  mMarginBottom = bottom;
  updateAxisRect();
}

/*!
  Sets whether the margins are calculated automatically depeding on the sizes
  of the tick labels, axis labels, paddings etc.
  If disabled, the margins must be set manually with the \a setMargin functions.
  \see setMargin, QCustomPlotAxis::setLabelPadding, QCustomPlotAxis::setTickLabelPadding
*/
void QCustomPlot::setAutoMargin(bool enabled)
{
  mAutoMargin = enabled;
}

/*!
  Sets the background color of the QCustomPlot widget.
*/
void QCustomPlot::setColor(const QColor &color)
{
  mColor = color;
}

/*!
  Sets which axis orientation may be range dragged by the user with mouse interaction.
  What orientation corresponds to which specific axis can be set with
  \ref setRangeDragAxes(QCustomPlotAxis *horizontal, QCustomPlotAxis *vertical). By
  default, the horizontal axis is the bottom axis (xAxis) and the vertical axis
  is the left axis (yAxis).
  To disable range dragging entirely, pass as \a orientations. To enable range dragging
  for both directions, pass <tt>Qt::Horizontal | Qt::Vertical</tt> as \a orientations.
*/
void QCustomPlot::setRangeDrag(Qt::Orientations orientations)
{
  mRangeDrag = orientations;
}

/*!
  Sets which axis orientation may be zoomed by the user with the mouse wheel.
  What orientation corresponds to which specific axis can be set with
  \ref setRangeZoomAxes(QCustomPlotAxis *horizontal, QCustomPlotAxis *vertical). By
  default, the horizontal axis is the bottom axis (xAxis) and the vertical axis
  is the left axis (yAxis).
  To disable range zooming entirely, pass as \a orientations. To enable range zooming
  for both directions, pass <tt>Qt::Horizontal | Qt::Vertical</tt> as \a orientations.
*/
void QCustomPlot::setRangeZoom(Qt::Orientations orientations)
{
  mRangeZoom = orientations;
}

/*!
  Sets the axes whose range will be dragged when \ref setRangeDrag enables mouse range dragging
  on the QCustomPlot widget.
*/
void QCustomPlot::setRangeDragAxes(QCustomPlotAxis *horizontal, QCustomPlotAxis *vertical)
{
  if (horizontal)
    mRangeDragHorzAxis = horizontal;
  if (vertical)
    mRangeDragVertAxis = vertical;
}

/*!
  Sets the axes whose range will be zoomed when \ref setRangeZoom enables mouse wheel zooming on the
  QCustomPlot widget. The two axes can be zoomed with different strengths, when different factors
  are passed to \ref setRangeZoomFactor(double horizontalFactor, double verticalFactor).
*/
void QCustomPlot::setRangeZoomAxes(QCustomPlotAxis *horizontal, QCustomPlotAxis *vertical)
{
  if (horizontal)
    mRangeZoomHorzAxis = horizontal;
  if (vertical)
    mRangeZoomVertAxis = vertical;
}

/*!
  Sets how strong one rotation step of the mouse wheel zooms, when range zoom was activated with
  \ref setRangeZoom. The two parameters \a horizontalFactor and \a verticalFactor provide a way to
  let the horizontal axis zoom at different rates as the vertical axis. Which axis is horizontal
  and which is vertical, can be set with \ref setRangeZoomAxes. When the zoom factor is greater
  than one, scrolling the mouse wheel backwards (towards the user) will zoom in. For zoom factors
  smaller than one, scrolling the mouse wheel forwards zooms in.
*/
void QCustomPlot::setRangeZoomFactor(double horizontalFactor, double verticalFactor)
{
  mRangeZoomFactorHorz = horizontalFactor;
  mRangeZoomFactorVert = verticalFactor;
}

/*!
  Sets how strong one rotation step of the mouse wheel zooms, when range zoom was activated with
  \ref setRangeZoom. The zoom \a factor is applied to both horizontal and vertical range zooming.
  When the zoom factor is greater than one, scrolling the mouse wheel backwards (towards the user)
  will zoom in. For zoom factors smaller than one, scrolling the mouse wheel forwards zooms in.
*/
void QCustomPlot::setRangeZoomFactor(double factor)
{
  mRangeZoomFactorHorz = factor;
  mRangeZoomFactorVert = factor;
}

/*!
  Sets which elements are drawn antialiased as an or combination of \ref AntialiasedElement.
*/
void QCustomPlot::setAntialiasedElements(const AntialiasedElements &antialiasedElements)
{
  mAntialiasedElements = antialiasedElements;
}

/*!
  Sets whether the specified \a antialiasedElement is drawn antialiased.
*/
void QCustomPlot::setAntialiasedElement(AntialiasedElement antialiasedElement, bool enabled)
{
  if (!enabled && mAntialiasedElements.testFlag(antialiasedElement))
    mAntialiasedElements &= ~antialiasedElement;
  else if (enabled && !mAntialiasedElements.testFlag(antialiasedElement))
    mAntialiasedElements |= antialiasedElement;
}

/*!
  If set to true, a call to QCustomPlot::addGraph automatically also calls
  QCustomPlotLegend::addGraph and adds the newly created graph to the legend.
  \see addGraph, QCustomPlotLegend::addGraph
*/
void QCustomPlot::setAutoAddGraphToLegend(bool on)
{
  mAutoAddGraphToLegend = on;
}

/*!
  Returns the graph with \a index. If the index is invalid, returns 0.
  
  There is an overloaded version of this function with no parameter which returns the last created
  graph, see QCustomPlot::graph()
  
  \see graphCount, addGraph
*/
QCustomPlotGraph *QCustomPlot::graph(int index)
{
  if (index >= 0 && index < mGraphs.size())
  {
    return mGraphs.at(index);
  } else
  {
    qDebug() << "Graph index out of bounds:" << index;
    return NULL;
  }
}

/*!
  Returns the last graph, that was created with \ref addGraph. If there are no graphs in the plot,
  returns 0.
  \see graphCount, addGraph
*/
QCustomPlotGraph *QCustomPlot::graph()
{
  if (!mGraphs.isEmpty())
  {
    return mGraphs.last();
  } else
    return NULL;
}

/*!
  Creates a new graph inside the plot. If \a keyAxis and \a valueAxis are left unspecified,
  the bottom (xAxis) is used as key and the left (yAxis) is used as value.
  
  \param keyAxis the axis that will be used as key axis for the graph (typically "x")
  \param valueAxis the axis that will be used as value axis for the graph (typically "y")
  \see graph, graphCount, removeGraph, clearGraphs
*/
int QCustomPlot::addGraph(QCustomPlotAxis *keyAxis, QCustomPlotAxis *valueAxis)
{
  if (!keyAxis) keyAxis = xAxis;
  if (!valueAxis) valueAxis = yAxis;
  QCustomPlotGraph *ng = new QCustomPlotGraph(keyAxis, valueAxis);
  ng->setName("Graph "+QString::number(mGraphs.size()));
  mGraphs.append(ng);
  if (mAutoAddGraphToLegend)
  {
    legend->addGraph(ng);
  }
  return mGraphs.size()-1;
}

/*!
  Removes the graph with the specified index from the plot and, if necessary, from the legend. If
  any other graphs in the plot have a channel fill set towards the removed graph, the channel fill
  property of those graphs is reset to zero (no channel fill).
  \see clearGraphs
*/
bool QCustomPlot::removeGraph(int index)
{
  if (index > 0 && index < mGraphs.size())
  {
    // remove graph from legend:
    legend->removeGraph(mGraphs.at(index));
    // if other graphs in plot have this graph as channel fill target, set their channel fill to zero:
    for (int i=0; i<mGraphs.size(); ++i)
    {
      if (mGraphs.at(i)->channelFillGraph() == mGraphs.at(index))
        mGraphs[i]->setChannelFillGraph(0);
    }
    // remove graph:
    delete mGraphs.at(index);
    mGraphs.removeAt(index);
    return true;
  } else
    return false;
}

/*!
  Removes all graphs from the plot (and the legend, if necessary)
  \see removeGraph
*/
int QCustomPlot::clearGraphs()
{
  int c = mGraphs.size();
  for (int i=c-1; i >= 0; --i)
    removeGraph(i);
  return c;
}

/*!
  Returns the number of currently existing graphs in the plot
  \see graph, addGraph
*/
int QCustomPlot::graphCount()
{
  return mGraphs.size();
}

/*!
  Causes a complete replot (axes, labels, graphs, etc.) into the internal buffer QPixmap. Finally,
  update() is called, to redraw the buffer on the QCustomPlot widget surface.
*/
void QCustomPlot::replot()
{
  QPainter painter(&buffer);
  painter.fillRect(rect(), mColor);
  draw(&painter);
  update();
}

/*!
  Convenience function to make the top and right axes visible and assign them the following
  properties from their corresponding bottom/left axes:
  
  \li range (\ref QCustomPlotAxis::setRange)
  \li scale type (\ref QCustomPlotAxis::setScaleType)
  \li scale log base  (\ref QCustomPlotAxis::setScaleLogBase)
  \li ticks (\ref QCustomPlotAxis::setTicks)
  \li auto (major) tick count (\ref QCustomPlotAxis::setAutoTickCount)
  \li sub tick count (\ref QCustomPlotAxis::setSubTickCount)
  \li auto sub ticks (\ref QCustomPlotAxis::setAutoSubTicks)
  \li tick step (\ref QCustomPlotAxis::setTickStep)
  \li auto tick step (\ref QCustomPlotAxis::setAutoTickStep)
  
  Tick labels (\ref QCustomPlotAxis::setTickLabels) however, is always set to false.

  This function does \a not connect the rangeChanged signals of the bottom and left axes to the \ref
  QCustomPlotAxis::setRange slots of the top and right axes in order to synchronize the ranges permanently.
*/
void QCustomPlot::setupFullAxesBox()
{
  xAxis2->setVisible(true);
  yAxis2->setVisible(true);
  
  xAxis2->setTickLabels(false);
  yAxis2->setTickLabels(false);
  
  xAxis2->setAutoSubTicks(xAxis->autoSubTicks());
  yAxis2->setAutoSubTicks(yAxis->autoSubTicks());
  
  xAxis2->setAutoTickCount(xAxis->autoTickCount());
  yAxis2->setAutoTickCount(yAxis->autoTickCount());
  
  xAxis2->setAutoTickStep(xAxis->autoTickStep());
  yAxis2->setAutoTickStep(yAxis->autoTickStep());
  
  xAxis2->setScaleType(xAxis->scaleType());
  yAxis2->setScaleType(yAxis->scaleType());
  
  xAxis2->setScaleLogBase(xAxis->scaleLogBase());
  yAxis2->setScaleLogBase(yAxis->scaleLogBase());
  
  xAxis2->setTicks(xAxis->ticks());
  yAxis2->setTicks(yAxis->ticks());
  
  xAxis2->setSubTickCount(xAxis->subTickCount());
  yAxis2->setSubTickCount(yAxis->subTickCount());
  
  xAxis2->setTickStep(xAxis->tickStep());
  yAxis2->setTickStep(yAxis->tickStep());
  
  xAxis2->setRange(xAxis->range());
  yAxis2->setRange(yAxis->range());
}

/*!
  Saves a PDF with the vectorized plot to the file \a fileName. The axis ratio as well as the scale
  of texts and lines is taken from the current state of the QCustomPlot widget. \a noCosmeticPen
  disables the use of cosmetic pens when drawing to the PDF file. Cosmetic pens are pens with
  numerical width 0 which are always drawn as a one pixel wide line, no matter what zoom factor is
  set in the PDF-Viewer. For more information about cosmetic pens, see QPainter and QPen
  documentation. \warning If you plan on editing the exported PDF file with a vector graphics
  editor like Inkscape, it is advised to set \a noCosmeticPen to true to avoid losing those
  cosmetic lines (which might be quite many, because cosmetic pens are the default for e.g. axes
  and tick marks). \todo allow setting of aspect ratio other than current aspect ratio of
  QCustomPlot widget. allow setting scale as in scaled savePng function.
\see savePng
*/
void QCustomPlot::savePdf(const QString &fileName, bool noCosmeticPen)
{
  QPrinter printer(QPrinter::ScreenResolution);
  printer.setOutputFileName(fileName);
  printer.setFullPage(true);
  printer.setPaperSize(rect().size(), QPrinter::DevicePixel);
  QPainter printpainter(&printer);
  printpainter.setWindow(rect());
  printpainter.begin(&printer);
  printpainter.setRenderHint(QPainter::NonCosmeticDefaultPen, noCosmeticPen);
  draw(&printpainter);
  printpainter.end();
}

/*
  Function for providing svg export. Requires the QtSvg module
  This is Not tested and will require some modifications!
  
void QCustomPlot::saveSvg(const QString &fileName)
{  
  QSvgGenerator generator;
  generator.setFileName(fileName);
  generator.setSize(QSize(200, 200));
  generator.setViewBox(QRect(0, 0, 200, 200));
  generator.setTitle("");
  generator.setDescription("");
  QPainter painter;
  painter.begin(&generator);
  draw(&painter);
  painter.end();
}
*/

/*!
  Saves a PNG image file to \a fileName on disc. The output plot will have the dimensions \a width
  and \a height in pixels. If either \a width or \a height is zero, the exported image will have
  the same dimensions as the QCustomPlot widget currently has. Line widths and texts etc. are not
  scaled up when larger widths/heights are used. If you want that effect, consider the scaled
  version of this function.
  \see savePngScaled
*/
void QCustomPlot::savePng(const QString &fileName, int width, int height)
{  
  int newWidth, newHeight;
  if (width == 0 || height == 0)
  {
    newWidth = this->width();
    newHeight = this->height();
  } else
  {
    newWidth = width;
    newHeight = height;
  }
  
  QPixmap pngBuffer(newWidth, newHeight);
  QPainter painter(&pngBuffer);
  painter.fillRect(pngBuffer.rect(), mColor);
  QRect oldViewport = mViewport;
  mViewport = QRect(0, 0, newWidth, newHeight);
  updateAxisRect();
  draw(&painter);
  mViewport = oldViewport;
  updateAxisRect();
  pngBuffer.save(fileName);
}

/*!
  Saves a PNG image file to \a fileName on disc. The output plot will have a base \a width and \a
  height which is then scaled by factor \a scale. If you for example set \a width and \a height to
  100 and \a scale to 2, you will end up with a PNG file of size 200*200 in which all graphical
  elements are scaled up by factor 2 (line widths, texts, etc.). This scaling is done not by
  stretching a 100*100 image but by actually scaling the painter, so the result will have full
  200*200 pixel resolution. \see savePng
*/
void QCustomPlot::savePngScaled(const QString &fileName, double scale, int width, int height)
{  
  int newWidth, newHeight;
  if (width == 0 || height == 0)
  {
    newWidth = this->width();
    newHeight = this->height();
  } else
  {
    newWidth = width;
    newHeight = height;
  }
  
  int scaledWidth = scale*newWidth;
  int scaledHeight = scale*newHeight;
  
  QPixmap pngBuffer(scaledWidth, scaledHeight);
  QPainter painter(&pngBuffer);
  painter.setRenderHint(QPainter::NonCosmeticDefaultPen);
  painter.fillRect(pngBuffer.rect(), mColor);
  QRect oldViewport = mViewport;
  mViewport = QRect(0, 0, newWidth, newHeight);
  updateAxisRect();
  painter.scale(scale, scale);
  draw(&painter);
  mViewport = oldViewport;
  updateAxisRect();
  pngBuffer.save(fileName);
}

/*!
  \internal
   Event handler for when the QCustomPlot widget needs repainting.
   This does not cause a replot, but draws the internal buffer QPixmap on
   the widget surface.
*/
void QCustomPlot::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QPainter painter(this);
  painter.drawPixmap(0, 0, buffer);
}

/*!
  \internal
   Event handler for a resize of the QCustomPlot widget. Causes the
   internal buffer QPixmap to be resized to the new size. The
   viewport and the axis rect are resized appropriately. Finally a
   replot is performed.
*/
void QCustomPlot::resizeEvent(QResizeEvent *event)
{
  // resize and repaint the buffer:
  buffer = QPixmap(event->size());
  mViewport = rect();
  setAxisRect(mViewport.adjusted(mMarginLeft, mMarginTop, -mMarginRight, -mMarginBottom));
  replot();
}

/*!
  \internal
  emits the mouseDoubleClick signal
*/
void QCustomPlot::mouseDoubleClickEvent(QMouseEvent *event)
{
  emit mouseDoubleClick(event);
}

/*!
  \internal
  Event handler for when a mouse button is pressed. If the left
  mouse button is pressed, the mDragging flag is set to true and
  some anchor points are set that are needed to determine the distance
  that the range was dragged in the mouseMoveEvent.
  Finally, the mousePress signal is emitted.
  \see mouseMoveEvent, mouseReleaseEvent
*/
void QCustomPlot::mousePressEvent(QMouseEvent *event)
{
  if (event->buttons() & Qt::LeftButton)
  {
    mDragging = true;
    mDragStart = event->pos();
    mDragStartHorzRange = mRangeDragHorzAxis->range();
    mDragStartVertRange = mRangeDragVertAxis->range();
  } else
    mDragging = false;
  emit mousePress(event);
}

/*!
  \internal
  Event handler for when the cursor is moved. First, the mouseMove signal is
  emitted. Then the built-in range dragging functionality is handled.
  \see mousePressEvent, mouseReleaseEvent
*/
void QCustomPlot::mouseMoveEvent(QMouseEvent *event)
{
  emit mouseMove(event);
  if (mDragging)
  {
    if (mRangeDrag.testFlag(Qt::Horizontal))
    {
      if (mRangeDragHorzAxis->mScaleType == QCustomPlotAxis::stLinear)
      {
        double diff = mRangeDragHorzAxis->pixelToCoord(mDragStart.x()) - mRangeDragHorzAxis->pixelToCoord(event->pos().x());
        mRangeDragHorzAxis->setRange(mDragStartHorzRange.lower+diff, mDragStartHorzRange.upper+diff);
      } else if (mRangeDragHorzAxis->mScaleType == QCustomPlotAxis::stLogarithmic)
      {
        double diff = mRangeDragHorzAxis->pixelToCoord(mDragStart.x()) / mRangeDragHorzAxis->pixelToCoord(event->pos().x());
        mRangeDragHorzAxis->setRange(mDragStartHorzRange.lower*diff, mDragStartHorzRange.upper*diff);
      }
    }
    if (mRangeDrag.testFlag(Qt::Vertical))
    {
      if (mRangeDragVertAxis->mScaleType == QCustomPlotAxis::stLinear)
      {
        double diff = mRangeDragVertAxis->pixelToCoord(mDragStart.y()) - mRangeDragVertAxis->pixelToCoord(event->pos().y());
        mRangeDragVertAxis->setRange(mDragStartVertRange.lower+diff, mDragStartVertRange.upper+diff);
      } else if (mRangeDragVertAxis->mScaleType == QCustomPlotAxis::stLogarithmic)
      {
        double diff = mRangeDragVertAxis->pixelToCoord(mDragStart.y()) / mRangeDragVertAxis->pixelToCoord(event->pos().y());
        mRangeDragVertAxis->setRange(mDragStartVertRange.lower*diff, mDragStartVertRange.upper*diff);
      }
    }
    if (mRangeDrag != 0) // if either vertical or horizontal drag was enabled, do a replot
      replot();
  }
}

/*!
  \internal
  Event handler for when a mouse button is released. First, the mDragging flag is set
  to false (used by range dragging functionality) and then the mouseRelease signal is
  emitted.
  \see mousePressEvent, mouseMoveEvent
*/
void QCustomPlot::mouseReleaseEvent(QMouseEvent *event)
{
  mDragging = false;
  emit mouseRelease(event);
}

/*!
  \internal
  Event handler for mouse wheel events. First, the mouseWheel signal is emitted.
  If rangeZoom is Qt::Horizontal, Qt::Vertical or both, the ranges of the axes defined as
  rangeZoomHorzAxis and rangeZoomVertAxis are scaled. The center of the scaling
  operation is the current cursor position inside the plot. The scaling factor
  is dependant on the mouse wheel delta (which direction the wheel was rotated)
  to provide a natural zooming feel. The Strength of the zoom can be controlled via
  \ref setRangeZoomFactor.
  
  Note, that event->delta() is usually +/-120 for single rotation steps. However, if the mouse
  wheel is turned rapidly, many steps may bunch up to one event, so the event->delta() may then be
  multiples of 120. This is taken into account here, by calculating \a wheelSteps and using it as
  exponent of the range zoom factor. This takes care of the wheel direction automatically, by
  inverting the factor, when the wheel step is negative (f^-1 = 1/f).
*/
void QCustomPlot::wheelEvent(QWheelEvent *event)
{
  emit mouseWheel(event);
  if (mRangeZoom != 0)
  {
    double factor;
    double wheelSteps = event->delta()/120.0; // a single step delta is +/-120 usually
    if (mRangeZoom.testFlag(Qt::Horizontal))
    {
      factor = pow(mRangeZoomFactorHorz, wheelSteps);
      mRangeZoomHorzAxis->scaleRange(factor, mRangeZoomHorzAxis->pixelToCoord(event->pos().x()));
    }
    if (mRangeZoom.testFlag(Qt::Vertical))
    {
      factor = pow(mRangeZoomFactorVert, wheelSteps);
      mRangeZoomVertAxis->scaleRange(factor, mRangeZoomVertAxis->pixelToCoord(event->pos().y()));
    }
    replot();
  }
}

/*!
  \internal
  This is the main draw function which first generates the tick vectors of all axes,
  calculates and applies appropriate margins if autoMargin is true and finally draws
  all elements with the passed \a painter. (title, subgrid, grid, axes, graphs)
*/
void QCustomPlot::draw(QPainter *painter)
{
  // prepare values of ticks and tick strings:
  xAxis->generateTickVectors();
  yAxis->generateTickVectors();
  xAxis2->generateTickVectors();
  yAxis2->generateTickVectors();

  // draw title:
  QRect titleBounds;
  if (!mTitle.isEmpty())
  {
    painter->setFont(mTitleFont);
    titleBounds = painter->fontMetrics().boundingRect(0, 0, titleBounds.width(), titleBounds.height(), Qt::TextDontClip | Qt::AlignHCenter | Qt::AlignVCenter, mTitle);
    painter->drawText(mViewport.left(), mViewport.top(), mViewport.width(), titleBounds.height(), Qt::TextDontClip | Qt::AlignHCenter | Qt::AlignVCenter, mTitle);
  }  
  
  // set margin:
  if (mAutoMargin)
  {
    setMargin(yAxis->calculateMargin(),
              yAxis2->calculateMargin(),
              xAxis2->calculateMargin()+titleBounds.height(),
              xAxis->calculateMargin());
  }

  // draw grids:
  xAxis->drawSubGrid(painter);
  yAxis->drawSubGrid(painter);
  xAxis2->drawSubGrid(painter);
  yAxis2->drawSubGrid(painter);
  xAxis->drawGrid(painter);
  yAxis->drawGrid(painter);
  xAxis2->drawGrid(painter);
  yAxis2->drawGrid(painter);
  
  // draw all graphs:
  for (int i=0; i < mGraphs.size(); ++i)
    mGraphs.at(i)->draw(painter);
  
  // draw axes, ticks and axis labels:
  xAxis->drawAxis(painter);
  yAxis->drawAxis(painter);
  xAxis2->drawAxis(painter);
  yAxis2->drawAxis(painter);
  
  // draw legend:
  legend->reArrange();
  legend->draw(painter);
}

/*!
  \internal
  calculates mAxisRect by applying the margins inward to mViewport. The axisRect is then
  passed on to all axes via QCustomPlotAxis::setAxisRect
  \see setMargin, setAxisRect
*/
void QCustomPlot::updateAxisRect()
{
  mAxisRect = mViewport.adjusted(mMarginLeft, mMarginTop, -mMarginRight, -mMarginBottom);
  xAxis->setAxisRect(mAxisRect);
  yAxis->setAxisRect(mAxisRect);
  xAxis2->setAxisRect(mAxisRect);
  yAxis2->setAxisRect(mAxisRect);
}

