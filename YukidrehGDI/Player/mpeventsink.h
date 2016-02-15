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
// File: MPEventSink.h
//
// Desc: This file contains the definition of an Event Sink for use by the
//       Media Player control.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MPEVENTSINK_H_
#define _MPEVENTSINK_H_

#include <olectl.h>
#include <msdxm.h>

class CMPEventSink : public _MediaPlayerEvents
{
 public:
   // Constructor/Destructor
   CMPEventSink();
   virtual ~CMPEventSink();

   bool Fini();

   bool Connect(IConnectionPoint *pCP);
   bool Disconnect ();

   // IUnknown Functions
   STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
   STDMETHODIMP_(ULONG) AddRef(void);
   STDMETHODIMP_(ULONG) Release(void);

   // IDispatch Functions
   STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned cNames, LCID lcid, DISPID FAR* rgdispid);
   STDMETHOD (GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
   STDMETHOD (GetTypeInfoCount)(unsigned int FAR* pctinfo);
   STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR* pvarResult, EXCEPINFO FAR* pexecinfo, unsigned int FAR* puArgErr);

   // _MediaPlayerEvents Functions
   virtual void DVDNotify(long lEventCode, long lEventParam1, long lEventParam2) = 0;
   virtual void EndOfStream(long lResult) = 0;
   virtual void KeyDown(short nKeyCode, short nShiftState) = 0;
   virtual void KeyUp(short nKeyCode, short nShiftState) = 0;
   virtual void KeyPress(short nCharacterCode) = 0;
   virtual void MouseMove(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y) = 0;
   virtual void MouseDown(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y) = 0;
   virtual void MouseUp(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y) = 0;
   virtual void Click(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y) = 0;
   virtual void DblClick(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y) = 0;
   virtual void OpenStateChange(long lOldState, long lNewState) = 0;
   virtual void PlayStateChange(long lOldState, long lNewState) = 0;
   virtual void ScriptCommand(BSTR strSCType, BSTR strParam) = 0;
   virtual void Buffering(VARIANT_BOOL fStart) = 0;
   virtual void Error(void) = 0;
   virtual void MarkerHit(long lMarkerNum) = 0;
   virtual void Warning(long lWarningType, long lParam, BSTR strDescription) = 0;
   virtual void NewStream(void) = 0;
   virtual void Disconnect(long lResult) = 0;
   virtual void PositionChange(double dOldPosition, double dNewPosition) = 0;
   virtual void DisplayModeChange(void) = 0;
   virtual void ReadyStateChange(ReadyStateConstants eReadyState) = 0;

   //////
   // CMPEventSink methods
   void SetCookie(DWORD dwCookie);

 protected:
   unsigned long     m_ulRef;
   DWORD             m_dwCookie;
   IConnectionPoint *m_pCP;

 private:
};

#endif /* _MPEVENTSINK_H_ */
