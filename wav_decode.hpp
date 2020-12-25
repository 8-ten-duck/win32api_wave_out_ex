#ifndef WAV_DECODE_HEADER
#define WAV_DECODE_HEADER

#include<cstdint>
#include<string>
#include<cstdio>
#include<cassert>
#include<algorithm>
#include<vector>

///波形データを持つバッファの型
using wav_buff = std::vector<int8_t>;

///RIFFヘッダ構造体
struct riff_head{
    ///ファイルタイプ。"RIFF"が入る。
    char file_type[4];

    ///ファイル全体のサイズから8引いた値が入る。
    uint32_t chunk_size;

    ///RIFFのタイプ。"WAVE"が入る。
    char riff_type[4];
};

/**
 * @brief RIFFのサブチャンク構造体
 * @details データの内容はチャンクIDによって適切に処理されると仮定する。
 */
struct riff_chunck_info{
    ///フォーマットチャンクID。
    char chunk_type[4];

    ///チャンクが持つデ－タのサイズ
    uint32_t chunk_data_size;
};

///RIFFチャンクのフォーマットチャンク構造体
struct riff_fmt{
    ///チャンク情報
    riff_chunck_info chunck_info;

    ///フォーマットID。リニアPMCの場合は1。
    uint16_t fmt_id;

    ///チャンネル数。1or2
    uint16_t ch;

    ///サンプリング周波数。単位はHz。
    uint32_t sampling_rate;

    ///1s当たりのデータ量
    uint32_t data_size_per_sec;

    ///1サンプル当たりのデータサイズ。
    uint16_t sample_size;

    ///量子化ビット数
    uint16_t bit_depth;
};

///RIFFチャンクのデータチャンク構造体
struct riff_data{
    ///チャンク情報
    riff_chunck_info chunck_info;

    ///波形データ
    wav_buff wave_data;
};

///wavデータをデコードする
wav_buff decode_wav(const std::string& path, riff_fmt& fmt);

#endif