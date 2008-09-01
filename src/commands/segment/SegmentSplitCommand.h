
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

#ifndef _RG_SEGMENTSPLITCOMMAND_H_
#define _RG_SEGMENTSPLITCOMMAND_H_

#include <string>
#include "document/Command.h"
#include "base/Event.h"




namespace Rosegarden
{

class Segment;


class SegmentSplitCommand : public NamedCommand
{
public:

    // If keepLabel is true, "(split)" is not append to the new segments label
    SegmentSplitCommand(Segment *segment,
                        timeT splitTime,
                        bool keepLabel = false);
    virtual ~SegmentSplitCommand();

    virtual void execute();
    virtual void unexecute();
    
    Segment *getSegmentA() { return m_newSegmentA; }
    Segment *getSegmentB() { return m_newSegmentB; }

private:
    Segment *m_segment;
    Segment *m_newSegmentA;
    Segment *m_newSegmentB;
    timeT m_splitTime;
    bool m_detached;
    bool m_keepLabel;
};


}

#endif
