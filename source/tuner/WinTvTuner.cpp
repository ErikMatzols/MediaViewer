/*
#include "Defines.hpp"
#include "Settings.hpp"
#include "VideoRenderer.hpp"
#include "WinTVTuner.hpp"

WinTvTuner::WinTvTuner(QObject *parent, VideoRenderer *renderer)
: TunerInterface(parent)
{
    this->renderer = renderer;
    stopped = false;
    pTuner = NULL;
    pCrossbar = NULL;
    pSampleGrab = NULL;
    pAudio = NULL;
}

WinTvTuner::WinTvTuner(QObject *parent)
: TunerInterface(parent)
{
    renderer = NULL;
    stopped = false;
    pTuner = NULL;
    pCrossbar = NULL;
    pSampleGrab = NULL;
    pAudio = NULL;
}

WinTvTuner::~WinTvTuner()
{
    std::cout << "WinTvTuner::Destructor() called\n";
}

void WinTvTuner::startTuner(enum ThreadMode mode)
{
    mPaused = false;
    stopped = false;
    mTogglePaused = false;
    setThreadMode(mode);
    start();
}

void WinTvTuner::stopTuner()
{
    stopped = true;
    wait();
    if (getThreadMode() ==  RUNGRAPH) {
        if (pTuner) {
            pTuner->Release();
            pTuner = NULL;
        }
        if (pCrossbar) {
            pCrossbar->Release();
            pCrossbar = NULL;
        }
        if (pSampleGrab) {
            pSampleGrab->Release();
            pSampleGrab = NULL;
        }
        if (pAudio) {
            pAudio->Release();
            pAudio = NULL;
        }
        grabberCallback.freeFrameGrabberCallback();
    }
}

void WinTvTuner::runGraph()
{
    HRESULT hr = CoInitializeEx(NULL,COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cout << "Error initializing COM library\n";
        emit tunerError("Error initializing COM library");
        return;
    }

    CComPtr<IFilterGraph> graph;
    hr = graph.CoCreateInstance(CLSID_FilterGraph);
    if (FAILED(hr)) {
        std::cout << "Error creating filter graph instance\n";
        emit tunerError("Error creating filter graph instance");
        CoUninitialize();
        return;
    }

    if (!getChannelList().size()) {
        std::cout << "Error No channel list configured, check Options\n";
        emit tunerError("Error no channel list configured, check Options");
        CoUninitialize();
        return;
    }

    hr = buildGraphFromFile(graph, "../graphs/"+currentItem().device);
    if (FAILED(hr)) {
        std::cout << "Error building graph for current channel\n";
        emit tunerError("Error building graph for current channel");
        CoUninitialize();
        return;
    }

    setCurrentInput(currentItem().input);
    setInputType(currentItem().inputType);
    setCountryCode(currentItem().country);
    setTuningSpace(currentItem().tuningSpace);
    setChannel(currentItem().channel.toInt());

    hr = initFrameGrabber();
    if (FAILED(hr)) {
        std::cout << "Error initializing frame grabber\n";
        emit tunerError("Error initializing frame grabber filter");
        CoUninitialize();
        return;
    }

    CComQIPtr<IMediaControl, &IID_IMediaControl> mediaControl(graph);
    hr = mediaControl->Run();
    if (FAILED(hr)) {
        std::cout << "Error starting graph\n";
        emit tunerError("Error starting graph");
        CoUninitialize();
        return;
    }

    std::cout << "Directshow graph started\n";
    renderer->openVideoRenderer(grabberCallback.getWidth(), grabberCallback.getHeight(), grabberCallback.getFormat());
    renderer->updateOverlayText(currentItem().channelName);

    CComQIPtr<IMediaEvent, &IID_IMediaEvent> mediaEvent(graph);
    while (!stopped) {
        long ev = 0, p1 = 0, p2 = 0;
        Sleep(100);
        if (mediaEvent->GetEvent(&ev,&p1,&p2,0) == S_OK){
            if (ev == EC_COMPLETE || ev == EC_USERABORT){
                std::cout << "Completed or user aborted\n";
                stopped = true;
            }
            else if (ev == EC_ERRORABORT){
                std::cout << "An error occured: EC_ERRORABORT\n";
                stopped = true;
            }
            mediaEvent->FreeEventParams(ev, p1, p2);
        }
        if (mTogglePaused) {
            if (mPaused)
                mediaControl->Pause();
            else
                mediaControl->Run();
            mTogglePaused = false;
        }
    }
    mediaControl->Stop();
    std::cout << "Directshow graph stopped\n";
    renderer->closeVideoRenderer();
    CoUninitialize();
}

void WinTvTuner::autoTune()
{
    QStringList channels;
    long lMin = 0, lMax = 0;
    HRESULT hr = pTuner->ChannelMinMax(&lMin, &lMax);
    if (SUCCEEDED(hr)) {
        for(int i = lMin; i < lMax; i++) {
            if (stopped) 
                break;
            bool channelFound = false;
            long SignalStrength = AMTUNER_NOSIGNAL;
            hr = pTuner->put_Channel(i, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);
            if (NOERROR == hr)
                pTuner->SignalPresent(&SignalStrength);
            if (SignalStrength != AMTUNER_NOSIGNAL)
                channelFound = true;
            emit channelUpdate(i, channelFound);
        }
    }
    emit autoTuneCompleted();
}

void WinTvTuner::setChannel(int channel)
{
    pTuner->put_Channel(channel,AMTUNER_SUBCHAN_NO_TUNE,AMTUNER_SUBCHAN_NO_TUNE);
    renderer->updateOverlayText("Channel "+QString::number(channel));
}

int WinTvTuner::getChannel()
{
    long channel;
    long tmp = AMTUNER_SUBCHAN_NO_TUNE;
    pTuner->get_Channel(&channel,&tmp,&tmp);
    return channel;
}

void WinTvTuner::run()
{
    switch (getThreadMode())
    {
    case RUNGRAPH:
        runGraph();
        break;
    case AUTOTUNE:
        autoTune();
        break;
    default:
        std::cout << "Error: Unknown tuner state!\n";
        break;
    }
}

HRESULT WinTvTuner::buildGraphFromFile(const CComPtr<IFilterGraph>& graph,const QString& graphFile)
{
    Settings settings;
    if (!settings.loadFile(graphFile)){
        std::cout << "Error cannot load graph file\n";
        return E_FAIL;
    }

    HRESULT hr = S_OK;

    // Load all filters in file
    const SettingsGroupMap& map = settings.retreiveMap();
    QMap<QString,SettingsMap>::const_iterator i = map.constBegin();
    for(; i != map.constEnd(); ++i) {	
        QString sDName,sName;
        settings.retreiveStringValue(i.key(),"sDisplayName",sDName);
        settings.retreiveStringValue(i.key(),"sName",sName);
        CComPtr<IBaseFilter> filter = createFilter(sDName);
        if (!filter) {
            std::cout << "Error creating filter " << sName.toStdString().c_str() << "\n";
            return E_FAIL;
        }
        hr = graph->AddFilter(filter,sName.toStdWString().c_str());
        if (FAILED(hr)) {
            std::cout << "Error adding filter " << sName.toStdString().c_str() << " to graph\n";
            return hr;
        }
    }

    // Connect and configure filters in graph
    i = map.constBegin();
    for(; i != map.constEnd(); ++i) {
        QString sInterface,sName;
        int iOutputs;

        settings.retreiveStringValue(i.key(),"sName",sName);
        settings.retreiveStringValue(i.key(),"sInterface",sInterface);
        settings.retreiveIntValue(i.key(),"iOut",iOutputs);

        CComPtr<IBaseFilter> outputFilter;
        hr = graph->FindFilterByName(sName.toStdWString().c_str(),&outputFilter);
        if (FAILED(hr)) {
            std::cout << "Error cannot find filter " << sName.toStdString().c_str() << " in graph\n";
            return hr;
        }
        
        // Query interfaces
        if (sInterface.compare("IID_IAMTVTuner") == 0)
        {
            hr = outputFilter->QueryInterface(IID_IAMTVTuner,(void**)&pTuner);
            if (FAILED(hr)) {
                std::cout << "Error query TV Tuner interface\n";
                return hr;
            }
            //pTuner->put_Channel(currentItem().channel.toInt(),AMTUNER_SUBCHAN_NO_TUNE,AMTUNER_SUBCHAN_NO_TUNE);			
        }
        else if (sInterface.compare("IID_ISampleGrabber") == 0) {
            hr = outputFilter->QueryInterface(IID_ISampleGrabber,(void**)&pSampleGrab);
            if (FAILED(hr)) {
                std::cout << "Error cannot query SampleGrabber interface\n";
                return hr;
            }
        }
        else if (sInterface.compare("IID_IAMCrossbar") == 0) {
            hr = outputFilter->QueryInterface(IID_IAMCrossbar,(void**)&pCrossbar);
            if (FAILED(hr)) {
                std::cout << "Error Unable to query crossbar interface\n";
                return hr;
            }
            int iRoutes;
            settings.retreiveIntValue(i.key(),"iRoutes",iRoutes);
            for (int j = 0; j < iRoutes; j++) {
                QString sRoute;
                settings.retreiveStringValue(i.key(),"sRoute"+QString::number(j),sRoute);
                QStringList r = sRoute.split(",");
                pCrossbar->Route(r.first().toInt(),r.last().toInt());
            }        			
        }
        else if (sInterface.compare("IID_IBasicAudio") == 0) {
            hr = outputFilter->QueryInterface(IID_IBasicAudio, (void**) &pAudio);
            if (FAILED(hr)) {
                std::cout << "Error Unable to query audio interface\n";
                return hr;
            }
        }

        for(int k = 0; k < iOutputs; k++) {
            QString sOutPin,sConnectFilter,sConnectPin;

            settings.retreiveStringValue(i.key(),"sOut"+QString::number(k)+"Name",sOutPin);
            settings.retreiveStringValue(i.key(),"sOut"+QString::number(k)+"ConnectFilter",sConnectFilter);
            settings.retreiveStringValue(i.key(),"sOut"+QString::number(k)+"ConnectPin",sConnectPin);

            CComPtr<IBaseFilter> inputFilter;
            hr = graph->FindFilterByName(sConnectFilter.toStdWString().c_str(),&inputFilter);
            if (FAILED(hr)) {
                std::cout << "Error cannot find input filter " << sConnectFilter.toStdString().c_str() << " in graph\n";
                return hr;
            }

            hr = graph->ConnectDirect(getPin(outputFilter, sOutPin.toStdString().c_str()),getPin(inputFilter, sConnectPin.toStdString().c_str()), NULL);
            if (FAILED(hr)) {
                std::cout << "Error cannot connect output filter " << sName.toStdString().c_str() << " with input filter " << sConnectFilter.toStdString().c_str() << "\n";
                return hr;
            }
        }
    }

    return hr;
}

CComPtr<IBaseFilter> WinTvTuner::createFilter(const QString& dName)
{
    CComPtr<IBindCtx> pBindCtx;
    HRESULT hr = CreateBindCtx(0, &pBindCtx);
    if (FAILED(hr)) {
        std::cout << "Error cannot create bind context\n";
        return NULL;
    }

    ULONG chEaten = 0;
    CComPtr<IMoniker> pMoniker;
    hr = MkParseDisplayName(pBindCtx, dName.toStdWString().c_str(), &chEaten, &pMoniker);
    if (FAILED(hr)) {
        std::cout << "Error cannot parse display name of the filter\n";
        return NULL;
    }

    CComPtr<IBaseFilter> filter;
    hr = pMoniker->BindToObject(pBindCtx, NULL, IID_IBaseFilter, (void**)&filter);
    if (FAILED(hr)) {
        std::cout << "Error cannot bind moniker to filter object\n";
        return NULL;
    }
    return filter;
}

CComPtr<IPin> WinTvTuner::getPin(IBaseFilter *pFilter,const QString& pinName)
{
    CComPtr<IEnumPins> pEnum;
    CComPtr<IPin> pPin;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr)) {
        std::cout << "Error Can't enumerate pins " << pinName.toStdString().c_str() << "\n";
        return NULL;
    }

    while(pEnum->Next(1, &pPin, 0) == S_OK) {
        PIN_INFO pinfo;  
        pPin->QueryPinInfo(&pinfo);
        BOOL found = !_wcsicmp(pinName.toStdWString().c_str(), pinfo.achName);
        if (pinfo.pFilter)
            pinfo.pFilter->Release();
        if (found)
            return pPin;
        pPin.Release();
    }

    std::cout << "Error Pin not found! " << pinName.toStdString().c_str() << "\n";
    return NULL;
}

HRESULT WinTvTuner::initFrameGrabber()
{
    HRESULT hr = S_OK;

    AM_MEDIA_TYPE connectedType;
    if (SUCCEEDED(pSampleGrab->GetConnectedMediaType(&connectedType))) {
        if (connectedType.formattype == FORMAT_VideoInfo) {
            VIDEOINFOHEADER* infoHeader = (VIDEOINFOHEADER*)connectedType.pbFormat;
            if (connectedType.subtype == MEDIASUBTYPE_YUY2){           
                hr = grabberCallback.initializeFrameGrabCallback(infoHeader->bmiHeader.biWidth,infoHeader->bmiHeader.biHeight,PIX_FMT_YUYV422,renderer);
                if (hr == S_OK)
                    pSampleGrab->SetCallback(&grabberCallback,1);
            }
            else{
                std::cout << "Failed to find suitable Media Subtype\n";
                return S_FALSE;
            }
        }
        CoTaskMemFree(connectedType.pbFormat);
    }

    return hr;
}

void WinTvTuner::getAvailableModes(QStringList& modes)
{
    long m;
    HRESULT hr = pTuner->GetAvailableModes(&m);
    if ((m & AMTUNER_MODE_DEFAULT))
        modes << "Default";
    if ((m & AMTUNER_MODE_TV))
        modes << "TV";
    if ((m & AMTUNER_MODE_FM_RADIO))
        modes << "FM Radio";
    if ((m & AMTUNER_MODE_AM_RADIO))
        modes << "AM Radio";
    if ((m & AMTUNER_MODE_DSS))
        modes << "DSS";
}

void WinTvTuner::setMode(const QString& mode)
{
    tagAMTunerModeType m;
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
    pTuner->put_Mode(m);
}

void WinTvTuner::getAvailableTVFormats(QStringList& TVformats)
{
    long f;
    HRESULT hr = pTuner->get_AvailableTVFormats(&f);
    if (f & AnalogVideo_NTSC_M)
        TVformats << "NTSC_M";
    if (f & AnalogVideo_NTSC_M_J)
        TVformats << "NTSC_M_J";
    if (f & AnalogVideo_NTSC_433)
        TVformats << "NTSC_433";
    if (f & AnalogVideo_PAL_B)
        TVformats << "PAL_B";
    if (f & AnalogVideo_PAL_D)
        TVformats << "PAL_D";
    if (f & AnalogVideo_PAL_H)
        TVformats << "PAL_H";
    if (f & AnalogVideo_PAL_I)
        TVformats << "PAL_I";
    if (f & AnalogVideo_PAL_M)
        TVformats << "PAL_M";
    if (f & AnalogVideo_PAL_N)
        TVformats << "PAL_N";
    if (f & AnalogVideo_PAL_60)
        TVformats << "PAL_60";
    if (f & AnalogVideo_SECAM_B)
        TVformats << "SECAM_B";
    if (f & AnalogVideo_SECAM_D)
        TVformats << "SECAM_D";
    if (f & AnalogVideo_SECAM_G)
        TVformats << "SECAM_G";
    if (f & AnalogVideo_SECAM_H)
        TVformats << "SECAM_H";
    if (f & AnalogVideo_SECAM_K)
        TVformats << "SECAM_K";
    if (f & AnalogVideo_SECAM_K1)
        TVformats << "SECAM_K1";
    if (f & AnalogVideo_SECAM_L)
        TVformats << "SECAM_L";
    if (f & AnalogVideo_SECAM_L1)
        TVformats << "SECAM_L1";
    if (f & AnalogVideo_PAL_N_COMBO)
        TVformats << "PAL_N_COMBO";
}

void WinTvTuner::setTVFormat(const QString& TVFormat)
{
    // Not Possible
}

void WinTvTuner::getInputType(QStringList& inputTypes)
{
    inputTypes << "Cable" << "Antenna";
}

void WinTvTuner::setInputType(const QString& inputType)
{
    long inputIndex;
    HRESULT hr = pTuner->get_ConnectInput(&inputIndex);
    if (inputType.compare("Antenna") == 0)
        hr = pTuner->put_InputType(inputIndex,TunerInputAntenna);
    else if(inputType.compare("Cable") == 0)
        hr = pTuner->put_InputType(inputIndex,TunerInputCable);
}

void WinTvTuner::getAvailableInputs(QStringList& inputs)
{
    long numInputs;
    HRESULT hr = pTuner->get_NumInputConnections(&numInputs);
    for (int i = 0; i < numInputs; i++)
        inputs << QString::number(i);
}

void WinTvTuner::setCurrentInput(const QString& input)
{
    int i = input.toInt();
    HRESULT hr = pTuner->put_ConnectInput(i);
}

void WinTvTuner::getCountryCode(QString& code)
{
    long c;
    HRESULT hr = pTuner->get_CountryCode(&c);
    code = QString::number(c);
}

void WinTvTuner::setCountryCode(const QString& code)
{
    // TODO per device instead of channel
    pTuner->put_CountryCode(code.toInt());
}

void WinTvTuner::getTuningSpace(QString& space)
{
    long c;
    HRESULT hr = pTuner->get_TuningSpace(&c);
    space = QString::number(c);
}

void WinTvTuner::setTuningSpace(const QString& space)
{
    pTuner->put_TuningSpace(space.toInt());
    // TODO per device instead of channel
}

int WinTvTuner::openChannelConfiguration(const QString& graphFile)
{
    // memory leak release pTuner
    Settings settings;
    if (!settings.loadFile(graphFile)){
        std::cout << "Error cannot load graph file\n";
        return -1;
    }

    const SettingsGroupMap& map = settings.retreiveMap();
    QMap<QString,SettingsMap>::const_iterator i = map.constBegin();
    for(; i != map.constEnd(); ++i) {	
        QString sDName,sName,sInterface;
        settings.retreiveStringValue(i.key(),"sDisplayName",sDName);
        settings.retreiveStringValue(i.key(),"SName",sName);
        settings.retreiveStringValue(i.key(),"sInterface",sInterface);
        if (sInterface.compare("IID_IAMTVTuner") == 0) {
            CComPtr<IBaseFilter> filter = createFilter(sDName);
            if (!filter) {
                std::cout << "Error creating filter " << sName.toStdString().c_str() << "\n";
                return -1;
            }
            HRESULT hr = filter->QueryInterface(IID_IAMTVTuner,(void**)&pTuner);
            if (FAILED(hr)) {
                std::cout << "Error query TV Tuner interface\n";
                return -1;
            }
        }
    }
    return 0;
}

void WinTvTuner::closeChannelConfiguration()
{
    if (pTuner) {
        pTuner->Release();
        pTuner = NULL;
    }
}

void WinTvTuner::getChannelMinMax(int& min,int& max)
{
    long lMin = 0, lMax = 0;
    HRESULT hr = pTuner->ChannelMinMax(&lMin, &lMax);
    min = lMin;
    max = lMax;
}

// 100 is full volume 0 is no volume
void WinTvTuner::setVolume(int volume)
{
    int dbVol = -2000 + (volume*20);
    pAudio->put_Volume(dbVol);
}

void WinTvTuner::nextChannel()
{
    const ChannelInfo& info = nextItem();
    // TODO: if channel is on another device switch graph
    pTuner->put_Channel(info.channel.toInt(),AMTUNER_SUBCHAN_NO_TUNE,AMTUNER_SUBCHAN_NO_TUNE);
    renderer->updateOverlayText(info.channelName);
}

void WinTvTuner::prevChannel()
{
    const ChannelInfo& info = prevItem();
    // TODO: if channel is on another device switch graph
    pTuner->put_Channel(info.channel.toInt(),AMTUNER_SUBCHAN_NO_TUNE,AMTUNER_SUBCHAN_NO_TUNE);
    renderer->updateOverlayText(info.channelName);
}

void WinTvTuner::gotoChannel(const QString& name)
{
    const ChannelInfo& info = findItem(name);
    // TODO: if channel is on another device switch graph
    pTuner->put_Channel(info.channel.toInt(),AMTUNER_SUBCHAN_NO_TUNE,AMTUNER_SUBCHAN_NO_TUNE);
    renderer->updateOverlayText(info.channelName);
}

void WinTvTuner::gotoChannelIndex(int index)
{
    if (itemCount()-1 < index)
        return;
    const ChannelInfo& info = findItem(index);
    pTuner->put_Channel(info.channel.toInt(),AMTUNER_SUBCHAN_NO_TUNE,AMTUNER_SUBCHAN_NO_TUNE);
    renderer->updateOverlayText(info.channelName);
}

void WinTvTuner::pauseTuner(bool pause)
{
    mPaused = pause;
    mTogglePaused = true;
}
*/
