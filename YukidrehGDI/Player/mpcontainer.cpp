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
// File: MPContainer.cpp
//
// Desc: This file contains the implementation for the MPContainer class,
//       which is used to host the Windows Media Player ActiveX control.
//
///////////////////////////////////////////////////////////////////////////////

#include "MPContainer.h"
#include "PlayerWindow.h"

#include <olectl.h>
#include <msdxm.h>
#include <dsdispid.h>
#define _COMCTL32_
#include <commctrl.h>
#undef _COMCTL32_

#include "resource.h"

//////
// Constructor/Destructor
CMPContainer::CMPContainer() : m_ulRef(0),
m_hWnd(NULL),
m_pUnk(NULL),
m_pIPOW(NULL),
m_pIPO(NULL),
m_pMP(NULL),
#ifdef CEPLAYER_SKIN
m_dwLevelOfControls(10) // Skin
#else /* !CEPLAYER_SKIN */
m_dwLevelOfControls(2) // Standard level
#endif /* CEPLAYER_SKIN */
{
}

CMPContainer::~CMPContainer()
{
    Fini();
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Init()
// Desc: This function does no work.
///////////////////////////////////////////////////////////////////////////////
bool CMPContainer::Init()
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Fini()
// Desc: This function releases all interfaces held by the container.
///////////////////////////////////////////////////////////////////////////////
bool CMPContainer::Fini()
{
    return true;
}

//////
// IUnknown Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::QueryInterface()
// Desc: This function is used to get various interfaces from this container.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    if (ppvObj == NULL)
    {
        return E_INVALIDARG;
    }
    
    *ppvObj = NULL;
    
    if (IsEqualIID(riid, IID_IDispatch))
    {
        *ppvObj = static_cast<IDispatch*>(this);
    }
    else if (IsEqualIID(riid, IID_IErrorInfo))
    {
        *ppvObj = static_cast<IErrorInfo*>(this);
    }
    else if (IsEqualIID(riid, IID_IOleClientSite))
    {
        *ppvObj = static_cast<IOleClientSite*>(this);
    }
    else if (IsEqualIID(riid, IID_IOleContainer))
    {
        *ppvObj = static_cast<IOleContainer*>(this);
    }
    else if (IsEqualIID(riid, IID_IOleControlSite))
    {
        *ppvObj = static_cast<IOleControlSite*>(this);
    }
    else if (IsEqualIID(riid, IID_IOleInPlaceFrame))
    {
        *ppvObj = static_cast<IOleInPlaceFrame*>(this);
    }
    else if (IsEqualIID(riid, IID_IOleInPlaceSite))
    {
        *ppvObj = static_cast<IOleInPlaceSite*>(this);
    }
    else if (IsEqualIID(riid, IID_IOleInPlaceSiteWindowless))
    {
        *ppvObj = static_cast<IOleInPlaceSiteWindowless*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }
    
    if (NULL != *ppvObj)
    {
        AddRef();
    }
    
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::AddRef()
// Desc: This function increments the container's reference count.
///////////////////////////////////////////////////////////////////////////////
ULONG   CMPContainer::AddRef()
{
    m_ulRef++;
    
    return m_ulRef;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Release()
// Desc: This function decrements the container's reference count and if
//       needed, destroys the object.
///////////////////////////////////////////////////////////////////////////////
ULONG   CMPContainer::Release()
{
#ifdef UNDER_CE
//    ASSERT(m_ulRef > 0);
#endif /* UNDER_CE */
    
    m_ulRef--;

    if (m_ulRef > 0)
    {
        return m_ulRef;
    }
    
    m_ulRef = 0;
    
    return 0;
}

//////
// IDispatch Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetIDsOfNames()
// Desc: This function is used by a control to determine what functionality
//       is provided by the container.  All the names are set to UNKNOWN.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int uiNames, LCID lcid, DISPID FAR* rgdispid)
{
    HRESULT   hResult;
    unsigned int i;
    
    hResult = S_OK;
    
    for (i = 0; i < uiNames; i++)
    {
        if (rgszNames[i] != NULL)
        {
            rgdispid[i] = DISPID_UNKNOWN;
            hResult     = DISP_E_UNKNOWNNAME;
        }
    }
    
    return hResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetTypeInfo()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetTypeInfo(unsigned int, LCID, ITypeInfo FAR* FAR*)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetTypeInfoCount()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetTypeInfoCount(unsigned int FAR*)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Invoke()
// Desc: This function is used by the control to access methods and properties
//       of the container.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult, EXCEPINFO FAR* pexecinfo, unsigned int FAR* puArgErr)
{
    HRESULT hr = S_OK;
    
    if ((NULL == pvarResult) && (DISPATCH_PROPERTYGET == wFlags))
    {
        return E_INVALIDARG;
    }
    
    switch (dispid)
    {
    case DISPID_AMBIENT_USERMODE:
    case DISPID_AMBIENT_MESSAGEREFLECT:
        if (NULL != pvarResult)
        {        
            pvarResult->vt      = VT_BOOL;
            pvarResult->boolVal = TRUE;
            hr                  = S_OK;
        }
        else
        {
            hr                  = E_INVALIDARG;
        }
        break;
    }
    
    return hr;
}

//////
// IErrorInfo Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetGUID()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetGUID(GUID *pGUID)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetSource()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetSource(BSTR *)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetDescription()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetDescription(BSTR *)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetHelpFile()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetHelpFile(BSTR *)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetHelpContext()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetHelpContext(DWORD *)
{
    return E_NOTIMPL;
}

//////
// IOleClientSite Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::SaveObject()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::SaveObject()
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetMoniker()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetMoniker(DWORD, DWORD, LPMONIKER *)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetContainer()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetContainer(LPOLECONTAINER *)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::ShowObject()
// Desc: This function directs the control to draw itself.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::ShowObject()
{
    if (NULL != m_pIPOW)
    {
        HDC hDC;
        IViewObjectEx *pVOEx = NULL;
        
        hDC = ::GetDC(m_hWnd);
        
        if (NULL == hDC)
        {
            return S_FALSE;
        }
        
        if (NULL != m_pUnk)
        {
            m_pUnk->QueryInterface(IID_IViewObjectEx, reinterpret_cast<void**>(&pVOEx));
        }
        
        if (NULL != pVOEx)
        {
            pVOEx->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, hDC,
                reinterpret_cast<RECTL*>(&m_rcPos),
                reinterpret_cast<RECTL*>(&m_rcPos), NULL, NULL);
            
            pVOEx->Release();
        }
        
        ::ReleaseDC(m_hWnd, hDC);
    }
    
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OnShowWindow()
// Desc: This window has no meaning since the control is in-place.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OnShowWindow(BOOL)
{
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::RequestNewObjectLayout()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::RequestNewObjectLayout()
{
    return E_NOTIMPL;
}

//////
// IParseDisplayName Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::ParseDisplayName()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::ParseDisplayName(IBindCtx *, LPOLESTR, ULONG *, IMoniker **)
{
    return E_NOTIMPL;
}

//////
// IOleContainer Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::EnumObjects()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::EnumObjects(DWORD, IEnumUnknown **)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::LockContainer()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::LockContainer(BOOL)
{
    return E_NOTIMPL;
}

//////
// IControlSite Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OnControlInfoChanged()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OnControlInfoChanged()
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::LockInPlaceActive()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::LockInPlaceActive(BOOL)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetExtendedControl()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetExtendedControl(IDispatch **)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::TransformCoords()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::TransformCoords(POINTL *, POINTF *, DWORD)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::TranslateAccelerator()
// Desc: This function returns S_FALSE because the container does not accept
//       any accelerator keys.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::TranslateAccelerator(LPMSG, DWORD)
{
    return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OnFocus()
// Desc: This function returns S_OK because it is supposed to return S_OK in
//       all cases.  From the container's point of view, the control always
//       has focus.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OnFocus(BOOL)
{
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::ShowPropertyFrame()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::ShowPropertyFrame()
{
    return E_NOTIMPL;
}

//////
// IOleWindow

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetWindow()
// Desc: This function returns a handle to the window containing the control.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetWindow(HWND *phWnd)
{
    if (NULL == m_hWnd)
    {
        return S_FALSE;
    }
    
    *phWnd = m_hWnd;
    
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::ContextSensitiveHelp()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::ContextSensitiveHelp(BOOL)
{
    return E_NOTIMPL;
}

//////
// IOleInPlaceUIWindow Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetBorder()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetBorder(LPRECT)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::RequestBorderSpace()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::RequestBorderSpace(LPCBORDERWIDTHS)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::SetBorderSpace()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::SetBorderSpace(LPCBORDERWIDTHS)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::SetActiveObject()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::SetActiveObject(IOleInPlaceActiveObject *, LPCOLESTR)
{
    return E_NOTIMPL;
}

//////
// IOleInPlaceFrame Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::InsertMenus()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::InsertMenus(HMENU, LPOLEMENUGROUPWIDTHS)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::SetMenu()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::SetMenu(HMENU, HOLEMENU, HWND)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::RemoveMenus()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::RemoveMenus(HMENU)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::SetStatusText()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::SetStatusText(LPCOLESTR)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::EnableModeless()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::EnableModeless(BOOL)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::TranslateAccelerator()
// Desc: This function always returns false because the container does not
//       have any accelerator keys.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::TranslateAccelerator(LPMSG, WORD)
{
    return S_FALSE;
}

//////
// IOleInPlaceSite Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::CanInPlaceActivate()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::CanInPlaceActivate()
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OnInPlaceActivate()
// Desc: This function tells the control what area it has available for
//       activation.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OnInPlaceActivate()
{
    HRESULT hr = E_FAIL;
    
    if (NULL != m_pIPOW)
    {
        hr = m_pIPOW->SetObjectRects(&m_rcPos, &m_rcPos);
    }
    else if (NULL != m_pIPO)
    {
        hr = m_pIPO->SetObjectRects(&m_rcPos, &m_rcPos);
    }
    
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OnUIActivate()
// Desc: This function returns S_OK because it allows for in-place activation.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OnUIActivate()
{
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetWindowContext()
// Desc: This function allows the control to position the parent window where
//       the in-place activation window should be placed.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    if (NULL == ppFrame || NULL == ppDoc)
    {
        return E_INVALIDARG;
    }
    
    *ppFrame = this;
    //   AddRef();
    
    *ppDoc = NULL;
    
    CopyRect(lprcPosRect, &m_rcPos);
    CopyRect(lprcClipRect, &m_rcPos);
    
    lpFrameInfo->cb            = sizeof (OLEINPLACEFRAMEINFO);
    lpFrameInfo->fMDIApp       = FALSE;
    lpFrameInfo->hwndFrame     = m_hWnd;
    lpFrameInfo->haccel        = 0;
    lpFrameInfo->cAccelEntries = 0;
    
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Scroll()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::Scroll(SIZE scrollExtent)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OnUIDeactivate()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OnUIDeactivate(BOOL)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OnInPlaceDeactivate()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OnInPlaceDeactivate()
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::DiscardUndoState()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::DiscardUndoState()
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::DeactivateAndUndo()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::DeactivateAndUndo()
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OnPosRectChange()
// Desc: This function allows the control to reposition and resize the window
//       which it is contained by.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OnPosRectChange(LPCRECT lprcPosRect)
{
    RECT rcClient, rcPos;
    long lNewClientWidth, lNewClientHeight;
    HRESULT hr = E_FAIL;
    
    GetClientRect(m_hWnd, &rcClient);
    GetWindowRect(m_hWnd, &rcPos);
    
    // See how much to change the size of the window
    lNewClientWidth  = lprcPosRect->right - lprcPosRect->left;
    lNewClientHeight = lprcPosRect->bottom - lprcPosRect->top;
    
    rcPos.bottom += (lNewClientHeight - rcClient.bottom);
    rcPos.right  += (lNewClientWidth  - rcClient.right);
    
    MoveWindow(m_hWnd,
        rcPos.left,
        rcPos.top,
        rcPos.right - rcPos.left,
        rcPos.bottom - rcPos.top, TRUE);
    
    // Store the new value for the window size
    GetClientRect(m_hWnd, &m_rcPos);
    
    // Have these settings take effect
    if (NULL != m_pIPOW)
    {
        hr = m_pIPOW->SetObjectRects(&m_rcPos, &m_rcPos);
    }
    else if (NULL != m_pIPO)
    {
        hr = m_pIPO->SetObjectRects(&m_rcPos, &m_rcPos);
    }
    
    return hr;
}

//////
// IOleInPlaceSiteEx Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OnInPlaceActivateEx()
// Desc: This function is synonymous with OnInPlaceActivate().
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OnInPlaceActivateEx(BOOL *, DWORD)
{
    return OnInPlaceActivate();
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OnInPlaceDeactivateEx()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OnInPlaceDeactivateEx(BOOL)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::RequestUIActivate()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::RequestUIActivate()
{
    return E_NOTIMPL;
}

//////
// IOleInPlaceSiteWindowless Methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::CanWindowlessActivate()
// Desc: This function returns S_OK to indicate that it can in-place activate.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::CanWindowlessActivate()
{
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetCapture()
// Desc: This function returns S_OK if the window containing the control
//       currently has the mouse capture.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetCapture()
{
    HWND    hWnd;
    HRESULT hResult = S_FALSE;
    
    hWnd = ::GetCapture();
    
    if (m_hWnd == hWnd)
    {
        hResult = S_OK;
    }
    
    return hResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::SetCapture()
// Desc: This function sets mouse capture to the current window or releases
//       it depending on fCapture.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::SetCapture(BOOL fCapture)
{
    if (fCapture)
    {
        ::SetCapture(m_hWnd);
    }
    else
    {
        ::ReleaseCapture();
    }
    
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetFocus()
// Desc: This function returns S_OK if the window containing the control has
//       focus.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetFocus()
{
    HWND    hWnd;
    HRESULT hResult = S_FALSE;
    
    hWnd = ::GetFocus();
    
    if (m_hWnd == hWnd)
    {
        hResult = S_OK;
    }
    
    return hResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::SetFocus()
// Desc: This function acquires or releases focus depending on the state of
//       fFocus.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::SetFocus(BOOL fFocus)
{
    if (FALSE != fFocus)
    {
        ::SetFocus(m_hWnd);
    }
    else
    {
        ::SetFocus(NULL);
    }
    
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::GetDC()
// Desc: This function gets the current device context.  This function is
//       required to allow the control to update the tracker bar and status
//       bar.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::GetDC(LPCRECT, DWORD, HDC *phDC)
{
    *phDC = ::GetDC(m_hWnd);
    
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::ReleaseDC()
// Desc: This function releases a device context previously acquired by a
//       call to GetDC().
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::ReleaseDC(HDC hDC)
{
    ::ReleaseDC(m_hWnd, hDC);
    
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::InvalidateRect()
// Desc: This function causes a portion of the window to be redrawn.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::InvalidateRect(LPCRECT pRect, BOOL fErase)
{
    ::InvalidateRect(m_hWnd, pRect, fErase);
    
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::InvalidateRgn()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::InvalidateRgn(HRGN, BOOL)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::ScrollRect()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::ScrollRect(INT, INT, LPCRECT, LPCRECT)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::AdjustRect()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::AdjustRect(LPRECT)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OnDefWindowMessage()
// Desc: Not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OnDefWindowMessage(UINT, WPARAM, LPARAM, LRESULT *)
{
    return E_NOTIMPL;
}

//////
// Methods defined in CMPContainer

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::CreateControl()
// Desc: This function attempts to create a MediaPlayer control as an
//       in-place object.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::CreateControl(REFIID rclsid)
{
    HRESULT hResult;
    IOleObject *pIOleObject = NULL;
    
    // Translate the IID to a CLSID
    hResult = CoCreateInstance(rclsid, NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
        IID_IUnknown, reinterpret_cast<LPVOID*>(&m_pUnk));
    if (FAILED(hResult))
    {
        return hResult;
    }
    
    hResult = m_pUnk->QueryInterface(IID_IOleObject, reinterpret_cast<LPVOID*>(&pIOleObject));
    
    if (SUCCEEDED(hResult))
    {
        pIOleObject->SetClientSite(this);
        
        // Tell the control to show itself
        pIOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, m_hWnd, &m_rcPos);
        pIOleObject->DoVerb(OLEIVERB_SHOW, NULL, this, 0, m_hWnd, &m_rcPos);
        pIOleObject->Release();

#ifndef UNDER_CE
        hResult = E_FAIL;
        m_pIPOW = NULL;
#else
        hResult = m_pUnk->QueryInterface(IID_IOleInPlaceObjectWindowless, reinterpret_cast<void**>(&m_pIPOW));
#endif // !under_ce
        
        if (FAILED(hResult))
        {
            // Not windowless, so try this
            hResult = m_pUnk->QueryInterface(IID_IOleInPlaceObject, reinterpret_cast<void**>(&m_pIPO));
        }
        
        // cache our favorite interface
        hResult = m_pUnk->QueryInterface(IID_IMediaPlayer, reinterpret_cast<LPVOID*>(&m_pMP));
    }

    if( SUCCEEDED( hResult ) )
    {
        // Put the appropriate tool-bars and controls onto the player
        UpdateDisplay();
    
        // Set the autostart property to TRUE
        m_pMP->put_AutoStart( TRUE );
        m_pMP->put_AutoSize( TRUE );
        m_pMP->put_AutoRewind( TRUE );
        // Set up the player-control for play-forever
        g_pPlayerWindow->ApplyPlayCount();

        // Setup the player-control for desired zoom-level
        g_pPlayerWindow->ApplyZoomLevel();
    }
    
    return hResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::DestroyControl()
// Desc: This function releases and all holds on the MediaPlayer control 
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::DestroyControl ()
{
    HRESULT hr = S_OK;

    if (NULL != m_pUnk)
    {
        IOleObject *pIOleObject = NULL;       
        m_pUnk->QueryInterface(IID_IOleObject, reinterpret_cast<LPVOID*>(&pIOleObject));
        
        if (pIOleObject)
        {
            pIOleObject->DoVerb(OLEIVERB_HIDE, NULL, this, 0, m_hWnd, &m_rcPos);
            pIOleObject->SetClientSite(NULL);
            pIOleObject->Release();
        }
        
        m_pUnk->Release();
        m_pUnk = NULL;
    }
    
    if (NULL != m_pMP)
    {
        m_pMP->Release();
        m_pMP = NULL;
    }
    
    if (NULL != m_pIPO)
    {
        m_pIPO->Release();
        m_pIPO = NULL;
    }
    
    if (NULL != m_pIPOW)
    {
        m_pIPOW->Release();
        m_pIPOW = NULL;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::SetWindow()
// Desc: This function stores the window associated with the container.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::SetWindow(HWND hWnd)
{
    m_hWnd = hWnd;
    
    if (GetClientRect(m_hWnd, &m_rcPos) == 0)
    {
        SetRectEmpty(&m_rcPos);
    }
    
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Paint()
// Desc: This function redraws the MediaPlayer control.
///////////////////////////////////////////////////////////////////////////////
void   CMPContainer::Paint(HDC hDC, RECT *pRect)
{
    IViewObjectEx *pVOEx = NULL;
    
    if (NULL != m_pUnk)
    {
        m_pUnk->QueryInterface(IID_IViewObjectEx, reinterpret_cast<void**>(&pVOEx));
    }
    
    if (NULL != pVOEx)
    {
        RECT *prcPos = pRect;
        
        if (NULL == m_pIPOW)
        {
            prcPos = &m_rcPos;
        }
        
        pVOEx->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, hDC,
            reinterpret_cast<RECTL*>(prcPos),
            reinterpret_cast<RECTL*>(prcPos), NULL, NULL);
        
        pVOEx->Release();
    }
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::UpdateDisplay()
// Desc: This function makes sure that the various components of the
//       MediaPlayer control are being displayed based on what the user
//       wishes to see.
///////////////////////////////////////////////////////////////////////////////
void CMPContainer::UpdateDisplay()
{
    RECT rect;
    VARIANT_BOOL fTracker;
    VARIANT_BOOL fControls;
    VARIANT_BOOL fAudioControls;
    VARIANT_BOOL fPositionControls;
    VARIANT_BOOL fStatusBar;

    switch( m_dwLevelOfControls )
    {
    case 0: // minimal
        fTracker = VARIANT_FALSE;
        fControls = VARIANT_TRUE;
        fAudioControls = VARIANT_TRUE;
        fPositionControls = VARIANT_TRUE;
        fStatusBar = VARIANT_FALSE;
    break;
    case 1: // compact
    case 2: // standard (same as compact for now...)
    default:
        fTracker = VARIANT_TRUE;
        fControls = VARIANT_TRUE;
        fAudioControls = VARIANT_TRUE;
        fPositionControls = VARIANT_TRUE;
        fStatusBar = VARIANT_TRUE;
    break;
    case 10: // skin
        fTracker          = VARIANT_FALSE;
        fControls         = VARIANT_FALSE;
        fAudioControls    = VARIANT_FALSE;
        fPositionControls = VARIANT_FALSE;
        fStatusBar        = VARIANT_TRUE;
    break;
    }

    m_pMP->put_ShowTracker(fTracker);
    m_pMP->put_EnableTracker(fTracker);
    m_pMP->put_ShowControls(fControls);
    m_pMP->put_ShowAudioControls(fAudioControls);
    m_pMP->put_ShowPositionControls(fPositionControls);
    m_pMP->put_EnablePositionControls(fPositionControls);
    m_pMP->put_ShowStatusBar(fStatusBar);
    
    GetWindowRect( m_hWnd, &rect );
    g_pPlayerWindow->SetMinimumRect( rect );
    return;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::OpenFile()
// Desc: This function attempts to open the file given by szFilename.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::OpenFile(LPCTSTR szFilename)
{
    HRESULT hr = S_FALSE;
    BSTR bstrFilename = NULL;

    if( m_pMP )
    {
        m_pMP->put_SendErrorEvents(TRUE);
        m_pMP->put_SendWarningEvents(TRUE);

        // put_FileName takes a BSTR, and we shouldn't just pass it szFilename
        if( bstrFilename = SysAllocString( szFilename ) )
        {
            hr = m_pMP->put_FileName(bstrFilename);
            SysFreeString( bstrFilename );
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Close()
// Desc: This function attempts to close a file.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::Close()
{
    if (NULL == m_pMP)
    {
        return S_FALSE;
    }
    
    return m_pMP->put_FileName(NULL);
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Play()
// Desc: This function plays the currently open media clip at the normal rate.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::Play()
{
    HRESULT hr;
    
    m_pMP->put_Rate(1.0);
    hr = m_pMP->Play();
    
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Pause()
// Desc: This function pauses playback.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::Pause()
{
    return m_pMP->Pause();
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Stop()
// Desc: This function stops playback and resets the position back to the
//       begining of the stream.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::Stop()
{
    double  dDuration = 0.0;
    HRESULT hr;
    
    // Stop playback
    hr = m_pMP->Stop();
    
    // check to see if the current position needs to be reset
    VARIANT_BOOL fDurationValid;
    m_pMP->get_Duration(&dDuration);
   	m_pMP->get_IsDurationValid(&fDurationValid);
   	if (!fDurationValid) dDuration = 0.0;

    // reset to the beginning of the stream
    if (SUCCEEDED(hr) && dDuration > 0.0)
    {
        hr = m_pMP->put_CurrentPosition(0.0);
    }
    
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Mute()
// Desc: This function toggles the mute status of the MediaPlayer.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::Mute()
{
    HRESULT hr;
    VARIANT_BOOL fMuted;
    
    hr = m_pMP->get_Mute(&fMuted);
    
    if (FAILED(hr))
    {
        return S_FALSE;
    }
    
    // Toggle the mute state
    if(VARIANT_FALSE == fMuted)
    {
        fMuted = VARIANT_TRUE;
    }
    else
    {
        fMuted = VARIANT_FALSE;
    }
    
    return m_pMP->put_Mute(fMuted);
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::Repeat()
// Desc: This function switchs between repeating a media clip and playing it
//       only once.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPContainer::Repeat()
{
    LONG    lCount;
    HRESULT hr;
    
    hr = m_pMP->get_PlayCount(&lCount);
    
    if (FAILED(hr))
    {
        return S_FALSE;
    }
    
    // This app only allows play once and infinite repeat
    if (0 == lCount)
    {
        lCount = 1;
    }
    else
    {
        lCount = 0;
    }
    
    return m_pMP->put_PlayCount(lCount);
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPContainer::IsRepeating()
// Desc: This function tells us if the media player control is set to
//       repeat forever (returns true) or play once (returns false).
///////////////////////////////////////////////////////////////////////////////
bool CMPContainer::IsRepeating()
{
	long lCount;
    HRESULT hr = m_pMP->get_PlayCount(&lCount);
    ASSERT(SUCCEEDED(hr));
    return (lCount == 0);
}

