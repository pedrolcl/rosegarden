// -*- c-basic-offset: 4 -*-

/*
    Rosegarden-4
    A sequencer and musical notation editor.

    This program is Copyright 2000-2003
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

#include <kcmdlineargs.h>
#include <dcopclient.h>

#include "rgapplication.h"
#include "Exception.h"
#include "rosegardengui.h"
#include "rosegardenguidoc.h"

int RosegardenApplication::newInstance()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (RosegardenGUIApp::self() && args->count() &&
        RosegardenGUIApp::self()->getDocument() &&
        RosegardenGUIApp::self()->getDocument()->saveIfModified())
    {
        // Check for modifications and save if necessary - if cancelled
        // then don't load the new file.
        //
        RosegardenGUIApp::self()->openFile(args->arg(0));
    }

    return KUniqueApplication::newInstance();
}

void RosegardenApplication::sequencerSend(QCString s, QByteArray params)
{
    if (!dcopClient()->send(ROSEGARDEN_SEQUENCER_APP_NAME,
                            ROSEGARDEN_SEQUENCER_IFACE_NAME,
                            s, params))
        throw Rosegarden::Exception("failed to reach the sequencer through DCOP");
}

void RosegardenApplication::sequencerCall(QCString s, QCString& replyType, QByteArray& replyData, QByteArray params)
{
    if (!dcopClient()->call(ROSEGARDEN_SEQUENCER_APP_NAME,
                            ROSEGARDEN_SEQUENCER_IFACE_NAME,
                            s, params, replyType, replyData))
        throw Rosegarden::Exception("failed to reach the sequencer through DCOP");
}

RosegardenApplication* RosegardenApplication::rgApp()
{
    return dynamic_cast<RosegardenApplication*>(kApplication());
}


QByteArray RosegardenApplication::Empty;

