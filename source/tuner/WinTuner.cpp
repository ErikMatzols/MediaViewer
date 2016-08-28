#include "WinTuner.hpp"
#include "Mediatypes.hpp"
#include "RendererFilter.hpp"
#include "Timer.hpp"
#include <Dvbsiparser.h>
#include <vector>
#include "DemuxFilter.hpp"

DEFINE_GUID(CLSID_DvbSiParser, 0xF6B96EDA, 0x1A94, 0x4476, 0xA8, 0x5F, 0x4D, 0x3D, 0xC7, 0xB3, 0x9C, 0x3F);

WinTuner::WinTuner(QObject* pParent, VideoRenderer* videoRenderer)
    : QThread(pParent)
{
    this->videoRenderer = videoRenderer;
}

WinTuner::~WinTuner()
{
}

void WinTuner::startDVB(const QString& deviceName)
{
    this->deviceName = deviceName;
    start();
}

void WinTuner::stopDVB()
{
    stopped = true;
    wait();
}

void WinTuner::pauseDVB(bool isPaused)
{
    paused = isPaused;
    togglePaused = true;
}

QStringList WinTuner::queryTuningSpaces()
{
    QStringList lst;
    if (pTunerAnalog)
        lst << "Analog";
    else if (pTunerBDA) {
       lst << "DVB-T";
//        IEnumTuningSpaces* pTuningSpaces;
//        HRESULT hr = pTunerBDA->EnumTuningSpaces(&pTuningSpaces);
//        if (FAILED(hr)) {
//            std::cout << "Failed to enumerate tuning spaces\n";
//            return lst;
//        }
//        ITuningSpace* pTuningSpace;
//        while (pTuningSpaces->Next(1, &pTuningSpace, 0) == S_OK) {
//            BSTR uniqueName;
//            pTuningSpace->get_UniqueName(&uniqueName);
//            QString str = QString((QChar*)uniqueName, static_cast<int>(wcslen(uniqueName)));
//            SysFreeString(uniqueName);
//            lst << str;
//        }
    }
    return lst;
}

QStringList WinTuner::queryFilters()
{
    QStringList lst;
    if (pGraph) {
        IEnumFilters* pEnumFilters;
        HRESULT hr = pGraph->EnumFilters(&pEnumFilters);
        if (FAILED(hr)) {
            std::cout << "Failed to enumerate filters in graph\n";
            return lst;
        }
        IBaseFilter* pFilter;
        while (pEnumFilters->Next(1, &pFilter, 0) == S_OK) {
            FILTER_INFO pInfo;
            hr = pFilter->QueryFilterInfo(&pInfo);
            if (FAILED(hr)) {
                std::cout << "Failed to query FILTER_INFO interface\n";
                return lst;
            }
            lst << QString::fromWCharArray(pInfo.achName);
            pInfo.pGraph->Release();
        }
    }
    return lst;
}

QList<ChannelInfo*> WinTuner::queryTables(long bandwidth, long frequency)
{
    QList<ChannelInfo*> lst;

    if (!pMpeg2Data) {
        std::cout << "IDvbSiTables interface pointer NULL\n";
        return lst;
    }

    IDvbSiParser* pDvbSiParser;
    HRESULT hr = CoCreateInstance(CLSID_DvbSiParser, 0, CLSCTX_ALL, __uuidof(IDvbSiParser2), (void**) &pDvbSiParser);

    if (FAILED(hr)) {
        std::cout << "Failed to create IDvbSiParser instance\n";
        return lst;
    }

    hr = pDvbSiParser->Initialize(pMpeg2Data);
    if (FAILED(hr)) {
        std::cout << "Failed to initialize IDvbSiParser interface\n";
        return lst;
    }

    IPAT* pIPAT;
    hr = pDvbSiParser->GetPAT(&pIPAT);
    if (FAILED(hr)) {
        std::cout << "Failed to retreive IPAT\n";
        return lst;
    }

    IDVB_SDT* pSDT;
    hr = pDvbSiParser->GetSDT(0x42, NULL, &pSDT);
    if (FAILED(hr)) {
        std::cout << "Failed to get IDVB_SDT interface\n";
        return lst;
    }

    WORD ONID, TSID;
    pSDT->GetOriginalNetworkId(&ONID);
    pSDT->GetTransportStreamId(&TSID);

    DWORD NumberOfSDTRecords;
    pSDT->GetCountOfRecords(&NumberOfSDTRecords);
    for (DWORD i = 0; i < NumberOfSDTRecords; i++) {
        WORD SID, pmtPID;
        BOOL freeCamMode;
        BYTE type;
        char* serviceName;
        char* serviceProviderName;
        QList<MediaPid> stream;

        pSDT->GetRecordServiceId(i, &SID);
        hr = pIPAT->FindRecordProgramMapPid(SID, &pmtPID);
        IPMT* pPMT;
        hr = pDvbSiParser->GetPMT(pmtPID, &SID, &pPMT);
        if (FAILED(hr)) {
            std::cout << "Failed to get IPMT\n";
            continue;
        }

        WORD pmtRecordsCount;
        pPMT->GetCountOfRecords(&pmtRecordsCount);
        for (int j = 0; j < pmtRecordsCount; j++) {
            WORD pid;
            BYTE stream_type = 0x00;
            MediaType type = UNKNOWN;

            pPMT->GetRecordElementaryPid(j, &pid);
            pPMT->GetRecordStreamType(j, &stream_type);

            switch (stream_type) {
            case 0x01:
                type = MPEG1_VIDEO;
                break;
            case 0x02:
                type = MPEG2_VIDEO;
                break;
            case 0x03:
                type = MPEG1_AUDIO;
                break;
            case 0x04:
                type = MPEG2_AUDIO;
                break;
            case 0x06: {
                IGenericDescriptor* pDescript;
                hr = pPMT->GetRecordDescriptorByTag(j, 0x6A, NULL, &pDescript);
                if (SUCCEEDED(hr))
                    type = AC3_AUDIO;
                else
                    type = TELETEXT;
            } break;
            case 0x10:
                type = MPEG4_VIDEO;
                break;
            case 0x11:
                type = MPEG4_AUDIO;
                break;
            case 0x1B:
                type = H264_VIDEO;
                break;
            default:
                type = UNKNOWN;
                break;
            }

            stream.push_back(MediaPid(pid, type));
        }

        pSDT->GetRecordFreeCAMode(i, &freeCamMode);

        IGenericDescriptor* pServiceDscr;
        hr = pSDT->GetRecordDescriptorByTag(i, 0x48, NULL, &pServiceDscr); // see ETSI EN 300 468 for 0x48 tag

        IDvbServiceDescriptor* pDVBServiceDscr;
        hr = pServiceDscr->QueryInterface(__uuidof(IDvbServiceDescriptor), (void**)&pDVBServiceDscr);

        pDVBServiceDscr->GetServiceType(&type);
        pDVBServiceDscr->GetServiceName(&serviceName);
        pDVBServiceDscr->GetServiceProviderName(&serviceProviderName);

        DVBTTuneRequest tuneReq;
        tuneReq.bandwidth = bandwidth;
        tuneReq.frequency = frequency;
        tuneReq.ONID = ONID;
        tuneReq.SID = SID;
        tuneReq.TSID = TSID;
        tuneReq.stream = stream;
        DVBTChannelInfo* channel = new DVBTChannelInfo(tuneReq, serviceName, serviceProviderName, deviceName);
        lst.push_back(channel);

        CoTaskMemFree(serviceName);
        CoTaskMemFree(serviceProviderName);
    }

    return lst;
}

void WinTuner::showPropertyPage(const QString& filterName)
{
    IBaseFilter* pFilter;
    pGraph->FindFilterByName(filterName.toStdWString().c_str(), &pFilter);

    ISpecifyPropertyPages* pProp;
    HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pProp);
    if (SUCCEEDED(hr)) {
        // Get the filter's name and IUnknown pointer.
        FILTER_INFO FilterInfo;
        hr = pFilter->QueryFilterInfo(&FilterInfo);
        if (FAILED(hr)) {
            std::cout << "Failed to query FILTER_INFO\n";
        }

        IUnknown* pFilterUnk;
        hr = pFilter->QueryInterface(IID_IUnknown, (void**)&pFilterUnk);
        if (FAILED(hr)) {
            std::cout << "Failed to query IUnknown interface\n";
        }

        // Show the page.
        CAUUID caGUID;

        hr = pProp->GetPages(&caGUID);
        if (FAILED(hr)) {
            std::cout << "Failed to get CAUUID pages\n";
        }
        pProp->Release();

        hr = OleCreatePropertyFrame(
            NULL, // Parent window
            0, 0, // Reserved
            FilterInfo.achName, // Caption for the dialog box
            1, // Number of objects (just the filter)
            &pFilterUnk, // Array of object pointers.
            caGUID.cElems, // Number of property pages
            caGUID.pElems, // Array of property page CLSIDs
            0, // Locale identifier
            0, NULL // Reserved
            );

        if (FAILED(hr)) {
            std::cout << "OleCreatePropertyFrame() failed\n";
        }

        // Clean up.
        pFilterUnk->Release();
        FilterInfo.pGraph->Release();
        CoTaskMemFree(caGUID.pElems);
    } else {
        std::cout << "Failed to query IID_ISpecifyPropertyPages interface\n";
    }
}

long WinTuner::AnalogTune(AnalogTuneRequest tuneReq)
{

    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return AMTUNER_NOSIGNAL;
    }

    AMTunerModeType mode;
    TunerInputType type;
    long input, countryCode, tuningSpace;

    pTunerAnalog->get_Mode(&mode);
    pTunerAnalog->get_ConnectInput(&input);
    pTunerAnalog->get_InputType(input, &type);
    pTunerAnalog->get_CountryCode(&countryCode);
    pTunerAnalog->get_TuningSpace(&tuningSpace);

    if (mode != tuneReq.mode)
        pTunerAnalog->put_Mode(tuneReq.mode);
    if (input != tuneReq.input)
        pTunerAnalog->put_ConnectInput(tuneReq.input);
    if (type != tuneReq.inputType)
        pTunerAnalog->put_InputType(tuneReq.input, tuneReq.inputType);
    if (countryCode != tuneReq.countryCode)
        pTunerAnalog->put_CountryCode(tuneReq.countryCode);
    if (tuningSpace != tuneReq.tuningSpace)
        pTunerAnalog->put_TuningSpace(tuneReq.tuningSpace);

    pTunerAnalog->put_Channel(tuneReq.channel, AMTUNER_SUBCHAN_NO_TUNE, AMTUNER_SUBCHAN_NO_TUNE);

    emit analogChannelChanged(tuneReq.channel);
    long signalStrength = AMTUNER_NOSIGNAL;
    pTunerAnalog->SignalPresent(&signalStrength);

    return signalStrength;
}

QList<ChannelInfo*> WinTuner::AnalogScanRequest(AnalogTuneRequest tuneReq)
{
    QList<ChannelInfo*> channels;
    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return channels;
    }

    long signalStrength = AnalogTune(tuneReq);
    if (signalStrength != AMTUNER_NOSIGNAL) {
        AnalogChannelInfo* channel = new AnalogChannelInfo(tuneReq, "Channel" + QString::number(tuneReq.channel), deviceName);
        channels.push_back(channel);
    }

    return channels;
}

long WinTuner::DVBTTune(DVBTTuneRequest tuneReq)
{
    if (!pTunerBDA) {
        std::cout << "ITuner interface pointer NULL\n";
        return -1;
    }

    ITuningSpace* pTuningSpace = retreiveTuningSpace("DVB-T");

    if (!pTuningSpace) {
        std::cout << "Failed to retreive tuning space\n";
        return -1;
    }

    ILocator* pLocator;
    HRESULT hr = pTuningSpace->get_DefaultLocator(&pLocator);
    if (FAILED(hr)) {
        std::cout << "Failed to retreive default locator\n";
        return -1;
    }

    IDVBTLocator* pDVBTLocator;
    hr = pLocator->QueryInterface(__uuidof(IDVBTLocator), (void**)&pDVBTLocator);
    if (FAILED(hr)) {
        std::cout << "Failed to query IDBTLocator interface\n";
        return -1;
    }

    hr = pDVBTLocator->put_CarrierFrequency(tuneReq.frequency * 1000);
    if (FAILED(hr)) {
        std::cout << "Failed to put carrier frequency\n";
        return -1;
    }

    hr = pDVBTLocator->put_Bandwidth(tuneReq.bandwidth);
    if (FAILED(hr)) {
        std::cout << "Failed to put bandwidth\n";
        return -1;
    }

    ITuneRequest* pTuneRequest;
    hr = pTuningSpace->CreateTuneRequest(&pTuneRequest);
    if (FAILED(hr)) {
        std::cout << "Failed to create tune request\n";
        return -1;
    }

    IDVBTuneRequest* pDVBTuneRequest;
    hr = pTuneRequest->QueryInterface(__uuidof(IDVBTuneRequest), (void**)&pDVBTuneRequest);
    if (FAILED(hr)) {
        std::cout << "Failed to query IDVBTuneRequest interface\n";
        return -1;
    }

    hr = pDVBTuneRequest->put_ONID(tuneReq.ONID);
    hr = pDVBTuneRequest->put_SID(tuneReq.SID);
    hr = pDVBTuneRequest->put_TSID(tuneReq.TSID);

    hr = pTuneRequest->put_Locator(pDVBTLocator);

    hr = pTunerBDA->put_TuningSpace(pTuningSpace);
    if (FAILED(hr)) {
        std::cout << "Failed to set tuning space on IScanningTuner interface\n";
        return -1;
    }

    hr = pTunerBDA->put_TuneRequest(pDVBTuneRequest);
    if (FAILED(hr)) {
        std::cout << "Failed to set tune request on IScanningTuner interface\n";
        return -1;
    }

    long strength = 0;
    pTunerBDA->get_SignalStrength(&strength);

    emit tuneRequestFinished();

    return strength;
}

QList<ChannelInfo*> WinTuner::DVBTScanRequest(DVBTTuneRequest tuneReq)
{
    tuneReq.ONID = -1;
    tuneReq.SID = -1;
    tuneReq.TSID = -1;

    DVBTTune(tuneReq);
    QList<ChannelInfo*> channels = queryTables(tuneReq.bandwidth, tuneReq.frequency);

    return channels;
}

long WinTuner::DVBCTune(DVBCTuneRequest tuneReq)
{
    return -1;
}

QList<ChannelInfo*> WinTuner::DVBCScanRequest(DVBCTuneRequest tuneReq)
{
    QList<ChannelInfo*> channels;

    long signalStrength = DVBCTune(tuneReq);
    if (signalStrength > 0)
        channels = queryTables(0, 0);

    return channels;
}

long WinTuner::DVBSTune(DVBSTuneRequest tuneReq)
{
    return -1;
}

QList<ChannelInfo*> WinTuner::DVBSScanRequest(DVBSTuneRequest tuneReq)
{
    QList<ChannelInfo*> channels;

    long signalStrength = DVBSTune(tuneReq);
    if (signalStrength > 0)
        channels = queryTables(0, 0);

    return channels;
}

QString WinTuner::getCurrentDeviceName()
{
    return deviceName;
}

bool WinTuner::initialize()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cout << "Error initializing COM library\n";
        return false;
    }

    stopped = false;
    paused = false;
    togglePaused = false;
    pGraph = NULL;
    pTunerAnalog = NULL;
    pTunerBDA = NULL;
    pMpeg2Data = NULL;
    pMpeg2Demux = NULL;
    pDumpFilter = NULL;
    pPin = NULL;
    pGuideData = NULL;
    pVideoPin = NULL;
    pAudioPin = NULL;

    return true;
}

void WinTuner::uninitialize()
{
    if (pGraph)
        pGraph->Release();
    if (pTunerAnalog)
        pTunerAnalog->Release();
    if (pTunerBDA)
        pTunerBDA->Release();
    if (pMpeg2Data)
        pMpeg2Data->Release();
    if (pMpeg2Demux)
        pMpeg2Demux->Release();
    if (pDumpFilter) {
        //pDumpFilter->Release();
        //delete pDumpFilter;
    }
    if (pPin) {
        pPin->Release();
    }
    if (pGuideData)
        pGuideData->Release();
    CoUninitialize();
}

void WinTuner::run()
{
    if (!initialize())
        return;

    if (!buildAndConnectGraphFromFile("../graphs/" + deviceName)) {
        uninitialize();
        return;
    }

    IMediaControl* pMediaControl;
    HRESULT hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pMediaControl);
    if (FAILED(hr)) {
        std::cout << "Failed to query IMediaControl interface\n";
        uninitialize();
        return;
    }

    IMediaSeeking* pMediaSeeking;
    hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pMediaSeeking);
    if (FAILED(hr)) {
        std::cout << "Failed to query IMediaControl interface\n";
        uninitialize();
        return;
    }

    hr = pMediaControl->Run();
    if (FAILED(hr)) {
        std::cout << "Error starting graph\n";
        uninitialize();
        return;
    }

    IMediaEvent* pMediaEvent;
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)&pMediaEvent);
    if (FAILED(hr)) {
        std::cout << "Failed to query IMediaEvent interface\n";
        pMediaControl->Stop();
        uninitialize();
        return;
    }

    std::cout << "Graph running\n";
    emit graphRunning();

    while (!stopped) {
        Sleep(100);
        long ev = 0;
        LONG_PTR p1 = 0, p2 = 0;
        if (pMediaEvent->GetEvent(&ev, &p1, &p2, 0) == S_OK) {
            printf("Event: %ld received\n", ev);
            if (ev == EC_COMPLETE || ev == EC_USERABORT) {
                std::cout << "Completed or user aborted\n";
                stopped = true;
            } else if (ev == EC_ERRORABORT) {
                std::cout << "An error occured: EC_ERRORABORT\n";
                stopped = true;
            } else if (ev == EC_USER) {
                // reconnect dumpfilter inputpin, because windows mpeg decoder is not pushing new samples
//                pMediaControl->Stop();
//                pGraph->Reconnect(pPin);
//                pMediaControl->Run();
            }
            pMediaEvent->FreeEventParams(ev, p1, p2);
        }

        if (togglePaused) {
            paused ? pMediaControl->Pause() : pMediaControl->Run();
            togglePaused = false;
        }
    }

    pMediaControl->Stop();
    std::cout << "Graph stopped\n";
    uninitialize();
}

bool WinTuner::buildAndConnectGraphFromFile(const QString& fileName)
{
    Settings settings;
    if (!settings.loadFile(fileName)) {
        std::cout << "Error loading graph file\n";
        return false;
    }

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_ALL, IID_IGraphBuilder, (void**)&pGraph);
    if (FAILED(hr)) {
        std::cout << "Error creating IGraphBuilder instance\n";
        return false;
    }

    foreach (SettingsMap map, settings.retreiveArray()) {
        QString sDisplayName = map["sDisplayName"];
        QString sName = map["sName"];
        std::cout << "Creating filter " << sName.toStdString().c_str() << "\n";
        IBaseFilter* pFilter = createFilterSmart(sDisplayName);
        if (!pFilter) {
            std::cout << "Error creating filter " << sName.toStdString().c_str() << "\n";
            return false;
        }
        hr = pGraph->AddFilter(pFilter, sName.toStdWString().c_str());
        if (FAILED(hr)) {
            std::cout << "Error adding filter " << sName.toStdString().c_str() << " to graph\n";
            return false;
        }
    }

    foreach (SettingsMap map, settings.retreiveArray()) {
        QString sName = map["sName"];
        QString sInterface = map["sInterface"];
        int iOut = map["iOut"].toInt();

        IBaseFilter* outFilter;
        hr = pGraph->FindFilterByName(sName.toStdWString().c_str(), &outFilter);
        if (FAILED(hr)) {
            std::cout << "Error cannot find output filter " << sName.toStdString().c_str() << " in graph\n";
            return false;
        }

        if (sInterface.compare("None") == 0) {
        } else if (sInterface.compare("IAMTVTuner") == 0) {
            hr = outFilter->QueryInterface(__uuidof(IAMTVTuner), (void**)&pTunerAnalog);
            if (FAILED(hr)) {
                std::cout << "Failed to query IAMTVTuner interface\n";
                return false;
            }
        } else if (sInterface.compare("IScanningTuner") == 0) {
            hr = outFilter->QueryInterface(__uuidof(IScanningTuner), (void**)&pTunerBDA);
            if (FAILED(hr)) {
                std::cout << "Failed to query ITuner interface\n";
                return false;
            }
        } else if (sInterface.compare("IGuideData") == 0) {
            hr = outFilter->QueryInterface(__uuidof(IGuideData), (void**)&pGuideData);
            if (FAILED(hr)) {
                std::cout << "Failed to query IGuideData interface\n";
                return false;
            }
        } else if (sInterface.compare("IMpeg2Data") == 0) {
            hr = outFilter->QueryInterface(__uuidof(IMpeg2Data), (void**)&pMpeg2Data);
            if (FAILED(hr)) {
                std::cout << "Failed to query IMpeg2Data interface\n";
                return false;
            }
        } else if (sInterface.compare("IMpeg2Demultiplexer") == 0) {
            hr = outFilter->QueryInterface(__uuidof(IMpeg2Demultiplexer), (void**)&pMpeg2Demux);
            if (FAILED(hr)) {
                std::cout << "Failed to query IMpeg2Demultiplexer\n";
                return false;
            }
            for (int i = 0; i < iOut; i++) {
                QString sOutPin = map["sOut" + QString::number(i) + "Name"];
                AM_MEDIA_TYPE mt;
                ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
                if (sOutPin.compare("Video Pin") == 0) {
                    mt.majortype = MEDIATYPE_Video;
                    mt.subtype = MEDIASUBTYPE_MPEG2_VIDEO;
                    mt.formattype = FORMAT_MPEG2Video;

                    //hr = pMpeg2Demux->CreateOutputPin(&mt, L"Video Pin", &pVideoPin);
                    if (FAILED(hr)) {
                        std::cout << "Failed to create demux pin Video Pin\n";
                        return false;
                    }
                } else if (sOutPin.compare("Audio Pin") == 0) {
                    mt.majortype = MEDIATYPE_Audio;
                    mt.subtype = MEDIASUBTYPE_MPEG2_AUDIO;
                    mt.formattype = FORMAT_WaveFormatEx;
                    mt.cbFormat = sizeof(MPEG2AudioFormat);
                    mt.pbFormat = MPEG2AudioFormat;
                    mt.bFixedSizeSamples = TRUE;
                    mt.bTemporalCompression = 0;
                    mt.lSampleSize = 1;
                    mt.pUnk = NULL;

                    //hr = pMpeg2Demux->CreateOutputPin(&mt, L"Audio Pin", &pAudioPin);
                    if (FAILED(hr)) {
                        std::cout << "Failed to create demux pin Audio Pin\n";
                        return false;
                    }
                }
            }
        } else if (sInterface.compare("IAMCrossbar") == 0) {
            IAMCrossbar* pCrossbar;
            hr = outFilter->QueryInterface(__uuidof(IAMCrossbar), (void**)&pCrossbar);
            if (FAILED(hr)) {
                std::cout << "Failed to query IAMCrossbar interface\n";
                return false;
            }

            int iRoutes = map["iRoutes"].toInt();
            for (int j = 0; j < iRoutes; j++) {
                QString sRoute = map["sRoute" + QString::number(j)];
                QStringList r = sRoute.split(",");
                pCrossbar->Route(r.first().toInt(), r.last().toInt());
            }
        }

        for (int i = 0; i < iOut; i++) {
            QString sOutPin        = map["sOut" + QString::number(i) + "Name"];
            QString sConnectFilter = map["sOut" + QString::number(i) + "ConnectFilter"];
            QString sConnectPin    = map["sOut" + QString::number(i) + "ConnectPin"];

            IBaseFilter* inFilter;
            if (sConnectFilter.compare("MediaViewerDemuxer") == 0) {
               pDumpFilter = new DemuxFilter(0, &hr);
               hr = pDumpFilter->QueryInterface(__uuidof(IBaseFilter), (void**)&inFilter);
               hr = pGraph->AddFilter(inFilter, L"MediaViewerDemuxer");
            } else {
                hr = pGraph->FindFilterByName(sConnectFilter.toStdWString().c_str(), &inFilter);
                if (FAILED(hr)) {
                    std::cout << "Error cannot find input filter " << sConnectFilter.toStdString().c_str() << " in graph\n";
                    return false;
                }
            }
            IPin* outPin = getPinSmart(outFilter, sOutPin, PINDIR_OUTPUT);
            IPin* inPin = getPinSmart(inFilter, sConnectPin, PINDIR_INPUT);
            hr = pGraph->ConnectDirect(outPin, inPin, NULL);
            if (FAILED(hr)) {
                std::cout << "Error cannot connect output filter " << sName.toStdString().c_str() << " with input filter " << sConnectFilter.toStdString().c_str() << "\n";
                return false;
            }
        }
    }

    return true;
}

ITuningSpace* WinTuner::retreiveTuningSpace(const QString& tuningSpaceName)
{
    IEnumTuningSpaces* pTuningSpaces;
    HRESULT hr = pTunerBDA->EnumTuningSpaces(&pTuningSpaces);
    if (FAILED(hr)) {
        std::cout << "Failed to enumerate tuning spaces\n";
        return NULL;
    }

    ITuningSpace* pTuningSpace;
    while (pTuningSpaces->Next(1, &pTuningSpace, 0) == S_OK) {
        BSTR uniqueName;
        pTuningSpace->get_UniqueName(&uniqueName);
        QString str = QString((QChar*)uniqueName, static_cast<int>(wcslen(uniqueName)));
        SysFreeString(uniqueName);
        if (str.compare(tuningSpaceName) == 0)
            return pTuningSpace;
    }

    return NULL;
}

IBaseFilter* WinTuner::createFilterSmart(const QString& dName)
{
    IBindCtx* pBindCtx;
    HRESULT hr = CreateBindCtx(0, &pBindCtx);
    if (FAILED(hr)) {
        std::cout << "Error cannot create bind context\n";
        return NULL;
    }

    ULONG chEaten = 0;
    IMoniker* pMoniker;
    hr = MkParseDisplayName(pBindCtx, dName.toStdWString().c_str(), &chEaten, &pMoniker);
    if (FAILED(hr)) {
        std::cout << "Error cannot parse display name of the filter\n";
        return NULL;
    }

    IBaseFilter* pFilter;
    hr = pMoniker->BindToObject(pBindCtx, NULL, IID_IBaseFilter, (void**)&pFilter);
    if (FAILED(hr)) {
        std::cout << "Error cannot bind moniker to filter object\n";
        return NULL;
    }

    return pFilter;
}

IPin* WinTuner::getPinSmart(IBaseFilter* pFilter, const QString& pinName, PIN_DIRECTION dir)
{
    IEnumPins* pEnum;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr)) {
        std::cout << "Error can't enumerate pins";
        return NULL;
    }

    IPin* pPin;
    while (pEnum->Next(1, &pPin, 0) == S_OK) {
        PIN_INFO pinInfo;
        hr = pPin->QueryPinInfo(&pinInfo);
        if (FAILED(hr)) {
            std::cout << "Failed to query pin info\n";
            return NULL;
        }
        pinInfo.pFilter->Release();

        BOOL found = !_wcsicmp(pinName.toStdWString().c_str(), pinInfo.achName);
        if (found && pinInfo.dir == dir) {
            return pPin;
        }
    }

    return NULL;
}

QStringList WinTuner::getAvailableModes()
{
    QStringList lst;

    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return lst;
    }

    long m;
    pTunerAnalog->GetAvailableModes(&m);

    if (m == AMTUNER_MODE_DEFAULT)
        lst << "Default";
    if ((m & AMTUNER_MODE_TV))
        lst << "TV";
    if ((m & AMTUNER_MODE_FM_RADIO))
        lst << "FM Radio";
    if ((m & AMTUNER_MODE_AM_RADIO))
        lst << "AM Radio";
    if ((m & AMTUNER_MODE_DSS))
        lst << "DSS";

    return lst;
}

void WinTuner::setMode(const QString& mode)
{
    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return;
    }

    AMTunerModeType m = AMTUNER_MODE_DEFAULT;

    if (mode.compare("Default") == 0)
        m = AMTUNER_MODE_DEFAULT;
    else if (mode.compare("TV") == 0)
        m = AMTUNER_MODE_TV;
    else if (mode.compare("FM Radio") == 0)
        m = AMTUNER_MODE_FM_RADIO;
    else if (mode.compare("AM Radio") == 0)
        m = AMTUNER_MODE_AM_RADIO;
    else if (mode.compare("DSS") == 0)
        m = AMTUNER_MODE_DSS;

    pTunerAnalog->put_Mode(m);
}

AMTunerModeType WinTuner::modeToEnum(const QString& mode)
{
    if (mode.compare("TV") == 0)
        return AMTUNER_MODE_TV;
    else if (mode.compare("FM Radio") == 0)
        return AMTUNER_MODE_FM_RADIO;
    else if (mode.compare("AM Radio") == 0)
        return AMTUNER_MODE_AM_RADIO;
    else if (mode.compare("DSS") == 0)
        return AMTUNER_MODE_DSS;
    else
        return AMTUNER_MODE_DEFAULT;
}

QStringList WinTuner::getInputTypes()
{
    return {"Cable", "Antenna"};
}

void WinTuner::setInputType(const QString& inputType)
{
    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return;
    }

    long inputIndex;
    HRESULT hr = pTunerAnalog->get_ConnectInput(&inputIndex);

    if (inputType.compare("Antenna") == 0)
        hr = pTunerAnalog->put_InputType(inputIndex, TunerInputAntenna);
    else if (inputType.compare("Cable") == 0)
        hr = pTunerAnalog->put_InputType(inputIndex, TunerInputCable);
}

TunerInputType WinTuner::inputTypeToEnum(const QString& inputType)
{
    if (inputType.compare("Antenna") == 0)
        return TunerInputAntenna;
    else
        return TunerInputCable;
}

QStringList WinTuner::getAvailableInputs()
{
    QStringList lst;

    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return lst;
    }

    long numInputs;
    pTunerAnalog->get_NumInputConnections(&numInputs);

    for (int i = 0; i < numInputs; i++)
        lst << QString::number(i);
    return lst;
}

void WinTuner::setCurrentInput(const QString& input)
{
    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return;
    }

    int i = input.toInt();
    pTunerAnalog->put_ConnectInput(i);
}

QString WinTuner::getCountryCode()
{
    long c;

    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return QString();
    }

    pTunerAnalog->get_CountryCode(&c);
    return QString::number(c);
}

void WinTuner::setCountryCode(const QString& code)
{
    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return;
    }
    pTunerAnalog->put_CountryCode(code.toInt());
}

QString WinTuner::getTuningSpace()
{
    long c;

    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return QString();
    }

    pTunerAnalog->get_TuningSpace(&c);
    return QString::number(c);
}

void WinTuner::setTuningSpace(const QString& space)
{
    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return;
    }

    pTunerAnalog->put_TuningSpace(space.toInt());
}

void WinTuner::getChannelMinMax(int& min, int& max)
{
    long lMin = 0, lMax = 0;

    if (!pTunerAnalog) {
        std::cout << "IAMTVTuner interface pointer NULL\n";
        return;
    }

    pTunerAnalog->ChannelMinMax(&lMin, &lMax);
    min = lMin;
    max = lMax;
}

void WinTuner::mapPidToVideoPin(MediaPid stream)
{
    if (pVideoPin) {
        IMPEG2PIDMap* pPidMap;
        HRESULT hr = pVideoPin->QueryInterface(&pPidMap);
        if (SUCCEEDED(hr)) {
            IEnumPIDMap* pEnumPidMap;
            hr = pPidMap->EnumPIDMap(&pEnumPidMap);
            if (SUCCEEDED(hr)) {
                PID_MAP pidM;
                ULONG recv = 0;
                while (pEnumPidMap->Next(1, &pidM, &recv) == S_OK)
                    pPidMap->UnmapPID(1, &pidM.ulPID);
                pEnumPidMap->Release();
            }

            AM_MEDIA_TYPE pmt;
            ZeroMemory(&pmt, sizeof(AM_MEDIA_TYPE));
            allocateMediaType(stream.type, &pmt);
            //hr = pMpeg2Demux->SetOutputPinMediaType(L"Video Pin", &pmt);

            if (FAILED(hr))
                std::cout << "Failed to set output media type on video pin\n";
            hr = pPidMap->MapPID(1, &stream.pid, MEDIA_ELEMENTARY_STREAM);
            if (FAILED(hr))
                std::cout << "Failed to map pid on video pin\n";

            pPidMap->Release();
        }
    }
}

void WinTuner::mapPidToAudioPin(MediaPid stream)
{
    if (pAudioPin) {
        IMPEG2PIDMap* pPidMap;
        HRESULT hr = pAudioPin->QueryInterface(&pPidMap);
        if (SUCCEEDED(hr)) {
            IEnumPIDMap* pEnumPidMap;
            hr = pPidMap->EnumPIDMap(&pEnumPidMap);
            if (SUCCEEDED(hr)) {
                PID_MAP pidM;
                ULONG recv = 0;
                while (pEnumPidMap->Next(1, &pidM, &recv) == S_OK)
                    pPidMap->UnmapPID(1, &pidM.ulPID);
                pEnumPidMap->Release();
            }

            AM_MEDIA_TYPE pmt;
            ZeroMemory(&pmt, sizeof(AM_MEDIA_TYPE));
            allocateMediaType(stream.type, &pmt);
            //hr = pMpeg2Demux->SetOutputPinMediaType(L"Audio Pin", &pmt);
            if (FAILED(hr))
                std::cout << "Failed to set output media type on audio pin\n";

            hr = pPidMap->MapPID(1, &stream.pid, MEDIA_ELEMENTARY_STREAM);
            if (FAILED(hr))
                std::cout << "Failed to map pid on audio pin\n";

            pPidMap->Release();
        }
    }
}
