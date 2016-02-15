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
// File: PlaylistMgr.h
//
// Desc: This file defines a class that will house the Playlist Manager.  The
//       Playlist manager is responsible for maintaining a list of playlists
//       on the local machine, a list of local media clips, and the list of
//       favorites.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PLAYLISTMGR_H_
#define _PLAYLISTMGR_H_

#include <windows.h>

#include "Plist.h"

#define MAX_PLAYLIST_HISTORY   5

#ifndef MAX_URL_LENGTH
#define MAX_URL_LENGTH 2048
#endif /* MAX_URL_LENGTH */

class CPlaylistMgr
{
	// forward decls
	struct duplist_t;
	struct list_t;

public:
    CPlaylist * CurrentPlaylist();
    CPlaylist * LocalContent();
    CPlaylist * GetPlaylist(int i);
    CPlaylist * GetFavorites();

    int         MRUPlaylistCount();
    CPlaylist * MRUPlaylist(int i = 0);
    void        MRURemove(int i);
    void        MRURename(int iIndex, LPCTSTR pszPath);

    bool SetCurrentPlaylist(LPCTSTR pszPath);
    bool SetCurrentPlaylist(int iIndex);
    int  GetCurrentPlaylistID();

    int  AddPlaylist(LPTSTR pszName, int i = 0);
    bool DeletePlaylist(int iIndex);
    bool RenamePlaylist(int iIndex, LPCTSTR pszName);

    bool IsValid(CPlaylist * pPlaylist);

    LPTSTR GetDisplayName(int iIndex);
    LPTSTR GetDisplayName(CPlaylist * pList);

    static bool CreateUniqueName(__in_ecount(iLength) LPTSTR pszName, int iLength);

    static CPlaylistMgr * GetInstance();

    struct playlist_t
    {
        LPCTSTR      pszName;
        playlist_t * pNext;
    };

    int GetPlaylistCount();

    void GetMediaShare(__in_ecount(nSizeShare) LPTSTR szShare, int nSizeShare);
    void SetMediaShare(LPCTSTR szShare);

    void UpdateLocalContent();
    void SaveRegState();
    void LoadRegState();

protected:
    CPlaylistMgr();
    ~CPlaylistMgr();

    void UpdateMRUPlaylists();

    void LoadLocalPlaylists();
    int  FindPlaylists(LPCTSTR pszPath, int iCount);

    int  FindMediaFiles(LPCTSTR pszPath, CPlaylist * pLocalContent, int iCount);
    bool IsMediaFile(LPCTSTR pszFile);

    LPTSTR GetDisplayNameHelper(list_t * pList);

private:
    static CPlaylistMgr * ms_pManager;

	// two-way linked list manages playlists
	struct list_t
	{
		list_t() : 
			pPlaylist(NULL),
			pNext(NULL),
			pPrev(NULL),
			pDupNames(NULL)
		{ }

		~list_t()
		{
			RemoveDupName();

            delete pPlaylist;

            pNext = NULL;
            pPrev = NULL;
            pDupNames = NULL;
        }

        CPlaylist * pPlaylist;
        list_t    * pNext;
        list_t    * pPrev;
        duplist_t * pDupNames;

		void RemoveDupName()
		{
			// remove ourselves from duplicate names list; delete
			// the duplicate names list if necessary
			if (!pDupNames)
				return;

			UINT cDups = 0;
			duplist_t* pDup = pDupNames;
			duplist_t* pPrevDup = NULL;
			while (pDup)
			{
				if (pDup->pList == this)
				{
					// we found our playlist, remove it

					if (pPrevDup)
					{
						pPrevDup->pNext = pDup->pNext;
						delete pDup;
						pDup = pPrevDup->pNext;
					}
					else
					{
						// ours was first in list; fix all
						// others to point to new head
						ASSERT(!cDups);

						pPrevDup = pDup;
						pDup = pDup->pNext;
						delete pPrevDup;

						pPrevDup = pDup; // use pPrevDup to save head
						while (pDup)
						{
							cDups++;
							pDup->pList->pDupNames = pPrevDup;
							pDup = pDup->pNext;
						}
					}
				}

				if (pDup)
				{
					cDups++;
					pPrevDup = pDup;
					pDup = pDup->pNext;
				}
			}

			if (cDups <= 1)
			{
				// we can remove the duplicate list altogether
				if (pDupNames)
				{
					ASSERT(!pDupNames->pNext);
					pDupNames->pList->pDupNames = NULL;
					delete pDupNames;
				}
			}

			pDupNames = NULL;
		}

		// pDupList is a pointer to any list_t object
		// whose playlist name is that same as in pNewList
		bool AddDupName(list_t* pDupList)
		{
			duplist_t* pDup, * pDupIter;

			// create the list if this is the first dup
			if (!pDupList->pDupNames)
			{
				pDup = new duplist_t;
				ASSERT(pDup);
				if (!pDup) return false;

				pDup->pList = pDupList;
				pDupList->pDupNames = pDup;
			}

			// now add new playlist to dups
			pDupIter = pDupList->pDupNames;
			while (pDupIter->pNext)
			{
				pDupIter = pDupIter->pNext;
			}

			pDup = new duplist_t;
			ASSERT(pDup);
			if (!pDup) return false;
			
			pDup->pList = this;
			pDupIter->pNext = pDup;
			pDupNames = pDupList->pDupNames;

#ifdef DEBUG
			// as a debug check, make sure we don't list
			// the same playlist twice

			duplist_t * pOuter, * pInner;
			pOuter = pDupNames;

			// list should contain at least 2 objects
			ASSERT(pOuter && pOuter->pNext);

			while (pOuter)
			{
				pInner = pOuter->pNext;
				while (pInner)
				{
					ASSERT(0 != 
						_tcscmp(pInner->pList->pPlaylist->GetPath(),
							 	pOuter->pList->pPlaylist->GetPath()));

					pInner = pInner->pNext;
				}

				pOuter = pOuter->pNext;
			}
#endif	// DEBUG

			return true;
		}
    };

	// one-way linked list keeps track of playlists
	// with duplicate names (but different paths)
	//
	// list_t objects representing playlists with
	// the same duplicate name all point to the head
	// of same linked list of duplist_t objects
	struct duplist_t
	{
		duplist_t() : pList(NULL), pNext(NULL) { }
		~duplist_t()
		{
			ASSERT(pList);
			pList = NULL;
			pNext = NULL;
		}

		list_t* pList;
		duplist_t* pNext;
	};

    list_t * m_pPlaylists;

    CPlaylist * m_pLocalContent;
    CPlaylist * m_pFavorites;
    TCHAR     * m_pszMRUPlaylist[MAX_PLAYLIST_HISTORY];
    LPTSTR      m_pszMediaShare;

    int  m_iCurrent;
    bool m_bLoadedLocalPlaylists;
};

#endif /* _PLAYLISTMGR_H_ */
