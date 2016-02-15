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
// File: mpcontainer.h
//
// Desc: This class defines an ActiveX container class for the Windows Media
//       Player ActiveX control.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MPCONTAINER_H_
#define _MPCONTAINER_H_

#include <oaidl.h>
#include <ocidl.h>
#include <oleidl.h>

#include <dxmplay.h>

class CMPContainer : public IDispatch,
                            IErrorInfo,
                            IOleClientSite,
                            IOleContainer,
                            IOleControlSite,
                            IOleInPlaceFrame,
                            IOleInPlaceSiteWindowless
{
 public:
   // Constructor/Destructor
   CMPContainer();
   virtual ~CMPContainer();

   bool Init();
   bool Fini();

   // IUnknown Methods
   STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
   STDMETHODIMP_(ULONG) AddRef(void);
   STDMETHODIMP_(ULONG) Release(void);

   // IDispatch Methods
   STDMETHOD (GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int uiNames, LCID lcid, DISPID FAR* rgdispid);
   STDMETHOD (GetTypeInfo)(unsigned int uiTInfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
   STDMETHOD (GetTypeInfoCount)(unsigned int FAR* puiTInfo);
   STDMETHOD (Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR* pexecinfo, unsigned int FAR* puArgErr);

   // IErrorInfo Methods
   STDMETHOD (GetGUID)(GUID *pGUID);
   STDMETHOD (GetSource)(BSTR *pstrDescription);
   STDMETHOD (GetDescription)(BSTR *pstrDescription);
   STDMETHOD (GetHelpFile)(BSTR *pstrHelpFile);
   STDMETHOD (GetHelpContext)(DWORD *pdwHelpContext);

   // IOleClientSite Methods
   STDMETHOD (SaveObject)();
   STDMETHOD (GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER *ppmk);
   STDMETHOD (GetContainer)(LPOLECONTAINER *ppContainer);
   STDMETHOD (ShowObject)();
   STDMETHOD (OnShowWindow)(BOOL fShow);
   STDMETHOD (RequestNewObjectLayout)();

   // IParseDisplayName Methods
   STDMETHOD (ParseDisplayName)(IBindCtx *pbc, LPOLESTR pszDisplayName, ULONG *pulEaten, IMoniker **ppmkOut);

   // IOleContainer Methods
   STDMETHOD (EnumObjects)(DWORD grfFlags, IEnumUnknown **ppenum);
   STDMETHOD (LockContainer)(BOOL fLock);

   // IOleControlSite Methods
   STDMETHOD (OnControlInfoChanged)();
   STDMETHOD (LockInPlaceActive)(BOOL fLock);
   STDMETHOD (GetExtendedControl)(IDispatch **ppDisp);
   STDMETHOD (TransformCoords)(POINTL *pptHimetric, POINTF *pptfContainer, DWORD dwFlags);
   STDMETHOD (TranslateAccelerator)(LPMSG pMsg, DWORD grfModifiers);
   STDMETHOD (OnFocus)(BOOL fGotFocus);
   STDMETHOD (ShowPropertyFrame)();

   // IOleWindow
   STDMETHOD (GetWindow)(HWND *phWnd);
   STDMETHOD (ContextSensitiveHelp)(BOOL fEnterMode);

   // IOleInPlaceUIWindow Methods
   STDMETHOD (GetBorder)(LPRECT lprectBorder);
   STDMETHOD (RequestBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
   STDMETHOD (SetBorderSpace)(LPCBORDERWIDTHS lpborderwidths);
   STDMETHOD (SetActiveObject)(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR lpszObjName);

   // IOleInPlaceFrame Methods
   STDMETHOD (InsertMenus)(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
   STDMETHOD (SetMenu)(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActivateObject);
   STDMETHOD (RemoveMenus)(HMENU hmenuShared);
   STDMETHOD (SetStatusText)(LPCOLESTR pszStatusText);
   STDMETHOD (EnableModeless)(BOOL fEnable);
   STDMETHOD (TranslateAccelerator)(LPMSG lpmsg, WORD wID);

   // IOleInPlaceSite Methods
   STDMETHOD (CanInPlaceActivate)();
   STDMETHOD (OnInPlaceActivate)();
   STDMETHOD (OnUIActivate)();
   STDMETHOD (GetWindowContext)(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
   STDMETHOD (Scroll)(SIZE scrollExtent);
   STDMETHOD (OnUIDeactivate)(BOOL fUndoable);
   STDMETHOD (OnInPlaceDeactivate)();
   STDMETHOD (DiscardUndoState)();
   STDMETHOD (DeactivateAndUndo)();
   STDMETHOD (OnPosRectChange)(LPCRECT lprcPosRect);

   // IOleInPlaceSiteEx Methods
   STDMETHOD (OnInPlaceActivateEx)(BOOL *pfNoRedraw, DWORD dwFlags);
   STDMETHOD (OnInPlaceDeactivateEx)(BOOL fNoRedraw);
   STDMETHOD (RequestUIActivate)();

   // IOleInPlaceSiteWindowless Methods
   STDMETHOD (CanWindowlessActivate)();
   STDMETHOD (GetCapture)();
   STDMETHOD (SetCapture)(BOOL fCapture);
   STDMETHOD (GetFocus)();
   STDMETHOD (SetFocus)(BOOL fFocus);
   STDMETHOD (GetDC)(LPCRECT pRect, DWORD grfFlags, HDC *phDC);
   STDMETHOD (ReleaseDC)(HDC hDC);
   STDMETHOD (InvalidateRect)(LPCRECT pRect, BOOL fErase);
   STDMETHOD (InvalidateRgn)(HRGN hRGN, BOOL fErase);
   STDMETHOD (ScrollRect)(INT dx, INT dy, LPCRECT pRectScroll, LPCRECT pRectClip);
   STDMETHOD (AdjustRect)(LPRECT prc);
   STDMETHOD (OnDefWindowMessage)(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);

   //////
   // CMPContainer Methods
   HRESULT CreateControl(REFIID rclsid);
   HRESULT DestroyControl();
   HRESULT SetWindow(HWND hWnd);

   void    Show(int iShow);
   void    Paint(HDC hDC, RECT *pRect);
   void    UpdateDisplay();

   HRESULT OpenFile(LPCTSTR szFilename);
   HRESULT Close();
   HRESULT Play();
   HRESULT Pause();
   HRESULT Stop();
   HRESULT Mute();
   HRESULT Repeat();

   bool IsRepeating();

   IMediaPlayer                *m_pMP;

 protected:
   unsigned long                m_ulRef;
   HWND                         m_hWnd;
   RECT                         m_rcPos;
   IUnknown                    *m_pUnk;
   IOleInPlaceObjectWindowless *m_pIPOW;
   IOleInPlaceObject           *m_pIPO;
   DWORD                        m_dwLevelOfControls; // 0 for minimal, 1 for compact, 2 for standard

 private:
};

#endif /* _MPCONTAINER_H_ */
