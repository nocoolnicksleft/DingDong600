//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
///////////////////////////////////////////////////////////////////////////////
//
// Sample Name: CEPlayer Sample
//
///////////////////////////////////////////////////////////////////////////////

Description
===========
  CEPlayer demonstrates the tasks required to initialize and run an
  application that hosts the Windows Media Player ActiveX control for audio
  and video playback.

User's Guide
============
  Input is accepted either through the menus or buttons on the player.

  If the player is started from a command line or with the Run command off
  of the Start Menu an optional paramater of a file to be played may be
  used, in which case playback will begin automatically once the file is
  loaded.

  The playlist dialogs maybe started from the File menu by selecting the
  Playlists item.  The first dialog displayed presents a view of the
  current playlist (or all local content if no playlists exist).  This
  dialog allows items to be added, removed, or re-ordered.  Playback and
  track info is also available.

  Selecting the Organize Playlists option from the drop down menu allows
  playlists to be created, deleted from the system, or renamed.  When a
  new playlist is created the user is given the opportunity to add content
  to that playlist.
  
General Notes
=============

  By default, the \Windows directory is scanned by the Playlist Manager
  for local content.  To exclude that directory simply un-comment the line
  in PlaylistMgr.cpp containing "\\Windows" in the exclude path.

  The registry key [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\CEPlayer\HomePage]
  contains the web pages launched by the [-> Web] button.

Programming Notes
=================

FILES:

   AddDeleteDlg.h        - The Organize Playlists dialog is defined in this file.  It
                           is used to add, delete, or rename playlists.
   AddDeleteDlg.cpp      - This file implements the member functions for the Organize
                           Playlists dialog.
   AddTracksDlg.h        - The Add Tracks dialog is defined in this file.  It allows
                           content to be added to an existing playlist.
   AddTracksDlg.cpp      - This file implements the member functions for the Add
                           Tracks dialog.
   BaseDialog.h          - All of the playlist dialogs derive from this class.
   BaseDialog.cpp        - The base class for all playlist dialogs is implemented in
                           this file.
   CEPlayer.cpp          - WinMain and WinProc are contained in this file
   CEPlayer.rc           - The menu structure is defined in this resouce file
   CEPlayerUtil.h        - This file contains the function prototypes for functions
                           required by multiple components.
   CEPlayerUtil.cpp      - The utility functions are implemented here.
   MPContainer.h         - This file defines a class (CMPContainer) that is used
                           as the ActiveX container for the WMP ActiveX control
   MPContainer.cpp       - The implementation of CMPContainer
   MPEventSink.h         - In order to handle events, an EventSink must be defined
                           this class (CMPEventSink) will handle MediaPlayer events
   MPEventSink.cpp       - The implementation of CMPEventSink
   OpenURLDlg.h          - The DialogProc for the OpenURL dialog box is declared
                           in this header.
   OpenURLDlg.cpp        - The only function implemented in this file is the main
                           DialogProc for the OpenURL dialog.
   OptionsDlg.h          - This dialog has three options.  The network share option
                           is used by the playlist manager to discover content on
                           a remote computer.
   OptionsDlg.cpp        - This file implements the functions for the dialog.
   PlayerWindow.h        - This class (CPlayerWindow) derives from CMPContainer
                           and CMPEventSink to provide a fully functional
                           environment for housing the WMP ActiveX control
   PlayerWindow.cpp      - The implementation of CPlayerWindow
   PlaylistDlg.h         - This dialog is used to view the contents of a playlist.
   PlaylistDlg.cpp       - The various dialog methods are implemented in this
                           file.
   PlaylistMgr.h         - The Playlist Manager class is used to discover content
                           and keep track of all of the playlists on the device.
   PlaylistMgr.cpp       - The playlist manager is implemeneted in this file.
   PList.h               - A class encapsulating a playlist is defined in this file
                           along with a class for an individual media clip.
   PList.cpp             - The implementations for the playlist and media clip 
                           classes are contained in this file.
   PropertyDlg.h         - The class encapsulating the Property dialog is defined
                           in this file.
   PropertyDlg.cpp       - This file implements the member functions of the
                           CPropertyDlg class as well as the DialogProc for this
                           dialog.
   SelectPlaylistDlg.h   - The class used to display the Select Playlist dialog.
   SelectPlaylistDlg.cpp - The DialogProc for this class is implemented in this
                           file.
   StatisticsDlg.h       - The class encapsulating the Statistics dialog is
                           defined in this file.
   StatisticsDlg.cpp     - This file implements the member functions of the
                           CStatisticsDlg class as well as the DialogProc for
                           this dialog.
   makefile              - This is a dummy makefile to ensure the use of the
                           build command
   readme.txt            - This file contains information relating the the
                           CEPlayer application not found in the source comments
   newres.h              - Addition resource definitions.
   resource.h            - The resource header contains the ID's of menus and
                           menu items
   sources               - This is the file that is used by build to compile and
                           link the CEPlayer application

NOTES:
   There are several function that are required to make the WMP control
function properly that do not seem necessary at first.  The CMPContainer's
methods GetDC() and ReleaseDC() are required to allow the control to update
it's tracker bar and status bar.  Without these two methods the WMP control
can only update those two items when it is redrawn by the container.

   In order to allow the control to respond to mouse events any event that
is not handled by the main WinProc must be passed to the control to allow it
a chance to respond to the event.  Handling accelerator keys is handled in a
similar manner.  Any key event must be passed to the
IControl::TranslateAccelerator() function.  If that function can translate
the accelerator, then the OnMnemonic() function should be passed the key
event.  It is this function that actually responds to the accelerator key.
