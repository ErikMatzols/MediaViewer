
#ifndef DUMPFILTER_HPP
#define DUMPFILTER_HPP

#if 0
#include <windows.h>
#include <commdlg.h>
#include <streams.h>
#include <initguid.h>
#include <strsafe.h>
#include <Dvdmedia.h>

class DumpInputPin;
class Dump;
class DumpFilter;
class VideoRenderer;

// {A1BCCF59-A87A-49be-BB01-A2D1D9DE97C7}
DEFINE_GUID(CLSID_Dump, 0xa1bccf59, 0xa87a, 0x49be, 0xbb, 0x1, 0xa2, 0xd1, 0xd9, 0xde, 0x97, 0xc7);

class DumpFilter : public CBaseFilter
{
    Dump * const m_pDump;

public:
    DumpFilter(Dump *pDump, LPUNKNOWN pUnk, CCritSec *pLock, HRESULT *phr);

    CBasePin * GetPin(int n);
    int GetPinCount();

    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP Pause();
    STDMETHODIMP Stop();
};

class DumpInputPin : public CRenderedInputPin
{
    Dump * const m_pDump;
    CCritSec * const m_pReceiveLock;
    REFERENCE_TIME m_tLast;

public:
    DumpInputPin(Dump *pDump,
                 LPUNKNOWN pUnk,
                 CBaseFilter *pFilter,
                 CCritSec *pLock,
                 CCritSec *pReceiveLock,
                 HRESULT *phr);

    // Do something with this media sample
    STDMETHODIMP Receive(IMediaSample *pSample);
    STDMETHODIMP EndOfStream(void);
    STDMETHODIMP ReceiveCanBlock();

    // Check if the pin can support this specific proposed type and format
    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT SetMediaType(const CMediaType *);
    HRESULT CheckMediaType(const CMediaType *);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    HRESULT __stdcall ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
   
    // Break connection
    HRESULT BreakConnect();

    // Track NewSegment
    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

    HRESULT __stdcall NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly);
    HRESULT __stdcall GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);
    HRESULT __stdcall GetAllocator(IMemAllocator **ppAllocator);


protected:

};

class Dump : public CUnknown
{
    friend class DumpFilter;
    friend class DumpInputPin;

    DumpFilter *m_pFilter;
    DumpInputPin *m_pPin;

    CCritSec m_Lock;
    CCritSec m_ReceiveLock;

    CPosPassThru *m_pPosition;

public:

    DECLARE_IUNKNOWN

    Dump(VideoRenderer *videoRenderer, LPUNKNOWN pUnk, HRESULT *phr);
    ~Dump();

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
    VideoRenderer* getRenderer()
    {
        return mVideoRenderer;
    }

    DumpInputPin* getInputPin()
    {
        return m_pPin;
    }
private:
    VideoRenderer *mVideoRenderer;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

};
#endif
#endif  // DUMPFILER_HPP

