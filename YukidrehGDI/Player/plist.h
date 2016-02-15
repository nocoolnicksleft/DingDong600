//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
///////////////////////////////////////////////////////////////////////////////
// File: PList.h
//
// Desc: A playlist is a collection of media clips, and this file declares
//       both objects.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PLAYLIST_H_
#define _PLAYLIST_H_

#include <windows.h>
#include <tchar.h>

enum EFileLocation
{
    LOCAL       = 0,
    NETWORK     = 1,
    STORAGECARD = 2,
    HARDDRIVE   = 3
};

class CMediaClip
{
public:
    CMediaClip();
    ~CMediaClip();

    bool SetLocation(LPCTSTR);

    LPCTSTR        GetTitle() const;
    LPCTSTR        GetArtist() const;
    LPCTSTR        GetPath() const;
    EFileLocation  GetLocation() const;

    bool IsLocal() const;
    bool IsAvailable() const;

private:
    LPTSTR  m_pszTitle;
    LPTSTR  m_pszPath;
    LPTSTR  m_pszArtist;
    UINT    m_cRef;

    EFileLocation m_eLocation;
};

class CPlaylist
{
public:
    CPlaylist(const TCHAR * pszPath);
    ~CPlaylist();

    static bool GetNameFromPath(__in_ecount(nSizePath) LPTSTR szPath, int nSizePath, LPTSTR* pszName);

    bool InsertTrack(UINT, LPCTSTR);
    bool DeleteTrack(UINT);
    void DeleteAll();

    void AddRef() {}
    void Release() {}

    bool ShiftTrackUp(UINT);
    bool ShiftTrackDown(UINT);

    CMediaClip * GetTrack(int i);
    CMediaClip * PlayCurrentTrack();
    void CurrentTrackDonePlaying();
    LPCTSTR      GetName();
    LPCTSTR      GetPath();

    UINT GetCount();
    void ResetPlayed();
    bool IsPlaying(int iIndex);
    int  GetCurrent();
    void SetCurrent(int iIndex);
    bool NextTrack(bool bRandom = false, bool bRepeat = false);
    bool PrevTrack(bool bRandom = false, bool bRepeat = false);

    bool Load();
    bool Save();
    bool Rename(LPCTSTR pszName);

    bool IsCEPlaylist();
    void IsCEPlaylist(bool b);
    bool IsHidden();
    void IsHidden(bool b);
    bool IsTransient();
    void IsTransient(bool b);

    struct playlist_t
    {
        playlist_t() :
        	pClip(NULL),
        	pNext(NULL),
        	pPrev(NULL),
        	bPlayed(false),
        	bIsPlaying(false)
        { }

        ~playlist_t()
        {
            delete pClip;

            // for debugging
            pClip = (CMediaClip*)0xDEADC0DE;
            pNext = (playlist_t*)0xDEADC0DE;
            pPrev = (playlist_t*)0xDEADC0DE;
        }

        CMediaClip * pClip;

        playlist_t * pNext;
        playlist_t * pPrev;
        bool         bPlayed;
        bool         bIsPlaying;
    };

    playlist_t * GetFirst();

private:
    bool LoadParam();
    void SwapItems(playlist_t *);

    playlist_t * GetListItem(int iIndex);

    LPTSTR m_pszPath;
    LPTSTR m_pszName;

    playlist_t * m_pList;
    int          m_iCurrentTrack;
    UINT         m_uCount;
    bool         m_bCEPlaylist;
    bool         m_bHidden;
    bool         m_bTransient;
};

#endif /* _PLAYLIST_H_ */
