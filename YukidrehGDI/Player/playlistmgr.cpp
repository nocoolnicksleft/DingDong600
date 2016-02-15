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
// File: PlaylistMgr.cpp
//
// Desc: This file implements a class which houses the Playlist Manager
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <windev.h>
#include "pkfuncs.h"
#include "resource.h"

#include "PlaylistMgr.h"

       TCHAR   s_szBasePath[]     = TEXT("\\My Documents");
static LPCTSTR s_pszExcludePath[] = { TEXT("\\Databases"), 
                                      TEXT("\\Release"),
//                                      TEXT("\\Windows"),
                                      TEXT("\\Recycled") };
static LPCTSTR s_pszIncludePath[] = { TEXT("\\"),
                                      TEXT("\\Windows\\Desktop"),
                                      TEXT("\\Windows\\Recent") };

static const int REGKEY_SIZE = 80;

const int s_iExcludePathLength = sizeof (s_pszExcludePath) / sizeof (LPCTSTR);
const int s_iIncludePathLength = sizeof (s_pszIncludePath) / sizeof (LPCTSTR);

extern HINSTANCE g_hInst;
extern DWORD     g_cAudioTypes;
extern DWORD     g_cVideoTypes;
extern TCHAR **  g_ppszAudioTypes;
extern TCHAR **  g_ppszVideoTypes;

CPlaylistMgr * CPlaylistMgr::ms_pManager = NULL;

CPlaylist * CPlaylistMgr::CurrentPlaylist()
{
    list_t * pList = NULL;
    int i = 0;

    if (m_iCurrent >= 0)
    {
        pList = m_pPlaylists;

        while (pList)
        {
            if (i == m_iCurrent) break;

            pList = pList->pNext;
            i++;
        }

        if (i != m_iCurrent)
        {
            m_iCurrent = -1;
            pList      = NULL;
        }
    }

    return (pList ? pList->pPlaylist : NULL);
}

CPlaylist * CPlaylistMgr::LocalContent()
{
    if (NULL == m_pLocalContent || 0 == m_pLocalContent->GetCount())
    {
        UpdateLocalContent();
    }

    return m_pLocalContent;
}

CPlaylist * CPlaylistMgr::GetPlaylist(int iIndex)
{
    list_t * pLists = m_pPlaylists;

    for (int i = 0; i < iIndex; i++)
    {
        if (NULL == pLists)
        {
           break;
        }

        pLists = pLists->pNext;
    }

    return (pLists ? pLists->pPlaylist : NULL);
}

CPlaylist * CPlaylistMgr::GetFavorites()
{
    return m_pFavorites;
}

int CPlaylistMgr::MRUPlaylistCount()
{
    int iCount;

    for (iCount = 0; iCount < MAX_PLAYLIST_HISTORY
                     && NULL != m_pszMRUPlaylist[iCount]; iCount++);

    return iCount;
}

CPlaylist * CPlaylistMgr::MRUPlaylist(int i)
{
    if (0 <= i && i < MAX_PLAYLIST_HISTORY
        && NULL != m_pszMRUPlaylist[i])
    {
        list_t * pTemp = m_pPlaylists;

        while (pTemp)
        {
            CPlaylist * pPlaylist = pTemp->pPlaylist;

            if (pPlaylist && pPlaylist->GetPath())
            {
                if (0 == _tcscmp(m_pszMRUPlaylist[i], pPlaylist->GetPath()))
                {
                    return pPlaylist;
                }
            }

            pTemp = pTemp->pNext;
        }

        //
        // If the playlist couldn't be found, try loading it.  GetPlaylist
        // will return NULL if the playlist can not be found.
        //
        return GetPlaylist(AddPlaylist(m_pszMRUPlaylist[i], 0));
    }

    return NULL;
}

void CPlaylistMgr::MRURemove(int iIndex)
{
    int     i;

    delete [] m_pszMRUPlaylist[iIndex];

    for (i = iIndex; i < MAX_PLAYLIST_HISTORY - 1; i++)
    {
        m_pszMRUPlaylist[i] = m_pszMRUPlaylist[i + 1];
    }

    m_pszMRUPlaylist[MAX_PLAYLIST_HISTORY - 1] = NULL;

    SaveRegState();
}

void CPlaylistMgr::MRURename(int iIndex, LPCTSTR pszPath)
{
	ASSERT(0 <= iIndex && iIndex < MAX_PLAYLIST_HISTORY);
	ASSERT(pszPath && m_pszMRUPlaylist[iIndex]);

	if( pszPath == 0 ) 
	{
		return;
	}
	
	delete [] m_pszMRUPlaylist[iIndex];

	UINT uLen = _tcslen(pszPath) + 1;

 	if (m_pszMRUPlaylist[iIndex] = new TCHAR[uLen])
	{
		m_pszMRUPlaylist[iIndex][uLen - 1] = 0;
		StringCchCopy(m_pszMRUPlaylist[iIndex], uLen, pszPath);
		ASSERT(0 == m_pszMRUPlaylist[iIndex][uLen - 1]);
 	}
 	else ASSERT(FALSE);
 
	SaveRegState();
}

bool CPlaylistMgr::SetCurrentPlaylist(LPCTSTR pszPath)
{
    int      iIndex  = 0;
    list_t * pItem   = m_pPlaylists;
    bool     bResult = false;

    while (pItem && !bResult)
    {
       if (0 == _tcsicmp(pszPath, pItem->pPlaylist->GetPath()))
       {
           m_iCurrent = iIndex;

           bResult = true;
       }

       iIndex++;

       pItem = pItem->pNext;
    }

    if (!bResult)
    {
        iIndex = AddPlaylist((LPTSTR)pszPath, iIndex);

        if (iIndex >= 0)
        {
            m_iCurrent = iIndex;

            bResult = true;
        }
    }

    if (bResult)
    {
        UpdateMRUPlaylists();
    }

    return bResult;
}

bool CPlaylistMgr::SetCurrentPlaylist(int iIndex)
{
    int      iPos    = 0;
    list_t * pItem   = m_pPlaylists;
    bool     bResult = false;

    if (-1 == iIndex)
    {
        m_iCurrent = -1;
        bResult    = true;
    }

    while (pItem && !bResult)
    {
        if (iIndex == iPos)
        {
            m_iCurrent = iIndex;
            bResult    = true;
        }

        iPos++;
        pItem = pItem->pNext;
    }

    if (bResult)
    {
        UpdateMRUPlaylists();
    }

    return bResult;
}

int  CPlaylistMgr::GetCurrentPlaylistID()
{
    return m_iCurrent;
}

int CPlaylistMgr::AddPlaylist(LPTSTR pszPath, int iIndex)
{
	int      iPos     = 0;
	int      iResult  = -1;
	list_t * pIter    = m_pPlaylists;
	list_t * pCur     = pIter;
	list_t * pPrev    = NULL;
	list_t * pNew     = NULL;
	list_t * pDupName = NULL;
	LPTSTR   pszName  = NULL;

	bool fRet = CPlaylist::GetNameFromPath(pszPath, MAX_PATH, &pszName);
	if (!fRet) return -1;

	//
	// We do SEVERAL things inside this while loop:
	// 1. Make sure we aren't adding a duplicate playlist (where
	//    "duplicate playlist" is a new reference to an existing
	//    playlist).
	// 2. Notice if new playlist has same name as an existing
	//    playlist, so we can mark them as having matching names
	//    later.
	// 3. Find the playlist at iIndex so we can insert the new
	//    playlist there (unless the new playlist is a dup, see #1).
 	//

	while (pIter)
	{
		LPCTSTR pszTemp = NULL;

		if (pIter->pPlaylist
			&& (pszTemp = pIter->pPlaylist->GetPath())
			&& 0 == _tcscmp(pszPath, pszTemp))
		{
			// This playlist is already in the list
			return iPos;
		}

		// if the path is different but the name is the
		// same, make a note of it and we'll add this
		// playlist to the dup names list farther down
		if (!pDupName && pIter->pPlaylist
			&& (pszTemp = pIter->pPlaylist->GetName())
			&& 0 == _tcscmp(pszName, pszTemp))
		{
			pDupName = pIter;
		}

		pIter = pIter->pNext;

		if (iPos < iIndex)
		{
			pPrev = pCur;
			pCur = pCur->pNext;
			ASSERT(pCur == pIter);
		}

		iPos++;
	}

	delete [] pszName;
	pszName = NULL;

	pNew = new list_t;

	if (NULL == pNew)
		return iResult;

	pNew->pPlaylist = new CPlaylist(pszPath);

	if (pPrev)
	{
		iResult = iIndex;
		pNew->pNext = pPrev->pNext;
		pNew->pPrev = pPrev;
		pPrev->pNext = pNew;
		if (pCur) pCur->pPrev = pNew;
	}
	else
	{
		iResult = 0;
		if (m_pPlaylists) m_pPlaylists->pPrev = pNew;
		pNew->pNext = m_pPlaylists;
		m_pPlaylists = pNew;
	}

	// if this playlist has a duplicate name, add it to
	// the dup names list
	if (pDupName)
		pNew->AddDupName(pDupName);

	return iResult;
}

bool CPlaylistMgr::DeletePlaylist(int iIndex)
{
    int         iPos = 0;
    CPlaylist * pPlaylist;
    list_t    * pCurr;
    BOOL        fResult;
    bool        bResult = false;

    pCurr = m_pPlaylists;

    while (pCurr && iPos != iIndex)
    {
        pCurr = pCurr->pNext;

        iPos++;
    }

    if (pCurr && iPos == iIndex)
    {
        pPlaylist = pCurr->pPlaylist;

        fResult = DeleteFile(pPlaylist->GetPath());

        if (fResult)
        {
            LPCTSTR pszPath    = pPlaylist->GetPath();

            //
            // If the playlist was the "current" playlist, make the current
            // playlist invalid
            //
            if (m_iCurrent == iIndex)
            {
                m_iCurrent = -1;
            }

            //
            // If necessary remove the playlist from the MRU playlist list
            //

            for (int i = 0; i < MAX_PLAYLIST_HISTORY; i++)
            {
                if (m_pszMRUPlaylist[i]
                    && 0 == _tcscmp(m_pszMRUPlaylist[i], pszPath))
                {
                    MRURemove(i);
                    break;
                }
            }

            //
            // Delete the playlist
            //

            list_t * pPrev = pCurr->pPrev;
            list_t * pNext = pCurr->pNext;

            if (pPrev) pPrev->pNext = pNext;
            else       m_pPlaylists = pNext;

            if (pNext) pNext->pPrev = pPrev;

            delete pCurr;

            bResult = true;
        }
    }

    return bResult;
}

bool CPlaylistMgr::RenamePlaylist(int iIndex, LPCTSTR pszName)
{
    int      iPos = 0;
    list_t * pCurr;
    bool     bResult = false;
    TCHAR    szOldPath[MAX_PATH];

    pCurr = m_pPlaylists;

    while (pCurr && iPos != iIndex)
    {
        pCurr = pCurr->pNext;

        iPos++;
    }

    if (pCurr && iPos == iIndex)
    {
        StringCchCopy(szOldPath, MAX_PATH, pCurr->pPlaylist->GetPath());
    	szOldPath[MAX_PATH-1] = TEXT('\0');

        bResult = pCurr->pPlaylist->Rename(pszName);
    }

    if (!bResult) return bResult;

    //
    // If necessary rename the playlist in the MRU list
    //

    for (int i = 0; i < MAX_PLAYLIST_HISTORY; i++)
    {
        if (m_pszMRUPlaylist[i]
            && 0 == _tcscmp(m_pszMRUPlaylist[i], szOldPath))
        {
            MRURename(i, pCurr->pPlaylist->GetPath());
            break;
        }
    }

	// if the old name was a duplicate, fix up the duplicate
	// name list appropriately
	pCurr->RemoveDupName();

	// if the new name is a dup, add it to the list
	list_t * pIter = m_pPlaylists;
	ASSERT(pIter);
	while (pIter)
	{
		if (0 == _tcscmp(pszName, pIter->pPlaylist->GetName())
			&& 0 != _tcscmp(pCurr->pPlaylist->GetPath(),
							pIter->pPlaylist->GetPath()))
		{
			break;
		}

		pIter = pIter->pNext;
	}

	if (pIter)
		pCurr->AddDupName(pIter);

    return bResult;
}

bool CPlaylistMgr::IsValid(CPlaylist * pPlaylist)
{
    list_t * pCurr = m_pPlaylists;

    while (pCurr && pCurr->pPlaylist != pPlaylist)
    {
        pCurr = pCurr->pNext;
    }

    return pCurr ? true : false;
}

// This method returns the portion of the playlist
// path that is unique to this playlist.  If this
// playlist's name is unique among all playlists,
// NULL is returned.
LPTSTR CPlaylistMgr::GetDisplayName(int iIndex)
{
	int iPos;
	list_t* pIter;

	for (pIter = m_pPlaylists, iPos = 0; 
		 pIter && iPos < iIndex;
		 pIter = pIter->pNext, iPos++);

	if (!pIter)
	{
		ASSERT(FALSE);
		return NULL;
	}

	return GetDisplayNameHelper(pIter);
}

LPTSTR CPlaylistMgr::GetDisplayName(CPlaylist * pList)
{
	list_t* pIter;

	for (pIter = m_pPlaylists; 
		 pIter && pIter->pPlaylist != pList;
		 pIter = pIter->pNext);

	if (!pIter)
	{
		ASSERT(FALSE);
		return NULL;
	}

	return GetDisplayNameHelper(pIter);
}

LPTSTR CPlaylistMgr::GetDisplayNameHelper(list_t * pList)
{
	LPCTSTR szPath = NULL;
	int iWhack = 0;

	while (pList->pDupNames)
	{
		// figure out how many dups there are
		UINT cDups = 1;
		duplist_t* pDup = pList->pDupNames;
		while (pDup->pNext)
		{
			cDups++;
			pDup = pDup->pNext;
		}

		// get pointers to all playlist paths
		LPCTSTR* pszPaths = new LPCTSTR[cDups];
		if (!pszPaths)
		{
			ASSERT(FALSE);
			break;
		}

		UINT i = 0;

		pDup = pList->pDupNames;
		while (pDup)
		{
			pszPaths[i] = pDup->pList->pPlaylist->GetPath();
			if (pDup->pList == pList)
				szPath = pszPaths[i];
			i++;
			pDup = pDup->pNext;
		}

		ASSERT(szPath);

		// find the unique part
		bool fFound = false;
		TCHAR chCur = 0, chNext = 0;
		for (UINT iOuter = 0; ; iOuter++)
		{
			for (i = 0; i < cDups; i++)
			{
				if (!i)
				{
					chCur = pszPaths[i][iOuter];
					ASSERT(chCur);	// hit end of string???
				}
				else
				{
					if (chCur != (chNext = pszPaths[i][iOuter]))
					{
						fFound = true;
						ASSERT(chNext);	// hit end of string???
						break;
					}

					if (TEXT('\\') == chCur)
						iWhack = iOuter;

					chCur = chNext;
				}
			}

			if (fFound) break;
		}

		delete [] pszPaths;
		break;
	}

	LPCTSTR szName = pList->pPlaylist->GetName();

	UINT cReturn = _tcslen(szName) + 1;
	if (_tcsrchr(szName, TEXT('\\')))
	{
		cReturn--;
		szName++;
	}

	if (szPath)
	{
		cReturn += 4;	// 4 = 2 spaces + surrounding parens
		cReturn += _tcslen(szPath + iWhack);
		if (iWhack > 0) cReturn++;	// prepended ellipsis
	}

	LPTSTR szReturn = new TCHAR[cReturn];
	if (!szReturn)
	{
		ASSERT(FALSE);
		return NULL;
	}

	szReturn[cReturn - 1] = 0;

	// add playlist name
	StringCchCopy(szReturn, cReturn, szName);
	
    // remove extension from name before continuing
	TCHAR *pchPeriod = _tcsrchr(szReturn, TEXT('.'));
	if (pchPeriod) *pchPeriod = 0;

	// add path
	if (szPath)
	{
		StringCchCat(szReturn, cReturn, TEXT("  ("));
		if (iWhack > 0)
		    StringCchCat(szReturn, cReturn, TEXT("\x2026"));

		StringCchCat(szReturn, cReturn, szPath + iWhack);

		// remove name from end of path
		LPTSTR szLast = _tcsrchr(szReturn, TEXT('\\'));
		if (szLast)
		{
			// handle case where playlist is in root dir
			if (0 == _tcscmp(szLast, szPath)
				|| TEXT('\x2026') == *(szLast-1))
				szLast++;

			*szLast = 0;
		}

		StringCchCat(szReturn, cReturn, TEXT(")"));
	}

	ASSERT(0 == szReturn[cReturn - 1]);
	szReturn[cReturn - 1] = 0;

	return szReturn;
}

bool CPlaylistMgr::CreateUniqueName(LPTSTR pszName, int iLength)
{
    LPCTSTR pszBase = TEXT("Playlist");
    LPCTSTR pszExt  = TEXT("asx");
    int     i       = 2;
    bool    bResult = false;

    _sntprintf(pszName, iLength, TEXT("%s\\%s.%s"), s_szBasePath, pszBase, pszExt);
    pszName[ iLength - 1] = 0;

    if (0xffffffff == GetFileAttributes(pszName))
    {
        bResult = true;
    }

    while (!bResult)
    {
        _sntprintf(pszName, iLength, TEXT("%s\\%s #%d.%s"), s_szBasePath, pszBase, i, pszExt);
	pszName[ iLength - 1 ] = 0;

        if (0xffffffff == GetFileAttributes(pszName))
        {
            bResult = true;
        }

        i++;
    }

    return bResult;
}

CPlaylistMgr * CPlaylistMgr::GetInstance()
{
    // Create only one instance of the Playlist Manager
    if (NULL == ms_pManager)
    {
        ms_pManager = new CPlaylistMgr;
    }

    return ms_pManager;
}

int CPlaylistMgr::GetPlaylistCount()
{
	int iCount;
	list_t* pIter;

    if (!m_bLoadedLocalPlaylists)
        LoadLocalPlaylists();

	for (pIter = m_pPlaylists, iCount = 0;
		 pIter;
		 pIter = pIter->pNext, iCount++);

	return iCount;
}

void CPlaylistMgr::GetMediaShare(LPTSTR szShare, int nSizeShare)
{
    if (m_pszMediaShare)
    {
        StringCchCopy(szShare, nSizeShare, m_pszMediaShare);
    }
    else
    {
        szShare[0] = TEXT('\0');
    }
}

void CPlaylistMgr::SetMediaShare(LPCTSTR szShare)
{
    if (m_pszMediaShare)
    {
        delete [] m_pszMediaShare;
    }

    size_t BufLen = _tcslen(szShare) + 1;
    if (m_pszMediaShare = new TCHAR [BufLen])
    {
        StringCchCopy(m_pszMediaShare, BufLen, szShare);
    }

    SaveRegState();
}

CPlaylistMgr::CPlaylistMgr()
{
    int i;

    m_pPlaylists    = NULL;
    m_iCurrent      = -1;
    m_pszMediaShare = NULL;

    //
    // Find all playlists in local store
    //

//    LoadLocalPlaylists();
    m_bLoadedLocalPlaylists = false;

    //
    // Figure out what the current playlist is from the registry.
    // (default it to 0)
    //

    for (i = 0; i < MAX_PLAYLIST_HISTORY; i++)
    {
        m_pszMRUPlaylist[i] = NULL;
    }

    m_iCurrent = -1;

    LoadRegState();

    //
    // Add Local Content
    //
    if (m_pLocalContent = new CPlaylist(TEXT("Local Content")))
    {
        m_pLocalContent->IsTransient(true);
    }

    //
    // Add Favorites
    //
    if (m_pFavorites = new CPlaylist(TEXT("\\My Documents\\Favorites.asx")))
    {
        m_pFavorites->IsHidden(true);

        if (false == m_pFavorites->Load())
        {
            m_pFavorites->IsCEPlaylist(true);
        }
    }
}

CPlaylistMgr::~CPlaylistMgr()
{
   while (m_pPlaylists)
   {
        list_t * pList = m_pPlaylists->pNext;

        delete m_pPlaylists;

        m_pPlaylists = pList;
    }
}

void CPlaylistMgr::UpdateMRUPlaylists()
{
    int         i;
    CPlaylist * pPlaylist;
    LPCTSTR     pszPath = NULL;
    bool        bInHistory = false;

    pPlaylist = CurrentPlaylist();

    if (NULL == pPlaylist)
    {
        return;
    }

    pszPath = pPlaylist->GetPath();

    for (i = 0; i < MAX_PLAYLIST_HISTORY; i++)
    {
        if (m_pszMRUPlaylist[i] &&
            0 == _tcscmp(m_pszMRUPlaylist[i], pszPath))
        {
            bInHistory = true;
            break;
        }
    }

    if (!bInHistory)
    {
        if (m_pszMRUPlaylist[MAX_PLAYLIST_HISTORY - 1])
        {
            delete [] m_pszMRUPlaylist[MAX_PLAYLIST_HISTORY - 1];
        }

        for (i = MAX_PLAYLIST_HISTORY - 1; i > 0; i--)
        {
            m_pszMRUPlaylist[i] = m_pszMRUPlaylist[i-1];
        }

        size_t BufLen = _tcslen(pszPath) + 1;
        if (m_pszMRUPlaylist[0] = new TCHAR [BufLen])
        {
            StringCchCopy(m_pszMRUPlaylist[0], BufLen, pszPath);
        }
    }

    SaveRegState();
}

void CPlaylistMgr::SaveRegState()
{
    HKEY        hkResult  = NULL;
    CPlaylist * pPlaylist = NULL;
    TCHAR     * pszPath   = NULL;
    TCHAR     * pszShare  = NULL;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      TEXT("SOFTWARE\\Microsoft\\CEPlayer"),
                                      0, 0, &hkResult))
    {
        TCHAR szRegKey[REGKEY_SIZE];

        for (int i = 0; i < MAX_PLAYLIST_HISTORY; i++)
        {
            LPCTSTR pszStr = TEXT("");

            if (NULL != m_pszMRUPlaylist[i])
            {
                pszStr = m_pszMRUPlaylist[i];
            }

            LoadString(g_hInst, ID_PLAYLIST1_REGKEY + i, szRegKey, REGKEY_SIZE);
            RegSetValueEx(hkResult, szRegKey, 0, REG_SZ,
                          (BYTE*)pszStr,
                          (_tcslen(pszStr) + 1) * sizeof (TCHAR));
        }

        pPlaylist = CurrentPlaylist();

        if (pPlaylist)
        {
            pszPath = (TCHAR*)pPlaylist->GetPath();
        }
        else
        {
            pszPath = (TCHAR*)TEXT("");
        }

        RegSetValueEx(hkResult,
                      TEXT("LastPlaylist"), 0,
                      REG_SZ, (BYTE*)pszPath,
                      (_tcslen(pszPath) + 1) * sizeof (TCHAR));

        if (m_pszMediaShare)
        {
            pszShare = m_pszMediaShare;
        }
        else
        {
            pszShare = (TCHAR*)TEXT("");
        }

        RegSetValueEx(hkResult,
                      TEXT("MediaShare"), 0,
                      REG_SZ, (BYTE*)pszShare,
                      (_tcslen(pszShare) + 1) * sizeof (TCHAR));

        RegCloseKey(hkResult);
    }
}

void CPlaylistMgr::LoadRegState()
{
    HKEY  hkResult = NULL;
    DWORD dwDisp;
    TCHAR * szPlaylist = new TCHAR[MAX_URL_LENGTH];
    TCHAR * szRegKey   = new TCHAR[REGKEY_SIZE];

    if (NULL != szPlaylist
        && NULL != szRegKey
        && ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                           TEXT("SOFTWARE\\Microsoft\\CEPlayer"),
                                           0, NULL, 0, 0, NULL, &hkResult, &dwDisp))
    {
        DWORD dwSize, dwKeyType;
        int iIter, iCur;

        for (iIter = iCur = 0; iCur < MAX_PLAYLIST_HISTORY; iIter++)
        {
            m_pszMRUPlaylist[iCur] = NULL;
            if (iIter >= MAX_PLAYLIST_HISTORY)
            {
            	iCur++;
            	continue;
            }

            szRegKey[0] = TEXT('\0');
            dwSize = MAX_URL_LENGTH;

            LoadString(g_hInst, ID_PLAYLIST1_REGKEY + iIter, szRegKey, REGKEY_SIZE);

            if (ERROR_SUCCESS != RegQueryValueEx(hkResult, szRegKey, 0, &dwKeyType,
                                                 (BYTE*)szPlaylist, &dwSize))
            {
                continue;
            }

            if (0 == _tcslen(szPlaylist))
                continue;

           	// make sure the file exists before adding it
	        WIN32_FIND_DATA fd;
    	    HANDLE hFind = FindFirstFile(szPlaylist, &fd);
    	    if (INVALID_HANDLE_VALUE == hFind)
    	    {
    	    	FindClose(hFind);
    	        continue;
    	    }

    	    FindClose(hFind);

            size_t BufLen = _tcslen(szPlaylist) + 1;
            m_pszMRUPlaylist[iCur] = new TCHAR [BufLen];
            if (!m_pszMRUPlaylist[iCur])
            	continue;

            StringCchCopy(m_pszMRUPlaylist[iCur], BufLen, szPlaylist);
            iCur++;
        }

        if (REG_OPENED_EXISTING_KEY == dwDisp)
        {
            DWORD   dwType;
            TCHAR * pszLastPlaylist = new TCHAR[MAX_URL_LENGTH];
            TCHAR * pszShare        = new TCHAR[MAX_PATH];

            dwSize = MAX_PATH * sizeof (TCHAR);

            if (NULL != pszShare
                && ERROR_SUCCESS == RegQueryValueEx(hkResult,
                                                    TEXT("MediaShare"),
                                                    NULL, &dwType,
                                                    (BYTE*)pszShare,
                                                    &dwSize))
            {
                if (_tcslen(pszShare) > 0)
                {
                    size_t BufLen = _tcslen(pszShare) + 1;
                    if (m_pszMediaShare = new TCHAR [BufLen])
                    {
                        StringCchCopy(m_pszMediaShare, BufLen, pszShare);
                    }
                    else
                    {
                        m_pszMediaShare = pszShare;
                        pszShare = NULL;
                    }
                }
            }

            dwSize = MAX_URL_LENGTH * sizeof (TCHAR);

            //
            // WARNING:  SetCurrentPlaylist has the side effect of
            //           calling SaveRegState, so it must be the last
            //           regkey loaded.
            //
            if (NULL != pszLastPlaylist
                && ERROR_SUCCESS == RegQueryValueEx(hkResult,
                                                    TEXT("LastPlaylist"),
                                                    NULL, &dwType,
                                                    (BYTE*)pszLastPlaylist,
                                                    &dwSize))
            {
                if (_tcslen(pszLastPlaylist) > 0)
                {
                    // make sure the file exists before adding it
                    WIN32_FIND_DATA fd;
                    HANDLE hFind = FindFirstFile(pszLastPlaylist, &fd);
                    if (INVALID_HANDLE_VALUE == hFind)
                    {
                        SaveRegState();
                    }
                    else
                    {
                        SetCurrentPlaylist(pszLastPlaylist);
                        FindClose(hFind);
                    }
                }
            }

            delete [] pszShare;
            delete [] pszLastPlaylist;
        }

        RegCloseKey(hkResult);
    }

    delete [] szPlaylist;
    delete [] szRegKey;
}

void CPlaylistMgr::LoadLocalPlaylists()
{
    list_t * pTemp  = m_pPlaylists;
    int      iCount = 0;

    //
    // Clear out any previously loaded playlists
    //
    while (pTemp)
    {
        m_pPlaylists = pTemp->pNext;
        delete pTemp;

        if (m_pPlaylists) m_pPlaylists->pPrev = NULL;
        pTemp = m_pPlaylists;
    }

    // Search the device for playlists
    for (int i = 0; i < s_iIncludePathLength; i++)
    {
        iCount += FindPlaylists(s_pszIncludePath[i], iCount);
    }

    m_bLoadedLocalPlaylists = true;
}

int CPlaylistMgr::FindPlaylists(LPCTSTR pszPath, int iCount)
{
    WIN32_FIND_DATA ffd;
    HANDLE          hFind;
    TCHAR           szBasePath[MAX_PATH];
    TCHAR           szFindPath[MAX_PATH];
    BOOL            bFoundFile = FALSE;

    for (int i = 0; i < s_iExcludePathLength; i++)
    {
        if (0 == _tcscmp(s_pszExcludePath[i], pszPath))
        {
            return 0;
        }
    }

    if (TEXT('\\') == pszPath[0])
    {
        if (TEXT('\0') != pszPath[1])
        {
            if( FAILED(StringCchPrintf(szFindPath, MAX_PATH, TEXT("%s\\*"), pszPath)) )
            {
                return 0;
            }
        }
        else
        {
            if( FAILED(StringCchPrintf(szFindPath, MAX_PATH, TEXT("%s*"), pszPath)))
            {
                return 0;
            }
        }

        StringCchCopy(szBasePath, MAX_PATH, pszPath);
    }
    else
    {
        if( FAILED(StringCchPrintf(szBasePath, MAX_PATH, TEXT("\\%s"), pszPath)) )
        {
            return 0;
        }
        
        if( FAILED(StringCchPrintf(szFindPath, MAX_PATH, TEXT("\\%s\\*"), pszPath)) )
        {
            return 0;
        }
    }

    hFind = FindFirstFile(szFindPath, &ffd);

    if (INVALID_HANDLE_VALUE != hFind)
    {
        bFoundFile = TRUE;
    }

    while (bFoundFile)
    {
        if (TEXT('\0') != szBasePath[0]
            && TEXT('\\') == szBasePath[_tcslen(szBasePath) - 1])
        {
            if( FAILED(StringCchPrintf(szFindPath, MAX_PATH, TEXT("%s%s"), szBasePath, ffd.cFileName)) )
            {
                return 0;
            }
        }
        else
        {
            if( FAILED(StringCchPrintf(szFindPath, MAX_PATH, TEXT("%s\\%s"), szBasePath, ffd.cFileName)) )
            {
                return 0;
            }
        }

        if (FILE_ATTRIBUTE_DIRECTORY & ffd.dwFileAttributes)
        {
            iCount += FindPlaylists(szFindPath, iCount);
        }

        if (!(FILE_ATTRIBUTE_HIDDEN & ffd.dwFileAttributes))
        {
            int iLength = _tcslen(ffd.cFileName);

            if (iLength >= 5
                && TEXT('.') == ffd.cFileName[iLength - 4]
                && (TEXT('A') == ffd.cFileName[iLength - 3]
                    || TEXT('a') == ffd.cFileName[iLength - 3])
                && (TEXT('S') == ffd.cFileName[iLength - 2]
                    || TEXT('s') == ffd.cFileName[iLength - 2])
                && (TEXT('X') == ffd.cFileName[iLength - 1]
                    || TEXT('x') == ffd.cFileName[iLength - 1]))
            {
                if (AddPlaylist(szFindPath, iCount + 1))
                {
                    iCount++;
                }
            }
        }

        bFoundFile = FindNextFile(hFind, &ffd);
    }

    if (INVALID_HANDLE_VALUE != hFind)
    {
        FindClose(hFind);
    }

    return iCount;
}

void CPlaylistMgr::UpdateLocalContent()
{
    int     iCount = 0;
    int     i;
    HCURSOR hPrevCursor;

    // Make the cursor appear as an hour glass to let the user know something
    // is happening.
    hPrevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    // Clear the old Local Content playlist
    if (m_pLocalContent)
    {
        m_pLocalContent->DeleteAll();
    }
    else if (m_pLocalContent = new CPlaylist(TEXT("Local Content")))
    {
            m_pLocalContent->IsTransient(true);
    }
    else
    {
        return;
    }

    for (i = 0; i < s_iIncludePathLength; i++)
    {
        iCount += FindMediaFiles(s_pszIncludePath[i], m_pLocalContent, iCount);
    }

    //
    // Search the optional network path (if we have the network redirector
    // installed)
    //
    if (NULL != m_pszMediaShare
        && (WAIT_OBJECT_0 == WaitForAPIReady(SH_WNET, 0)))
    {
        iCount += FindMediaFiles(m_pszMediaShare, m_pLocalContent, iCount);
    }

    //
    // Add anything from "Favorites" to the bottom of the local content list
    //
    int iFavCount = 0;

    if (m_pFavorites) iFavCount = (int)m_pFavorites->GetCount();

    if (iFavCount > 0)
    {
        for (i = 0; i < iFavCount; i++)
        {
            CMediaClip * pClip = m_pFavorites->GetTrack(i);

            if (NULL != pClip
                && NULL != pClip->GetPath()
                && m_pLocalContent->InsertTrack(iCount + 1, pClip->GetPath()))
            {
                iCount++;
            }
        }
    }

    // Restore the old cursor
    SetCursor(hPrevCursor);
}

int CPlaylistMgr::FindMediaFiles(LPCTSTR pszPath, CPlaylist * pLocalContent, int iCount)
{
    WIN32_FIND_DATA ffd;
    HANDLE          hFind;
    TCHAR           szBasePath[MAX_PATH];
    TCHAR           szFindPath[MAX_PATH];
    BOOL            bFoundFile = FALSE;

    for (int i = 0; i < s_iExcludePathLength; i++)
    {
        if (0 == _tcscmp(s_pszExcludePath[i], pszPath))
        {
            return 0;
        }
    }

    if (TEXT('\\') == pszPath[0])
    {
        if (TEXT('\0') != pszPath[1])
        {
            if( FAILED(StringCchPrintf(szFindPath, MAX_PATH, TEXT("%s\\*"), pszPath)) )
            {
                return 0;
            }
        }
        else
        {
            if( FAILED(StringCchPrintf(szFindPath, MAX_PATH, TEXT("%s*"), pszPath)) )
            {
                return 0;
            }
        }

        StringCchCopy(szBasePath, MAX_PATH, pszPath);
        szBasePath[MAX_PATH-1] = TEXT('\0');
    }
    else
    {
        if( FAILED(StringCchPrintf(szBasePath, MAX_PATH, TEXT("\\%s"), pszPath)) )
        {
            return 0;
        }
        if( FAILED(StringCchPrintf(szFindPath, MAX_PATH, TEXT("\\%s\\*"), pszPath)) )
        {
            return 0;
        }
    }

    hFind = FindFirstFile(szFindPath, &ffd);

    if (INVALID_HANDLE_VALUE != hFind)
    {
        bFoundFile = TRUE;
    }

    while (bFoundFile)
    {
        if (TEXT('\0') != szBasePath[0]
            && TEXT('\\') == szBasePath[_tcslen(szBasePath) - 1])
        {
            if( FAILED(StringCchPrintf(szFindPath, MAX_PATH, TEXT("%s%s"), szBasePath, ffd.cFileName)) )
            {
                return 0;
            }
        }
        else
        {
            if( FAILED(StringCchPrintf(szFindPath, MAX_PATH, TEXT("%s\\%s"), szBasePath, ffd.cFileName)) )
            {
                return 0;
            }
        }

        if (FILE_ATTRIBUTE_DIRECTORY & ffd.dwFileAttributes)
        {
            iCount += FindMediaFiles(szFindPath, pLocalContent, iCount);
        }

        if (!(FILE_ATTRIBUTE_HIDDEN & ffd.dwFileAttributes)
            && IsMediaFile(ffd.cFileName))
        {
            if (pLocalContent->InsertTrack(iCount + 1, szFindPath))
            {
                iCount++;
            }
        }

        bFoundFile = FindNextFile(hFind, &ffd);
    }

    if (INVALID_HANDLE_VALUE != hFind)
    {
        FindClose(hFind);
    }

    return iCount;
}

bool CPlaylistMgr::IsMediaFile(LPCTSTR pszFile)
{
    DWORD   i;
    const TCHAR * pszExt  = _tcsrchr(pszFile, TEXT('.'));

    if (NULL == pszExt)
    {
        return false;
    }

    for (i = 0; i < g_cAudioTypes; i++)
    {
        if (0 == _tcsicmp(pszExt, g_ppszAudioTypes[i]))
        {
            return true;
        }
    }

    for (i = 0; i < g_cVideoTypes; i++)
    {
        if (0 == _tcsicmp(pszExt, g_ppszVideoTypes[i]))
        {
            return true;
        }
    }

    return false;
}
