QT += core gui opengl network webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mediaviewer
DESTDIR = ../mediaviewer-repository/bin
TEMPLATE = app
CONFIG += console

# Library dependencies
win32:LIBS += C:\Development\Libraries\openal\libs\Win64\OpenAL32.lib
win32:LIBS += C:\Development\Libraries\ffmpeg\lib\avcodec.lib
win32:LIBS += C:\Development\Libraries\ffmpeg\lib\avformat.lib
win32:LIBS += C:\Development\Libraries\ffmpeg\lib\avutil.lib
win32:LIBS += C:\Development\Libraries\ffmpeg\lib\swresample.lib

win32:LIBS += C:\Development\Libraries\glew\lib\glew32.lib

CONFIG(debug, debug|release) {
  win32:LIBS += C:\Development\Libraries\ftgl\ftgld.lib
  win32:LIBS += C:\Development\Libraries\freetype\lib\freetyped.lib
  win32:LIBS += C:\Development\Libraries\baseclasses\lib\strmbasd.lib
  win32:LIBS += -lAdvapi32
  win32:LIBS += -lUser32 -lopengl32 -lWinmm -lStrmiids -lOle32 -lOleAut32
}
CONFIG(release, debug|release) {
  win32:LIBS += C:\Development\Libraries\ftgl\ftgld.lib
  win32:LIBS += C:\Development\Libraries\freetype\lib\freetype.lib
  win32:LIBS += C:\Development\Libraries\baseclasses\lib\strmbase.lib
  win32:LIBS += -lUser32 -lOpengl32 -lWinmm -lStrmiids -lOle32 -lOleAut32
}

# Include dependencies
INCLUDEPATH += $$PWD/incl
INCLUDEPATH += C:\Development\Libraries\openal\include
INCLUDEPATH += C:\Development\Libraries\ffmpeg\include
INCLUDEPATH += C:\Development\Libraries\glew\include
INCLUDEPATH += C:\Development\Libraries\ftgl
INCLUDEPATH += C:\Development\Libraries\freetype\include\freetype2
INCLUDEPATH += C:\Development\Libraries\baseclasses


INCLUDEPATH += album
INCLUDEPATH += core
INCLUDEPATH += gui
INCLUDEPATH += logger
INCLUDEPATH += playlist
INCLUDEPATH += remote
INCLUDEPATH += settings
INCLUDEPATH += tuner

SOURCES += \ 
    album/AlbumController.cpp \
    album/Albumlist.cpp \
    album/ThumbnailExtractor.cpp \
    core/AudioDecoder.cpp \
    core/AudioRenderer.cpp \
    core/Demuxer.cpp \
    core/FileStreamer.cpp \
    core/GLProgram.cpp \
    core/GLShader.cpp \
    core/PixelFormatConverter.cpp \
    core/SubParser.cpp \
    core/TcpCommand.cpp \
    core/Timer.cpp \
    core/VideoDecoder.cpp \
    core/VideoRenderer.cpp \
    gui/ColorControlDialog.cpp \
    gui/ConnectServerDialog.cpp \
    gui/ControllerWidget.cpp \
    gui/CustomButton.cpp \
    gui/CustomSlider.cpp \
    gui/FilterDialog.cpp \
    gui/FilterPresetDialog.cpp \
    gui/ListenServerDialog.cpp \
    gui/MainWindow.cpp \
    gui/MenuWidget.cpp \
    gui/OptionsAddChannel.cpp \
    gui/OptionsDialog.cpp \
    gui/StretchDialog.cpp \
    gui/StretchPresetDialog.cpp \
    gui/UrlDialog.cpp \
    gui/WebView.cpp \
    logger/Log.cpp \
    playlist/Playlist.cpp \
    playlist/PlaylistTableWidget.cpp \
    remote/RemoteControl.cpp \
    settings/Settings.cpp \
    tuner/ChannelInfo.cpp \
    tuner/DumpFilter.cpp \
    tuner/RendererFilter.cpp \
    tuner/TunerChannels.cpp \
    tuner/TunerInterface.cpp \
    tuner/WinTuner.cpp \
    tuner/WinTunerScanner.cpp \
    tuner/WinTvTuner.cpp \
    Main.cpp \
    tuner/DecoderFilter.cpp \
    tuner/Demuxfilter.cpp

HEADERS  += \ 
    album/AlbumController.hpp \
    album/AlbumEntity.hpp \
    album/Albumlist.hpp \
    album/AlbumModel.hpp \
    album/ThumbnailExtractor.hpp \
    core/AudioDecoder.hpp \
    core/AudioRenderer.hpp \
    core/Defines.hpp \
    core/Demuxer.hpp \
    core/GLProgram.hpp \
    core/GLShader.hpp \
    core/FileStreamer.hpp \
    core/PixelFormatConverter.hpp \
    core/StreamState.hpp \
    core/SubParser.hpp \
    core/TcpCommand.hpp \
    core/Timer.hpp \
    core/VideoDecoder.hpp \
    core/VideoRenderer.hpp \
    gui/ColorControlDialog.hpp \
    gui/ConnectServerDialog.hpp \
    gui/ControllerWidget.hpp \
    gui/CustomButton.hpp \
    gui/CustomSlider.hpp \
    gui/CustomWindow.hpp \
    gui/FilterDialog.hpp \
    gui/FilterPresetDialog.hpp \
    gui/ListenServerDialog.hpp \
    gui/MainWindow.hpp \
    gui/MenuWidget.hpp \
    gui/OptionsAddChannel.hpp \
    gui/OptionsDialog.hpp \
    gui/StretchDialog.hpp \
    gui/StretchPresetDialog.hpp \
    gui/UrlDialog.hpp \
    gui/WebView.hpp \
    logger/Log.hpp \
    logger/StdRedirector.hpp \
    playlist/Playlist.hpp \
    playlist/PlaylistTableWidget.hpp \
    remote/RemoteControl.hpp \
    settings/Settings.hpp \
    tuner/ChannelInfo.hpp \
    tuner/DumpFilter.hpp \
    tuner/Mediatypes.hpp \
    tuner/qedit.h \
    tuner/RendererFilter.hpp \
    tuner/TunerChannels.hpp \
    tuner/TunerInterface.hpp \
    tuner/WinFrameGrabCallback.hpp \
    tuner/WinTuner.hpp \
    tuner/WinTunerScanner.hpp \
    tuner/WinTunerScanRequest.hpp \
    tuner/WinTvTuner.hpp \
    tuner/DecoderFilter.hpp \
    tuner/DemuxFilter.hpp

DISTFILES += \
    ../shaders/PixelFormat/bgr24.frag \
    ../shaders/PixelFormat/nv12.frag \
    ../shaders/PixelFormat/rgb24.frag \
    ../shaders/PixelFormat/rgb555le.frag \
    ../shaders/PixelFormat/yuv410p.frag \
    ../shaders/PixelFormat/yuv420p.frag \
    ../shaders/PixelFormat/yuvj420p.frag \
    ../shaders/PixelFormat/yuyv422.frag
