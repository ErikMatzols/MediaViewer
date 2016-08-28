#ifndef DECODERFILTER_HPP
#define DECODERFILTER_HPP

#include <windows.h>
#include <commdlg.h>
#include <streams.h>
#include <initguid.h>
#include <strsafe.h>
#include <Dvdmedia.h>
#include <memory>
//#include <ksmedia.h>
//#include <bdamedia.h>

class DumpInputPin;
class Dump;
class DumpFilter;

// {A1BCCF59-A87A-49be-BB01-A2D1D9DE97C7}
DEFINE_GUID(CLSID_Dump, 0xa1bccf59, 0xa87a, 0x49be, 0xbb, 0x1, 0xa2, 0xd1, 0xd9, 0xde, 0x97, 0xc7);

class DumpFilter : public CBaseFilter
{
public:
    DumpFilter(Dump *pDump, LPUNKNOWN pUnk, CCritSec *pLock, HRESULT *phr);
    ~DumpFilter();

    CBasePin* GetPin(int n);
    int GetPinCount();

    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP Pause();
    STDMETHODIMP Stop();

private:
    Dump* const m_dump;
};

class DumpInputPin : public CRenderedInputPin
{
public:
    DumpInputPin(Dump* pDump, LPUNKNOWN pUnk, CBaseFilter* pFilter, CCritSec* pLock, CCritSec* pReceiveLock, HRESULT* phr);
    ~DumpInputPin();

    // Do something with this media sample
    STDMETHODIMP Receive(IMediaSample *pSample);
    STDMETHODIMP EndOfStream(void);
    STDMETHODIMP ReceiveCanBlock();

    // Check if the pin can support this specific proposed type and format
    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT SetMediaType(const CMediaType*);
    HRESULT CheckMediaType(const CMediaType*);
    HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);
    HRESULT __stdcall ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE* pmt);

    // Break connection
    HRESULT BreakConnect();

    // Track NewSegment
    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

    HRESULT __stdcall NotifyAllocator(IMemAllocator* pAllocator, BOOL bReadOnly);
    HRESULT __stdcall GetAllocatorRequirements(ALLOCATOR_PROPERTIES* pProps);
    HRESULT __stdcall GetAllocator(IMemAllocator** ppAllocator);

protected:

private:
    Dump* const m_dump;
    CCritSec* const m_receiveLock;
    REFERENCE_TIME m_last;
};

class Dump : public CUnknown
{
    friend class DumpFilter;
    friend class DumpInputPin;

public:

    DECLARE_IUNKNOWN

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT* phr);

    DumpInputPin* getInputPin();

private:
    Dump(LPUNKNOWN pUnk, HRESULT *phr);
    ~Dump();

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    std::unique_ptr<DumpFilter>   m_filter;
    std::unique_ptr<DumpInputPin> m_pin;
    std::unique_ptr<CPosPassThru> m_position;

    CCritSec m_lock;
    CCritSec m_receiveLock;
};


#endif // DECODERFILTER_HPP
