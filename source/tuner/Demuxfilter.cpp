#include "DemuxFilter.hpp"

#include <iostream>

STDMETHODIMP DemuxFilter::NonDelegatingQueryInterface(const IID& riid, void** ppv) {
   std::cout << "DemuxFilter::NonDelegatingQueryInterface()\n";
   return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}

CBasePin*DemuxFilter::GetPin(int) {
   std::cout << "DemuxFilter::GetPin()\n";
   return m_pin;
}

int DemuxFilter::GetPinCount() {
   std::cout << "DemuxFilter::GetPinCount()\n";
   return 1;
}

STDMETHODIMP DemuxFilter::Stop() {
   std::cout << "DemuxFilter::Stop()\n";
   return CBaseFilter::Stop();
}

STDMETHODIMP DemuxFilter::Pause() {
   std::cout << "DemuxFilter::Pause()\n";
   return CBaseFilter::Pause();
}

STDMETHODIMP DemuxFilter::Run(REFERENCE_TIME tStart) {
   std::cout << "DemuxFilter::Run()\n";
   return CBaseFilter::Run(tStart);
}

DemuxFilter::DemuxFilter(LPUNKNOWN lpunk, HRESULT* phr)
   : CBaseFilter(NAME(""), lpunk, &m_lock, CLSID_DemuxFilter){
   std::cout << "DemuxFilter::DemuxFilter()\n";
   m_pin = new DemuxInputPin(phr, this, NAME("In"));
}

DemuxFilter::~DemuxFilter() {
   std::cout << "DemuxFilter::~DemuxFilter()\n";
   delete m_pin;
}

HRESULT DemuxInputPin::SetMediaType(const CMediaType* pmt) {
   std::cout << "DemuxInputPin::SetMediaType\n";
   return CBaseInputPin::SetMediaType(pmt);
}

HRESULT DemuxInputPin::CheckMediaType(const CMediaType* pMediaType) {
   std::cout << "DemuxInputPin::CheckMediaType\n";

//   pMediaType->majortype
//   pMediaType->subtype
//   pMediaType->formattype
//   pMediaType->bFixedSizeSamples
//   pMediaType->bTemporalCompression
//   pMediaType->lSampleSize
//   pMediaType->cbFormat
//   pMediaType->pbFormat

   OLECHAR* majorStr;
   OLECHAR* subStr;
   OLECHAR* formatStr;

   StringFromCLSID(pMediaType->majortype,  &majorStr);
   StringFromCLSID(pMediaType->subtype,    &subStr);
   StringFromCLSID(pMediaType->formattype, &formatStr);

   std::cout << "majortype:    " << majorStr    << "\n";
   std::cout << "subtype:      " << subStr      << "\n";
   std::cout << "formattype:   " << formatStr   << "\n";

   CoTaskMemFree(majorStr);
   CoTaskMemFree(subStr);
   CoTaskMemFree(formatStr);

   return S_OK;
}

HRESULT DemuxInputPin::GetMediaType(int iPosition, CMediaType* pMediaType) {
   std::cout << "DemuxInputPin::GetMediaType\n";
   return CBaseInputPin::GetMediaType(iPosition, pMediaType);
}

HRESULT DemuxInputPin::CompleteConnect(IPin* pPin) {
   std::cout << "DemuxInputPin::CompleteConnect\n";
   return CBaseInputPin::CompleteConnect(pPin);
}

HRESULT DemuxInputPin::BreakConnect() {
   std::cout << "DemuxInputPin::BreakConnect\n";
   return CBaseInputPin::BreakConnect();
}

HRESULT DemuxInputPin::BeginFlush() {
   std::cout << "DemuxInputPin::BeginFlush\n";
   return CBaseInputPin::BeginFlush();
}

HRESULT DemuxInputPin::EndFlush() {
   std::cout << "DemuxInputPin::EndFlush\n";
   return CBaseInputPin::EndFlush();
}

HRESULT DemuxInputPin::Active() {
   std::cout << "DemuxInputPin::Active\n";
   return CBasePin::Active();
}

HRESULT DemuxInputPin::Inactive() {
   std::cout << "DemuxInputPin::Inactive\n";
   return CBasePin::Inactive();
}

DemuxInputPin::DemuxInputPin(HRESULT* phr, DemuxFilter* pParent, LPCWSTR pPinName)
   : CBaseInputPin(NAME("DemuxInputPin"), pParent, &m_lock, phr, pPinName) {
   std::cout << "DemuxInputPin::DemuxInputPin()\n";
}

DemuxInputPin::~DemuxInputPin() {
   std::cout << "DemuxInputPin::~DemuxInputPin()\n";
}
