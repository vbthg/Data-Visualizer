// src/Utils/FileDropManager.cpp
#ifdef _WIN32
#include "FileDropManager.h"
#include <windows.h>
#include <ole2.h>
#include <shellapi.h>
#include <mutex>

static bool g_isDragging = false;
static sf::Vector2f g_hoverPosition(0.f, 0.f);
static std::vector<std::string> g_droppedFiles;
static std::mutex g_mutex;
static HWND g_hwnd = NULL;

class DropTargetImpl : public IDropTarget
{
private:
    ULONG m_refCount;

public:
    DropTargetImpl() : m_refCount(1)
    {
    }

    virtual ~DropTargetImpl()
    {
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (riid == IID_IUnknown || riid == IID_IDropTarget)
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return InterlockedIncrement(&m_refCount);
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG count = InterlockedDecrement(&m_refCount);
        if (count == 0)
        {
            delete this;
        }
        return count;
    }

    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_isDragging = true;
        updateHoverPosition(pt);
        *pdwEffect = DROPEFFECT_COPY;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        updateHoverPosition(pt);
        *pdwEffect = DROPEFFECT_COPY;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragLeave() override
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_isDragging = false;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_isDragging = false;
        updateHoverPosition(pt);

        FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM medium;

        if (SUCCEEDED(pDataObj->GetData(&fmt, &medium)))
        {
            HDROP hDrop = (HDROP)GlobalLock(medium.hGlobal);
            if (hDrop)
            {
                UINT fileCount = DragQueryFileA(hDrop, 0xFFFFFFFF, NULL, 0);
                for (UINT i = 0; i < fileCount; ++i)
                {
                    char filePath[MAX_PATH];
                    if (DragQueryFileA(hDrop, i, filePath, MAX_PATH))
                    {
                        g_droppedFiles.push_back(std::string(filePath));
                    }
                }
                DragFinish(hDrop);
                GlobalUnlock(medium.hGlobal);
            }
            ReleaseStgMedium(&medium);
        }

        *pdwEffect = DROPEFFECT_COPY;
        return S_OK;
    }

private:
    void updateHoverPosition(POINTL pt)
    {
        POINT p = { pt.x, pt.y };
        ScreenToClient(g_hwnd, &p);
        g_hoverPosition.x = static_cast<float>(p.x);
        g_hoverPosition.y = static_cast<float>(p.y);
    }
};

static DropTargetImpl* g_dropTarget = nullptr;

void FileDropManager::init(sf::WindowHandle handle)
{
    g_hwnd = static_cast<HWND>(handle);
    OleInitialize(NULL);
    g_dropTarget = new DropTargetImpl();
    RegisterDragDrop(g_hwnd, g_dropTarget);
}

void FileDropManager::shutdown()
{
    if (g_hwnd)
    {
        RevokeDragDrop(g_hwnd);
    }
    if (g_dropTarget)
    {
        g_dropTarget->Release();
        g_dropTarget = nullptr;
    }
    OleUninitialize();
}

bool FileDropManager::isDragging()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_isDragging;
}

sf::Vector2f FileDropManager::getHoverPosition()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_hoverPosition;
}

bool FileDropManager::hasDroppedFiles()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return !g_droppedFiles.empty();
}

std::vector<std::string> FileDropManager::popDroppedFiles()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    std::vector<std::string> files = g_droppedFiles;
    g_droppedFiles.clear();
    return files;
}
#endif
