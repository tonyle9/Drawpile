/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2013-2017 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QPainter>
#include <QFontMetrics>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>

#include "usermarkeritem.h"

namespace drawingboard {


namespace {
static const float ARROW = 10;

}
UserMarkerItem::UserMarkerItem(int id, QGraphicsItem *parent)
	: QGraphicsItem(parent), m_id(id), _fadeout(0), m_showSubtext(false)
{
	setFlag(ItemIgnoresTransformations);
	_bgbrush.setStyle(Qt::SolidPattern);
	QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
	shadow->setOffset(0);
	shadow->setBlurRadius(10);
	setGraphicsEffect(shadow);
	setZValue(9999);
}

void UserMarkerItem::setColor(const QColor &color)
{
	_bgbrush.setColor(color);

	if ((color.red() * 30) + (color.green() * 59) + (color.blue() * 11) > 12800)
		_textpen = QPen(Qt::black);
	else
		_textpen = QPen(Qt::white);

	update();
}

const QColor &UserMarkerItem::color() const
{
	return _bgbrush.color();
}

void UserMarkerItem::setText(const QString &text)
{
	if(_text1 != text) {
		_text1 = text;
		updateFullText();
	}
}

void UserMarkerItem::setSubtext(const QString &text)
{
	if(_text2 != text) {
		_text2 = text;
		if(m_showSubtext)
			updateFullText();
	}
}

void UserMarkerItem::setShowSubtext(bool show)
{
	if(m_showSubtext != show) {
		m_showSubtext = show;
		updateFullText();
	}
}

void UserMarkerItem::updateFullText()
{
	prepareGeometryChange();

	if(_text2.isEmpty() || !m_showSubtext)
		_fulltext = _text1;
	else
		_fulltext = _text1 + "\n[" + _text2 + ']';

	// Make a new bubble for the text
	QRect textrect = qApp->fontMetrics().boundingRect(QRect(0, 0, 0xffff, 0xffff), 0, _fulltext);

	const float round = 3;
	const float padding = 5;
	const float width = qMax((ARROW+round)*2, textrect.width() + 2*padding);
	const float rad = width / 2.0;
	const float height = textrect.height() + ARROW + 2 * padding;

	_bounds = QRectF(-rad, -height, width, height);

	_textrect = _bounds.adjusted(padding, padding, -padding, -padding);

	_bubble = QPainterPath(QPointF(0, 0));

	_bubble.lineTo(-ARROW, -ARROW);
	_bubble.lineTo(-rad+round, -ARROW);

	_bubble.quadTo(-rad, -ARROW, -rad, -ARROW-round);
	_bubble.lineTo(-rad, -height+round);
	_bubble.quadTo(-rad, -height, -rad+round, -height);

	_bubble.lineTo(rad-round, -height);
	_bubble.quadTo(rad, -height, rad, -height+round);
	_bubble.lineTo(rad, -ARROW-round);

	_bubble.quadTo(rad, -ARROW, rad-round, -ARROW);
	_bubble.lineTo(ARROW, -ARROW);

	_bubble.closeSubpath();
}

QRectF UserMarkerItem::boundingRect() const
{
	return _bounds;
}

void UserMarkerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	painter->setRenderHint(QPainter::Antialiasing);
	painter->setPen(Qt::NoPen);

	painter->setBrush(_bgbrush);
	painter->drawPath(_bubble);

	painter->setFont(qApp->font());
	painter->setPen(_textpen);
	painter->drawText(_textrect, Qt::AlignHCenter|Qt::AlignTop, _fulltext);
}

void UserMarkerItem::fadein()
{
	_fadeout = 0;
	setOpacity(1);
	show();
}

void UserMarkerItem::fadeout()
{
	_fadeout = 1.0;
}

bool UserMarkerItem::fadeoutStep(float dt)
{
	if(_fadeout>0) {
		_fadeout -= dt;
		if(_fadeout <= 0.0) {
			hide();
			return true;
		} else if(_fadeout < 1.0)
			setOpacity(_fadeout);
	}
	return false;
}

}
