#include "DecoderFilter.hpp"

#include <iostream>

DumpFilter::DumpFilter(Dump* pDump, LPUNKNOWN pUnk, CCritSec* pLock, HRESULT* phr)
   : CBaseFilter(NAME("DumpFilter"), pUnk, pLock, CLSID_Dump),
     m_dump(pDump)
{
   std::cout << "DumpFilter::DumpFilter()\n";
}

DumpFilter::~DumpFilter()
{
   std::cout << "DumpFilter::~DumpFilter()\n";
}

CBasePin* DumpFilter::GetPin(int n)
{
   std::cout << "DumpFilter::GetPin\n";
   return m_dump->m_pin.get();
}

int DumpFilter::GetPinCount()
{
   std::cout << "DumpFilter::GetPinCount\n";
   return 1;
}

HRESULT DumpFilter::Run(REFERENCE_TIME tStart)
{
   std::cout << "DumpFilter::Run\n";
   //CAutoLock lock(m_lock);
   return CBaseFilter::Run(tStart);
}

HRESULT DumpFilter::Pause()
{
   std::cout << "DumpFilter::Pause\n";
   return CBaseFilter::Pause();
}

HRESULT DumpFilter::Stop()
{
   std::cout << "DumpFilter::Stop\n";
   return CBaseFilter::Stop();
}

DumpInputPin::DumpInputPin(Dump* pDump, LPUNKNOWN pUnk, CBaseFilter* pFilter, CCritSec* pLock, CCritSec* pReceiveLock, HRESULT* phr)
   : CRenderedInputPin(NAME("DumpInputPin"),
                       pFilter,
                       pLock,
                       phr,
                       L"In"),
     m_dump(pDump),
     m_receiveLock(pReceiveLock),
     m_last(0)
{
}

DumpInputPin::~DumpInputPin()
{
}

HRESULT DumpInputPin::Receive(IMediaSample* pSample)
{
   std::cout << "DumpInputPin::Receive\n";
   CheckPointer(pSample, E_POINTER);
   CAutoLock lock(m_receiveLock);
   return S_OK;
}

HRESULT DumpInputPin::EndOfStream()
{
   std::cout << "DumpInputPin::EndOfStream\n";
   CAutoLock lock(m_receiveLock);
   return CRenderedInputPin::EndOfStream();
}

HRESULT DumpInputPin::ReceiveCanBlock()
{
   std::cout << "DumpInputPin::ReceiveCanBlock\n";
   return S_OK;
}

HRESULT DumpInputPin::CompleteConnect(IPin* pReceivePin)
{
   std::cout << "DumpInputPin::CompleteConnect\n";
   HRESULT hr = CRenderedInputPin::CompleteConnect(pReceivePin);
   return hr;
}

HRESULT DumpInputPin::SetMediaType(const CMediaType* pType)
{
   std::cout << "DumpInputPin::SetMediaType\n";
   HRESULT hr = CRenderedInputPin::SetMediaType(pType);
   return hr;
}

HRESULT DumpInputPin::CheckMediaType(const CMediaType* pType)
{
   std::cout << "DumpInputPin::CheckMediaType\n";
   return S_OK;
}

HRESULT DumpInputPin::GetMediaType(int iPosition, CMediaType* pMediaType)
{
   std::cout << "DumpInputPin::GetMediaType\n";
   return CRenderedInputPin::GetMediaType(iPosition, pMediaType);
}

HRESULT __stdcall DumpInputPin::ReceiveConnection(IPin* pConnector, const AM_MEDIA_TYPE* pmt)
{
   std::cout << "DumpInputPin::ReceiveConnection\n";
   HRESULT hr = S_OK;

   hr = CRenderedInputPin::ReceiveConnection(pConnector, pmt);

   return hr;
}

HRESULT DumpInputPin::BreakConnect()
{
   std::cout << "DumpInputPin::BreakConnect\n";
   if (m_dump->m_position) {
       m_dump->m_position->ForceRefresh();
   }

   return CRenderedInputPin::BreakConnect();
}

HRESULT DumpInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
   std::cout << "DumpInputPin::NewSegment\n";
   m_last = 0;
   return S_OK;
}

HRESULT DumpInputPin::NotifyAllocator(IMemAllocator* pAllocator, BOOL bReadOnly)
{
   std::cout << "DumpInputPin::NotifyAllocator\n";
   return CRenderedInputPin::NotifyAllocator(pAllocator, bReadOnly);
}

HRESULT DumpInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES* pProps)
{
   std::cout << "DumpInputPin::GetAllocatorRequirements\n";
   return CRenderedInputPin::GetAllocatorRequirements(pProps);
}

HRESULT DumpInputPin::GetAllocator(IMemAllocator** ppAllocator)
{
   std::cout << "DumpInputPin::GetAllocator\n";
   return CRenderedInputPin::GetAllocator(ppAllocator);
}

CUnknown* WINAPI Dump::CreateInstance(LPUNKNOWN punk, HRESULT* phr)
{
   std::cout << "Dump::CreateInstance\n";
   return new Dump(punk, phr);
}

DumpInputPin* Dump::getInputPin()
{
   std::cout << "Dump::getInputPin\n";
   return m_pin.get();
}

Dump::Dump(LPUNKNOWN pUnk, HRESULT* phr)
   : CUnknown(NAME("Dump"), pUnk)
{
   m_filter = std::make_unique<DumpFilter>(this, GetOwner(), &m_lock, phr);
   m_pin    = std::make_unique<DumpInputPin>(this, GetOwner(), m_filter.get(), &m_lock, &m_receiveLock, phr);
}

Dump::~Dump()
{
}

STDMETHODIMP Dump::NonDelegatingQueryInterface(const IID& riid, void** ppv)
{
   std::cout << "Dump::NonDelegatingQueryInterface\n";

   CheckPointer(ppv, E_POINTER);
   CAutoLock lock(&m_lock);

   if (riid == IID_IBaseFilter  ||
       riid == IID_IMediaFilter ||
       riid == IID_IPersist)
   {
      return m_filter->NonDelegatingQueryInterface(riid, ppv);
   } else
   if (riid == IID_IMediaPosition ||
       riid == IID_IMediaSeeking)
   {
      CheckPointer(m_position, E_POINTER);
      HRESULT hr = S_OK;
      m_position = std::make_unique<CPosPassThru>(NAME("Dump Pass Through"),
                                                  static_cast<IUnknown*>(GetOwner()),
                                                  static_cast<HRESULT*>(&hr),
                                                  m_pin.get());
      if (FAILED(hr))
      {
         m_position.release();
         return hr;
      }

      return m_position->NonDelegatingQueryInterface(riid, ppv);
   }

   return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}
