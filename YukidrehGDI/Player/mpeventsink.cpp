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
// File: MPEventSink.cpp
//
// Desc: This file contains the implementation for an Event Sink used by the
//        Media Player control.
//
///////////////////////////////////////////////////////////////////////////////

#include "MPEventSink.h"

#include <dsdispid.h>

//////
// Constructor/Destructor
CMPEventSink::CMPEventSink() : m_ulRef(0),
m_dwCookie(0),
m_pCP(NULL)
{
}

CMPEventSink::~CMPEventSink()
{
    (void)Fini();
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPEventSink::Init()
// Desc: This function initializes the event sink.
///////////////////////////////////////////////////////////////////////////////
bool CMPEventSink::Connect(IConnectionPoint *pCP)
{
    bool bResult = false;
    
    m_pCP = pCP;
    
    if (NULL != m_pCP)
    {
        m_pCP->AddRef();
        
        bResult = true;
    }
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPEventSink::Disconnect()
// Desc: This function shutdown the event sink, and free's any resources.
///////////////////////////////////////////////////////////////////////////////
bool CMPEventSink::Disconnect()
{    
    bool bResult = false;
    
    if (NULL != m_pCP)
    {
        if (SUCCEEDED(m_pCP->Unadvise(m_dwCookie)))
        {
            bResult = true;
        }
        
        m_pCP->Release();
        m_pCP = NULL;
    }
    
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPEventSink::Fini()
// Desc: This function free's any resources.
///////////////////////////////////////////////////////////////////////////////
bool CMPEventSink::Fini()
{
    bool bResult = true;
    
    return bResult;
}

//////
// IUnknown Functions

///////////////////////////////////////////////////////////////////////////////
// Name: CMPEventSink::QueryInterface()
// Desc: This function is used to find other interfaces implemented by this
//       object.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPEventSink::QueryInterface(REFIID riid, LPVOID *ppvObj)
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
    else if (IsEqualIID(riid, DIID__MediaPlayerEvents))
    {
        *ppvObj = static_cast<_MediaPlayerEvents*>(this);
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
// Name: CMPEventSink::AddRef()
// Desc: This function increases the reference count for this object.
///////////////////////////////////////////////////////////////////////////////
ULONG   CMPEventSink::AddRef(void)
{
    m_ulRef++;
    
    return m_ulRef;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPEventSink::Release()
// Desc: This function decreases the reference count for this object.  If the
//       reference count hits 0, the object is destroyed.
///////////////////////////////////////////////////////////////////////////////
ULONG   CMPEventSink::Release(void)
{
//    ASSERT(m_ulRef > 0);
    
    m_ulRef--;

    if (m_ulRef > 0)
    {
        return m_ulRef;
    }
    
    m_ulRef = 0;
    
    return 0;
}

//////
// IDispatch Functions

///////////////////////////////////////////////////////////////////////////////
// Name: CMPEventSink::GetIDsOfNames()
// Desc: This function is not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPEventSink::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned cNames, LCID lcid, DISPID FAR* rgdispid)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPEventSink::GetTypeInfo
// Desc: This function is not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPEventSink::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPEventSink::GetTypeInfoCount
// Desc: This function is not implemented.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPEventSink::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// Name: CMPEventSink::Invoke()
// Desc: This function is used to call the various function associated with
//       a particular event.
///////////////////////////////////////////////////////////////////////////////
HRESULT CMPEventSink::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR* pvarResult, EXCEPINFO FAR* pexecinfo, unsigned int FAR* puArgErr)
{
    HRESULT hResult = S_OK;
    
    if ((NULL == pvarResult) && (DISPATCH_PROPERTYGET == wFlags))
    {
        return E_INVALIDARG;
    }
    
    switch (dispid)
    {
    case DISPID_ENDOFSTREAM:
        EndOfStream(pdispparams->rgvarg[0].lVal);  // lResult
        
        break;
        
    case DISPID_KEYDOWN:
        KeyDown(pdispparams->rgvarg[1].iVal,       // nKeyCode
            pdispparams->rgvarg[0].iVal);      // nShiftState
        
        break;
        
    case DISPID_KEYUP:
        KeyUp(pdispparams->rgvarg[1].iVal,         // nKeyCode
            pdispparams->rgvarg[0].iVal);        // nShiftState
        
        break;
        
    case DISPID_KEYPRESS:
        KeyPress(pdispparams->rgvarg[0].iVal);     // nCharacterCode
        
        break;
        
    case DISPID_MOUSEMOVE:
        MouseMove(pdispparams->rgvarg[3].iVal,     // nButton
            pdispparams->rgvarg[2].iVal,     // nShiftState
            pdispparams->rgvarg[1].lVal,     // x
            pdispparams->rgvarg[0].lVal);    // y
        
        break;
        
    case DISPID_MOUSEDOWN:
        MouseDown(pdispparams->rgvarg[3].iVal,     // nButton
            pdispparams->rgvarg[2].iVal,     // nShiftState
            pdispparams->rgvarg[1].lVal,     // x
            pdispparams->rgvarg[0].lVal);    // y
        
        break;
        
    case DISPID_MOUSEUP:
        MouseUp(pdispparams->rgvarg[3].iVal,       // nButton
            pdispparams->rgvarg[2].iVal,       // nShiftState
            pdispparams->rgvarg[1].lVal,       // x
            pdispparams->rgvarg[0].lVal);      // y
        
        break;
        
    case DISPID_CLICK:
        Click(pdispparams->rgvarg[3].iVal,         // nButton
            pdispparams->rgvarg[2].iVal,         // nShiftState
            pdispparams->rgvarg[1].lVal,         // x
            pdispparams->rgvarg[0].lVal);        // y
        
        break;
        
    case DISPID_DBLCLICK:
        DblClick(pdispparams->rgvarg[3].iVal,      // nButton
            pdispparams->rgvarg[2].iVal,      // nShiftState
            pdispparams->rgvarg[1].lVal,      // x
            pdispparams->rgvarg[0].lVal);     // y
        
        break;
        
    case DISPID_OPENSTATECHANGE:
        OpenStateChange(pdispparams->rgvarg[1].lVal,  // lOldState
            pdispparams->rgvarg[0].lVal); // lNewState
        
        break;
        
    case DISPID_PLAYSTATECHANGE:
        PlayStateChange(pdispparams->rgvarg[1].lVal,  // lOldState
            pdispparams->rgvarg[0].lVal); // lNewState
        
        break;
        
    case DISPID_SCRIPTCOMMAND:
        ScriptCommand(pdispparams->rgvarg[1].bstrVal,  // strSCType
            pdispparams->rgvarg[0].bstrVal); // strParam
        
        break;
        
    case DISPID_BUFFERING:
        Buffering(pdispparams->rgvarg[0].boolVal);     // bStart
        
        break;
        
    case DISPID_ERROR:
        Error();
        
        break;
        
    case DISPID_MARKERHIT:
        MarkerHit(pdispparams->rgvarg[0].lVal);       // lMarkerNum
        
        break;
        
    case DISPID_WARNING:
        Warning(pdispparams->rgvarg[2].lVal,          // lWarningType
            pdispparams->rgvarg[1].lVal,          // lParam
            pdispparams->rgvarg[0].bstrVal);      // strDescription
        
        break;
        
    case DISPID_NEWSTREAM:
        NewStream();
        
        break;
        
    case DISPID_DISCONNECT:
        Disconnect(pdispparams->rgvarg[0].lVal);      // lResult
        
        break;
        
    case DISPID_POSITIONCHANGE:
        PositionChange(pdispparams->rgvarg[1].dblVal,   // dOldPosition
            pdispparams->rgvarg[0].dblVal);  // dNewPosition
        
        break;
        
    case DISPID_DISPLAYMODECHANGE:
        DisplayModeChange();
        
        break;
        
    case DISPID_READYSTATECHANGE:
        ReadyStateChange((ReadyStateConstants)(pdispparams->rgvarg[0].lVal));
        
        break;
        
    default:
        hResult = DISP_E_MEMBERNOTFOUND;
        break;
   }
   
   return hResult;
}

//////
// CMPEventSink methods

///////////////////////////////////////////////////////////////////////////////
// Name: CMPEventSink::SetCookie()
// Desc: This method stores the cookie associated with the IConnectionPoint
//       interface with this object.
///////////////////////////////////////////////////////////////////////////////
void CMPEventSink::SetCookie(DWORD dwCookie)
{
    m_dwCookie = dwCookie;
    
    return;
}
