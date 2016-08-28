#ifndef DEMUXFILTER_HPP
#define DEMUXFILTER_HPP

#include <windows.h>
#include <commdlg.h>
#include <streams.h>
#include <initguid.h>
#include <strsafe.h>
#include <Dvdmedia.h>

// {A1BCCF59-A87A-49be-BB01-A2D1D9DE97C7}
DEFINE_GUID(CLSID_DemuxFilter, 0xa1bccf59, 0xa87a, 0x49be, 0xbb, 0x1, 0xa2, 0xd1, 0xd9, 0xde, 0x97, 0xc7);

class DemuxInputPin : public CBaseInputPin
{
public:
   DECLARE_IUNKNOWN;

   HRESULT SetMediaType(const CMediaType *pmt);

   HRESULT CheckMediaType(const CMediaType *pMediaType);

   HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);

   HRESULT CompleteConnect(IPin* pPin);

   HRESULT BreakConnect(void);

   HRESULT BeginFlush();

   HRESULT EndFlush();

   HRESULT Active();

   HRESULT Inactive();

//   HRESULT ReadBytes(QWORD offset, DWORD size, BYTE *pDestination) {

//   }

   DemuxInputPin(HRESULT *phr, class DemuxFilter *pParent, LPCWSTR pPinName);

   ~DemuxInputPin();

private:
   CCritSec m_lock;
};

class DemuxFilter : public CBaseFilter
{
public:
    DECLARE_IUNKNOWN;

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    CBasePin* GetPin(int);

    int GetPinCount();

    STDMETHODIMP Stop();

    STDMETHODIMP Pause();

    STDMETHODIMP Run(REFERENCE_TIME tStart);

    DemuxFilter(LPUNKNOWN lpunk, HRESULT *phr);

    ~DemuxFilter();

private:
    DemuxInputPin* m_pin;
    CCritSec m_lock;
};

#endif // DEMUXFILTER_HPP
