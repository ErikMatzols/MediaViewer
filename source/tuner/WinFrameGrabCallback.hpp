#ifndef WINFRAMEGRABCALLBACK_HPP
#define WINFRAMEGRABCALLBACK_HPP

#include <atlbase.h>
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#include "qedit.h"
#include <DShow.h>
#include "VideoRenderer.hpp"
#include "StreamState.hpp"

class WinFrameGrabCallback : public ISampleGrabberCB
{
public:
	WinFrameGrabCallback()
	{
	}

	~WinFrameGrabCallback()
	{
	}

	HRESULT initializeFrameGrabCallback(long width,long height,enum PixelFormat format,VideoRenderer* renderer)
	{
		this->width = width;
		this->height = height;
		this->format = format;
		this->renderer = renderer;
		return S_OK;
	}

	void freeFrameGrabberCallback()
	{
	}

	STDMETHODIMP_(ULONG) AddRef() { return 2; }
	STDMETHODIMP_(ULONG) Release() { return 1; }
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
	{
        if (NULL == ppv)
            return E_POINTER;
        *ppv = NULL;
        if (IID_IUnknown == iid) {
            *ppv = (IUnknown*)this;
            AddRef();
            return S_OK;
        }
        else if (IID_ISampleGrabberCB == iid) {
            *ppv = (ISampleGrabberCB*)this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
	}

	STDMETHODIMP SampleCB(double n,IMediaSample *pms)
	{
        return 0;
	}

	STDMETHODIMP BufferCB(double SampleTime,BYTE * pBuffer,long BufferSize)
	{
		int linesize[4] = {width, 0, 0, 0};
		uint8_t* data[4] = {pBuffer, NULL, NULL, NULL};
		renderer->renderVideo(data, linesize, NULL);
		return 0;
	}

    long getWidth()
    {
        return width;
    }
    long getHeight()
    {
        return height;
    }

    enum PixelFormat getFormat()
    {
        return format;   
    }

protected:

private:
	long width;
	long height;
	VideoRenderer* renderer;
	enum PixelFormat format;
};

#endif