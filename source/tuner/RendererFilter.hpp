#ifndef RENDERERFILTER_HPP
#define RENDERERFILTER_HPP

#include <windows.h>
#include <commdlg.h>
#include <streams.h>
#include <initguid.h>
#include <strsafe.h>
#include <Dvdmedia.h>
class VideoRenderer;

DEFINE_GUID(CLSID_RendererFilter, 0x36a5f770, 0xfe4c, 0x11ce, 0xa8, 0xed, 0x00, 0xaa, 0x00, 0x2f, 0xea, 0xb5);

class RendererFilter : public CBaseRenderer
{
    CCritSec m_Lock;
    REFERENCE_TIME m_tLast;
    VideoRenderer *m_videoRenderer;
    int m_width;
    int m_height;

public:
    RendererFilter(VideoRenderer *renderer, LPUNKNOWN pUnk, HRESULT *phr);
    ~RendererFilter();

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP Pause();
    STDMETHODIMP Stop();

    HRESULT ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);

    HRESULT DoRenderSample(IMediaSample *pMediaSample);

protected:

private:
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

};

#endif
