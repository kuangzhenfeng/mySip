#-------------------------------------------------
#
# Project created by QtCreator 2019-07-22T13:19:38
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia

#msvc:QMAKE_CXXFLAGS += -execution-charset:utf-8

#QMAKE_LFLAGS += -Wl,-rpath=/mnt/qt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sip
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        alsa_audio.cpp \
        callwidget.cpp \
        g711/g711.c \
        g711/g711_encode.c \
        g711/g711_table.c \
        global.cpp \
        main.cpp \
        mainwidget.cpp \
        mediaTrans/mediatrans.cpp \
        mediaTrans/ortptrans.cpp \
        myLib/codec/myCodec.cpp \
        myLib/log/myLog.cpp \
        myLib/timer/myTimer.cpp \
        mypushbutton.cpp \
        network.cpp \
        opencv_process.cpp \
        sipCall.cpp \
        unused/ffmpegtest.cpp \
        unused/videostreamreceiver.cpp \
        v4l2_video.cpp

HEADERS += \
        alsa_audio.h \
        callwidget.h \
        g711/g711.h \
        g711/g711_encode.h \
        g711/g711_table.h \
        global.h \
        mainwidget.h \
        mediaTrans/mediatrans.h \
        mediaTrans/ortptrans.h \
        myLib/codec/myCodec.h \
        myLib/log/myLog.h \
        myLib/timer/myTimer.h \
        mypushbutton.h \
        network.h \
        opencv_process.h \
        sipCall.h \
        unused/ffmpegtest.h \
        unused/videostreamreceiver.h \
        v4l2_video.h

FORMS += \
        callwidget.ui \
        mainwidget.ui \
        callwidget.ui \
        unused/ffmpegtest.ui \
        unused/videostreamreceiver.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target





DISTFILES += \
    TODO

RESOURCES += \
    image.qrc






LIBS += -lrt \
        -lpthread


LIBS += -LC:/Users/StevenK/Sync/qt/lib/libcares_install/lib/ -lcares \
        -LC:/Users/StevenK/Sync/qt/lib/openssl_install/lib/ -lcrypto -lssl \
        -LC:/Users/StevenK/Sync/qt/lib/libosip2_install/lib/ -losip2 -losipparser2 \
        -LC:/Users/StevenK/Sync/qt/lib/libexosip2_install/lib/ -leXosip2 \
        -LC:/Users/StevenK/Sync/qt/lib/alsa_install/lib/ -lasound \
        -LC:/Users/stevenk/Sync/qt/lib/ffmpeg_install/lib/ -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale \
        -LC:/Users/stevenk/Sync/qt/lib/ortp_install/lib/ -lortp \
        -LC:/Users/stevenk/Sync/qt/lib/libx264_install/lib/ -lx264 \
        -LC:/Users/stevenk/Sync/qt/lib/sqlite3_install/lib/ -lsqlite3 \
        -LC:/Users/stevenk/Sync/qt/lib/opencv_install/lib/ -lopencv_core -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lopencv_dnn -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_flann





INCLUDEPATH += C:/Users/StevenK/Sync/qt/lib/libcares_install/include \
               C:/Users/StevenK/Sync/qt/lib/openssl_install/include \
               C:/Users/StevenK/Sync/qt/lib/libosip2_install/include \
               C:/Users/StevenK/Sync/qt/lib/libexosip2_install/include \
               C:/Users/StevenK/Sync/qt/lib/alsa_install/include \
               C:/Users/stevenk/Sync/qt/lib/ffmpeg_install/include \
               C:/Users/stevenk/Sync/qt/lib/ortp_install/include \
               C:/Users/stevenk/Sync/qt/lib/libx264_install/include \
               C:/Users/stevenk/Sync/qt/lib/sqlite3_install/include \
               C:/Users/stevenk/Sync/qt/lib/opencv_install/include/opencv4


