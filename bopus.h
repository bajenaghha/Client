#ifndef BOPUS_H
#define BOPUS_H

#include <QString>
#include <opus.h>
#include <QSharedPointer>
#include <QAudioFormat>

class BOpus
{
    BOpus(){}
public:
    enum Application
    {
        VOIP = 2048,
        AUDIO = 2049,
        RESTRICTED_LOWDELAY = 2051
    };

    static const char * error(int err);

    static QString version();

    static int frameSize(QAudioFormat f);

    class Encoder
    {
        OpusEncoder *encoder=0;
        QAudioFormat f;
    public:
        Encoder(QAudioFormat format, Application application, int bitrate);

        //data must be able to be converted to 16bit
        int encode(unsigned char *ptrdatapcm8bit, int size , unsigned char* output , int max_data_bytes);

        QByteArray encode(QByteArray pcmdata);

        ~Encoder();
    };

    class Decoder
    {
        OpusDecoder* decoder=0;
        QAudioFormat f;
    public:
        Decoder(QAudioFormat format);

        QByteArray decode(unsigned char *ptrdataopus, int size);

        QByteArray decode(QByteArray encodeddata);

        ~Decoder();
    };
};
#endif // BOPUS_H
