#include "qt/graphics/QtGraphicsView.h"

#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>

QtGraphicsView::QtGraphicsView(QWidget* parent)
	: QGraphicsView(parent)
	, m_zoomFactor(1.0f)
	, m_appZoomFactor(1.0f)
	, m_up(false)
	, m_down(false)
	, m_left(false)
	, m_right(false)
	, m_shift(false)
{
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	m_timer = std::make_shared<QTimer>(this);
	connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(update()));

	m_timerStopper = std::make_shared<QTimer>(this);
	m_timerStopper->setSingleShot(true);
	connect(m_timerStopper.get(), SIGNAL(timeout()), this, SLOT(stopTimer()));
}

float QtGraphicsView::getZoomFactor() const
{
	return m_appZoomFactor;
}

void QtGraphicsView::setAppZoomFactor(float appZoomFactor)
{
	m_appZoomFactor = appZoomFactor;
	updateTransform();
}

void QtGraphicsView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && !itemAt(event->pos()))
	{
		m_last = event->pos();
	}

	QGraphicsView::mousePressEvent(event);
}

void QtGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && !itemAt(event->pos()) && event->pos() == m_last)
	{
		emit emptySpaceClicked();
	}

	QGraphicsView::mouseReleaseEvent(event);
	viewport()->setCursor(Qt::ArrowCursor);
}

void QtGraphicsView::keyPressEvent(QKeyEvent* event)
{
	bool moved = moves();

	switch (event->key())
	{
		case Qt::Key_W:
			m_up = true;
			break;
		case Qt::Key_A:
			m_left = true;
			break;
		case Qt::Key_S:
			m_down = true;
			break;
		case Qt::Key_D:
			m_right = true;
			break;
		case Qt::Key_0:
			m_zoomFactor = 1.0f;
			updateTransform();
			break;
		case Qt::Key_Shift:
			m_shift = true;
			break;
		default:
			QGraphicsView::keyPressEvent(event);
			return;
	}

	if (!moved && moves())
	{
		m_timer->start(20);
	}

	m_timerStopper->start(1000);
}

void QtGraphicsView::keyReleaseEvent(QKeyEvent* event)
{
	switch (event->key())
	{
		case Qt::Key_W:
			m_up = false;
			break;
		case Qt::Key_A:
			m_left = false;
			break;
		case Qt::Key_S:
			m_down = false;
			break;
		case Qt::Key_D:
			m_right = false;
			break;
		case Qt::Key_Shift:
			m_shift = false;
			break;
		default:
			return;
	}

	if (!moves())
	{
		m_timer->stop();
	}
}

void QtGraphicsView::wheelEvent(QWheelEvent* event)
{
	if (event->modifiers() == Qt::ShiftModifier && event->delta() != 0.0f)
	{
		updateZoom(event->delta());
		return;
	}

	QGraphicsView::wheelEvent(event);
}

void QtGraphicsView::update()
{
	float ds = 30.0f;
	float dz = 50.0f;

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	if (m_shift)
	{
		if (m_up)
		{
			z += dz;
		}
		else if (m_down)
		{
			z -= dz;
		}
	}
	else
	{
		if (m_up)
		{
			y -= ds;
		}
		else if (m_down)
		{
			y += ds;
		}

		if (m_left)
		{
			x -= ds;
		}
		else if (m_right)
		{
			x += ds;
		}
	}

	if (x != 0)
	{
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() + x);
	}

	if (y != 0)
	{
		verticalScrollBar()->setValue(verticalScrollBar()->value() + y);
	}

	if (z != 0)
	{
		updateZoom(z);
	}
}

void QtGraphicsView::stopTimer()
{
	m_timer->stop();
}

bool QtGraphicsView::moves() const
{
	return m_up || m_down || m_left || m_right;
}

void QtGraphicsView::updateTransform()
{
	float zoomFactor = m_appZoomFactor * m_zoomFactor;
	setTransform(QTransform(zoomFactor, 0, 0, zoomFactor, 0, 0));
}

void QtGraphicsView::updateZoom(float delta)
{
	float factor = 1.0f + 0.001 * delta;

	if (factor <= 0.0f)
	{
		factor = 0.000001;
	}

	double newZoom = m_zoomFactor * factor;
	m_zoomFactor = qBound(0.1, newZoom, 100.0);

	updateTransform();
}