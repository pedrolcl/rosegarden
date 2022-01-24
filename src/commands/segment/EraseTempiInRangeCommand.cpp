/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2022 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#include "EraseTempiInRangeCommand.h"
#include "base/Composition.h"
#include "base/Selection.h"


namespace Rosegarden
{

EraseTempiInRangeCommand::EraseTempiInRangeCommand(Composition *composition,
        timeT rangeBegin,
        timeT rangeEnd) :
        NamedCommand(tr("Erase Tempos in Range")),
        m_composition(composition),
	m_temposPre(TempoSelection(*composition, rangeBegin,
				   rangeEnd, false))
{}

EraseTempiInRangeCommand::~EraseTempiInRangeCommand()
{}

void
EraseTempiInRangeCommand::execute()
{
    m_temposPre.RemoveFromComposition(m_composition);
}

void
EraseTempiInRangeCommand::unexecute()
{
    m_temposPre.AddToComposition(m_composition);
}
}
