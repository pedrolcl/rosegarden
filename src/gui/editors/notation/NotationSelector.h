
/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2009 the Rosegarden development team.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RG_NOTATIONSELECTOR_H_
#define _RG_NOTATIONSELECTOR_H_

#include <Q3CanvasRectangle>
#include "NotationTool.h"
#include "NotationElement.h"
#include <QString>
#include "base/Event.h"


class QMouseEvent;
class Q3CanvasRectangle;
class m_clickedElement;


namespace Rosegarden
{

class ViewElement;
class NotationView;
class NotationStaff;
class NotationElement;
class EventSelection;
class Event;


/**
 * Rectangular note selection
 */
class NotationSelector : public NotationTool
{
    Q_OBJECT

    friend class NotationToolBox;

public:

    ~NotationSelector();

    virtual void handleLeftButtonPress(timeT,
                                       int height,
                                       int staffNo,
                                       QMouseEvent*,
                                       ViewElement* el);

    virtual void handleRightButtonPress(timeT time,
                                        int height,
                                        int staffNo,
                                        QMouseEvent*,
                                        ViewElement*);

    virtual int handleMouseMove(timeT,
                                int height,
                                QMouseEvent*);

    virtual void handleMouseRelease(timeT time,
                                    int height,
                                    QMouseEvent*);

    virtual void handleMouseDoubleClick(timeT,
                                        int height,
                                        int staffNo,
                                        QMouseEvent*,
                                        ViewElement*);

    virtual void handleMouseTripleClick(timeT,
                                        int height,
                                        int staffNo,
                                        QMouseEvent*,
                                        ViewElement*);

    /**
     * Create the selection rect
     *
     * We need this because NotationView deletes all QCanvasItems
     * along with it. This happens before the NotationSelector is
     * deleted, so we can't delete the selection rect in
     * ~NotationSelector because that leads to double deletion.
     */
    virtual void ready();

    /**
     * Delete the selection rect.
     */
    virtual void stow();

    /**
     * Returns the currently selected events
     *
     * The returned result is owned by the caller
     */
    EventSelection* getSelection();

    /**
     * Respond to an event being deleted -- it may be the one the tool
     * is remembering as the current event.
     */
    virtual void handleEventRemoved(Event *event) {
        if (m_clickedElement && m_clickedElement->event() == event) {
            m_clickedElement = 0;
        }
    }

    static const QString ToolName;

signals:
    void editElement(NotationStaff *, NotationElement *, bool advanced);

public slots:
    /**
     * Hide the selection rectangle
     *
     * Should be called after a cut or a copy has been
     * performed
     */
    void slotHideSelection();
    
    void slotInsertSelected();
    void slotEraseSelected();
//    void slotCollapseRests();
    void slotCollapseRestsHard();
    void slotRespellFlat();
    void slotRespellSharp();
    void slotRespellNatural();
    void slotCollapseNotes();
    void slotInterpret();
    void slotStaffAbove();
    void slotStaffBelow();
    void slotMakeInvisible();
    void slotMakeVisible();

    void slotClickTimeout();

protected:
    NotationSelector(NotationView*);

    /**
     * Set the current selection on the parent NotationView
     */
    void setViewCurrentSelection(bool preview);

    /**
     * Look up the staff containing the given notation element
     */
    NotationStaff *getStaffForElement(NotationElement *elt);

    void drag(int x, int y, bool final);
    void dragFine(int x, int y, bool final);

    //--------------- Data members ---------------------------------

    Q3CanvasRectangle* m_selectionRect;
    bool m_updateRect;

    NotationStaff *m_selectedStaff;
    NotationElement *m_clickedElement;
    bool m_clickedShift;
    bool m_startedFineDrag;

    EventSelection *m_selectionToMerge;

    long m_lastDragPitch;
    timeT m_lastDragTime;

    bool m_justSelectedBar;
    bool m_wholeStaffSelectionComplete;
};



}

#endif
