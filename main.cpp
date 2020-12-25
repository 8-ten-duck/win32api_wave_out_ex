#include"wav_out.hpp"
#include"wav_decode.hpp"
#include<iostream>
#include<chrono>
#include<thread>

int main(int argc, char** argv){
    //引数の個数が正しいかを判断する
    if(argc < 2){
        //エラーメッセージを出力する
        std::cerr << "wavファイルへのパスを指定して下さい" << std::endl;;
        return -1;
    }
    
    //wavファイルの情報を読み取る
    riff_fmt wav_fmt = { 0 };
    const wav_buff wav_data = decode_wav(argv[1], wav_fmt);
    assert(!wav_data.empty());

    //出力フォーマットの設定
    WAVEFORMATEXTENSIBLE output_ext_format = { 0 };

    set_wave_handle_ext(output_ext_format, KSDATAFORMAT_SUBTYPE_PCM, wav_fmt.ch, wav_fmt.sampling_rate, wav_fmt.bit_depth);

    //出力デバイスのハンドル
    HWAVEOUT output_handle = NULL;
    //出力デバイスの波形情報のアドレス
    const WAVEFORMATEX* const wavOut_info_ptr = reinterpret_cast<WAVEFORMATEX*>(&output_ext_format);
    //出力デバイスのハンドルを取得する
    MMRESULT result = waveOutOpen(&output_handle, WAVE_MAPPER, wavOut_info_ptr, (DWORD_PTR)(NULL), 0, CALLBACK_NULL);
    assert( result == MMSYSERR_NOERROR );

    //再生ヘッダ
    WAVEHDR wav_hdr = { 0 };
    //ヘッダにデータを設定する
    wav_hdr.lpData = (LPSTR)const_cast<int8_t*>(wav_data.data());
    wav_hdr.dwBufferLength = wav_data.size();
    wav_hdr.dwBytesRecorded = 0;
    wav_hdr.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
    wav_hdr.dwLoops = 1;
    wav_hdr.dwUser = 0;

    //ヘッダを使用可能にする
    waveOutPrepareHeader(output_handle, &wav_hdr, sizeof(wav_hdr));

    //バッファを再生する
    waveOutWrite(output_handle, &wav_hdr, sizeof(wav_hdr));

    //再生が終了するまで待機する
    while((wav_hdr.dwFlags & WHDR_DONE) != WHDR_DONE){
        //1ミリ秒停止する
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    //ヘッダーの破棄
    release_wavehdr(output_handle, wav_hdr);
    wav_hdr.lpData = NULL;

    //ライブラリの終了
    close_output_device(output_handle);

    return 0;
}