#include "drag_drop.h"
#include <shlobj_core.h>

//--------------------------------------------------------------------------
//                         EXAMPLE
//--------------------------------------------------------------------------
// FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
// STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };
// stgmed.hGlobal = StringToHandle("Hello, World!");
// IDataObject* pDataObject;
// CreateDataObject(&fmtetc, &stgmed, 1, &pDataObject);
//--------------------------------------------------------------------------

HRESULT CreateDataObject(FORMATETC* fmtetc, STGMEDIUM* stgmeds, UINT count, IDataObject** ppDataObject)
{
    if (ppDataObject == 0)
        return E_INVALIDARG;

    *ppDataObject = new CDataObject(fmtetc, stgmeds, count);

    return (*ppDataObject) ? S_OK : E_OUTOFMEMORY;
}

HGLOBAL DupGlobalMem(HGLOBAL hMem)
{
    size_t len = GlobalSize(hMem);
    PVOID source = GlobalLock(hMem);
    PVOID dest = GlobalAlloc(GMEM_FIXED, len);

    if (dest != NULL && source != NULL)
        memcpy(dest, source, len);

    GlobalUnlock(hMem);
    return dest;
}

void DropData(HWND hwnd, IDataObject* pDataObject, POINTL pt)
{
    // construct a FORMATETC object
    FORMATETC fmtetc = { FOLDER_FRMT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stgmed;

    // See if the dataobject contains any TEXT stored as a HGLOBAL
    if (pDataObject->QueryGetData(&fmtetc) == S_OK)
    {
        // Yippie! the data is there, so go get it!
        if (pDataObject->GetData(&fmtetc, &stgmed) == S_OK)
        {
            // we asked for the data as a HGLOBAL, so access it appropriately
            FOLDER_ITEM* pfi = (FOLDER_ITEM * )GlobalLock(stgmed.hGlobal);
            if (pfi->hWndParent != NULL && IsWindow(pfi->hWndParent))
                SendMessage(pfi->hWndParent, LVM_DELETEITEM, pfi->move_index, NULL);

            //SetWindowText(hwnd, (wchar_t*)data);
            SendMessage(hwnd, LVM_INSERTITEM, (WPARAM)pfi, MAKELPARAM(pt.x, pt.y));

            GlobalUnlock(stgmed.hGlobal);

            // release the data using the COM API
            ReleaseStgMedium(&stgmed);
        }
    }
}

HRESULT RegisterDropWindow(HWND hwnd, IDropTarget** ppDropTarget)
{
    CDropTarget* pDropTarget = new CDropTarget(hwnd);

    // acquire a strong lock
    HRESULT hr = CoLockObjectExternal(pDropTarget, TRUE, FALSE);

    // tell OLE that the window is a drop target
    RegisterDragDrop(hwnd, pDropTarget);

    *ppDropTarget = pDropTarget;

    return hr;
}

HRESULT UnregisterDropWindow(HWND hwnd, IDropTarget* pDropTarget)
{
    // remove drag+drop
    RevokeDragDrop(hwnd);

    // remove the strong lock
    HRESULT hr = CoLockObjectExternal(pDropTarget, FALSE, TRUE);

    // release our own reference
    pDropTarget->Release();

    return hr;
}

HRESULT CreateEnumFormatEtc(UINT cfmt, FORMATETC* afmt, IEnumFORMATETC** ppEnumFormatEtc)
{
    if (cfmt == 0 || afmt == 0 || ppEnumFormatEtc == 0)
        return E_INVALIDARG;

    *ppEnumFormatEtc = new CEnumFormatEtc(afmt, cfmt);

    return (*ppEnumFormatEtc) ? S_OK : E_OUTOFMEMORY;
}

static void DeepCopyFormatEtc(FORMATETC* dest, FORMATETC* source)
{
    // copy the source FORMATETC into dest
    *dest = *source;

    if (source->ptd)
    {
        // allocate memory for the DVTARGETDEVICE if necessary
        dest->ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));

        // copy the contents of the source DVTARGETDEVICE into dest->ptd
        if (dest->ptd != NULL)
            *(dest->ptd) = *(source->ptd);
    }
}

HRESULT CreateDropSource(IDropSource** ppDropSource)
{
    if (ppDropSource == 0)
        return E_INVALIDARG;

    *ppDropSource = new CDropSource();

    return (*ppDropSource) ? S_OK : E_OUTOFMEMORY;

}

HANDLE StringToHandle(wchar_t* szText, int nTextLen)
{
    // if text length is -1 then treat as a nul-terminated string
    if (nTextLen == -1)
        nTextLen = lstrlen(szText);

    // allocate and lock a global memory buffer. Make it fixed data so we don't have to use GlobalLock
    char* ptr = (char*)GlobalAlloc(GMEM_FIXED, nTextLen + 1);
    //GlobalLock(ptr);

    // copy the string into the buffer
    if (ptr != NULL)
    {
        memcpy(ptr, szText, nTextLen);
        ptr[nTextLen] = '\0';
        //GlobalUnlock(ptr);
    }

    return ptr;
}

HGLOBAL DupMem(HGLOBAL hMem)
{
    // lock the source memory object
    SIZE_T   len = GlobalSize(hMem);
    PVOID   source = GlobalLock(hMem);

    // create a fixed "global" block - i.e. just
    // a regular lump of our process heap
    PVOID   dest = GlobalAlloc(GMEM_FIXED, len);

    if (dest!= NULL && source != NULL)
        memcpy(dest, source, len);

    GlobalUnlock(hMem);

    return dest;
}

//==================================================================================================
// CDropSource
//==================================================================================================

//	Constructor
CDropSource::CDropSource()
{
    m_lRefCount = 1;
    pDragSourceNotify = new CDragSourceNotify();
    pDragSourceNotify->AddRef();

}

//	Destructor
CDropSource::~CDropSource()
{
    if (pDragSourceNotify)
    {
        pDragSourceNotify->Release();
        pDragSourceNotify = nullptr;
    }

}

//	IUnknown::AddRef
ULONG __stdcall CDropSource::AddRef(void)
{
    // increment object reference count
    return InterlockedIncrement(&m_lRefCount);
}

//	IUnknown::Release
ULONG __stdcall CDropSource::Release(void)
{
    // decrement object reference count
    LONG count = InterlockedDecrement(&m_lRefCount);

    if (count == 0)
    {
        delete this;
        return 0;
    }
    else
    {
        return count;
    }
}

//	IUnknown::QueryInterface
HRESULT __stdcall CDropSource::QueryInterface(REFIID iid, void** ppvObject)
{
    if (!ppvObject)
    {
        return E_POINTER;
    }

    if (iid == IID_IUnknown)
    {
        *ppvObject = static_cast<IDropSource*>(this);
    }
    else if (IsEqualIID(iid, IID_IDropSource))
    {
        *ppvObject = static_cast<IDropSource*>(this);
    }
    else if (IsEqualIID(iid, IID_IDropSourceNotify) && pDragSourceNotify)
    {
        return pDragSourceNotify->QueryInterface(iid, ppvObject);
    }
    else
    {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;

    //// check to see what interface has been requested
    //if (iid == IID_IDropSource || iid == IID_IUnknown)
    //{
    //    AddRef();
    //    *ppvObject = this;
    //    return S_OK;
    //}
    //else
    //{
    //    *ppvObject = 0;
    //    return E_NOINTERFACE;
    //}
}

//	CDropSource::QueryContinueDrag - Called by OLE whenever Escape/Control/Shift/Mouse buttons have changed
HRESULT __stdcall CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    // if the <Escape> key has been pressed since the last call, cancel the drop
    if (fEscapePressed == TRUE)
        return DRAGDROP_S_CANCEL;

    // if the <LeftMouse> button has been released, then do the drop!
    if ((grfKeyState & MK_LBUTTON) == 0)
        return DRAGDROP_S_DROP;

    // continue with the drag-drop
    return S_OK;
}

//	CDropSource::GiveFeedback - Return either S_OK, or DRAGDROP_S_USEDEFAULTCURSORS to instruct OLE to use the default mouse cursor images
HRESULT __stdcall CDropSource::GiveFeedback(DWORD dwEffect)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;
}


//==================================================================================================
// CDropTarget
//==================================================================================================

//	Constructor for the CDropTarget class
CDropTarget::CDropTarget(HWND hwnd)
{
    m_lRefCount = 1;
    m_hWnd = hwnd;
    m_fAllowDrop = false;
    m_pDataObject = NULL;
}

//	Destructor for the CDropTarget class
CDropTarget::~CDropTarget()
{

}

//	Position the edit control's caret under the mouse
void PositionCursor(HWND hwndEdit, POINTL pt)
{
    //LRESULT curpos;

    // get the character position of mouse
    ScreenToClient(hwndEdit, (POINT*)&pt);
    SendMessage(hwndEdit, WM_DDD_FOLDERS, 0, MAKELPARAM(pt.x, pt.y));
    //curpos = SendMessage(hwndEdit, EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));

    // set cursor position
    //SendMessage(hwndEdit, EM_SETSEL, LOWORD(curpos), LOWORD(curpos));
}

//	IUnknown::QueryInterface
HRESULT __stdcall CDropTarget::QueryInterface(REFIID iid, void** ppvObject)
{
    if (iid == IID_IDropTarget || iid == IID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

//	IUnknown::AddRef
ULONG __stdcall CDropTarget::AddRef(void)
{
    return InterlockedIncrement(&m_lRefCount);
}

//	IUnknown::Release
ULONG __stdcall CDropTarget::Release(void)
{
    LONG count = InterlockedDecrement(&m_lRefCount);

    if (count == 0)
    {
        delete this;
        return 0;
    }
    else
    {
        return count;
    }
}

//	QueryDataObject private helper routine
bool CDropTarget::QueryDataObject(IDataObject* pDataObject)
{
    //FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    FORMATETC fmtetc = { FOLDER_FRMT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

    // does the data object support CF_TEXT using a HGLOBAL?
    return pDataObject->QueryGetData(&fmtetc) == S_OK ? true : false;
}

//	DropEffect private helper routine
DWORD CDropTarget::DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed)
{
    DWORD dwEffect = 0;

    // 1. check "pt" -> do we allow a drop at the specified coordinates?

    // 2. work out that the drop-effect should be based on grfKeyState
    if (grfKeyState & MK_CONTROL)
    {
        dwEffect = dwAllowed & DROPEFFECT_COPY;
    }
    else if (grfKeyState & MK_SHIFT)
    {
        dwEffect = dwAllowed & DROPEFFECT_MOVE;
    }

    // 3. no key-modifiers were specified (or drop effect not allowed), so
    //    base the effect on those allowed by the dropsource
    if (dwEffect == 0)
    {
        if (dwAllowed & DROPEFFECT_COPY) dwEffect = DROPEFFECT_COPY;
        if (dwAllowed & DROPEFFECT_MOVE) dwEffect = DROPEFFECT_MOVE;
    }

    return dwEffect;
}

//	IDropTarget::DragEnter
HRESULT __stdcall CDropTarget::DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    // does the dataobject contain data we want?
    m_fAllowDrop = QueryDataObject(pDataObject);

    if (m_fAllowDrop)
    {
        // get the dropeffect based on keyboard state
        *pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);

        SetFocus(m_hWnd);

        PositionCursor(m_hWnd, pt);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }

    return S_OK;
}

//	IDropTarget::DragOver
HRESULT __stdcall CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (m_fAllowDrop)
    {
        *pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
        PositionCursor(m_hWnd, pt);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }

    return S_OK;
}

//	IDropTarget::DragLeave
HRESULT __stdcall CDropTarget::DragLeave(void)
{
    SendMessage(m_hWnd, WM_DDD_FOLDERS, 1, NULL);
    return S_OK;
}

//	IDropTarget::Drop
HRESULT __stdcall CDropTarget::Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    PositionCursor(m_hWnd, pt);

    if (m_fAllowDrop)
    {
        DropData(m_hWnd, pDataObject, pt);

        *pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }

    return S_OK;
}


//==================================================================================================
// CDataObject
//==================================================================================================

//	Constructor
CDataObject::CDataObject(FORMATETC* fmtetc, STGMEDIUM* stgmed, int count)
{
    m_lRefCount = 1;
    m_nNumFormats = count;

    m_pFormatEtc = new FORMATETC[count];
    m_pStgMedium = new STGMEDIUM[count];

    for (int i = 0; i < count; i++)
    {
        m_pFormatEtc[i] = fmtetc[i];
        m_pStgMedium[i] = stgmed[i];
    }
}

//	Destructor
CDataObject::~CDataObject()
{
    // cleanup
    if (m_pFormatEtc) delete[] m_pFormatEtc;
    if (m_pStgMedium) delete[] m_pStgMedium;
}

//	IUnknown::AddRef
ULONG __stdcall CDataObject::AddRef(void)
{
    // increment object reference count
    return InterlockedIncrement(&m_lRefCount);
}

//	IUnknown::Release
ULONG __stdcall CDataObject::Release(void)
{
    // decrement object reference count
    LONG count = InterlockedDecrement(&m_lRefCount);

    if (count == 0)
    {
        delete this;
        return 0;
    }
    else
    {
        return count;
    }
}

//	IUnknown::QueryInterface
HRESULT __stdcall CDataObject::QueryInterface(REFIID iid, void** ppvObject)
{
    // check to see what interface has been requested
    if (iid == IID_IDataObject || iid == IID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

int CDataObject::LookupFormatEtc(FORMATETC* pFormatEtc)
{
    for (int i = 0; i < m_nNumFormats; i++)
    {
        if ((pFormatEtc->tymed & m_pFormatEtc[i].tymed) &&
            pFormatEtc->cfFormat == m_pFormatEtc[i].cfFormat &&
            pFormatEtc->dwAspect == m_pFormatEtc[i].dwAspect)
        {
            return i;
        }
    }

    return -1;

}

//	IDataObject::GetData
HRESULT __stdcall CDataObject::GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
{
    int idx;

    //
    // try to match the requested FORMATETC with one of our supported formats
    //
    if ((idx = LookupFormatEtc(pFormatEtc)) == -1)
    {
        return DV_E_FORMATETC;
    }

    //
    // found a match! transfer the data into the supplied storage-medium
    //
    pMedium->tymed = m_pFormatEtc[idx].tymed;
    pMedium->pUnkForRelease = 0;

    switch (m_pFormatEtc[idx].tymed)
    {
    case TYMED_HGLOBAL:

        pMedium->hGlobal = DupMem(m_pStgMedium[idx].hGlobal);
        //return S_OK;
        break;

    default:
        return DV_E_FORMATETC;
    }

    return S_OK;
}

//	IDataObject::GetDataHere
HRESULT __stdcall CDataObject::GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
{
    // GetDataHere is only required for IStream and IStorage mediums
    // It is an error to call GetDataHere for things like HGLOBAL and other clipboard formats
    //
    //	OleFlushClipboard 
    //
    return DATA_E_FORMATETC;
}

//	IDataObject::QueryGetData
HRESULT __stdcall CDataObject::QueryGetData(FORMATETC* pFormatEtc)//	Called to see if the IDataObject supports the specified format of data
{
    return (LookupFormatEtc(pFormatEtc) == -1) ? DV_E_FORMATETC : S_OK;
}

//	IDataObject::GetCanonicalFormatEtc
HRESULT __stdcall CDataObject::GetCanonicalFormatEtc(FORMATETC* pFormatEct, FORMATETC* pFormatEtcOut)
{
    // Apparently we have to set this field to NULL even though we don't do anything else
    pFormatEtcOut->ptd = NULL;
    return E_NOTIMPL;
}

//	IDataObject::SetData
HRESULT __stdcall CDataObject::SetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium, BOOL fRelease)
{
    return E_NOTIMPL;
}

//	IDataObject::EnumFormatEtc
HRESULT __stdcall CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc)
{
    if (dwDirection == DATADIR_GET)
    {
        // for Win2k+ you can use the SHCreateStdEnumFmtEtc API call, however
        // to support all Windows platforms we need to implement IEnumFormatEtc ourselves.
        return CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);
    }
    else
    {
        // the direction specified is not support for drag+drop
        return E_NOTIMPL;
    }
}

//	IDataObject::DAdvise
HRESULT __stdcall CDataObject::DAdvise(FORMATETC* pFormatEtc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

//	IDataObject::DUnadvise
HRESULT __stdcall CDataObject::DUnadvise(DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

//	IDataObject::EnumDAdvise
HRESULT __stdcall CDataObject::EnumDAdvise(IEnumSTATDATA** ppEnumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

//======================================================================================================
// CreateEnumFormatEtc 
//======================================================================================================

CEnumFormatEtc::CEnumFormatEtc(FORMATETC* pFormatEtc, int nNumFormats)
{
    m_lRefCount = 1;
    m_nIndex = 0;
    m_nNumFormats = nNumFormats;
    m_pFormatEtc = new FORMATETC[nNumFormats];

    // copy the FORMATETC structures
    for (int i = 0; i < nNumFormats; i++)
    {
        DeepCopyFormatEtc(&m_pFormatEtc[i], &pFormatEtc[i]);
    }
}

CEnumFormatEtc::~CEnumFormatEtc()
{
    if (m_pFormatEtc)
    {
        for (ULONG i = 0; i < m_nNumFormats; i++)
        {
            if (m_pFormatEtc[i].ptd)
                CoTaskMemFree(m_pFormatEtc[i].ptd);
        }

        delete[] m_pFormatEtc;
    }
}

ULONG __stdcall CEnumFormatEtc::AddRef(void)
{
    // increment object reference count
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall CEnumFormatEtc::Release(void)
{
    // decrement object reference count
    LONG count = InterlockedDecrement(&m_lRefCount);

    if (count == 0)
    {
        delete this;
        return 0;
    }
    else
    {
        return count;
    }
}

HRESULT __stdcall CEnumFormatEtc::QueryInterface(REFIID iid, void** ppvObject)
{
    // check to see what interface has been requested
    if (iid == IID_IEnumFORMATETC || iid == IID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

//	If the returned FORMATETC structure contains a non-null "ptd" member, then
//  the caller must free this using CoTaskMemFree (stated in the COM documentation)
HRESULT __stdcall CEnumFormatEtc::Next(ULONG celt, FORMATETC* pFormatEtc, ULONG* pceltFetched)
{
    ULONG copied = 0;

    // validate arguments
    if (celt == 0 || pFormatEtc == 0)
        return E_INVALIDARG;

    // copy FORMATETC structures into caller's buffer
    while (m_nIndex < m_nNumFormats && copied < celt)
    {
        DeepCopyFormatEtc(&pFormatEtc[copied], &m_pFormatEtc[m_nIndex]);
        copied++;
        m_nIndex++;
    }

    // store result
    if (pceltFetched != 0)
        *pceltFetched = copied;

    // did we copy all that was requested?
    return (copied == celt) ? S_OK : S_FALSE;
}

HRESULT __stdcall CEnumFormatEtc::Skip(ULONG celt)
{
    m_nIndex += celt;
    return (m_nIndex <= m_nNumFormats) ? S_OK : S_FALSE;
}

HRESULT __stdcall CEnumFormatEtc::Reset(void)
{
    m_nIndex = 0;
    return S_OK;
}

HRESULT __stdcall CEnumFormatEtc::Clone(IEnumFORMATETC** ppEnumFormatEtc)
{
    HRESULT hResult;

    // make a duplicate enumerator
    hResult = CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);

    if (hResult == S_OK)
    {
        // manually set the index state
        ((CEnumFormatEtc*)*ppEnumFormatEtc)->m_nIndex = m_nIndex;
    }

    return hResult;
}
