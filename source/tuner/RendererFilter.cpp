#include "RendererFilter.hpp"
#include "VideoRenderer.hpp"

RendererFilter::RendererFilter(VideoRenderer* renderer, LPUNKNOWN pUnk, HRESULT* phr)
    : CBaseRenderer(CLSID_RendererFilter, NAME("RendererFilter"), pUnk, phr)
    , m_videoRenderer(renderer)
{
    m_width = 0;
    m_height = 0;
}

RendererFilter::~RendererFilter()
{
}

CUnknown* WINAPI RendererFilter::CreateInstance(LPUNKNOWN punk, HRESULT* phr)
{
    RendererFilter* pNewObject = new RendererFilter(NULL, punk, phr);
    if (!pNewObject) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }
    return pNewObject;
}

STDMETHODIMP RendererFilter::Stop()
{
    // lock?
    if (m_videoRenderer) {
        // if (m_videoRenderer->isVideoRendererOpen())
        //      m_videoRenderer->closeVideoRenderer();
    }
    return CBaseRenderer::Stop();
}

STDMETHODIMP RendererFilter::Pause()
{
    return CBaseRenderer::Pause();
}

STDMETHODIMP RendererFilter::Run(REFERENCE_TIME tStart)
{
    return CBaseRenderer::Run(tStart);
}

STDMETHODIMP RendererFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    if (riid == IID_IFileSinkFilter) {
        return GetInterface((IFileSinkFilter*)this, ppv);
    } else if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {
        return GetMediaPositionInterface(riid, ppv);
    }

    return CBaseRenderer::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT RendererFilter::ReceiveConnection(IPin* pConnector, const AM_MEDIA_TYPE* pmt)
{
    HRESULT hr = E_UNEXPECTED;

    if (m_pInputPin->GetConnected() == pConnector) {
        // Dynamic change
        // printf("ReceiveConnection: Dynamic change\n");
        hr = CheckMediaType(static_cast<const CMediaType*>(pmt));
        if (FAILED(hr))
            return hr;

        BITMAPINFO bmpInfo = { 0 };

        if (pmt->formattype == FORMAT_VideoInfo) {
            VIDEOINFOHEADER* pVih = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
            bmpInfo.bmiHeader = pVih->bmiHeader;
        } else if (pmt->formattype == FORMAT_VideoInfo2) {
            VIDEOINFOHEADER2* pVih2 = reinterpret_cast<VIDEOINFOHEADER2*>(pmt->pbFormat);
            // printf("width: %d height: %d\n", pVih2->bmiHeader.biWidth, pVih2->bmiHeader.biHeight);
            bmpInfo.bmiHeader = pVih2->bmiHeader;
        } else
            return E_UNEXPECTED;

        hr = SetMediaType(static_cast<const CMediaType*>(pmt));
        if (FAILED(hr))
            return hr;

        ALLOCATOR_PROPERTIES allocProperties = { 0 };

        hr = m_pInputPin->Allocator()->GetProperties(&allocProperties);
        if (FAILED(hr))
            return hr;
        allocProperties.cbBuffer = bmpInfo.bmiHeader.biSizeImage;

        hr = m_pInputPin->Allocator()->Decommit();
        if (FAILED(hr))
            return hr;

        ALLOCATOR_PROPERTIES newAllocProperties = { 0 };
        hr = m_pInputPin->Allocator()->SetProperties(&allocProperties, &newAllocProperties);
        if (FAILED(hr))
            return hr;

        hr = m_pInputPin->Allocator()->Commit();
        if (FAILED(hr))
            return hr;

        //NotifyEvent(EC_USER, 0 ,0);
    }

    return hr;
}

HRESULT RendererFilter::CheckMediaType(const CMediaType* pmt)
{
    // printf("CheckMediaType() %s\n", GuidNames[pmt->subtype]);
    if (pmt->subtype == MEDIASUBTYPE_YUY2)
        return S_OK;
    return S_FALSE;
}

HRESULT RendererFilter::SetMediaType(const CMediaType* pmt)
{
    //printf("SetMediaType() %s\n", GuidNames[pmt->subtype]);
    HRESULT hr = CBaseRenderer::SetMediaType(pmt);
    if (!FAILED(hr)) {
        if (pmt->formattype == FORMAT_VideoInfo) {
            VIDEOINFOHEADER* pVih = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
            m_width = pVih->bmiHeader.biWidth;
            m_height = pVih->bmiHeader.biHeight;
            // if (m_videoRenderer) {
            //     m_videoRenderer->openVideoRenderer(m_width, m_height, AV_PIX_FMT_YUYV422, 1);
            //  }
        } else if (pmt->formattype == FORMAT_VideoInfo2) {
            VIDEOINFOHEADER2* pVih2 = reinterpret_cast<VIDEOINFOHEADER2*>(pmt->pbFormat);
            m_width = pVih2->bmiHeader.biWidth;
            m_height = pVih2->bmiHeader.biHeight;
            // printf("width: %d height %d\n", m_width, m_height);
            if (m_videoRenderer) {
                float aspect = 1;
                if (pVih2->dwPictAspectRatioY != 0)
                    aspect = pVih2->dwPictAspectRatioX / (float)pVih2->dwPictAspectRatioY;
                //     m_videoRenderer->openVideoRenderer(m_width, m_height, AV_PIX_FMT_YUYV422, aspect);
            }
        }
    }
    return hr;
}

HRESULT RendererFilter::DoRenderSample(IMediaSample* pMediaSample)
{
    CheckPointer(pMediaSample, E_POINTER);

    CAutoLock lock(&m_Lock);

    PBYTE pbData;
    REFERENCE_TIME tStart, tStop;

    pMediaSample->GetTime(&tStart, &tStop);
    /*
    printf("tStart(%d), tStop(%d), Diff(%d ms), Bytes(%d)\n",
          (LONG)tStart,(LONG)tStop,
		  (LONG)((tStart - m_tLast) / 10000),
		  pMediaSample->GetActualDataLength());
    */
    m_tLast = tStart;

    AM_MEDIA_TYPE* pMediaType = NULL;
    pMediaSample->GetMediaType(&pMediaType);
    if (pMediaType) {
        // std::cout << "DoRenderSample() Media Type Change\n";
        if (pMediaType->formattype == FORMAT_VideoInfo2) {
            VIDEOINFOHEADER2* pVih2 = reinterpret_cast<VIDEOINFOHEADER2*>(pMediaType->pbFormat);
            m_width = pVih2->bmiHeader.biWidth;
            m_height = pVih2->bmiHeader.biHeight;
            if (m_videoRenderer) {
                if (pVih2->dwPictAspectRatioY != 0)
                    m_videoRenderer->changeAspectRatio(pVih2->dwPictAspectRatioX / (float)pVih2->dwPictAspectRatioY);
            }
        }
        DeleteMediaType(pMediaType);
    }

    HRESULT hr = pMediaSample->GetPointer(&pbData);

    int linesize[4] = { m_width, 0, 0, 0 };
    uint8_t* data[4] = { pbData, NULL, NULL, NULL };

    if (m_videoRenderer)
        m_videoRenderer->renderVideo(data, linesize, NULL);

    return hr;
}

