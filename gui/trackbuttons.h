// -*- c-basic-offset: 4 -*-

/*
    Rosegarden-4 v0.1
    A sequencer and musical notation editor.

    This program is Copyright 2000-2001
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <bownie@bownie.com>

    The moral right of the authors to claim authorship of this work
    has been asserted.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#ifndef _TRACKBUTTONS_H_
#define _TRACKBUTTONS_H_

#include <qvbox.h>
#include <qheader.h>
#include <qbuttongroup.h>

#include <list>

#include "trackheader.h"
#include "rosegardenguidoc.h"


// This class creates a list of mute and record buttons
// based on the rosegarden document and a specialisation
// of the Vertical Box widget.
//
//
// 

class TrackButtons : public QVBox
{
    
    Q_OBJECT

public:
    TrackButtons(RosegardenGUIDoc* doc,
                 QWidget* parent = 0,
                 Rosegarden::TrackHeader* trackHeader = 0,
                 QHeader *hHeader = 0,
                 const char* name = 0,
                 WFlags f=0);

    TrackButtons(RosegardenGUIDoc* doc,
                 QWidget* parent = 0,
                 const char* name = 0,
                 WFlags f=0);

    TrackButtons(QWidget* parent = 0, const char* name = 0, WFlags f=0);
    ~TrackButtons();

    // Return the track selected for recording
    //
    int selectedRecordTrack();

    // Return a list of muted tracks
    //
    list<int> mutedTracks();


public slots:

    // We use these to set the Track and Composition
    // objects with our returned values
    //
    void setRecordTrack(int id);
    void toggleMutedTrack(int mutedTrack);


private:

    void drawButtons();

    //--------------- Data members ---------------------------------

    RosegardenGUIDoc *m_doc;

    Rosegarden::TrackHeader *vHeader;
    QHeader *hHeader;

    QButtonGroup *m_recordButtonGroup;
    QButtonGroup *m_muteButtonGroup;

    int m_tracks;
    int m_offset;
    int m_cellSize;
    int m_lastID;

};


#endif // _TRACKBUTTONS_H_


