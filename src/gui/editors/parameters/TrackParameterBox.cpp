/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2016 the Rosegarden development team.
 
    This file is Copyright 2006
        Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
        D. Michael McIntyre <dmmcintyr@users.sourceforge.net>

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#define RG_MODULE_STRING "[TrackParameterBox]"

#include "TrackParameterBox.h"

#include "base/AudioPluginInstance.h"
#include "gui/general/ClefIndex.h"  // Clef enum
#include "gui/widgets/CollapsingFrame.h"
#include "base/Colour.h"
#include "base/ColourMap.h"
#include "base/Composition.h"
#include "misc/ConfigGroups.h"
#include "misc/Debug.h"
#include "base/Device.h"
#include "base/Exception.h"
#include "gui/general/GUIPalette.h"
#include "gui/widgets/InputDialog.h"
#include "base/Instrument.h"
#include "base/InstrumentStaticSignals.h"
#include "gui/widgets/LineEdit.h"
#include "base/MidiDevice.h"
#include "gui/dialogs/PitchPickerDialog.h"
#include "sound/PluginIdentifier.h"
#include "gui/general/PresetHandlerDialog.h"
#include "document/RosegardenDocument.h"
#include "RosegardenParameterBox.h"
#include "commands/segment/SegmentSyncCommand.h"
#include "gui/widgets/SqueezedLabel.h"
#include "base/StaffExportTypes.h"  // StaffTypes, Brackets
#include "base/Studio.h"
#include "base/Track.h"

#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QFontMetrics>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QString>
#include <QWidget>


namespace Rosegarden
{


TrackParameterBox::TrackParameterBox(RosegardenDocument *doc,
                                     QWidget *parent) :
    RosegardenParameterBox(tr("Track"), tr("Track Parameters"), parent),
    m_doc(doc),
    m_selectedTrackId(NO_TRACK),
    m_lastInstrumentType(Instrument::InvalidInstrument),
    m_lowestPlayable(0),
    m_highestPlayable(127)
{
    setObjectName("Track Parameter Box");

    QFontMetrics metrics(m_font);
    const int width11 = metrics.width("12345678901");
    const int width20 = metrics.width("12345678901234567890");
    const int width22 = metrics.width("1234567890123456789012");
    const int width25 = metrics.width("1234567890123456789012345");

    // Widgets

    // Label
    m_trackLabel = new SqueezedLabel(tr("<untitled>"), this);
    m_trackLabel->setAlignment(Qt::AlignCenter);
    m_trackLabel->setFont(m_font);

    // Playback parameters

    // Outer collapsing frame
    CollapsingFrame *playbackParametersFrame = new CollapsingFrame(
            tr("Playback parameters"), this, "trackparametersplayback", true);

    // Inner fixed widget
    // We need an inner widget so that we can have a layout.  The outer
    // CollapsingFrame already has its own layout.
    QWidget *playbackParameters = new QWidget(playbackParametersFrame);
    playbackParametersFrame->setWidget(playbackParameters);
    playbackParameters->setContentsMargins(3, 3, 3, 3);

    // Device
    QLabel *playbackDeviceLabel = new QLabel(tr("Device"), playbackParameters);
    playbackDeviceLabel->setFont(m_font);
    m_playbackDevice = new QComboBox(playbackParameters);
    m_playbackDevice->setToolTip(tr("<qt><p>Choose the device this track will use for playback.</p><p>Click <img src=\":pixmaps/toolbar/manage-midi-devices.xpm\"> to connect this device to a useful output if you do not hear sound</p></qt>"));
    m_playbackDevice->setMinimumWidth(width25);
    m_playbackDevice->setFont(m_font);
    connect(m_playbackDevice, SIGNAL(activated(int)),
            this, SLOT(slotPlaybackDeviceChanged(int)));

    // Instrument
    QLabel *instrumentLabel = new QLabel(tr("Instrument"), playbackParameters);
    instrumentLabel->setFont(m_font);
    m_instrument = new QComboBox(playbackParameters);
    m_instrument->setFont(m_font);
    m_instrument->setToolTip(tr("<qt><p>Choose the instrument this track will use for playback. (Configure the instrument in <b>Instrument Parameters</b>).</p></qt>"));
    m_instrument->setMaxVisibleItems(16);
    m_instrument->setMinimumWidth(width22);
    connect(m_instrument, SIGNAL(activated(int)),
            this, SLOT(slotInstrumentChanged(int)));

    // Archive
    QLabel *archiveLabel = new QLabel(tr("Archive"), playbackParameters);
    archiveLabel->setFont(m_font);
    m_archive = new QCheckBox(playbackParameters);
    m_archive->setFont(m_font);
    m_archive->setToolTip(tr("<qt><p>Check this to archive a track.  Archived tracks will not make sound.</p></qt>"));
    connect(m_archive, SIGNAL(clicked(bool)),
            this, SLOT(slotArchiveChanged(bool)));

    // Playback parameters layout

    // This automagically becomes playbackParameters's layout.
    QGridLayout *groupLayout = new QGridLayout(playbackParameters);
    groupLayout->setContentsMargins(5,0,0,5);
    groupLayout->setVerticalSpacing(2);
    groupLayout->setHorizontalSpacing(5);
    // Row 0: Device
    groupLayout->addWidget(playbackDeviceLabel, 0, 0);
    groupLayout->addWidget(m_playbackDevice, 0, 1);
    // Row 1: Instrument
    groupLayout->addWidget(instrumentLabel, 1, 0);
    groupLayout->addWidget(m_instrument, 1, 1);
    // Row 2: Archive
    groupLayout->addWidget(archiveLabel, 2, 0);
    groupLayout->addWidget(m_archive, 2, 1);
    // Let column 1 fill the rest of the space.
    groupLayout->setColumnStretch(1, 1);

    // Recording filters

    m_recordingFiltersFrame = new CollapsingFrame(
            tr("Recording filters"), this, "trackparametersrecord", false);

    QWidget *recordingFilters = new QWidget(m_recordingFiltersFrame);
    m_recordingFiltersFrame->setWidget(recordingFilters);
    recordingFilters->setContentsMargins(3, 3, 3, 3);

    // Device
    QLabel *recordDeviceLabel = new QLabel(tr("Device"), recordingFilters);
    recordDeviceLabel->setFont(m_font);
    m_recordingDevice = new QComboBox(recordingFilters);
    m_recordingDevice->setFont(m_font);
    m_recordingDevice->setToolTip(tr("<qt><p>This track will only record Audio/MIDI from the selected device, filtering anything else out</p></qt>"));
    m_recordingDevice->setMinimumWidth(width25);
    connect(m_recordingDevice, SIGNAL(activated(int)),
            this, SLOT(slotRecordingDeviceChanged(int)));

    // Channel
    QLabel *channelLabel = new QLabel(tr("Channel"), recordingFilters);
    channelLabel->setFont(m_font);
    m_recordingChannel = new QComboBox(recordingFilters);
    m_recordingChannel->setFont(m_font);
    m_recordingChannel->setToolTip(tr("<qt><p>This track will only record Audio/MIDI from the selected channel, filtering anything else out</p></qt>"));
    m_recordingChannel->setMaxVisibleItems(17);
    m_recordingChannel->setMinimumWidth(width11);
    m_recordingChannel->addItem(tr("All"));
    for (int i = 1; i < 17; ++i) {
        m_recordingChannel->addItem(QString::number(i));
    }
    connect(m_recordingChannel, SIGNAL(activated(int)),
            this, SLOT(slotRecordingChannelChanged(int)));

    // Thru Routing
    QLabel *thruLabel = new QLabel(tr("Thru Routing"), recordingFilters);
    thruLabel->setFont(m_font);
    m_thruRouting = new QComboBox(recordingFilters);
    m_thruRouting->setFont(m_font);
    //m_thruRouting->setToolTip(tr("<qt><p>Routing from the input device and channel to the instrument.</p></qt>"));
    m_thruRouting->setMinimumWidth(width11);
    m_thruRouting->addItem(tr("Auto"), Track::Auto);
    m_thruRouting->addItem(tr("On"), Track::On);
    m_thruRouting->addItem(tr("Off"), Track::Off);
    m_thruRouting->addItem(tr("When Armed"), Track::WhenArmed);
    connect(m_thruRouting, SIGNAL(activated(int)),
            this, SLOT(slotThruRoutingChanged(int)));

    // Recording filters layout

    groupLayout = new QGridLayout(recordingFilters);
    groupLayout->setContentsMargins(5,0,0,5);
    groupLayout->setVerticalSpacing(2);
    groupLayout->setHorizontalSpacing(5);
    // Row 0: Device
    groupLayout->addWidget(recordDeviceLabel, 0, 0);
    groupLayout->addWidget(m_recordingDevice, 0, 1);
    // Row 1: Channel
    groupLayout->addWidget(channelLabel, 1, 0);
    groupLayout->addWidget(m_recordingChannel, 1, 1);
    // Row 2: Thru Routing
    groupLayout->addWidget(thruLabel, 2, 0);
    groupLayout->addWidget(m_thruRouting, 2, 1);
    // Let column 1 fill the rest of the space.
    groupLayout->setColumnStretch(1, 1);

    // Staff export options

    m_staffExportOptionsFrame = new CollapsingFrame(
            tr("Staff export options"), this, "trackstaffgroup", false);

    QWidget *staffExportOptions = new QWidget(m_staffExportOptionsFrame);
    m_staffExportOptionsFrame->setWidget(staffExportOptions);
    staffExportOptions->setContentsMargins(2, 2, 2, 2);

    // Notation size (export only)
    //
    // NOTE: This is the only way to get a \small or \tiny inserted before the
    // first note in LilyPond export.  Setting the actual staff size on a
    // per-staff (rather than per-score) basis is something the author of the
    // LilyPond documentation has no idea how to do, so we settle for this,
    // which is not as nice, but actually a lot easier to implement.
    QLabel *notationSizeLabel = new QLabel(tr("Notation size:"), staffExportOptions);
    notationSizeLabel->setFont(m_font);
    m_notationSize = new QComboBox(staffExportOptions);
    m_notationSize->setFont(m_font);
    m_notationSize->setToolTip(tr("<qt><p>Choose normal, \\small or \\tiny font size for notation elements on this (normal-sized) staff when exporting to LilyPond.</p><p>This is as close as we get to enabling you to print parts in cue size</p></qt>"));
    m_notationSize->setMinimumWidth(width11);
    m_notationSize->addItem(tr("Normal"), StaffTypes::Normal);
    m_notationSize->addItem(tr("Small"), StaffTypes::Small);
    m_notationSize->addItem(tr("Tiny"), StaffTypes::Tiny);
    connect(m_notationSize, SIGNAL(activated(int)),
            this, SLOT(slotNotationSizeChanged(int)));

    // Bracket type
    // Staff bracketing (export only at the moment, but using this for GUI
    // rendering would be nice in the future!) //!!! 
    QLabel *bracketTypeLabel = new QLabel(tr("Bracket type:"), staffExportOptions);
    bracketTypeLabel->setFont(m_font);
    m_bracketType = new QComboBox(staffExportOptions);
    m_bracketType->setFont(m_font);
    m_bracketType->setToolTip(tr("<qt><p>Bracket staffs in LilyPond<br>(fragile, use with caution)</p><qt>"));
    m_bracketType->setMinimumWidth(width11);
    m_bracketType->addItem(tr("-----"), Brackets::None);
    m_bracketType->addItem(tr("[----"), Brackets::SquareOn);
    m_bracketType->addItem(tr("----]"), Brackets::SquareOff);
    m_bracketType->addItem(tr("[---]"), Brackets::SquareOnOff);
    m_bracketType->addItem(tr("{----"), Brackets::CurlyOn);
    m_bracketType->addItem(tr("----}"), Brackets::CurlyOff);
    m_bracketType->addItem(tr("{[---"), Brackets::CurlySquareOn);
    m_bracketType->addItem(tr("---]}"), Brackets::CurlySquareOff);
    connect(m_bracketType, SIGNAL(activated(int)),
            this, SLOT(slotBracketTypeChanged(int)));

    // Staff export options layout

    groupLayout = new QGridLayout(staffExportOptions);
    groupLayout->setContentsMargins(5,0,0,5);
    groupLayout->setVerticalSpacing(2);
    groupLayout->setHorizontalSpacing(5);
    groupLayout->setColumnStretch(1, 1);
    // Row 0: Notation size
    groupLayout->addWidget(notationSizeLabel, 0, 0, Qt::AlignLeft);
    groupLayout->addWidget(m_notationSize, 0, 1, 1, 2);
    // Row 1: Bracket type
    groupLayout->addWidget(bracketTypeLabel, 1, 0, Qt::AlignLeft);
    groupLayout->addWidget(m_bracketType, 1, 1, 1, 2);

    // Create segments with

    m_createSegmentsWithFrame = new CollapsingFrame(
            tr("Create segments with"), this, "trackparametersdefaults", false);

    QWidget *createSegmentsWith = new QWidget(m_createSegmentsWithFrame);
    m_createSegmentsWithFrame->setWidget(createSegmentsWith);
    createSegmentsWith->setContentsMargins(3, 3, 3, 3);

    // Preset
    QLabel *presetLabel = new QLabel(tr("Preset"), createSegmentsWith);
    presetLabel->setFont(m_font);

    m_preset = new QLabel(tr("<none>"), createSegmentsWith);
    m_preset->setFont(m_font);
    m_preset->setObjectName("SPECIAL_LABEL");
    m_preset->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_preset->setMinimumWidth(width20);

    m_load = new QPushButton(tr("Load"), createSegmentsWith);
    m_load->setFont(m_font);
    m_load->setToolTip(tr("<qt><p>Load a segment parameters preset from our comprehensive database of real-world instruments.</p><p>When you create new segments, they will have these parameters at the moment of creation.  To use these parameters on existing segments (eg. to convert an existing part in concert pitch for playback on a Bb trumpet) use <b>Segments -> Convert notation for</b> in the notation editor.</p></qt>"));
    connect(m_load, SIGNAL(released()),
            SLOT(slotLoadPressed()));

    // Clef
    QLabel *clefLabel = new QLabel(tr("Clef"), createSegmentsWith);
    clefLabel->setFont(m_font);
    m_clef = new QComboBox(createSegmentsWith);
    m_clef->setFont(m_font);
    m_clef->setToolTip(tr("<qt><p>New segments will be created with this clef inserted at the beginning</p></qt>"));
    m_clef->setMinimumWidth(width11);
    m_clef->addItem(tr("treble", "Clef name"), TrebleClef);
    m_clef->addItem(tr("bass", "Clef name"), BassClef);
    m_clef->addItem(tr("crotales", "Clef name"), CrotalesClef);
    m_clef->addItem(tr("xylophone", "Clef name"), XylophoneClef);
    m_clef->addItem(tr("guitar", "Clef name"), GuitarClef);
    m_clef->addItem(tr("contrabass", "Clef name"), ContrabassClef);
    m_clef->addItem(tr("celesta", "Clef name"), CelestaClef);
    m_clef->addItem(tr("old celesta", "Clef name"), OldCelestaClef);
    m_clef->addItem(tr("french", "Clef name"), FrenchClef);
    m_clef->addItem(tr("soprano", "Clef name"), SopranoClef);
    m_clef->addItem(tr("mezzosoprano", "Clef name"), MezzosopranoClef);
    m_clef->addItem(tr("alto", "Clef name"), AltoClef);
    m_clef->addItem(tr("tenor", "Clef name"), TenorClef);
    m_clef->addItem(tr("baritone", "Clef name"), BaritoneClef);
    m_clef->addItem(tr("varbaritone", "Clef name"), VarbaritoneClef);
    m_clef->addItem(tr("subbass", "Clef name"), SubbassClef);
    m_clef->addItem(tr("twobar", "Clef name"), TwoBarClef);
    connect(m_clef, SIGNAL(activated(int)),
            this, SLOT(slotClefChanged(int)));

    // Transpose
    QLabel *transposeLabel = new QLabel(tr("Transpose"), createSegmentsWith);
    transposeLabel->setFont(m_font);
    m_transpose = new QComboBox(createSegmentsWith);
    m_transpose->setFont(m_font);
    m_transpose->setToolTip(tr("<qt><p>New segments will be created with this transpose property set</p></qt>"));
    connect(m_transpose, SIGNAL(activated(int)),
            SLOT(slotTransposeChanged(int)));

    int transposeRange = 48;
    for (int i = -transposeRange; i < transposeRange + 1; i++) {
        m_transpose->addItem(QString("%1").arg(i));
        if (i == 0)
            m_transpose->setCurrentIndex(m_transpose->count() - 1);
    }

    // Pitch
    QLabel *pitchLabel = new QLabel(tr("Pitch"), createSegmentsWith);
    pitchLabel->setFont(m_font);

    // Lowest playable note
    QLabel *lowestLabel = new QLabel(tr("Lowest"), createSegmentsWith);
    lowestLabel->setFont(m_font);

    m_lowest = new QPushButton(tr("---"), createSegmentsWith);
    m_lowest->setFont(m_font);
    m_lowest->setToolTip(tr("<qt><p>Choose the lowest suggested playable note, using a staff</p></qt>"));
    connect(m_lowest, SIGNAL(released()),
            SLOT(slotLowestPressed()));

    // Highest playable note
    QLabel *highestLabel = new QLabel(tr("Highest"), createSegmentsWith);
    highestLabel->setFont(m_font);

    m_highest = new QPushButton(tr("---"), createSegmentsWith);
    m_highest->setFont(m_font);
    m_highest->setToolTip(tr("<qt><p>Choose the highest suggested playable note, using a staff</p></qt>"));
    connect(m_highest, SIGNAL(released()),
            SLOT(slotHighestPressed()));

    updateHighLow();

    // Color
    QLabel *colorLabel = new QLabel(tr("Color"), createSegmentsWith);
    colorLabel->setFont(m_font);
    m_color = new QComboBox(createSegmentsWith);
    m_color->setFont(m_font);
    m_color->setToolTip(tr("<qt><p>New segments will be created using this color</p></qt>"));
    m_color->setEditable(false);
    m_color->setMaxVisibleItems(20);
    connect(m_color, SIGNAL(activated(int)),
            SLOT(slotColorChanged(int)));

    // "Create segments with" layout

    groupLayout = new QGridLayout(createSegmentsWith);
    groupLayout->setContentsMargins(5,0,0,5);
    groupLayout->setVerticalSpacing(2);
    groupLayout->setHorizontalSpacing(5);
    // Row 0: Preset/Load
    groupLayout->addWidget(presetLabel, 0, 0, Qt::AlignLeft);
    groupLayout->addWidget(m_preset, 0, 1, 1, 3);
    groupLayout->addWidget(m_load, 0, 4, 1, 2);
    // Row 1: Clef/Transpose
    groupLayout->addWidget(clefLabel, 1, 0, Qt::AlignLeft);
    groupLayout->addWidget(m_clef, 1, 1, 1, 2);
    groupLayout->addWidget(transposeLabel, 1, 3, 1, 2, Qt::AlignRight);
    groupLayout->addWidget(m_transpose, 1, 5, 1, 1);
    // Row 2: Pitch/Lowest/Highest
    groupLayout->addWidget(pitchLabel, 2, 0, Qt::AlignLeft);
    groupLayout->addWidget(lowestLabel, 2, 1, Qt::AlignRight);
    groupLayout->addWidget(m_lowest, 2, 2, 1, 1);
    groupLayout->addWidget(highestLabel, 2, 3, Qt::AlignRight);
    groupLayout->addWidget(m_highest, 2, 4, 1, 2);
    // Row 3: Color
    groupLayout->addWidget(colorLabel, 3, 0, Qt::AlignLeft);
    groupLayout->addWidget(m_color, 3, 1, 1, 5);

    groupLayout->setColumnStretch(1, 1);
    groupLayout->setColumnStretch(2, 2);

    // populate combo from doc colors
    slotDocColoursChanged();

    // Connections

    // Detect when the document colours are updated
    connect(m_doc, SIGNAL(docColoursChanged()),
            this, SLOT(slotDocColoursChanged()));

    connect(Instrument::getStaticSignals().data(),
            SIGNAL(changed(Instrument *)),
            this,
            SLOT(slotInstrumentChanged(Instrument *)));

    m_doc->getComposition().addObserver(this);

    // Layout

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(1);
    mainLayout->addWidget(m_trackLabel, 0, 0);
    mainLayout->addWidget(playbackParametersFrame, 1, 0);
    mainLayout->addWidget(m_recordingFiltersFrame, 2, 0);
    mainLayout->addWidget(m_staffExportOptionsFrame, 3, 0);
    mainLayout->addWidget(m_createSegmentsWithFrame, 4, 0);

    // Box

    setContentsMargins(2, 7, 2, 2);

    updateWidgets2();
}

void
TrackParameterBox::setDocument(RosegardenDocument *doc)
{
    // No change?  Bail.
    if (m_doc == doc)
        return;

    m_doc = doc;

    m_doc->getComposition().addObserver(this);

    updateWidgets2();
}

void
TrackParameterBox::slotPopulateDeviceLists()
{
    updateWidgets2();
}

void
TrackParameterBox::updateHighLow()
{
    Track *track = getTrack();
    if (!track)
        return;

    // Set the highest/lowest in the Track.

    track->setHighestPlayable(m_highestPlayable);
    track->setLowestPlayable(m_lowestPlayable);

    // Update the text on the highest/lowest pushbuttons.

    const Accidental accidental = Accidentals::NoAccidental;

    const Pitch highest(m_highestPlayable, accidental);
    const Pitch lowest(m_lowestPlayable, accidental);

    QSettings settings;
    settings.beginGroup(GeneralOptionsConfigGroup);

    const int octaveBase = settings.value("midipitchoctave", -2).toInt() ;
    settings.endGroup();

    const bool includeOctave = false;

    // NOTE: this now uses a new, overloaded version of Pitch::getAsString()
    // that explicitly works with the key of C major, and does not allow the
    // calling code to specify how the accidentals should be written out.
    //
    // Separate the note letter from the octave to avoid undue burden on
    // translators having to retranslate the same thing but for a number
    // difference
    QString tmp = QObject::tr(highest.getAsString(includeOctave, octaveBase).c_str(), "note name");
    tmp += tr(" %1").arg(highest.getOctave(octaveBase));
    m_highest->setText(tmp);

    tmp = QObject::tr(lowest.getAsString(includeOctave, octaveBase).c_str(), "note name");
    tmp += tr(" %1").arg(lowest.getOctave(octaveBase));
    m_lowest->setText(tmp);

    m_preset->setEnabled(false);
}

void
TrackParameterBox::trackChanged(const Composition *, Track *track)
{
    if (!track)
        return;

    if (track->getId() != m_selectedTrackId)
        return;

    updateWidgets2();
}

void
TrackParameterBox::trackSelectionChanged(const Composition *, TrackId newTrackId)
{
    // No change?  Bail.
    if (newTrackId == m_selectedTrackId)
        return;

    m_preset->setEnabled(true);

    m_selectedTrackId = newTrackId;

    updateWidgets2();
}

void
TrackParameterBox::slotPlaybackDeviceChanged(int index)
{
    //RG_DEBUG << "slotPlaybackDeviceChanged(" << index << ")";

    // If nothing is selected
    // ??? Can this ever happen?
    if (index == -1) {
        // Update from the Track.
        updateWidgets2();
        return;
    }

    DeviceId deviceId = m_playbackDeviceIds2[index];

    Track *track = getTrack();
    if (!track)
        return;

    // Switch the Track to the same instrument # on this new Device.

    Device *device = m_doc->getStudio().getDevice(deviceId);
    if (!device)
        return;

    // Query the Studio to get an Instrument for this Device.
    InstrumentList instrumentList = device->getPresentationInstruments();

    // Try to preserve the Instrument number (channel) if possible.
    int instrumentIndex = m_instrument->currentIndex();
    if (instrumentIndex >= static_cast<int>(instrumentList.size()))
        instrumentIndex = 0;

    // Set the Track's Instrument to the new Instrument.
    track->setInstrument(instrumentList[instrumentIndex]->getId());

    m_doc->slotDocumentModified();

    // Notify observers
    // ??? Redundant notification.  Track::setInstrument() already does
    //     this.  It shouldn't.
    Composition &comp = m_doc->getComposition();
    comp.notifyTrackChanged(track);

    // ??? Ugh.  This is so that TrackButtons updates properly.  This in
    //     turn causes the IPB to update.  TrackButtons and the IPB
    //     should take care of themselves in response to the TrackChanged
    //     notification above!  This needs to go.
    // ??? Redundant notification.
    slotInstrumentChanged(instrumentIndex);
}

void
TrackParameterBox::slotInstrumentChanged(int index)
{
    //RG_DEBUG << "slotInstrumentChanged(" << index << ")";

    // If nothing is selected
    // ??? Can this ever happen?
    if (index == -1) {
        // Update from the Track.
        updateWidgets2();
        return;
    }

#if 1
    // Invalid Track?  Bail.
    if (!getTrack())
        return;

    //devId = m_playbackDeviceIds[m_playbackDevice->currentIndex()];

    // Calculate an index to use in Studio::getInstrumentFromList() which
    // gets emitted to TrackButtons, and TrackButtons actually does the work
    // of assigning the instrument to the track, for some bizarre reason.
    //
    // This new method for calculating the index works by:
    //
    // 1. for every play device combo index between 0 and its current index,
    //
    // 2. get the device that corresponds with that combo box index, and
    //
    // 3. figure out how many instruments that device contains, then
    //
    // 4. Add it all up.  That's how many slots we have to jump over to get
    //    to the point where the instrument combo box index we're working
    //    with here will target the correct instrument in the studio list.
    //
    // I'm sure this whole architecture seemed clever once, but it's an
    // unmaintainable pain in the ass is what it is.  We changed one
    // assumption somewhere, and the whole thing fell on its head,
    // swallowing two entire days of my life to put back with the following
    // magic lines of code:
    int prepend = 0;
    // For each device that needs to be skipped.
    for (int n = 0; n < m_playbackDevice->currentIndex(); ++n) {
        DeviceId id = m_playbackDeviceIds2[n];
        Device *dev = m_doc->getStudio().getDevice(id);

        InstrumentList il = dev->getPresentationInstruments();

        // Accumulate the number of instruments that need to be skipped.
        // get the number of instruments belonging to the device (not the
        // studio)
        prepend += il.size();
    }

    // Convert from TrackParameterBox index to TrackButtons index.
    int trackButtonsInstrumentIndex = index + prepend;

    //RG_DEBUG << "slotInstrumentChanged() trackButtonsIndex = " << trackButtonsIndex;

    // Emit the index we've calculated, relative to the studio list.
    // TrackButtons::slotTPBInstrumentSelected() does the rest of the
    // work for us.
    emit instrumentSelected(m_selectedTrackId, trackButtonsInstrumentIndex);
    // Or directly, avoiding the signal/slot:
    //RosegardenMainWindow::self()->getView()->getTrackEditor()->
    //        getTrackButtons()->slotTPBInstrumentSelected(
    //                m_selectedTrackId, trackButtonsInstrumentIndex);

#else
// ??? This is how it should be done.  All of the work that is done by
//     TrackButtons::slotTPBInstrumentSelected() should be spread out
//     into the trackChanged() handlers for those that care.

    Track *track = getTrack();
    if (!track)
        return;

    track->setInstrument(m_instrumentIds[index]);

    // Notify observers
    Composition &comp = m_doc->getComposition();
    comp.notifyTrackChanged(track);

#endif
}

void
TrackParameterBox::slotArchiveChanged(bool checked)
{
    //RG_DEBUG << "slotArchiveChanged(" << checked << ")";

    Track *track = getTrack();

    if (!track)
        return;

    track->setArchived(checked);
    m_doc->slotDocumentModified();

    // Notify observers
    Composition &comp = m_doc->getComposition();
    comp.notifyTrackChanged(track);
}

void
TrackParameterBox::slotRecordingDeviceChanged(int index)
{
    //RG_DEBUG << "slotRecordingDeviceChanged(" << index << ")";

    Track *trk = getTrack();
    if (!trk)
        return;

    Instrument *inst = m_doc->getStudio().getInstrumentFor(trk);
    if (!inst)
        return;

    // Audio instruments do not support different recording devices.
    if (inst->getInstrumentType() == Instrument::Audio)
        return;

    trk->setMidiInputDevice(m_recordingDeviceIds2[index]);
}

void
TrackParameterBox::slotRecordingChannelChanged(int index)
{
    //RG_DEBUG << "slotRecordingChannelChanged(" << index << ")";

    Track *trk = getTrack();
    if (!trk)
        return;

    Instrument *inst = m_doc->getStudio().getInstrumentFor(trk);
    if (!inst)
        return;

    // Audio instruments do not support different recording channels.
    if (inst->getInstrumentType() == Instrument::Audio)
        return;

    trk->setMidiInputChannel(index - 1);
}

void
TrackParameterBox::slotThruRoutingChanged(int index)
{
    Track *track = getTrack();
    if (!track)
        return;

    Instrument *inst = m_doc->getStudio().getInstrumentFor(track);
    if (!inst)
        return;

    // Thru routing is only supported for MIDI instruments.
    if (inst->getInstrumentType() != Instrument::Midi)
        return;

    track->setThruRouting(static_cast<Track::ThruRouting>(index));
    m_doc->slotDocumentModified();

    // Notify observers
    Composition &comp = m_doc->getComposition();
    comp.notifyTrackChanged(track);
}

void
TrackParameterBox::slotInstrumentChanged(Instrument *)
{
    updateWidgets2();
}

void
TrackParameterBox::slotClefChanged(int clef)
{
    //RG_DEBUG << "slotClefChanged(" << clef << ")";

    Track *trk = getTrack();
    if (!trk)
        return;

    trk->setClef(clef);
    m_preset->setEnabled(false);
}

void
TrackParameterBox::transposeChanged(int transpose)
{
    //RG_DEBUG << "transposeChanged(" << transpose << ")";

    // ??? Inline into only caller.

    Track *trk = getTrack();
    if (!trk)
        return;

    trk->setTranspose(transpose);
    m_preset->setEnabled(false);
}

void
TrackParameterBox::slotTransposeChanged(int index)
{
    QString text = m_transpose->itemText(index);

    if (text.isEmpty())
        return;

    int value = text.toInt();
    transposeChanged(value);
}

void
TrackParameterBox::slotDocColoursChanged()
{
    // The color combobox is handled differently from the others.  Since
    // there are 420 strings of up to 25 chars in here, it would be
    // expensive to detect changes by comparing vectors of strings.

    // For now, we'll handle the document colors changed notification
    // and reload the combobox then.

    // See the comments on RosegardenDocument::docColoursChanged()
    // in RosegardenDocument.h.

    // Note that as of this writing (August 2016) there is no way
    // to modify the document colors.  See ColourConfigurationPage
    // which was probably meant to be used by DocumentConfigureDialog.

    m_color->clear();

    // Populate it from Composition::m_segmentColourMap
    ColourMap temp = m_doc->getComposition().getSegmentColourMap();

    // For each color in the segment color map
    for (RCMap::const_iterator colourIter = temp.begin();
         colourIter != temp.end();
         ++colourIter) {
        QString colourName(QObject::tr(colourIter->second.second.c_str()));

        QPixmap colourIcon(15, 15);
        colourIcon.fill(GUIPalette::convertColour(colourIter->second.first));

        if (colourName == "") {
            m_color->addItem(colourIcon, tr("Default"));
        } else {
            // truncate name to 25 characters to avoid the combo forcing the
            // whole kit and kaboodle too wide (This expands from 15 because the
            // translators wrote books instead of copying the style of
            // TheShortEnglishNames, and because we have that much room to
            // spare.)
            if (colourName.length() > 25)
                colourName = colourName.left(22) + "...";

            m_color->addItem(colourIcon, colourName);
        }
    }

#if 0
// Removing this since it has never been in there.
    m_color->addItem(tr("Add New Color"));
    m_addColourPos = m_color->count() - 1;
#endif

    const Track *track = getTrack();

    if (track)
        m_color->setCurrentIndex(track->getColor());
}

void
TrackParameterBox::slotColorChanged(int index)
{
    //RG_DEBUG << "slotColorChanged(" << index << ")";

    Track *trk = getTrack();
    if (!trk)
        return;

    trk->setColor(index);

#if 0
// This will never happen since the "Add Color" option is never added.
    if (index == m_addColourPos) {
        ColourMap newMap = m_doc->getComposition().getSegmentColourMap();
        QColor newColour;
        bool ok = false;
        
        QString newName = InputDialog::getText(this,
                                               tr("New Color Name"),
                                               tr("Enter new name:"),
                                               LineEdit::Normal,
                                               tr("New"), &ok);
        
        if ((ok == true) && (!newName.isEmpty())) {
//             QColorDialog box(this, "", true);
//             int result = box.getColor(newColour);
            
            //QRgb QColorDialog::getRgba(0xffffffff, &ok, this);
            QColor newColor = QColorDialog::getColor(Qt::white, this);

            if (newColor.isValid()) {
                Colour newRColour = GUIPalette::convertColour(newColour);
                newMap.addItem(newRColour, qstrtostr(newName));
                slotDocColoursChanged();
            }
        }
        // Else we don't do anything as they either didn't give a name
        // or didn't give a colour
    }
#endif
}

void
TrackParameterBox::slotHighestPressed()
{
    if (m_selectedTrackId == NO_TRACK)
        return;

    Composition &comp = m_doc->getComposition();

    // Make sure the selected track is valid.
    if (!comp.haveTrack(m_selectedTrackId)) {
        m_selectedTrackId = NO_TRACK;
        return;
    }

    PitchPickerDialog dialog(0, m_highestPlayable, tr("Highest playable note"));

    if (dialog.exec() == QDialog::Accepted) {
        m_highestPlayable = dialog.getPitch();
        updateHighLow();
    }

    m_preset->setEnabled(false);
}

void
TrackParameterBox::slotLowestPressed()
{
    if (m_selectedTrackId == NO_TRACK)
        return;

    Composition &comp = m_doc->getComposition();

    // Make sure the selected track is valid.
    if (!comp.haveTrack(m_selectedTrackId)) {
        m_selectedTrackId = NO_TRACK;
        return;
    }

    PitchPickerDialog dialog(0, m_lowestPlayable, tr("Lowest playable note"));

    if (dialog.exec() == QDialog::Accepted) {
        m_lowestPlayable = dialog.getPitch();
        updateHighLow();
    }

    m_preset->setEnabled(false);
}

void
TrackParameterBox::slotLoadPressed()
{
    // Inherits style.  Centers on main window.
    //PresetHandlerDialog dialog(this);
    // Does not inherit style?  Centers on monitor #1?
    PresetHandlerDialog dialog(0);

    Track *trk = getTrack();
    if (!trk)
        return;

    try {
        if (dialog.exec() == QDialog::Accepted) {
            m_preset->setText(dialog.getName());
            trk->setPresetLabel(qstrtostr(dialog.getName()));

            // If we need to convert the track's segments
            if (dialog.getConvertAllSegments()) {
                Composition &comp = m_doc->getComposition();
                SegmentSyncCommand* command = new SegmentSyncCommand(
                        comp.getSegments(), m_selectedTrackId,
                        dialog.getTranspose(), dialog.getLowRange(), 
                        dialog.getHighRange(),
                        clefIndexToClef(dialog.getClef()));
                CommandHistory::getInstance()->addCommand(command);
            }

            m_clef->setCurrentIndex(dialog.getClef());
            trk->setClef(dialog.getClef());
                     
            m_transpose->setCurrentIndex(m_transpose->findText(
                    QString("%1").arg(dialog.getTranspose())));
            trk->setTranspose(dialog.getTranspose());

            m_highestPlayable = dialog.getHighRange();
            m_lowestPlayable = dialog.getLowRange();
            updateHighLow();

            // updateHighLow() will have set this disabled, so we
            // re-enable it until it is subsequently re-disabled by the
            // user overriding the preset, calling one of the above slots
            // in the normal course
            // ??? This is too subtle.  We should probably just clear it
            //     when modifications are made.  After all, it's no longer
            //     the selected preset.  Or add "(modified)"?  Or change
            //     color?  Or allow the user to edit it and save it to the
            //     .rg file as part of the Track.
            m_preset->setEnabled(true);
        }
    } catch (Exception e) {  // from PresetHandlerDialog
        // !!! This should be a more verbose error to pass along the
        //     row/column of the corruption.
        QMessageBox::warning(0, tr("Rosegarden"),
                tr("The instrument preset database is corrupt.  Check your installation."));
    }
}

void
TrackParameterBox::slotNotationSizeChanged(int index)
{
    Track *trk = getTrack();
    if (!trk)
        return;

    trk->setStaffSize(index);
}

void
TrackParameterBox::slotBracketTypeChanged(int index)
{
    Track *trk = getTrack();
    if (!trk)
        return;

    trk->setStaffBracket(index);
}

Track *
TrackParameterBox::getTrack()
{
    if (m_selectedTrackId == NO_TRACK)
        return NULL;

    if (!m_doc)
        return NULL;

    Composition &comp = m_doc->getComposition();

    // If the track is gone, bail.
    if (!comp.haveTrack(m_selectedTrackId)) {
        m_selectedTrackId = NO_TRACK;
        return NULL;
    }

    return comp.getTrackById(m_selectedTrackId);
}

void
TrackParameterBox::updatePlaybackDevice(DeviceId deviceId)
{
    const DeviceList &deviceList = *(m_doc->getStudio().getDevices());

    // Generate local device name and ID lists to compare against the members.

    std::vector<DeviceId> deviceIds;
    std::vector<std::string> deviceNames;

    // For each Device
    for (size_t deviceIndex = 0;
         deviceIndex < deviceList.size();
         ++deviceIndex) {

        const Device &device = *(deviceList[deviceIndex]);

        // ??? A Device::isInput() would be simpler.  Derivers would
        //     implement appropriately.  Then this would simplify to:
        //
        //        // If this is an input device, skip it.
        //        if (device.isInput())
        //            continue;

        const MidiDevice *midiDevice =
                dynamic_cast<const MidiDevice *>(deviceList[deviceIndex]);

        // If this is a MIDI input device, skip it.
        if (midiDevice  &&
            midiDevice->getDirection() == MidiDevice::Record)
            continue;

        deviceIds.push_back(device.getId());
        deviceNames.push_back(device.getName());
    }

    // If there has been an actual change
    if (deviceIds != m_playbackDeviceIds2  ||
        deviceNames != m_playbackDeviceNames) {

        // Update the cache.
        m_playbackDeviceIds2 = deviceIds;
        m_playbackDeviceNames = deviceNames;

        // Reload the combobox

        m_playbackDevice->clear();

        // For each playback Device, add the name to the combobox.
        // ??? If we used a QStringList, we could just call addItems().
        for (size_t deviceIndex = 0;
             deviceIndex < m_playbackDeviceNames.size();
             ++deviceIndex) {
            m_playbackDevice->addItem(
                    QObject::tr(m_playbackDeviceNames[deviceIndex].c_str()));
        }
    }

    // Find the current device in the device ID list.

    // Assume not found.
    int currentIndex = -1;

    // For each Device
    for (size_t deviceIndex = 0;
         deviceIndex < m_playbackDeviceIds2.size();
         ++deviceIndex) {
        // If this is the selected device
        if (m_playbackDeviceIds2[deviceIndex] == deviceId) {
            currentIndex = deviceIndex;
            break;
        }
    }

    // Set the index.
    m_playbackDevice->setCurrentIndex(currentIndex);
}

void
TrackParameterBox::updateInstrument(const Instrument *instrument)
{
    // As with the Device field above, this will rarely change and it is
    // expensive to clear and reload.  So, we should cache enough info to
    // detect a real change.  This would be Instrument names and IDs.

    const DeviceId deviceId = instrument->getDevice()->getId();
    const Device &device = *(m_doc->getStudio().getDevice(deviceId));

    const InstrumentList instrumentList = device.getPresentationInstruments();

    // Generate local instrument name and ID lists to compare against the
    // members.

    std::vector<InstrumentId> instrumentIds;
    std::vector<QString> instrumentNames;

    // For each instrument
    for (size_t instrumentIndex = 0;
         instrumentIndex < instrumentList.size();
         ++instrumentIndex) {
        const Instrument &loopInstrument = *(instrumentList[instrumentIndex]);

        instrumentIds.push_back(loopInstrument.getId());

        QString instrumentName(QObject::tr(loopInstrument.getName().c_str()));
        QString programName(
                QObject::tr(loopInstrument.getProgramName().c_str()));

        if (loopInstrument.getType() == Instrument::SoftSynth) {

            instrumentName.replace(QObject::tr("Synth plugin"), "");

            programName = "";

            AudioPluginInstance *plugin =
                    instrument->getPlugin(Instrument::SYNTH_PLUGIN_POSITION);
            if (plugin)
                programName = strtoqstr(plugin->getDisplayName());
        }

        if (programName != "")
            instrumentName += " (" + programName + ")";

        // cut off the redundant eg. "General MIDI Device" that appears in the
        // combo right above here anyway
        instrumentName = instrumentName.mid(
                instrumentName.indexOf("#"), instrumentName.length());

        instrumentNames.push_back(instrumentName);
    }

    // If there has been an actual change
    if (instrumentIds != m_instrumentIds2  ||
        instrumentNames != m_instrumentNames2) {

        // Update the cache.
        m_instrumentIds2 = instrumentIds;
        m_instrumentNames2 = instrumentNames;

        // Reload the combobox

        m_instrument->clear();

        // For each instrument, add the name to the combobox.
        // ??? If we used a QStringList, we could just call addItems().
        for (size_t instrumentIndex = 0;
             instrumentIndex < m_instrumentNames2.size();
             ++instrumentIndex) {
            m_instrument->addItem(m_instrumentNames2[instrumentIndex]);
        }
    }

    // Find the current instrument in the instrument ID list.

    const InstrumentId instrumentId = instrument->getId();

    // Assume not found.
    int currentIndex = -1;

    // For each Instrument
    for (size_t instrumentIndex = 0;
         instrumentIndex < m_instrumentIds2.size();
         ++instrumentIndex) {
        // If this is the selected Instrument
        if (m_instrumentIds2[instrumentIndex] == instrumentId) {
            currentIndex = instrumentIndex;
            break;
        }
    }

    // Set the index.
    m_instrument->setCurrentIndex(currentIndex);
}

void
TrackParameterBox::updateRecordingDevice(DeviceId deviceId)
{
    // As with playback devices, the list of record devices will rarely
    // change and it is expensive to clear and reload.  Handle like the
    // others.  Cache names and IDs and only reload if a real change is
    // detected.

    const DeviceList &deviceList = *(m_doc->getStudio().getDevices());

    // Generate local recording device name and ID lists to compare against
    // the members.

    std::vector<DeviceId> recordingDeviceIds;
    std::vector<QString> recordingDeviceNames;

    recordingDeviceIds.push_back(Device::ALL_DEVICES);
    recordingDeviceNames.push_back(tr("All"));

    // For each Device
    for (size_t deviceIndex = 0;
         deviceIndex < deviceList.size();
         ++deviceIndex) {

        const Device &device = *(deviceList[deviceIndex]);

        // ??? A Device::isOutput() would be simpler.  Derivers would
        //     implement appropriately.  Then this would simplify to:
        //
        //        // If this is an output device, skip it.
        //        if (device.isOutput())
        //            continue;
        //
        //        // Add it to the recording device lists.
        //        ...

        const MidiDevice *midiDevice =
                dynamic_cast<const MidiDevice *>(deviceList[deviceIndex]);

        // If this isn't a MIDI device, try the next.
        if (!midiDevice)
            continue;

        // If this is a device capable of being recorded
        // ??? What does isRecording() really mean?
        if (midiDevice->getDirection() == MidiDevice::Record  &&
            midiDevice->isRecording()) {
            // Add it to the recording device lists.
            recordingDeviceIds.push_back(device.getId());
            recordingDeviceNames.push_back(
                    QObject::tr(midiDevice->getName().c_str()));
        }
    }

    // If there has been an actual change
    if (recordingDeviceIds != m_recordingDeviceIds2  ||
        recordingDeviceNames != m_recordingDeviceNames) {

        // Update the cache
        m_recordingDeviceIds2 = recordingDeviceIds;
        m_recordingDeviceNames = recordingDeviceNames;

        // Reload the combobox

        m_recordingDevice->clear();

        // For each playback Device, add the name to the combobox.
        // ??? If we used a QStringList, we could just call addItems().
        for (size_t deviceIndex = 0;
             deviceIndex < m_recordingDeviceNames.size();
             ++deviceIndex) {
            m_recordingDevice->addItem(m_recordingDeviceNames[deviceIndex]);
        }
    }

    // Find the current record device in the record device ID list.

    // Assume not found.
    int currentIndex = -1;

    // For each ID
    for (size_t deviceIndex = 0;
         deviceIndex < m_recordingDeviceIds2.size();
         ++deviceIndex) {
        // If this is the selected device
        if (m_recordingDeviceIds2[deviceIndex] == deviceId) {
            currentIndex = deviceIndex;
            break;
        }
    }

    // Set the index.
    m_recordingDevice->setCurrentIndex(currentIndex);
}

void
TrackParameterBox::updateWidgets2()
{
    Track *track = getTrack();
    if (!track)
        return;

    Instrument *instrument = m_doc->getStudio().getInstrumentFor(track);
    if (!instrument)
        return;

    RG_DEBUG << "updateWidgets2()";

    // *** Track Label

    QString trackName = strtoqstr(track->getLabel());
    if (trackName.isEmpty())
        trackName = tr("<untitled>");
    else
        trackName.truncate(20);

    const int trackNum = track->getPosition() + 1;

    m_trackLabel->setText(tr("[ Track %1 - %2 ]").arg(trackNum).arg(trackName));

    // *** Playback parameters

    // Device
    updatePlaybackDevice(instrument->getDevice()->getId());

    // Instrument
    updateInstrument(instrument);

    // Archive
    m_archive->setChecked(track->isArchived());

    // If the current Instrument is an Audio Instrument...
    if (instrument->getInstrumentType() == Instrument::Audio) {
        // Hide the remaining three sections.
        m_recordingFiltersFrame->setVisible(false);
        m_staffExportOptionsFrame->setVisible(false);
        // ??? Overzealous.  The color combobox is still useful.
        m_createSegmentsWithFrame->setVisible(false);

        // And bail.
        return;
    } else {  // MIDI or soft synth
        // Show the remaining three sections.
        m_recordingFiltersFrame->setVisible(true);
        m_staffExportOptionsFrame->setVisible(true);
        m_createSegmentsWithFrame->setVisible(true);
    }

    // *** Recording filters

    // Device
    updateRecordingDevice(track->getMidiInputDevice());

    // Channel
    m_recordingChannel->setCurrentIndex((int)track->getMidiInputChannel() + 1);

    // Thru Routing
    m_thruRouting->setCurrentIndex((int)track->getThruRouting());

    // *** Staff export options

    // Notation size
    m_notationSize->setCurrentIndex(track->getStaffSize());

    // Bracket type
    m_bracketType->setCurrentIndex(track->getStaffBracket());

    // *** Create segments with

    // Preset (Label)
    m_preset->setText(strtoqstr(track->getPresetLabel()));

    // Clef
    m_clef->setCurrentIndex(track->getClef());

    // Transpose
    m_transpose->setCurrentIndex(
            m_transpose->findText(QString("%1").arg(track->getTranspose())));

    // Pitch Lowest

    QSettings settings;
    settings.beginGroup(GeneralOptionsConfigGroup);
    const int octaveBase = settings.value("midipitchoctave", -2).toInt() ;
    settings.endGroup();

    const bool includeOctave = false;

    // ??? Do we really need m_lowestPlayable now?
    m_lowestPlayable = track->getLowestPlayable();
    const Pitch lowest(m_lowestPlayable, Accidentals::NoAccidental);

    // NOTE: this now uses a new, overloaded version of Pitch::getAsString()
    // that explicitly works with the key of C major, and does not allow the
    // calling code to specify how the accidentals should be written out.
    //
    // Separate the note letter from the octave to avoid undue burden on
    // translators having to retranslate the same thing but for a number
    // difference
    QString tmp = QObject::tr(lowest.getAsString(includeOctave, octaveBase).c_str(), "note name");
    tmp += tr(" %1").arg(lowest.getOctave(octaveBase));
    m_lowest->setText(tmp);

    // Pitch Highest

    // ??? Do we really need m_highestPlayable now?
    m_highestPlayable = track->getHighestPlayable();
    const Pitch highest(m_highestPlayable, Accidentals::NoAccidental);

    tmp = QObject::tr(highest.getAsString(includeOctave, octaveBase).c_str(), "note name");
    tmp += tr(" %1").arg(highest.getOctave(octaveBase));
    m_highest->setText(tmp);

    // Color
    // Note: We only update the combobox contents if there is an actual
    //       change to the document's colors.  See slotDocColoursChanged().
    m_color->setCurrentIndex(track->getColor());
}


}
