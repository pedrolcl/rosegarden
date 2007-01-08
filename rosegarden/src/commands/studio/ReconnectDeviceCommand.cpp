/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
 
    This program is Copyright 2000-2007
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


#include "ReconnectDeviceCommand.h"

#include "misc/Strings.h"
#include "misc/Debug.h"
#include "base/Device.h"
#include "base/Studio.h"
#include <qcstring.h>
#include <qdatastream.h>
#include <qstring.h>
#include "gui/application/RosegardenApplication.h"


namespace Rosegarden
{

void
ReconnectDeviceCommand::execute()
{
    Device *device = m_studio->getDevice(m_deviceId);

    if (device) {
        m_oldConnection = device->getConnection();

        QByteArray data;
        QByteArray replyData;
        QCString replyType;
        QDataStream arg(data, IO_WriteOnly);

        arg << (unsigned int)m_deviceId;
        arg << strtoqstr(m_newConnection);

        if (!rgapp->sequencerCall("setConnection(unsigned int, QString)",
                                  replyType, replyData, data)) {
            SEQMAN_DEBUG << "ReconnectDeviceCommand::execute - "
            << "failure in sequencer setConnection" << endl;
            return ;
        }

        SEQMAN_DEBUG << "ReconnectDeviceCommand::execute - "
        << " reconnected device " << m_deviceId
        << " to " << m_newConnection << endl;
    }
}

void
ReconnectDeviceCommand::unexecute()
{
    Device *device = m_studio->getDevice(m_deviceId);

    if (device) {

        QByteArray data;
        QByteArray replyData;
        QCString replyType;
        QDataStream arg(data, IO_WriteOnly);

        arg << (unsigned int)m_deviceId;
        arg << strtoqstr(m_oldConnection);

        if (!rgapp->sequencerCall("setConnection(unsigned int, QString)",
                                  replyType, replyData, data)) {
            SEQMAN_DEBUG << "ReconnectDeviceCommand::unexecute - "
            << "failure in sequencer setConnection" << endl;
            return ;
        }

        SEQMAN_DEBUG << "ReconnectDeviceCommand::unexecute - "
        << " reconnected device " << m_deviceId
        << " to " << m_oldConnection << endl;
    }
}

}
