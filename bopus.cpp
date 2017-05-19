#include "bopus.h"

const char * BOpus::error(int err)
{
    return opus_strerror(err);
}

QString BOpus::version()
{
    return opus_get_version_string();
}

BOpus::Encoder::Encoder(QAudioFormat format, Application application ,int bitrate):f(format) {
    int err;
    encoder = opus_encoder_create(format.sampleRate(), format.channelCount(), application, &err);
    if (err<0) {
        qFatal("opus_encoder_create: %s", error(err));
    }

    err = opus_encoder_ctl(encoder,OPUS_SET_BITRATE(bitrate));
    if (err<0) {
        qFatal("opus_encoder_ctl: %s",error(err));
    }

}


int BOpus::Encoder::encode(unsigned char *ptrdatapcm8bit, int size , unsigned char* output , int max_data_bytes)
{
    // convert 8 bit to 16 bit array
    // so each 16 bit array element is 2 times greater than 8 bit
    QByteArray datapcm16bit;
    datapcm16bit.resize(size);
    opus_int16 *ptrdataopus16bit =
            reinterpret_cast<opus_int16 *>(datapcm16bit.data());

    for (int i=0;i<size/2;i++)
        ptrdataopus16bit[i]=ptrdatapcm8bit[2*i+1]<<8|ptrdatapcm8bit[2*i];

    return opus_encode(encoder, ptrdataopus16bit, size/(f.channelCount()*2), output, max_data_bytes);
}

QByteArray BOpus::Encoder::encode(QByteArray pcmdata)
{
    QByteArray cbr;
    cbr.resize(4000);
    int size = encode(reinterpret_cast<unsigned char *>(pcmdata.data()),
                      pcmdata.size(),reinterpret_cast<unsigned char *>(cbr.data()),cbr.size());
    cbr.resize(size);
    return cbr;
}

BOpus::Encoder::~Encoder()
{
    if (encoder)
        opus_encoder_destroy(encoder);
}

BOpus::Decoder::Decoder(QAudioFormat format):f(format) {
    int err;
    decoder = opus_decoder_create(format.sampleRate(), format.channelCount(), &err);
    if (err<0)
        qFatal("opus_decoder_create: %s",error(err));
}

int BOpus::frameSize(QAudioFormat f)
{
    //samplerate * samplesize * channel /8
    return (f.sampleRate() * f.sampleSize() * f.channelCount()) / 8;
}

QByteArray BOpus::Decoder::decode(unsigned char *ptrdataopus, int size)
{
    QByteArray pcmdata;
    pcmdata.resize(((frameSize(f)*60)/1000)*2);
    opus_int16* ptrpcmdata = reinterpret_cast<opus_int16*>(pcmdata.data());

    int frame_size = opus_decode(decoder, ptrdataopus,
                                 size,ptrpcmdata ,
                                 pcmdata.size(), 0);
    QByteArray pcmdata8bit;
    pcmdata8bit.resize(frame_size*2*f.channelCount());
    unsigned char* ptrpcmdata8bit = reinterpret_cast<unsigned char*>( pcmdata8bit.data() );

    for(int i=0;i<frame_size*f.channelCount();i++)
    {
        ptrpcmdata8bit[2*i]=ptrpcmdata[i]&0xFF;
        ptrpcmdata8bit[2*i+1]=(ptrpcmdata[i]>>8)&0xFF;
    }
    return pcmdata8bit;
}

QByteArray BOpus::Decoder::decode(QByteArray encodeddata)
{
    return decode(reinterpret_cast<unsigned char*>(encodeddata.data()),encodeddata.size());
}

BOpus::Decoder::~Decoder()
{
    if (decoder)
        opus_decoder_destroy(decoder);
}
