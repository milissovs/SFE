#ifndef DRAGDROP_H_INCLUDED
#define DRAGDROP_H_INCLUDED

#include <oleidl.h>
#include "globals.h"
#include "folder_item.h"

const CLIPFORMAT FOLDER_FRMT = RegisterClipboardFormat(L"FOLDER_ITEM");

//=====================================================================================================
// Helper functions

HRESULT CreateDataObject(FORMATETC* fmtetc, STGMEDIUM* stgmeds, UINT count, IDataObject** ppDataObject);

HGLOBAL DupGlobalMem(HGLOBAL hMem);

void    DropData(HWND hwnd, IDataObject* pDataObject, POINTL pt);

HRESULT RegisterDropWindow(HWND hwnd, IDropTarget** ppDropTarget);

HRESULT UnregisterDropWindow(HWND hwnd, IDropTarget* pDropTarget);

HRESULT CreateEnumFormatEtc(UINT cfmt, FORMATETC* afmt, IEnumFORMATETC** ppEnumFormatEtc);

static void DeepCopyFormatEtc(FORMATETC* dest, FORMATETC* source);

HRESULT CreateDropSource(IDropSource** ppDropSource);

HANDLE StringToHandle(wchar_t* szText, int nTextLen);


//=====================================================================================================
// Drop objects

class CDragSourceNotify : IDropSourceNotify
{
private:
    LONG refCount = 0;

public:
    CDragSourceNotify() = default;
    ~CDragSourceNotify() = default;

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (!ppvObject)
            return E_POINTER;

        if (IsEqualIID(riid, IID_IUnknown))
            *ppvObject = static_cast<IUnknown*>(this);
        else if (IsEqualIID(riid, IID_IDropSourceNotify))
            *ppvObject = static_cast<IDropSourceNotify*>(this);
        else
        {
            *ppvObject = nullptr;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return InterlockedIncrement(&refCount);
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG ret = InterlockedDecrement(&refCount);
        if (!ret) {
            delete this;
        }
        return ret;
    }

    HRESULT STDMETHODCALLTYPE DragEnterTarget(HWND /*hWndTarget*/) override
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragLeaveTarget() override
    {
        return S_OK;
    }
};

class CDropSource : public IDropSource
{
public:
    CDragSourceNotify* pDragSourceNotify;

    // IUnknown members
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject);
    ULONG   __stdcall AddRef(void);
    ULONG   __stdcall Release(void);

    // IDropSource members
    HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
    HRESULT __stdcall GiveFeedback(DWORD dwEffect);

    // Constructor / Destructor
    CDropSource();
    ~CDropSource();

private:
    LONG	   m_lRefCount;
};

class CDropTarget : public IDropTarget
{
public:
    // IUnknown implementation
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject);
    ULONG	__stdcall AddRef(void);
    ULONG	__stdcall Release(void);

    // IDropTarget implementation
    HRESULT __stdcall DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    HRESULT __stdcall DragLeave(void);
    HRESULT __stdcall Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

    // Constructor
    CDropTarget(HWND hwnd);
    ~CDropTarget();

private:
    // internal helper function
    DWORD DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed);
    bool  QueryDataObject(IDataObject* pDataObject);

    // Private member variables
    LONG	m_lRefCount;
    HWND	m_hWnd;
    bool    m_fAllowDrop;

    IDataObject* m_pDataObject;
};

class CDataObject : public IDataObject
{
public:

    // IUnknown members
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject); // { return S_OK; };
    ULONG __stdcall AddRef(void); // { return ++m_lRefCount; };
    ULONG __stdcall Release(void); // { return --m_lRefCount; };

    // IDataObject members
    HRESULT __stdcall GetData(FORMATETC* pFormatEtc, STGMEDIUM* pmedium);
    HRESULT __stdcall GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pmedium);
    HRESULT __stdcall QueryGetData(FORMATETC* pFormatEtc);
    HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC* pFormatEct, FORMATETC* pFormatEtcOut);
    HRESULT __stdcall SetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium, BOOL fRelease);
    HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc);
    HRESULT __stdcall DAdvise(FORMATETC* pFormatEtc, DWORD advf, IAdviseSink*, DWORD*);
    HRESULT __stdcall DUnadvise(DWORD dwConnection);
    HRESULT __stdcall EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);

    // Constructor / Destructor
    CDataObject(FORMATETC* fmtetc, STGMEDIUM* stgmed, int count);
    ~CDataObject();

private:
    // any private members and functions
    LONG m_lRefCount;
    int m_nNumFormats;
    FORMATETC * m_pFormatEtc;
    STGMEDIUM* m_pStgMedium;

    int LookupFormatEtc(FORMATETC* pFormatEtc);
};

class CEnumFormatEtc : public IEnumFORMATETC
{
public:

    //
    // IUnknown members
    //
    HRESULT __stdcall  QueryInterface(REFIID iid, void** ppvObject);
    ULONG	__stdcall  AddRef(void);
    ULONG	__stdcall  Release(void);

    //
    // IEnumFormatEtc members
    //
    HRESULT __stdcall  Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched);
    HRESULT __stdcall  Skip(ULONG celt);
    HRESULT __stdcall  Reset(void);
    HRESULT __stdcall  Clone(IEnumFORMATETC** ppEnumFormatEtc);

    //
    // Construction / Destruction
    //
    CEnumFormatEtc(FORMATETC* pFormatEtc, int nNumFormats);
    ~CEnumFormatEtc();

private:

    LONG		m_lRefCount;		// Reference count for this COM interface
    ULONG		m_nIndex;			// current enumerator index
    ULONG		m_nNumFormats;		// number of FORMATETC members
    FORMATETC*  m_pFormatEtc;		// array of FORMATETC objects
};

#endif // DRAGDROP_H_INCLUDED
