#if 0
#include "DumpFilter.hpp"
#include "Timer.hpp"
#include "VideoRenderer.hpp"
#include <iostream>

GUID guidType;
int width = 0;
int height = 0;
bool noUpdate = false;

DumpFilter::DumpFilter(Dump *pDump,
                       LPUNKNOWN pUnk,
                       CCritSec *pLock,
                       HRESULT *phr) 
                       : CBaseFilter(NAME("DumpFilter"), pUnk, pLock, CLSID_Dump),
                         m_pDump(pDump)
{
}

CBasePin * DumpFilter::GetPin(int n)
{
   std::cout << "DumpFilter::GetPin " << n << "\n";
    return m_pDump->m_pPin;
}

int DumpFilter::GetPinCount()
{
    return 1;
}

STDMETHODIMP DumpFilter::Stop()
{
   std::cout << "DumpFilter::Stop\n";

    VideoRenderer *renderer = m_pDump->getRenderer();
    if (renderer) {
        if (renderer->isVideoRendererOpen())
            renderer->closeVideoRenderer();
    }
    return CBaseFilter::Stop();
}

STDMETHODIMP DumpFilter::Pause()
{
    return CBaseFilter::Pause();
}

STDMETHODIMP DumpFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock cObjectLock(m_pLock);
    //printf("Dump filter Run()\n");
    return CBaseFilter::Run(tStart);
}

DumpInputPin::DumpInputPin(Dump *pDump,
                           LPUNKNOWN pUnk,
                           CBaseFilter *pFilter,
                           CCritSec *pLock,
                           CCritSec *pReceiveLock,
                           HRESULT *phr) 
                           :

    CRenderedInputPin(NAME("DumpInputPin"),
                           pFilter,     // Filter
                           pLock,       // Locking
                           phr,         // Return code
                           L"In"),   // Pin name
    m_pReceiveLock(pReceiveLock),
    m_pDump(pDump),
    m_tLast(0)
{
}

HRESULT DumpInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = CRenderedInputPin::CompleteConnect(pReceivePin);
    return hr;
}

HRESULT DumpInputPin::SetMediaType(const CMediaType *pType)
{
    HRESULT hr = CRenderedInputPin::SetMediaType(pType);
    if (!FAILED(hr))
    {
        std::cout << "DumpFilter " << GuidNames[pType->subtype] << "\n";
        guidType = pType->subtype;
        if (noUpdate)  // if samples are not handled
            noUpdate = false;
        if (pType->formattype == FORMAT_VideoInfo) {
            VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(pType->pbFormat);
            width = pVih->bmiHeader.biWidth;
            height = pVih->bmiHeader.biHeight;
//            VideoRenderer *renderer = m_pDump->getRenderer();
//            if (renderer) {
//                renderer->openVideoRenderer(width, height, PIX_FMT_YUYV422, 1);

//            }
        }
        else if (pType->formattype == FORMAT_VideoInfo2) {
            VIDEOINFOHEADER2 *pVih2 = reinterpret_cast<VIDEOINFOHEADER2*>(pType->pbFormat);
            width = pVih2->bmiHeader.biWidth;
            height = pVih2->bmiHeader.biHeight;
//            VideoRenderer *renderer = m_pDump->getRenderer();
//            if (renderer) {
//                float aspect = 1;
//                if (pVih2->dwPictAspectRatioY != 0)
//                    aspect = pVih2->dwPictAspectRatioX / (float)pVih2->dwPictAspectRatioY;
//                renderer->openVideoRenderer(width, height, PIX_FMT_YUYV422, aspect);
//            }
        }   
    }
    return hr;
}
HRESULT DumpInputPin::CheckMediaType(const CMediaType *pType)
{
    if (pType->subtype == MEDIASUBTYPE_YUY2)
        return S_OK;
    return S_FALSE;
}

HRESULT DumpInputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    pMediaType->subtype = MEDIASUBTYPE_YUY2;
    return CRenderedInputPin::GetMediaType(iPosition, pMediaType);
}

HRESULT __stdcall DumpInputPin::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
    // TODO: use lock to prevent receive call ???
    HRESULT hr;
    if (m_Connected == pConnector) 
    {
        // Dynamic change
        printf("ReceiveConnection: Dynamic change\n");
        hr = CheckMediaType(static_cast<const CMediaType*>(pmt));
        if (FAILED(hr)) 
            return hr;
        
        BITMAPINFO bmpInfo = {0};
  
        if (pmt->formattype == FORMAT_VideoInfo) {
            VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
            bmpInfo.bmiHeader = pVih->bmiHeader;
        }
        else if (pmt->formattype == FORMAT_VideoInfo2) {
            VIDEOINFOHEADER2 *pVih2 = reinterpret_cast<VIDEOINFOHEADER2*>(pmt->pbFormat);
            bmpInfo.bmiHeader = pVih2->bmiHeader;
        }
        else
            return E_UNEXPECTED;

        ALLOCATOR_PROPERTIES allocProperties = {0};
        
        hr = m_pAllocator->GetProperties(&allocProperties);
        if (FAILED(hr))
            return hr;
        allocProperties.cbBuffer = bmpInfo.bmiHeader.biSizeImage;

        hr = SetMediaType(static_cast<const CMediaType*>(pmt));
        if (FAILED(hr))
            return hr;

        hr = m_pAllocator->Decommit();
        if (FAILED(hr))
            return hr;

        ALLOCATOR_PROPERTIES newAllocProperties = {0};
        hr = m_pAllocator->SetProperties(&allocProperties, &newAllocProperties);
        if (FAILED(hr))
            return hr;

        hr = m_pAllocator->Commit();
        if (FAILED(hr))
            return hr;
        
        noUpdate = true;
        m_pFilter->NotifyEvent(EC_USER, 0 ,0);
    }
    else
        hr =  CRenderedInputPin::ReceiveConnection(pConnector, pmt);

    return hr;
}

HRESULT DumpInputPin::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
    return CRenderedInputPin::NotifyAllocator(pAllocator, bReadOnly);
}

HRESULT DumpInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
    return CRenderedInputPin::GetAllocatorRequirements(pProps);
}

HRESULT DumpInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    return CRenderedInputPin::GetAllocator(ppAllocator);
}

HRESULT DumpInputPin::BreakConnect()
{
    if (m_pDump->m_pPosition != NULL)
        m_pDump->m_pPosition->ForceRefresh();
    return CRenderedInputPin::BreakConnect();
}

STDMETHODIMP DumpInputPin::ReceiveCanBlock()
{
    return S_OK;
}

STDMETHODIMP DumpInputPin::Receive(IMediaSample *pSample)
{
    CheckPointer(pSample, E_POINTER);

    CAutoLock lock(m_pReceiveLock);
    PBYTE pData;

    VideoRenderer *renderer = m_pDump->getRenderer();
    AM_MEDIA_TYPE *pMediaType = NULL;

    if (noUpdate)  // not needed?
        return S_OK;

    REFERENCE_TIME tStart, tStop;
    pSample->GetTime(&tStart, &tStop);
    printf("tStart(%d), tStop(%d), Diff(%d ms), Bytes(%d)\n",
          (LONG)tStart,(LONG)tStop,
		  (LONG)((tStart - m_tLast) / 10000),
		  pSample->GetActualDataLength());

    HRESULT hr = pSample->GetMediaType(&pMediaType);
    if (pMediaType) {
        printf("Receive() detect media change\n");
        if (pMediaType->formattype == FORMAT_VideoInfo2) {
            VIDEOINFOHEADER2 *pVih2 = reinterpret_cast<VIDEOINFOHEADER2*>(pMediaType->pbFormat);
            VIDEOINFOHEADER2 *pVih2_old = reinterpret_cast<VIDEOINFOHEADER2*>(m_mt.pbFormat);
            if (pVih2->dwPictAspectRatioX != pVih2_old->dwPictAspectRatioX
                && pVih2->dwPictAspectRatioY != pVih2_old->dwPictAspectRatioY) {
                    if (renderer)
                        renderer->changeAspectRatio(pVih2->dwPictAspectRatioX / (float) pVih2->dwPictAspectRatioY);
            }
            else {
                // something else changed
                int halt = 0;
            }

        }
        m_mt = *pMediaType;
        DeleteMediaType(pMediaType);
    }
    m_tLast = tStart;

    pSample->GetPointer(&pData);
    printf("Data length: %d\n", pSample->GetActualDataLength());
    // YUY2
	int linesize[4] = {width, 0, 0, 0};
	uint8_t* data[4] = {pData, NULL, NULL, NULL};

    if (renderer)
        renderer->renderVideo(data, linesize, NULL);

    return S_OK;
}

STDMETHODIMP DumpInputPin::EndOfStream(void)
{
    CAutoLock lock(m_pReceiveLock);
    return CRenderedInputPin::EndOfStream();
}

STDMETHODIMP DumpInputPin::NewSegment(REFERENCE_TIME tStart,
                                      REFERENCE_TIME tStop,
                                      double dRate)
{
    m_tLast = 0;
    return S_OK;
}

Dump::Dump(VideoRenderer *videoRenderer, LPUNKNOWN pUnk, HRESULT *phr)
    : CUnknown(NAME("Dump"), pUnk),
    m_pFilter(NULL),
    m_pPin(NULL),
    m_pPosition(NULL)
{
    ASSERT(phr);
    mVideoRenderer = videoRenderer;
    m_pFilter = new DumpFilter(this, GetOwner(), &m_Lock, phr);
    if (!m_pFilter) {
        if (phr)
            *phr = E_OUTOFMEMORY;
        return;
    }

    m_pPin = new DumpInputPin(this,GetOwner(),
                               m_pFilter,
                               &m_Lock,
                               &m_ReceiveLock,
                               phr);

    if (!m_pPin) {
        if (phr)
            *phr = E_OUTOFMEMORY;
        return;
    }
}

Dump::~Dump()
{
    delete m_pPin;
    delete m_pFilter;
    delete m_pPosition;
}

CUnknown * WINAPI Dump::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{

    ASSERT(phr);
    
    //Dump *pNewObject = new Dump(punk, phr);
    //if (!pNewObject) {
    //    if (phr)
    //        *phr = E_OUTOFMEMORY;
    //}

    return NULL;//pNewObject;
}

STDMETHODIMP Dump::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);
    CAutoLock lock(&m_Lock);

    if (riid == IID_IBaseFilter || riid == IID_IMediaFilter || riid == IID_IPersist) {
        return m_pFilter->NonDelegatingQueryInterface(riid, ppv);
    } 
    else if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {
        if (m_pPosition == NULL) 
        {

            HRESULT hr = S_OK;
            m_pPosition = new CPosPassThru(NAME("Dump Pass Through"),
                                           (IUnknown *) GetOwner(),
                                           (HRESULT *) &hr, m_pPin);
            if (m_pPosition == NULL) 
                return E_OUTOFMEMORY;

            if (FAILED(hr)) 
            {
                delete m_pPosition;
                m_pPosition = NULL;
                return hr;
            }
        }

        return m_pPosition->NonDelegatingQueryInterface(riid, ppv);
    } 

    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}

#endif
