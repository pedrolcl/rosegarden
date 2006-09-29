
/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.

    This program is Copyright 2000-2006
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <richard.bown@ferventsoftware.com>

    The moral rights of Guillaume Laurent, Chris Cannam, and Richard
    Bown to claim authorship of this work have been asserted.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RG_NOTEINSERTIONCOMMAND_H_
#define _RG_NOTEINSERTIONCOMMAND_H_

#include "base/NotationTypes.h"
#include "document/BasicCommand.h"
#include "base/Event.h"
#include "gui/editors/notation/NoteStyle.h"



namespace Rosegarden
{

class Segment;
class Event;


class NoteInsertionCommand : public BasicCommand
{
public:
    NoteInsertionCommand(Segment &segment,
			 timeT time,
			 timeT endTime,
			 Note note,
			 int pitch,
			 Accidental accidental,
			 bool autoBeam,
			 bool matrixType,
			 NoteStyleName noteStyle);
    virtual ~NoteInsertionCommand();

    Event *getLastInsertedEvent() { return m_lastInsertedEvent; }

protected:
    virtual void modifySegment();

    timeT getModificationStartTime(Segment &,
					       timeT);

    timeT m_insertionTime;
    Note m_note;
    int m_pitch;
    Accidental m_accidental;
    bool m_autoBeam;
    bool m_matrixType;
    NoteStyleName m_noteStyle;

    Event *m_lastInsertedEvent;
};


}

#endif
