
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

#ifndef _RG_SEGMENTSELECTOR_H_
#define _RG_SEGMENTSELECTOR_H_

#include "SegmentTool.h"
#include <QPoint>
#include <QString>


class QMouseEvent;


namespace Rosegarden
{

class RosegardenGUIDoc;
class CompositionView;


class SegmentSelector : public SegmentTool
{
    Q_OBJECT

    friend class SegmentToolBox;
    friend class SegmentTool;

public:

    virtual ~SegmentSelector();

    virtual void ready();
    virtual void stow();

    virtual void handleMouseButtonPress(QMouseEvent*);
    virtual void handleMouseButtonRelease(QMouseEvent*);
    virtual int  handleMouseMove(QMouseEvent*);

    // These two alter the behaviour of the selection mode
    //
    // - SegmentAdd (usually when Qt::SHIFT is held down) allows
    //   multiple selections of Segments.
    //
    // - SegmentCopy (usually CONTROL) allows draw and drop
    //   copying of Segments - it's a quick shortcut
    //
    void setSegmentAdd(const bool &value)  { m_segmentAddMode = value; }
    void setSegmentCopy(const bool &value) { m_segmentCopyMode = value; }

    bool isSegmentAdding() const { return m_segmentAddMode; }
    bool isSegmentCopying() const { return m_segmentCopyMode; }

    // Return the SegmentItem list for other tools to use
    //
    SegmentItemList* getSegmentItemList() { return &m_selectedItems; }

    static const QString ToolName;

protected slots:
    void slotCanvasScrolled(int newX, int newY);

protected:
    SegmentSelector(CompositionView*, RosegardenGUIDoc*);

    void setContextHelpFor(QPoint p, bool ctrlPressed = false);

    //--------------- Data members ---------------------------------

    SegmentItemList m_selectedItems;

    bool m_segmentAddMode;
    bool m_segmentCopyMode;
    QPoint m_clickPoint;
    bool m_segmentQuickCopyDone;
    bool m_passedInertiaEdge;
    bool m_buttonPressed;
    bool m_selectionMoveStarted;

    SegmentTool *m_dispatchTool;
};



}

#endif
