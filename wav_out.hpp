#ifndef WAV_OUT_HEADER
#define WAV_OUT_HEADER

#include<windows.h>
#include<mmsystem.h>
#include<mmreg.h>
#include<ksmedia.h>
#include<cassert>
#include<cstdint>
#include<cmath>

//再生フォーマットを設定する
void set_wave_handle(
    WAVEFORMATEX& m_handle,
    const WORD format,
    const uint8_t ch,
    const uint32_t sampling_rate,
    const uint8_t sample_bit_depth,
    const size_t ex_size
);

void set_wave_handle_ext(
    WAVEFORMATEXTENSIBLE& handle,
    const GUID format,
    const uint8_t ch,
    const uint32_t sampling_rate,
    const uint8_t sample_bit_depth
);

//ヘッダを削除して領域を確保する
void release_wavehdr(HWAVEOUT& out_handle_ptr, WAVEHDR& header_ptr);

//デバイスを閉じる
void close_output_device(HWAVEOUT& out_handle_ptr);

#endif