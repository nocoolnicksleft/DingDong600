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
// plist.cpp
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <uuids.h>
#include <playlist.h>

#include "plist.h"

const char c_szASXHeaderFormat[] = "<ASX version = \"3.0\">\n"
                                   "    <PARAM name = \"Last Entry\" value = \"%d\"/>\n"
                                   "    <PARAM name = \"Generator\" value = \"CEPlayer\"/>\n";
const char c_szASXEntryFormat[]  = "    <ENTRY>\n"
                                   "        <REF href=\"%s\"/>\n"
                                   "    </ENTRY>\n";
const char c_szASXFooterFormat[] = "</ASX>\n";

CMediaClip::CMediaClip() : m_pszTitle(NULL), m_pszPath(NULL), m_cRef(0), m_eLocation(LOCAL)
{
}

CMediaClip::~CMediaClip()
{
    delete [] m_pszTitle;
    delete [] m_pszPath;
}

bool CMediaClip::SetLocation(LPCTSTR pszPath)
{
    if (NULL != m_pszPath)
    {
        delete [] m_pszPath;
    }

    // strip out a "file://" if it's there
    const TCHAR * pszTemp = _tcsstr(pszPath, TEXT("://"));

    if (pszTemp
        && _tcslen(pszPath) > 7
        && TEXT('F') == toupper(pszPath[0])
        && TEXT('I') == toupper(pszPath[1])
        && TEXT('L') == toupper(pszPath[2])
        && TEXT('E') == toupper(pszPath[3]))
    {
        pszPath = pszTemp + _tcslen(TEXT("://"));
    }

    size_t BufLen = _tcslen(pszPath) + 1;
    m_pszPath = new TCHAR [BufLen];

    if (m_pszPath && _tcslen(pszPath) > 1)
    {
        StringCchCopy(m_pszPath, BufLen, pszPath);
    }
    else if (m_pszPath)
    {
        delete [] m_pszPath;
        m_pszPath = NULL;

        return false;
    }
    else
    {
        return false;
    }

    // if path is of the form "\\blah..." or "blah://" then call it
    // a network file.
    if ((TEXT('\\') == m_pszPath[0] && TEXT('\\') == m_pszPath[1])
        || NULL != _tcsstr(m_pszPath, TEXT("://")))
    {
        m_eLocation = NETWORK;
    }
    else
    {
        LPTSTR pszSlash;
        DWORD dwAttrib;

        m_eLocation = LOCAL;

        pszSlash = _tcschr(m_pszPath + 1, TEXT('\\'));

        if (pszSlash)
        {
            *pszSlash = TEXT('\0');

            dwAttrib = GetFileAttributes(m_pszPath);

            if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY 
                && dwAttrib & FILE_ATTRIBUTE_TEMPORARY)
            {
                m_eLocation = STORAGECARD;
            }

            *pszSlash = TEXT('\\');
        }

        dwAttrib = GetFileAttributes(m_pszPath);

        if (STORAGECARD != m_eLocation
            && 0xffffffff == dwAttrib)
        {
            delete [] m_pszPath;
            m_pszPath = NULL;

            return false;
        }
    }

    BufLen = _tcslen(TEXT("Unknown")) + 1;
    m_pszArtist = new TCHAR [BufLen];
    StringCchCopy(m_pszArtist, BufLen, TEXT("Unknown"));
    if (NETWORK != m_eLocation)
    {
        LPCTSTR pszSlash;

        pszSlash = _tcsrchr(m_pszPath, TEXT('\\'));

        if (NULL == pszSlash)
        {
            pszSlash = m_pszPath;
        }
        else
        {
            pszSlash++;
        }

        size_t BufLen = _tcslen(pszSlash) + 1;
        m_pszTitle = new TCHAR[BufLen];
        StringCchCopy(m_pszTitle, BufLen, pszSlash);
    }
    else
    {
        LPCTSTR pszSlash;

        // if the title begins with "\" it's a network share, otherwise,
        // assume it's a URL
        if (TEXT('\\') == m_pszPath[0])
        {
            pszSlash = _tcsrchr(m_pszPath, TEXT('\\'));
        }
        else
        {
            pszSlash = _tcsrchr(m_pszPath, TEXT('/'));
        }

        if (NULL == pszSlash)
        {
            pszSlash = m_pszPath;
        }
        else
        {
            pszSlash++;
        }

        size_t BufLen = _tcslen(pszSlash) + 1;
        m_pszTitle = new TCHAR[BufLen];
        StringCchCopy(m_pszTitle, BufLen, pszSlash);
    }

    return true;
}

LPCTSTR CMediaClip::GetTitle() const
{
    return m_pszTitle;
}

LPCTSTR CMediaClip::GetArtist() const
{
    return m_pszArtist;
}

LPCTSTR CMediaClip::GetPath() const
{
    return m_pszPath;
}

EFileLocation CMediaClip::GetLocation() const
{
    return m_eLocation;
}

bool CMediaClip::IsLocal() const
{
    return (m_eLocation == LOCAL || m_eLocation == STORAGECARD);
}

bool CMediaClip::IsAvailable() const
{
    if (NETWORK != m_eLocation)
    {
        DWORD dwAttrib;

        dwAttrib = GetFileAttributes(m_pszPath);

        return (0xffffffff != dwAttrib ? true : false);
    }
    else
    {
        return true;
    }
}

CPlaylist::CPlaylist(LPCTSTR pszPath) :
    m_pszPath(NULL),
    m_pszName(NULL),
    m_pList(NULL),
    m_iCurrentTrack(-1),
    m_uCount(0),
    m_bCEPlaylist(false),
    m_bHidden(false),
    m_bTransient(false)
{
    LPCTSTR pszName = NULL,
            pszExt  = NULL;

    if (pszPath)
    {
        size_t BufLen = _tcslen(pszPath) + 1;
        m_pszPath = new TCHAR [BufLen];

        if (m_pszPath)
        {
            StringCchCopy(m_pszPath, BufLen, pszPath); 

            GetNameFromPath(m_pszPath, BufLen, &m_pszName);
        }
    }
}

CPlaylist::~CPlaylist()
{
    playlist_t * pTemp = m_pList;

    while (NULL != m_pList)
    {
        m_pList = m_pList->pNext;

        delete pTemp;

        pTemp = m_pList;
    }

    delete [] m_pszPath;
}

bool CPlaylist::GetNameFromPath(LPTSTR szPath, int nSizePath, LPTSTR* pszName)
{
	LPCTSTR szName = NULL;
	TCHAR * szExt  = NULL;
	LONG  length=0;

	ASSERT(szPath && !*pszName);

	szName = _tcsrchr(szPath, TEXT('\\'));
	szExt  = _tcsrchr(szPath, TEXT('.'));

	if (!szName)
	{
		szName = szPath;
	}
	else
	{
		szName++;
	}

	if (szExt)
		*szExt = TEXT('\0');
	
	if( szName && szName[0] )
	{
		length = _tcslen(szName);
	}

	*pszName = new TCHAR[length + 1];
	if (*pszName)
	    StringCchCopy(*pszName, length + 1, szName);

	if (szExt)
		*szExt = TEXT('.');

	return (*pszName ? true : false);
}

bool CPlaylist::InsertTrack(UINT uPos, LPCTSTR pszLocation)
{
    // uPos is 0 based
    // If uPos is larger than the length of the playlist,
    // append it to the end of the list.

    playlist_t * pTemp = m_pList;

    for (UINT i = 0; i < uPos; i++)
    {
        if (NULL != pTemp
            && NULL != pTemp->pNext)
        {
            pTemp = pTemp->pNext;
        }
        else
        {
            break;
        }
    }

    // insert after the pTemp pointer
    if (NULL != pTemp)
    {
        playlist_t * pNew = new playlist_t;

        if (NULL == pNew)
        {
            return false;
        }

        pNew->pClip = new CMediaClip;

        if (NULL == pNew->pClip)
        {
            delete pNew;
            return false;
        }

        if (false == pNew->pClip->SetLocation(pszLocation))
        {
            delete pNew;
            return false;
        }

        pNew->pNext = pTemp->pNext;
        pNew->pPrev = pTemp;

        pTemp->pNext = pNew;
    }
    else
    {
        // insert at the front of the list
        pTemp = new playlist_t;

        if (NULL == pTemp)
        {
            return false;
        }

        pTemp->pClip = new CMediaClip;

        if (NULL == pTemp->pClip)
        {
            delete pTemp;
            return false;
        }

        if (false == pTemp->pClip->SetLocation(pszLocation))
        {
            delete pTemp;
            return false;
        }

        pTemp->pNext = m_pList;
        m_pList = pTemp;
    }

    m_uCount++;

    // adjust current index pointer
    if (uPos > m_uCount - 1)
    	uPos = m_uCount - 1;

    if (0 <= m_iCurrentTrack 
        && (UINT)m_iCurrentTrack >= uPos)
    {
    	if ((UINT)m_iCurrentTrack + 1 <= m_uCount - 1)
            m_iCurrentTrack++;
    }

    return true;
}

bool CPlaylist::DeleteTrack(UINT uPos)
{
    // uPos is 0 based
    playlist_t * pTemp, * pPrev, * pNext;
    bool bResult = false;

    pPrev = NULL;
    pTemp = m_pList;

    for (UINT i = 0; i < uPos; i++)
    {
        if (pTemp)
        {
            pPrev = pTemp;
            pTemp = pTemp->pNext;
        }
    }

    if (pTemp)
    {
        pNext = pTemp->pNext;

        if (pNext)
        {
            pNext->pPrev = pPrev;
        }

        if (pPrev)
        {
           pPrev->pNext = pNext;
        }
        else
        {
            m_pList = pNext;
        }

        delete pTemp;
		m_uCount--;

        bResult = true;
    }

    // adjust current index pointer
    if (0 <= m_iCurrentTrack 
    	&& (UINT)m_iCurrentTrack >= uPos)
    {
    	if ((UINT)m_iCurrentTrack > m_uCount - 1)
    		m_iCurrentTrack = m_uCount - 1;
    	else if ((UINT)m_iCurrentTrack != uPos)
    		m_iCurrentTrack--;
    }

    return bResult;
}

void CPlaylist::DeleteAll()
{
    playlist_t * pTemp = m_pList;

    while (NULL != m_pList)
    {
        m_pList = m_pList->pNext;

        delete pTemp;

        pTemp = m_pList;
    }
	m_uCount = 0;
}

bool CPlaylist::ShiftTrackUp(UINT uIndex)
{
    UINT         uPos    = 0;
    playlist_t * pCurr   = m_pList;

    if (0 == uIndex)
    {
        return false;
    }

    while (uPos < (uIndex - 1)
           && NULL != pCurr)
    {
        uPos++;

        pCurr = pCurr->pNext;
    }

    if (NULL == pCurr || NULL == pCurr->pNext || uPos < (uIndex - 1))
    {
        return false;
    }

    SwapItems(pCurr);

    if ((UINT)m_iCurrentTrack + 1 == uIndex)
        m_iCurrentTrack++;
    else if ((UINT)m_iCurrentTrack == uIndex)
    	m_iCurrentTrack--;

    return true;
}

bool CPlaylist::ShiftTrackDown(UINT uIndex)
{
    UINT         uPos    = 0;
    playlist_t * pCurr   = m_pList;

    while (uPos < uIndex
           && NULL != pCurr)
    {
        uPos++;

        pCurr = pCurr->pNext;
    }

    if (NULL == pCurr || NULL == pCurr->pNext || uPos < uIndex)
    {
        return false;
    }

    SwapItems(pCurr);

    if ((UINT)m_iCurrentTrack - 1 == uIndex)
        m_iCurrentTrack--;
    else if ((UINT)m_iCurrentTrack == uIndex)
    	m_iCurrentTrack++;

    return true;
}

CMediaClip * CPlaylist::GetTrack(int iIndex)
{
    playlist_t * pTemp = GetListItem(iIndex);
    //ASSERT(-1 == iIndex || pTemp);
    return (pTemp ? pTemp->pClip : NULL);
}

CMediaClip * CPlaylist::PlayCurrentTrack()
{
    playlist_t * pTemp = GetListItem(m_iCurrentTrack);
	//ASSERT(pTemp);

    if (pTemp)
    {
        pTemp->bPlayed = true;
        pTemp->bIsPlaying = true;
        return pTemp->pClip;
    }

    return NULL;
}

void CPlaylist::CurrentTrackDonePlaying()
{
	playlist_t * pTemp = GetListItem(m_iCurrentTrack);

#ifdef DEBUG
	// If none of the clips in the playlist are
	// playing then this may just be a brand new
	// playlist.  But if a clip we didn't expect
	// is playing, then we need investigate whether
	// we're updating this flag in all the right
	// places!!!
	if (pTemp && !pTemp->bIsPlaying)
	{
		playlist_t * pDbg = m_pList;
		while (pDbg)
		{
			ASSERT(!pDbg->bIsPlaying);
			pDbg = pDbg->pNext;
		}
	}
#endif

	if (pTemp)
	{
		pTemp->bIsPlaying = false;
	}
}

UINT CPlaylist::GetCount()
{
	return m_uCount;
}

void CPlaylist::ResetPlayed()
{
    playlist_t * pTemp = m_pList;

    while (pTemp)
    {
        pTemp->bPlayed = false;

        pTemp = pTemp->pNext;
    }
}

bool CPlaylist::IsPlaying(int iIndex)
{
	playlist_t * pTemp = GetListItem(iIndex);
	ASSERT(pTemp);
	return (pTemp ? pTemp->bIsPlaying : false);
}

int CPlaylist::GetCurrent()
{
    return m_iCurrentTrack;
}

void CPlaylist::SetCurrent(int iIndex)
{
    if (-1 <= iIndex && iIndex <= (int)GetCount())
    {
        m_iCurrentTrack = iIndex;
    }
    else
    {
    	ASSERT(FALSE);
    }
}

bool CPlaylist::NextTrack(bool bRandom, bool bRepeat)
{
     UINT uCount    = GetCount();
     int  iIndex    = m_iCurrentTrack;
     int  iOldIndex = m_iCurrentTrack;
     int  iNumTried = 0;

     playlist_t * pTemp = m_pList;
     CMediaClip * pClip = NULL;

     bool bResult = false;

     if (0 == uCount)
     {
         return false;
     }

     //
     // If all the tracks have been played, reset their "played" flag
     //

     if (bRepeat)
     {
         bool bReset = true;

         while (NULL != pTemp)
         {
             if (false == pTemp->bPlayed) bReset = false;

             pTemp = pTemp->pNext;
         }

         if (bReset)
         {
             ResetPlayed();
         }
     }

     //
     // Get the "next" track
     //

     if (bRandom)
     {

         do
         {
             do
             {
                 iIndex = rand() % uCount;
             } while (iIndex == iOldIndex && uCount > 1);

             iNumTried++;

             if(( iNumTried < (int)uCount) || ( bRepeat ))
             {
                 pTemp = GetListItem(iIndex);

                 if (NULL != pTemp)
                     pClip = pTemp->pClip;
                 else
                     pClip = NULL;
             }
             else
             {
                 pClip = NULL;
                 pTemp = NULL;
             }
         } while (pClip && (true == pTemp->bPlayed
                            || !pClip->IsAvailable()));
     }
     else
     {
         do
         {
             iIndex++;

             if (iIndex >= (int)uCount)
             {
                 iIndex = 0;

                 if (false == bRepeat)
                 {
                     pClip = NULL;
                     break;
                 }
             }

             if (iIndex != iOldIndex)
             {
                 pClip = GetTrack(iIndex);
             }
             else
             {
                 pClip = NULL;
             }
         } while (pClip && !pClip->IsAvailable());
    }

    SetCurrent(iIndex);

    Save();

    if (pClip)
    {
        if (bRandom && pTemp && false == pTemp->bPlayed)
            bResult = true;
        else if (!bRandom)
            bResult = true;
    }

    return bResult;
}

bool CPlaylist::PrevTrack(bool bRandom, bool bRepeat)
{
     UINT uCount    = GetCount();
     int  iIndex    = m_iCurrentTrack;
     int  iOldIndex = m_iCurrentTrack;
     int  iNumTried = 0;

     playlist_t * pTemp = m_pList;
     CMediaClip * pClip = NULL;

     bool bPlayed = false;
     bool bResult = false;

     if (0 == uCount)
     {
         return false;
     }

     while (NULL != pTemp)
     {
         if (pTemp->bPlayed) bPlayed = true;

         pTemp = pTemp->pNext;
     }

     //
     // Get the "previous" track
     //

     if (bRandom)
     {
         do
         {
             do
             {
                 iIndex = rand() % uCount;
             } while (iIndex == iOldIndex && uCount > 1);

             iNumTried++;

             if (iNumTried < (int)uCount)
             {
                 pTemp = GetListItem(iIndex);

                 if (NULL != pTemp)
                     pClip = pTemp->pClip;
                 else
                     pClip = NULL;
             }
             else
             {
                 pClip = NULL;
             }
         } while (pClip && ((false == pTemp->bPlayed && bPlayed)
                            || !pClip->IsAvailable()));
     }
     else
     {
         do
         {
             iIndex--;

             if (iIndex < 0)
             {
                 iIndex = (int)uCount - 1;
             }

             if (iIndex != iOldIndex)
             {
                 pClip = GetTrack(iIndex);
             }
             else
             {
                 pClip = NULL;
             }
         } while (pClip && !pClip->IsAvailable());
    }

    SetCurrent(iIndex);

    Save();

    if (pClip)
    {
        bResult = true;
    }

    return bResult;
}

LPCTSTR CPlaylist::GetName()
{
    return m_pszName;
}

LPCTSTR CPlaylist::GetPath()
{
    return m_pszPath;
}

bool CPlaylist::Save()
{
    DWORD        dwToWrite, dwWritten;
    DWORD        dwASXDataLen = 0;
    DWORD        dwFileAttrib;
    char       * pszASXData   = NULL;
    playlist_t * pTrack;
    char         szFilename[MAX_PATH];
    HANDLE       hFile;
    BOOL         bWrite;
    bool         bResult      = true;

    // Don't save a playlist that wasn't created by the player
    if (false == m_bCEPlaylist || true == m_bTransient)
    {
        return true;
    }

    if (m_bHidden)
    {
        dwFileAttrib = FILE_ATTRIBUTE_HIDDEN;
    }
    else
    {
        dwFileAttrib = FILE_ATTRIBUTE_NORMAL;
    }

    hFile = CreateFile(m_pszPath, GENERIC_WRITE, 0, NULL,
                       CREATE_ALWAYS, dwFileAttrib, NULL);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        bWrite = FALSE;

        // write out the header
        dwASXDataLen = sizeof (c_szASXHeaderFormat) + 10;

        if (pszASXData = new char[dwASXDataLen])
        {
            pszASXData[0] = '\0';

            if(SUCCEEDED(StringCbPrintfA(pszASXData, dwASXDataLen, c_szASXHeaderFormat, m_iCurrentTrack)))
            {                
                if(SUCCEEDED(StringCbLengthA(pszASXData, dwASXDataLen, (size_t*)&dwToWrite)))
                {
                    dwWritten = 0;
                    bWrite = WriteFile(hFile, pszASXData, dwToWrite, &dwWritten, NULL);
                }
                else
                {
                    bWrite = FALSE;
                }
            }
            else
            {
                bWrite = FALSE;
            }
        }

        if (FALSE == bWrite || dwToWrite != dwWritten)
        {
            bResult = false;
        }
    }
    else
    {
        bResult = false;
    }

    pTrack = m_pList;

    while (NULL != pTrack && bResult)
    {
        LPCTSTR pszPath = pTrack->pClip->GetPath();

        szFilename[0] = '\0';

        if (NETWORK != pTrack->pClip->GetLocation())
        {
#ifdef _UNICODE
            _snprintf(szFilename, MAX_PATH-1, "file://%ls", pszPath);
#else
            _snprintf(szFilename, MAX_PATH-1, "file://%s", pszPath);
#endif /* _UNICODE */

            szFilename[MAX_PATH-1] = '\0';
        }
        else
        {
#ifdef _UNICODE
            WideCharToMultiByte(CP_ACP, 0, pszPath, -1, szFilename, MAX_PATH, NULL, NULL);
#else
            strncpy(szFilename, pszPath, MAX_PATH);
            pszPath[MAX_PATH-1] = '\0';
#endif /* _UNICODE */
        }

        dwToWrite = sizeof (c_szASXEntryFormat) + strlen(szFilename);

        if (dwASXDataLen < dwToWrite)
        {
            delete [] pszASXData;

            pszASXData  = new char [dwToWrite];
            dwASXDataLen = dwToWrite;
        }
        if((NULL != pszASXData) && (SUCCEEDED(StringCbPrintfA(pszASXData, dwToWrite, c_szASXEntryFormat, szFilename))))
        {            
            if(SUCCEEDED(StringCbLengthA(pszASXData, dwASXDataLen, (size_t*)&dwToWrite)))
            {
                dwWritten = 0;
                bWrite = WriteFile(hFile, pszASXData, dwToWrite, &dwWritten, NULL);
            }
            else
            {
                bWrite = FALSE;
            }
        }
        else
        {
            bWrite = FALSE;
        }

        if (FALSE == bWrite || dwToWrite != dwWritten)
        {
            bResult = false;
        }

        pTrack = pTrack->pNext;
    }


    if (INVALID_HANDLE_VALUE != hFile && NULL != pszASXData)
    {
        // write out the footer
        size_t ASXFooterFormatLen = sizeof (c_szASXFooterFormat);
        if (dwASXDataLen < ASXFooterFormatLen)
        {
            delete [] pszASXData;

            pszASXData = new char [ASXFooterFormatLen];
        }
        if((NULL != pszASXData) && (SUCCEEDED(StringCbCopyA(pszASXData, ASXFooterFormatLen, c_szASXFooterFormat))))
        {            
            if(SUCCEEDED(StringCbLengthA(pszASXData, ASXFooterFormatLen, (size_t*)&dwToWrite)))
            {
                dwWritten = 0;
                bWrite = WriteFile(hFile, pszASXData, dwToWrite, &dwWritten, NULL);
            }
            else
            {
                bWrite = FALSE;
            }
        }
        else
        {
            bWrite = FALSE;
        }

        if (FALSE == bWrite || dwToWrite != dwWritten)
        {
            bResult = false;
        }

        CloseHandle(hFile);
    }
    else if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }

    delete [] pszASXData;

    return bResult;
}

bool CPlaylist::Load()
{
    IGraphBuilder * pGraph       = NULL;
    IAMPlayList   * pPlaylist    = NULL;
    HRESULT         hr;
    bool            bResult;

    if (NULL != m_pList || true == m_bTransient)
    {
        return true;
    }

    //
    // Make sure that this is one of our playlist read the last played element
    //
    bResult = LoadParam();

    hr = CoCreateInstance(CLSID_FilterGraph,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder,
                          (void**) &pGraph);

    if (SUCCEEDED(hr))
    {
        hr = pGraph->RenderFile(m_pszPath, NULL);
    }

    if (SUCCEEDED(hr))
    {
        IEnumFilters * pEnum   = NULL;
        IBaseFilter  * pFilter = NULL;

        hr = pGraph->EnumFilters(&pEnum);

        if (pEnum)
        {
            while (!pPlaylist && pEnum->Next(1, &pFilter, NULL) == S_OK)
            {
                hr = pFilter->QueryInterface(IID_IAMPlayList, (void**)&pPlaylist);
                pFilter->Release();
            }

            if (!pPlaylist)
            {
                hr = E_NOINTERFACE;
            }

            pEnum->Release();
        }
    }

    if (SUCCEEDED(hr))
    {
        DWORD             dwCount;
        IAMPlayListItem * pItem   = NULL;

        if(pPlaylist)
            hr = pPlaylist->GetItemCount(&dwCount);
        else
            hr = E_FAIL;

        if (SUCCEEDED(hr))
        {
            for (DWORD i = 0; i < dwCount; i++)
            {
                hr = pPlaylist->GetItem(i, &pItem);

                if (SUCCEEDED(hr))
                {
                    BSTR pszSource = NULL;

                    hr = pItem->GetSourceURL(0, &pszSource);

                    if (SUCCEEDED(hr))
                    {
                        InsertTrack(i, pszSource);
                    }

                    pItem->Release();
                }
            }
        }
    }

    if (pPlaylist)
    {
        pPlaylist->Release();
    }

    if (pGraph)
    {
        pGraph->Release();
    }

    if (SUCCEEDED(hr))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CPlaylist::Rename(LPCTSTR pszName)
{
    TCHAR   szOldPath[MAX_PATH];
    TCHAR   szNewPath[MAX_PATH];
    TCHAR * pszExt  = NULL;
    bool    bResult = false;

    StringCchCopy(szOldPath, MAX_PATH, m_pszPath);
	szOldPath[ MAX_PATH -1 ] = TEXT('\0' );

    pszExt = _tcsrchr(m_pszPath, TEXT('.'));


    if (NULL == pszExt)
    {
        pszExt = m_pszPath + _tcslen(m_pszPath);
    }

    pszExt -= _tcslen(m_pszName);

    *pszExt = TEXT('\0');

    StringCchPrintf( szNewPath, sizeof( szNewPath ) / sizeof( szNewPath[0] ), L"%s%s.asx", m_pszPath, pszName);

    if (MoveFile(szOldPath, szNewPath))
    {
        TCHAR * pszOldName = m_pszName;
        TCHAR * pszOldPath = m_pszPath;

        size_t NewPathLen = _tcslen(szNewPath) + 1;
        size_t NameLen = _tcslen(pszName) + 1;
        m_pszPath = new TCHAR [NewPathLen];
        m_pszName = new TCHAR [NameLen];

        if (NULL != m_pszPath && NULL != m_pszName)
        {
            StringCchCopy(m_pszPath, NewPathLen, szNewPath);
            StringCchCopy(m_pszName, NameLen, pszName);

            bResult = true;

            delete [] pszOldName;
            delete [] pszOldPath;
        }
        else
        {
            m_pszName = pszOldName;
            m_pszPath = pszOldPath;
        }
    }
    else
    {
        StringCchCopy(m_pszPath, MAX_PATH, szOldPath);
    }

    return bResult;
}

bool CPlaylist::IsCEPlaylist()
{
    return m_bCEPlaylist;
}

void CPlaylist::IsCEPlaylist(bool b)
{
    m_bCEPlaylist = b;
}

bool CPlaylist::IsHidden()
{
    return m_bHidden;
}

void CPlaylist::IsHidden(bool b)
{
    m_bHidden = b;
}

bool CPlaylist::IsTransient()
{
    return m_bTransient;
}

void CPlaylist::IsTransient(bool b)
{
    m_bTransient = b;
}

CPlaylist::playlist_t * CPlaylist::GetFirst()
{
    return m_pList;
}

// case insensitive substring (with bounds checking)
const char * strnistr(const char * pszSource, DWORD dwLength, const char * pszFind)
{
    DWORD        dwIndex   = 0;
    DWORD        dwStrLen  = 0;
    const char * pszSubStr = NULL;

    // check for valid arguments
    if (!pszSource || !pszFind)
    {
        return pszSubStr;
    }

    dwStrLen = strlen(pszFind);

    // can pszSource possibly contain pszFind?
    if (dwStrLen > dwLength)
    {
        return pszSubStr;
    }

    while (dwIndex <= dwLength - dwStrLen)
    {
        if (0 == _strnicmp(pszSource + dwIndex, pszFind, dwStrLen))
        {
            pszSubStr = pszSource + dwIndex;
            break;
        }

        dwIndex ++;
    }

    return pszSubStr;
}

bool CPlaylist::LoadParam()
{
    DWORD    dwIndex;
    DWORD    dwFileSize;
    DWORD    dwFileAttrib;
    const char * pszASXFile;
    const char * pszParam;
    HANDLE   hFile;
    HANDLE   hMap;
    bool     bLastEntry = false;
    bool     bGenerator = false;
    enum  {E_INVALID = 0, E_NOERROR, E_EOF, E_BADMATCH} eError = E_NOERROR;

    if (m_bHidden)
    {
        dwFileAttrib = FILE_ATTRIBUTE_HIDDEN;
    }
    else
    {
        dwFileAttrib = FILE_ATTRIBUTE_NORMAL;
    }

    hFile = CreateFileForMapping(m_pszPath, GENERIC_READ, FILE_SHARE_READ,
                                 NULL, OPEN_EXISTING, dwFileAttrib, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        return false;
    }

    hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

    if (NULL == hMap)
    {
        CloseHandle(hFile);
        return false;
    }

    dwFileSize = GetFileSize(hFile, NULL);

    pszASXFile = (char*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);

    if (NULL == pszASXFile)
    {
        CloseHandle(hMap);
        CloseHandle(hFile);
        return false;
    }

    // find the occurance of "LAST ENTRY" (case insensitive)
    pszParam = (char*)strnistr(pszASXFile, dwFileSize, "LAST ENTRY");
    if((((DWORD)pszParam)-(DWORD)pszASXFile) > (DWORD)pszASXFile)
    {
        CloseHandle(hMap);
        CloseHandle(hFile);
        return false;
    }

    dwIndex = (DWORD)pszParam - (DWORD)pszASXFile;

    if (pszParam) 
    {
        dwIndex += strlen("LAST ENTRY");

        // skip the trailing quote
        if (dwIndex < dwFileSize)
        {
            if ('\"' == pszASXFile[dwIndex])
            {
                dwIndex++;
            }
        }
        else if (E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // skip spaces
        while (E_NOERROR == eError
               && dwIndex < dwFileSize
               && isspace(pszASXFile[dwIndex]))
        {
            dwIndex++;
        }

        if (dwIndex >= dwFileSize && E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // match "VALUE"
        if (E_NOERROR == eError
            && dwIndex < dwFileSize
            && 0 == _strnicmp(pszASXFile + dwIndex, "VALUE", 5))
        {
            dwIndex += strlen("VALUE");
        }
        else if (E_NOERROR == eError && dwIndex > dwFileSize)
        {
            eError = E_EOF;
        }

        // skip spaces
        while (E_NOERROR == eError
               && dwIndex < dwFileSize
               && isspace(pszASXFile[dwIndex]))
        {
            dwIndex++;
        }

        if (dwIndex >= dwFileSize && E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // match "="
        if (dwIndex < dwFileSize && E_NOERROR == eError)
        {
            if ('=' == pszASXFile[dwIndex])
            {
                dwIndex++;
            }
            else
            {
                eError = E_BADMATCH;
            }
        }
        else if (E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // skip spaces
        while (E_NOERROR == eError
               && dwIndex < dwFileSize
               && isspace(pszASXFile[dwIndex]))
        {
            dwIndex++;
        }

        if (dwIndex >= dwFileSize && E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // match quote
        if (dwIndex < dwFileSize && E_NOERROR == eError)
        {
            if ('\"' == pszASXFile[dwIndex])
            {
                dwIndex++;
            }
        }
        else if (E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // get the number
        if (E_NOERROR == eError
            && dwIndex < dwFileSize
            && 1 == sscanf(pszASXFile + dwIndex, "%d", &m_iCurrentTrack))
        {
            bLastEntry = true;
        }
        else if (E_NOERROR == eError && dwIndex >= dwFileSize)
        {
            eError = E_BADMATCH;
            SetCurrent(-1);
        }
    }

    // find the occurance of "GENERATOR" (case insensitive)
    pszParam = (char*)strnistr(pszASXFile, dwFileSize, "GENERATOR");
    dwIndex = (DWORD)pszParam - (DWORD)pszASXFile;

    if (pszParam) 
    {
        dwIndex += strlen("GENERATOR");

        // skip the trailing quote
        if (dwIndex < dwFileSize)
        {
            if ('\"' == pszASXFile[dwIndex])
            {
                dwIndex++;
            }
        }
        else if (E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // skip spaces
        while (E_NOERROR == eError
               && dwIndex < dwFileSize
               && isspace(pszASXFile[dwIndex]))
        {
            dwIndex++;
        }

        if (dwIndex >= dwFileSize && E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // match "VALUE"
        if (E_NOERROR == eError
            && dwIndex < dwFileSize
            && 0 == _strnicmp(pszASXFile + dwIndex, "VALUE", 5))
        {
            dwIndex += strlen("VALUE");
        }
        else if (E_NOERROR == eError && dwIndex > dwFileSize)
        {
            eError = E_EOF;
        }

        // skip spaces
        while (E_NOERROR == eError
               && dwIndex < dwFileSize
               && isspace(pszASXFile[dwIndex]))
        {
            dwIndex++;
        }

        if (dwIndex >= dwFileSize && E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // match "="
        if (dwIndex < dwFileSize && E_NOERROR == eError)
        {
            if ('=' == pszASXFile[dwIndex])
            {
                dwIndex++;
            }
            else
            {
                eError = E_BADMATCH;
            }
        }
        else if (E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // skip spaces
        while (E_NOERROR == eError
               && dwIndex < dwFileSize
               && isspace(pszASXFile[dwIndex]))
        {
            dwIndex++;
        }

        if (dwIndex >= dwFileSize && E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // match quote
        if (dwIndex < dwFileSize && E_NOERROR == eError)
        {
            if ('\"' == pszASXFile[dwIndex])
            {
                dwIndex++;
            }
        }
        else if (E_NOERROR == eError)
        {
            eError = E_EOF;
        }

        // examine the value 
        if (E_NOERROR == eError
            && dwIndex < dwFileSize
            && 0 == _strnicmp(pszASXFile + dwIndex, "CEPLAYER", 8))
        {
            bGenerator    = true;
            m_bCEPlaylist = true;
        }
        else if (E_NOERROR == eError && dwIndex >= dwFileSize)
        {
            eError = E_BADMATCH;
        }
    }

    UnmapViewOfFile(pszASXFile);
    CloseHandle(hMap);
    CloseHandle(hFile);

    return bLastEntry && bGenerator;
}

void CPlaylist::SwapItems(playlist_t * pItem)
{
    playlist_t * pTemp = pItem->pNext;
    playlist_t * pNext = pTemp->pNext;
    playlist_t * pPrev = pItem->pPrev;

    pItem->pNext = pNext;
    pItem->pPrev = pTemp;

    pTemp->pNext = pItem;
    pTemp->pPrev = pPrev;

    if (pPrev)
    {
        pPrev->pNext = pTemp;
    }
    else
    {
        m_pList = pTemp;
    }

    if (pNext)
    {
        pNext->pPrev = pItem;
    }
}

CPlaylist::playlist_t * CPlaylist::GetListItem(int iIndex)
{
	if (iIndex < 0) return NULL;

    playlist_t * pTemp = m_pList;
    int          iPos  = 0;

    while (NULL != pTemp)
    {
        if (iPos == iIndex) break;

        pTemp = pTemp->pNext;
        iPos++;
    }

    return pTemp;
}
