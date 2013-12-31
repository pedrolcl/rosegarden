/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2014 the Rosegarden development team.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef SELECTDIALOG_H
#define SELECTDIALOG_H

#include <QDialog>

class QPushButton;
class QTabWidget;
class QWidget;
class QCheckBox;
class QRadioButton;

namespace Rosegarden
{


class CheckButton;

/**Creates a complex dialog that allows the user to choose what events to select
 * using everything and the kitchen sink as possible selection critera.
 *
 * Note: This class breaks Rosegarden coding standards and uses
 * underscore_format_variables instead of camelCase.  A large amount of this
 * code was generated by scripts, and it was far easier to assemble things this
 * way.  Since there were so many of them, I just used the same pattern
 * throughout, and anything that is effectively used as a bool has a
 * verbose_variable_name_with_underscores.
 *
 * \author D. Michael McIntyre
 */
class SelectDialog : public QDialog
{
    Q_OBJECT

public:
    SelectDialog(QWidget *parent = 0
                );
    ~SelectDialog();

public slots:
    void accept();
    void help();

protected:

    // Top Level Layout Widgets  ///////////////////////////////
    
    QTabWidget *m_tabWidget;
    
    QRadioButton *m_replaceExistingSelection;
    QRadioButton *m_addToExistingSelection;

    // QDialogButtonBox and various QLabel and minor widgets created in ctor.
    //
    // global select all/none buttons would be useful, to set the check
    // everything per tab, but can you call something on a tab that's hidden?

    void makeDurationTab();
    void makePitchTab();
    void makeSpecialTab();
    void makeAdvancedTab();


    // Duration Widgets ////////////////////////////////////////////////

    // a widget for the tab page
    QWidget *m_durationTab;

    // breve buttons
    CheckButton *m_use_duration_breve;
    CheckButton *m_use_duration_breve_dotted;
    CheckButton *m_use_duration_breve_double_dotted;
    CheckButton *m_use_duration_breve_tuplet;
    CheckButton *m_use_duration_breve_rest;
    CheckButton *m_use_duration_breve_dotted_rest;
    CheckButton *m_use_duration_breve_double_dotted_rest;
    CheckButton *m_use_duration_breve_rest_tuplet;
    CheckButton *m_use_all_breves;

    // semibreve buttons
    CheckButton *m_use_duration_semibreve;
    CheckButton *m_use_duration_semibreve_dotted;
    CheckButton *m_use_duration_semibreve_double_dotted;
    CheckButton *m_use_duration_semibreve_tuplet;
    CheckButton *m_use_duration_semibreve_rest;
    CheckButton *m_use_duration_semibreve_dotted_rest;
    CheckButton *m_use_duration_semibreve_double_dotted_rest;
    CheckButton *m_use_duration_semibreve_rest_tuplet;
    CheckButton *m_use_all_semibreves;

    // minim buttons
    CheckButton *m_use_duration_minim;
    CheckButton *m_use_duration_minim_dotted;
    CheckButton *m_use_duration_minim_double_dotted;
    CheckButton *m_use_duration_minim_tuplet;
    CheckButton *m_use_duration_minim_rest;
    CheckButton *m_use_duration_minim_dotted_rest;
    CheckButton *m_use_duration_minim_double_dotted_rest;
    CheckButton *m_use_duration_minim_rest_tuplet;
    CheckButton *m_use_all_minims;

    // crotchet buttons
    CheckButton *m_use_duration_crotchet;
    CheckButton *m_use_duration_crotchet_dotted;
    CheckButton *m_use_duration_crotchet_double_dotted;
    CheckButton *m_use_duration_crotchet_tuplet;
    CheckButton *m_use_duration_crotchet_rest;
    CheckButton *m_use_duration_crotchet_dotted_rest;
    CheckButton *m_use_duration_crotchet_double_dotted_rest;
    CheckButton *m_use_duration_crotchet_rest_tuplet;
    CheckButton *m_use_all_crotchets;

    // quaver buttons
    CheckButton *m_use_duration_quaver;
    CheckButton *m_use_duration_quaver_dotted;
    CheckButton *m_use_duration_quaver_double_dotted;
    CheckButton *m_use_duration_quaver_tuplet;
    CheckButton *m_use_duration_quaver_rest;
    CheckButton *m_use_duration_quaver_dotted_rest;
    CheckButton *m_use_duration_quaver_double_dotted_rest;
    CheckButton *m_use_duration_quaver_rest_tuplet;
    CheckButton *m_use_all_quavers;

    // semiquaver buttons
    CheckButton *m_use_duration_semiquaver;
    CheckButton *m_use_duration_semiquaver_dotted;
    CheckButton *m_use_duration_semiquaver_double_dotted;
    CheckButton *m_use_duration_semiquaver_tuplet;
    CheckButton *m_use_duration_semiquaver_rest;
    CheckButton *m_use_duration_semiquaver_dotted_rest;
    CheckButton *m_use_duration_semiquaver_double_dotted_rest;
    CheckButton *m_use_duration_semiquaver_rest_tuplet;
    CheckButton *m_use_all_semiquavers;

    // demisemi buttons
    CheckButton *m_use_duration_demisemi;
    CheckButton *m_use_duration_demisemi_dotted;
    CheckButton *m_use_duration_demisemi_double_dotted;
    CheckButton *m_use_duration_demisemi_tuplet;
    CheckButton *m_use_duration_demisemi_rest;
    CheckButton *m_use_duration_demisemi_dotted_rest;
    CheckButton *m_use_duration_demisemi_double_dotted_rest;
    CheckButton *m_use_duration_demisemi_rest_tuplet;
    CheckButton *m_use_all_demisemis;

    // hemidemisemi buttons
    CheckButton *m_use_duration_hemidemisemi;
    CheckButton *m_use_duration_hemidemisemi_dotted;
    CheckButton *m_use_duration_hemidemisemi_double_dotted;
    CheckButton *m_use_duration_hemidemisemi_tuplet;
    CheckButton *m_use_duration_hemidemisemi_rest;
    CheckButton *m_use_duration_hemidemisemi_dotted_rest;
    CheckButton *m_use_duration_hemidemisemi_double_dotted_rest;
    CheckButton *m_use_duration_hemidemisemi_rest_tuplet;
    CheckButton *m_use_all_hemidemisemis;

    // use everything per column
    CheckButton *m_use_all_normals;
    CheckButton *m_use_all_dotteds;
    CheckButton *m_use_all_double_dotteds;
    CheckButton *m_use_all_tuplets;
    CheckButton *m_use_all_rests;
    CheckButton *m_use_all_dotted_rests;
    CheckButton *m_use_all_double_dotted_rests;
    CheckButton *m_use_all_rest_tuplets;
    CheckButton *m_use_all_duration;

    // checkboxes
    QCheckBox *m_include_shorter_performance_durations;
    QCheckBox *m_include_longer_performance_durations;


    // Pitch Widgets ///////////////////////////////////////////////////
    //
    //
    // Pitch stuff goes in here.  I haven't got a plan for this yet.
    //
    // How are we going to handle picking pitch?  The standard pitch picker is
    // hard to use if you're trying to pick off all the unplayable low notes for
    // a guitar, for example, because it shows concert pitch in a variety of
    // clefs that change to keep the display compact.
    //
    // We want some controls to automatically grab notes outside the playable
    // range, and it needs to handle the high end and low end as separate use
    // cases.  I can't play below low F# on trumpet to save my life, but maybe I
    // can squeak out da dubba high C and maybe I can't.
    //
    // [x] Include notes above playable range
    // [x] Include notes within playable range
    // [x] Include notes below playable range
    // [x] Use specified pitch only
    //
    // Hmmmm....  The duration picker lets you pick a range here and a range
    // there and a range somewhere else.  It would be nice to dream up a way to
    // do the equivalent here.
    //
    // Also, find an absolute letter note in every octave, ie. the find all Db
    // and respell as C# use case
    //
    // I really need to go to the drawing board on this.
    //
    // Other than the high/low range stuff, picking ranges of pitches is best
    // done in the matrix, with the bigass piano keyboard.
    //
    ///////////////////////////////////////////////////////////////////

    // a widget for the tab page
    QWidget *m_pitchTab;



    // Special Widgets /////////////////////////////////////////////////
    //
    // This page lumps together everything that can't be selected by a pitch or
    // a duration, including note properties and all manner of special kinds of
    // events that aren't notes at all (guitar chords, indications, symbols,
    // etc.)

    // a widget for the tab page
    QWidget *m_specialTab;

    // row 1
    CheckButton *m_useAccent;         
    CheckButton *m_useTenuto;         
    CheckButton *m_useStaccato;       
    CheckButton *m_useStaccatissimo;  
    CheckButton *m_useMarcato;        
    CheckButton *m_useOpen;           
    CheckButton *m_useStopped;        
    CheckButton *m_useHarmonic;
    CheckButton *m_useRow1;


    // row 2
    CheckButton *m_useUpBow;          
    CheckButton *m_useDownBow;        
    CheckButton *m_useOneSlash;
    CheckButton *m_useTwoSlash;
    CheckButton *m_useThreeSlash;
    CheckButton *m_useFourSlash;
    CheckButton *m_useFiveSlash;
    CheckButton *m_useRow2;
    
    // row 3
    CheckButton *m_useSforzando;      
    CheckButton *m_useRinforzando;    
    CheckButton *m_useTrill;          
    CheckButton *m_useTrillLineIndication;
    CheckButton *m_useTurn;           
    CheckButton *m_useMordent;
    CheckButton *m_useMordentInverted;
    CheckButton *m_useRow3;

    // row 4
    CheckButton *m_useMordentLong;
    CheckButton *m_useMordentLongInverted;
    CheckButton *m_useCrescendo;
    CheckButton *m_useDecrescendo;
    CheckButton *m_useSlur;
    CheckButton *m_usePhrasingSlur;
    CheckButton *m_usePause;          
    CheckButton *m_useRow4;

    // row 5
    CheckButton *m_useQuindicesimaUp;
    CheckButton *m_useOttavaUp;
    CheckButton *m_useOttavaDown;
    CheckButton *m_useQuindicesimaDown;
    CheckButton *m_useSegno;
    CheckButton *m_useCoda;
    CheckButton *m_useBreath;
    CheckButton *m_useRow5;

    // row 6
    CheckButton *m_usePedalDown;
    CheckButton *m_usePedalUp;
    CheckButton *m_useNatural;
    CheckButton *m_useSharp;
    CheckButton *m_useDoubleSharp;
    CheckButton *m_useFlat;
    CheckButton *m_useDoubleFlat;
    CheckButton *m_useRow6;

    // row 7
    CheckButton *m_useNoAccidental;
    CheckButton *m_useTextEvents;
    CheckButton *m_useFingering;
    CheckButton *m_useTextMark;
    CheckButton *m_useGuitarChord;
    // ChordEvent  *
    // Barre        *____ all sort of semantically related; close enough
    // String       *
    // PIMA        *
    CheckButton *m_useRow7;

    // row 8
    // semantics for lower row up arrow buttons don't make sense, except this:
    CheckButton *m_useAllSpecial;


    // Advanced Widgets ////////////////////////////////////////////////
    //
    // This page is a big collection of check boxes for special properties, text
    // types, and so forth, that can't realistically be represented by icon
    // buttons, but are worth being able to dial in.  Potential uses consist of
    // an abundance of wild but credible edge cases.
    
    // a widget for the tab page
    QWidget *m_advancedTab;
    
    QCheckBox *m_include_key_signatures;
    QCheckBox *m_include_clefs;
    QCheckBox *m_include_tied_forward;
    QCheckBox *m_include_tied_back;
    QCheckBox *m_include_is_beamed;
    QCheckBox *m_include_is_grace; //!! pay some special attention to grace notes
    QCheckBox *m_text_is_UnspecifiedType;
    QCheckBox *m_text_is_StaffName;
    QCheckBox *m_text_is_ChordName;
    QCheckBox *m_text_is_KeyName;
    QCheckBox *m_text_is_Lyric;
    QCheckBox *m_text_is_Chord;
    QCheckBox *m_text_is_Dynamic;
    QCheckBox *m_text_is_Direction;
    QCheckBox *m_text_is_LocalDirection;
    QCheckBox *m_text_is_Tempo;
    QCheckBox *m_text_is_LocalTempo;
    QCheckBox *m_text_is_Annotation;
    QCheckBox *m_text_is_LilyPondDirective;

    //TODO - this definitely needs a master toggle switch for everything on the
    // page; probably a QCheckButton down in the lower right corner or
    // something, but let's get it farther along before figuring that out


    // The Island of Misfit Toys ///////////////////////////////////////
    //
    // ChordEvent is coming in from Niek's branch eventually, and will
    // definitely be supported.
    //
    // I plan to add a new barré indication, string [ (1) (2) ... ] marks, and
    // picking marks [ p i m a x ] in the near future, and they will be worked
    // in.
    //
    // ParameterChord and Figuration represent something Tom Breton said only
    // 2-3 people use, something obscure and impossible to figure out by
    // accident, and something available for selection through the EventList
    // event filter.  I think we'll skip those.
    //
    // Glissando exists in NotationTypes for some reason, but probably never
    // will exist.  If it ever does, it should be included.
    //
    // Quartertone accidentals?  Meh.
    //
//    CheckButton *m_useFigParameterChord;
//    CheckButton *m_useFiguration;            
//    CheckButton *m_useBarre;  // not implemented yet; future indication, need to plan in layout
//    CheckButton *m_useTrillLineMark; // the old legacy mark; should we just skip it then? 
//    CheckButton *m_useString; // not implemented yet
//    CheckButton *m_useChordNotation; // still in branch, not in NotationTypes in branch
//    CheckButton *m_useGlissando; // indication type never implemented, and likely won't be, but it's in NotationTypes
//    CheckButton *m_useQuarterFlat; // how do you even use these quarter tone accidentals?
//    CheckButton *m_useThreeQuarterFlat; 
//    CheckButton *m_useQuarterSharp; 
//    CheckButton *m_useThreeQuarterSharp;
//    CheckButton *m_useLongTrill;  // not in GUI, totally obsolete I think
//
//    Notes in Chords
//    We could check to see if notes are part of a chord and let you pick notes
//    that are or are not inside a chord.  This doesn't sound so useful, so I'm
//    tossing the idea down here.
//
//    Velocity
//    The original idea came from Cakewalk's event filter.  I don't think it's
//    very useful in practice, and I'm not implementing it at this time.
//
//    Grace Notes
//    I haven't really thought about them in this.  I'm pretty much about going
//    to have to build this entire thing and play with it to see how they
//    behave, and what it needs from there.  A special case to consider for a
//    future revision.
//


    // Implementation notes:
    //
    // Accidentals; semantically this will select notes that have this showing
    // explicitly, and not when it's merely coming from the key; possible uses,
    // find all the Db to respell as C#
    //
    // Link useTextEvents to the check boxes offering up text types.  If the T
    // icon is checked, the text types are available for checking.  This is
    // problematic now that the check boxes are going on a different page.
    //
    // I'm thinking about semantics for the accidentals buttons.  You have 4.
    // checked, it's going to select a note of 4. duration when it finds one.
    // If you have n # x b bb checked


protected slots:

    void slotUseAllBreve(bool);
    void slotUseAllSemiBreve(bool);
    void slotUseAllMinim(bool);
    void slotUseAllCrotchet(bool);
    void slotUseAllQuaver(bool);
    void slotUseAllSemiQuaver(bool);
    void slotUseAllDemiSemi(bool);
    void slotUseAllHemiDemiSemi(bool);

    void slotUseAllNormal(bool);
    void slotUseAllDotted(bool);
    void slotUseAllDoubleDotted(bool);
    void slotUseAllTuplet(bool);
    void slotUseAllRestNormal(bool);
    void slotUseAllRestDotted(bool);
    void slotUseAllRestDoubleDotted(bool);
    void slotUseAllRestTuplet(bool);

    void slotUseAllDuration(bool);

    void slotUseRow1(bool);
    void slotUseRow2(bool);
    void slotUseRow3(bool);
    void slotUseRow4(bool);
    void slotUseRow5(bool);
    void slotUseRow6(bool);
    void slotUseRow7(bool);
    void slotUseAllSpecial(bool);

};   

}

#endif
