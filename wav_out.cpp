#include"wav_out.hpp"

//ハンドルの設定関数
void set_wave_handle(
    WAVEFORMATEX& handle,
    const WORD format,
    const uint8_t ch,
    const uint32_t sampling_rate,
    const uint8_t sample_bit_depth,
    const size_t ex_size){
        //1バイトに必要なビット数
        const size_t nbit_of_byte = 8;
        //フォーマットが有効であることを保証する
        assert(format != WAVE_FORMAT_UNKNOWN);
        //サンプルのビット深度が有効であることを確かめる
        assert((sample_bit_depth % nbit_of_byte) == 0);
        //ハンドルに値をセットする
        handle.wFormatTag = format;
        handle.nChannels = ch;
        handle.nSamplesPerSec = sampling_rate;
        handle.wBitsPerSample = sample_bit_depth;
        handle.cbSize = ex_size;
        handle.nAvgBytesPerSec = sampling_rate * ch * sample_bit_depth / nbit_of_byte;
        handle.nBlockAlign = ch * sample_bit_depth / nbit_of_byte;
        //サンプルのビット深度が有効であることを確かめる
        assert((handle.wBitsPerSample % nbit_of_byte) == 0);
}

//ハンドルの設定関数
void set_wave_handle_ext(
    WAVEFORMATEXTENSIBLE& handle,
    const GUID format,
    const uint8_t ch,
    const uint32_t sampling_rate,
    const uint8_t sample_bit_depth){
        //1バイトに必要なビット数
        const size_t nbit_of_byte = 8;
        
        /*ハンドルに値をセットする*/
        //再生形式
        handle.SubFormat = format;
        //1サンプルのビット数
        handle.Samples.wReserved = 0;
        handle.Samples.wValidBitsPerSample = sample_bit_depth;
        //チャンネルの形式
        handle.dwChannelMask = std::pow(2, ch) - 1;

        //Formatのビット数を求める
        const size_t quoti_bit_num = sample_bit_depth / nbit_of_byte;
        const size_t remain_bit_num = sample_bit_depth % nbit_of_byte;
        const size_t addition_bit_num = std::floor(remain_bit_num / (remain_bit_num + 1));
        const uint8_t wav_format_bit = (quoti_bit_num + addition_bit_num) * nbit_of_byte;
        assert((wav_format_bit % nbit_of_byte) == 0);

        //FormatのcbSizeを求める
        const size_t ext_size = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);

        //Formatを設定する
        set_wave_handle(handle.Format, WAVE_FORMAT_EXTENSIBLE, ch, sampling_rate, wav_format_bit, ext_size);
}

void release_wavehdr(HWAVEOUT& out_handle_ptr, WAVEHDR& header_ptr){
    //再生を停止する
    waveOutReset(out_handle_ptr);

    //バッファを解除する
    waveOutUnprepareHeader(out_handle_ptr, &header_ptr, sizeof(WAVEHDR));
}

void close_output_device(HWAVEOUT& out_handle_ptr){
    //出力デバイスを閉じる
    waveOutClose(out_handle_ptr);
}