/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2008 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#include <Q3Canvas>
#include <Q3CanvasItem>
#include <Q3CanvasItemList>
#include <Q3CanvasText>
#include <Q3CanvasView>
#include "NotationCanvasView.h"
#include "misc/Debug.h"

#include "misc/Strings.h"
#include "gui/general/LinedStaffManager.h"
#include "gui/general/RosegardenCanvasView.h"
#include "gui/kdeext/QCanvasGroupableItem.h"
#include "gui/kdeext/QCanvasSimpleSprite.h"
#include "NotationElement.h"
#include "NotationProperties.h"
#include "NotationStaff.h"

#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>

namespace Rosegarden
{

NotationCanvasView::NotationCanvasView(const LinedStaffManager &staffmgr,
                                       Q3Canvas *viewing, QWidget *parent,
                                       const char *name
					): //, WFlags f) :
        RosegardenCanvasView(viewing, parent, name), // f),
        m_linedStaffManager(staffmgr),
        m_lastYPosNearStaff(0),
        m_currentStaff(0),
        m_currentHeight( -1000),
        m_legerLineOffset(false),
        m_heightTracking(false)
{
    // -- switching mandolin-sonatina first staff to page mode:
    // default params (I think 16,100): render 1000ms position 1070ms
    // 64,100: 1000ms 980ms
    // 8, 100: 1140ms 1140ms
    // 128, 100: 1060ms 980ms
    // 256, 100: 1060ms 980ms / 930ms 920ms

    //    canvas()->retune(256, 100);

    viewport()->setMouseTracking(true);

    m_heightMarker = new QCanvasItemGroup(viewing);

    m_vert1 = new QCanvasLineGroupable(viewing, m_heightMarker);
    m_vert1->setPoints(0, 0, 0, 8);
    m_vert1->setPen(QPen(QColor(64, 64, 64), 1));

    m_vert2 = new QCanvasLineGroupable(viewing, m_heightMarker);
    m_vert2->setPoints(17, 0, 17, 8);
    m_vert2->setPen(QPen(QColor(64, 64, 64), 1));

    m_heightMarker->hide();
}

NotationCanvasView::~NotationCanvasView()
{
    // All canvas items are deleted in ~NotationView()
}

void
NotationCanvasView::setHeightTracking(bool t)
{
    m_heightTracking = t;
    if (!t) {
        m_heightMarker->hide();
        canvas()->update();
    }
}

void
NotationCanvasView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    emit mouseReleased(e);
}

void
NotationCanvasView::contentsMouseMoveEvent(QMouseEvent *e)
{
    NotationStaff *prevStaff = m_currentStaff;
    int prevHeight = m_currentHeight;

    m_currentStaff = dynamic_cast<NotationStaff *>
                     (m_linedStaffManager.getStaffForCanvasCoords(e->x(), e->y()));

    if (!m_currentStaff) {

        emit hoveredOverNoteChanged(QString::null);
        if (prevStaff) {
            m_heightMarker->hide();
            canvas()->update();
        }

    } else {

        m_currentHeight = m_currentStaff->getHeightAtCanvasCoords(e->x(), e->y());

        int x = e->x() - 8; // magic based on mouse cursor size
        bool needUpdate = (m_heightTracking && (m_heightMarker->x() != x));
        m_heightMarker->setX(x);

        if (prevStaff != m_currentStaff ||
                prevHeight != m_currentHeight) {

            if (m_heightTracking) {
                setHeightMarkerHeight(e);
                m_heightMarker->show();
                needUpdate = true;
            }

            emit hoveredOverNoteChanged
            (strtoqstr
             (m_currentStaff->getNoteNameAtCanvasCoords(e->x(), e->y())));
        }

        if (needUpdate)
            canvas()->update();
    }

    NotationElement *elt = getElementAtXCoord(e);
    if (elt) {
        emit hoveredOverAbsoluteTimeChanged(elt->getViewAbsoluteTime());
    }

    // if(tracking) ??
    emit mouseMoved(e);
}

void NotationCanvasView::contentsMousePressEvent(QMouseEvent *e)
{
    NOTATION_DEBUG << "NotationCanvasView::contentsMousePressEvent() - btn : "
    << e->button() << " - state : " << e->state()
    << endl;

    Q3CanvasItemList itemList = canvas()->collisions(e->pos());

    // We don't want to use m_currentStaff/Height, because we want
    // to make sure the event happens at the point we clicked at
    // rather than the last point for which contentsMouseMoveEvent
    // happened to be called

    NotationStaff *staff = dynamic_cast<NotationStaff *>
                           (m_linedStaffManager.getStaffForCanvasCoords(e->x(), e->y()));

    Q3CanvasItemList::Iterator it;
    NotationElement *clickedNote = 0;
    NotationElement *clickedVagueNote = 0;
    NotationElement *clickedNonNote = 0;

    bool haveClickHeight = false;
    int clickHeight = 0;
    if (staff) {
        clickHeight = staff->getHeightAtCanvasCoords(e->x(), e->y());
        haveClickHeight = true;
    }

    for (it = itemList.begin(); it != itemList.end(); ++it) {

        if ((*it)->active()) {
            emit activeItemPressed(e, *it);
            return ;
        }

        QCanvasNotationSprite *sprite =
            dynamic_cast<QCanvasNotationSprite*>(*it);
        if (!sprite) {
            if (dynamic_cast<QCanvasNonElementSprite *>(*it)) {
                emit nonNotationItemPressed(e, *it);
                return ;
            } else if (dynamic_cast<Q3CanvasText *>(*it)) {
                emit textItemPressed(e, *it);
                return ;
            }
            continue;
        }

        NotationElement &el = sprite->getNotationElement();

        // #957364 (Notation: Hard to select upper note in chords of
        // seconds) -- adjust x-coord for shifted note head

        double cx = el.getCanvasX();
        int nbw = 10;

        if (staff) {
            nbw = staff->getNotePixmapFactory(false).getNoteBodyWidth();
            bool shifted = false;

            if (el.event()->get
                    <Bool>
                    (staff->getProperties().NOTE_HEAD_SHIFTED, shifted) && shifted) {
                cx += nbw;
            }
        }

        if (el.isNote() && haveClickHeight) {
            long eventHeight = 0;
            if (el.event()->get
                    <Int>
                    (NotationProperties::HEIGHT_ON_STAFF, eventHeight)) {

                if (eventHeight == clickHeight) {

                    if (!clickedNote &&
                            e->x() >= cx &&
                            e->x() <= cx + nbw) {
                        clickedNote = &el;
                    } else if (!clickedVagueNote &&
                               e->x() >= cx - 2 &&
                               e->x() <= cx + nbw + 2) {
                        clickedVagueNote = &el;
                    }

                } else if (eventHeight - 1 == clickHeight ||
                           eventHeight + 1 == clickHeight) {
                    if (!clickedVagueNote)
                        clickedVagueNote = &el;
                }
            }
        } else if (!el.isNote()) {
            if (!clickedNonNote)
                clickedNonNote = &el;
        }
    }

    int staffNo = -1;
    if (staff)
        staffNo = staff->getId();

    if (clickedNote)
        handleMousePress(clickHeight, staffNo, e, clickedNote);
    else if (clickedNonNote)
        handleMousePress(clickHeight, staffNo, e, clickedNonNote);
    else if (clickedVagueNote)
        handleMousePress(clickHeight, staffNo, e, clickedVagueNote);
    else
        handleMousePress(clickHeight, staffNo, e);
}

void NotationCanvasView::contentsMouseDoubleClickEvent(QMouseEvent* e)
{
    NOTATION_DEBUG << "NotationCanvasView::contentsMouseDoubleClickEvent()\n";

    contentsMousePressEvent(e);
}

void
NotationCanvasView::processActiveItems(QMouseEvent* e,
                                       Q3CanvasItemList itemList)
{
    Q3CanvasItem* pressedItem = 0;
    Q3CanvasItemList::Iterator it;

    for (it = itemList.begin(); it != itemList.end(); ++it) {

        Q3CanvasItem *item = *it;
        if (item->active() && !pressedItem) {
            NOTATION_DEBUG << "mousepress : got active item\n";
            pressedItem = item;
        }
    }

    if (pressedItem)
        emit activeItemPressed(e, pressedItem);

}

void
NotationCanvasView::handleMousePress(int height,
                                     int staffNo,
                                     QMouseEvent *e,
                                     NotationElement *el)
{
    NOTATION_DEBUG << "NotationCanvasView::handleMousePress() at height "
    << height << endl;

    emit itemPressed(height, staffNo, e, el);
}

bool
NotationCanvasView::posIsTooFarFromStaff(const QPoint &pos)
{
    // return true if pos.y is more than m_staffLineThreshold away from
    // the last pos for which a collision was detected
    //
    return (pos.y() > m_lastYPosNearStaff) ?
           (pos.y() - m_lastYPosNearStaff) > (int)m_staffLineThreshold :
           (m_lastYPosNearStaff - pos.y()) > (int)m_staffLineThreshold;

}

int
NotationCanvasView::getLegerLineCount(int height, bool &offset)
{
    //!!! This is far too specifically notation-related to be here, really

    if (height < 0) {

        offset = (( -height % 2) == 1);
        return height / 2;

    } else if (height > 8) {

        offset = ((height % 2) == 1);
        return (height - 8) / 2;
    }

    return 0;
}

void
NotationCanvasView::setHeightMarkerHeight(QMouseEvent *e)
{
    NotationStaff *staff = dynamic_cast<NotationStaff *>
                           (m_linedStaffManager.getStaffForCanvasCoords(e->x(), e->y()));

    int height = staff->getHeightAtCanvasCoords(e->x(), e->y());
    int lineY = staff->getCanvasYForHeight(height, e->x(), e->y());

    //    NOTATION_DEBUG << "NotationCanvasView::setHeightMarkerHeight: "
    //			 << e->y() << " snapped to line -> " << lineY
    //			 << " (height " << height << ")" << endl;

    int spacing = staff->getLineSpacing() - 1;

    m_staffLineThreshold = spacing;
    m_vert1->setPoints(0, -spacing / 2, 0, spacing / 2);
    m_vert2->setPoints(17, -spacing / 2, 17, spacing / 2); // magic based on mouse cursor size
    m_heightMarker->setY(lineY);

    bool legerLineOffset = false;
    int legerLineCount = getLegerLineCount(height, legerLineOffset);

    if (legerLineCount != (int)m_legerLines.size() ||
            legerLineOffset != m_legerLineOffset) {

        bool above = false;
        if (legerLineCount < 0) {
            above = true;
            legerLineCount = -legerLineCount;
        }

        int i;
        for (i = 0; i < (int)m_legerLines.size(); ++i) {
            delete m_legerLines[i];
        }
        m_legerLines.clear();

        for (i = 0; i < legerLineCount; ++i) {

            QCanvasLineGroupable *line =
                new QCanvasLineGroupable(canvas(), m_heightMarker);

            line->setPen(QPen(QColor(64, 64, 64), 1));

            int y = (int)m_heightMarker->y() +
                    (above ? -1 : 1) * (i * (spacing + 1));
            int x = (int)m_heightMarker->x() + 1;

            if (legerLineOffset) {
                if (above)
                    y -= spacing / 2 + 1;
                else
                    y += spacing / 2;
            }

            line->setPoints(x, y, x + 15, y); // magic based on mouse cursor size
            m_legerLines.push_back(line);
        }

        m_legerLineOffset = legerLineOffset;
    }
}

NotationElement *
NotationCanvasView::getElementAtXCoord(QMouseEvent *e) // any old element
{
    QRect threshold(e->pos(), QSize(4, 100)); //!!!
    threshold.moveCenter(e->pos());

    Q3CanvasItemList itemList = canvas()->collisions(threshold);

    Q3CanvasItemList::Iterator it;
    QCanvasNotationSprite* sprite = 0;

    for (it = itemList.begin(); it != itemList.end(); ++it)
    {

        Q3CanvasItem *item = *it;

        if ((sprite = dynamic_cast<QCanvasNotationSprite*>(item))) {
            return & (sprite->getNotationElement());
        }
    }

    return 0;
}

void
NotationCanvasView::viewportPaintEvent(QPaintEvent *e)
{
    int cx(e->rect().x()),
    cy(e->rect().y()),
    cw(e->rect().width()) /*,
            ch(e->rect().height())*/; 
    //    NOTATION_DEBUG << "NotationCanvasView::viewportPaintEvent: (" << cx << ","
    //		   << cy << ") size (" << cw << "x" << ch << ")" << endl;
    Q3CanvasView::viewportPaintEvent(e);

    cx += contentsX();
    cy += contentsY();
    m_lastRender = e->rect();
    emit renderRequired(std::min(contentsX(), cx),
                        std::max(contentsX() + visibleWidth(), cx + cw));
}

void
NotationCanvasView::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
    /*
        m_lastRender = QRect(cx, cy, cw, ch);
        NOTATION_DEBUG << "NotationCanvasView::drawContents: (" << cx << ","
    		   << cy << ") size (" << cw << "x" << ch << ")" << endl;
    */
    Q3CanvasView::drawContents(p, cx, cy, cw, ch);
    /*
        emit renderRequired(std::min(contentsX(), cx),
    			std::max(contentsX() + visibleWidth(), cx + cw));
    */
}

void
NotationCanvasView::slotRenderComplete()
{
    /*    QPainter painter(viewport());
        int cx(m_lastRender.x()),
    	cy(m_lastRender.y()),
    	cw(m_lastRender.width()),
    	ch(m_lastRender.height());
        NOTATION_DEBUG << "NotationCanvasView::slotRenderComplete: (" << cx << ","
    		   << cy << ") size (" << cw << "x" << ch << ")" << endl;
        Q3CanvasView::drawContents(&painter, cx, cy, cw, ch);
    */
    QPaintEvent ev(m_lastRender);
    Q3CanvasView::viewportPaintEvent(&ev);
}

void
NotationCanvasView::slotExternalWheelEvent(QWheelEvent* e)
{
    wheelEvent(e);
}

}
#include "NotationCanvasView.moc"
