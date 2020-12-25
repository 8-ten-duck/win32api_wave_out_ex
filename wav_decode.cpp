#include"wav_decode.hpp"

//RIFFのサブチャンクの名前の文字数
static const std::size_t riff_chnk_name_len = 4;

/**
 * WAVEを持つRIFFのヘッダかを判断する
 * @param header 調べるヘッダ
 * @return 判断結果
 */
static const bool is_riff_wav_header(const riff_head& header){
    //RIFFのサブチャンクの名前の文字数
    constexpr std::size_t file_type_name_len = 4;
    
    //ファイルがRIFFかを判断する
    if(std::string("RIFF").find(header.file_type, 0, file_type_name_len) == std::string::npos){
        return false;
    }

    //RIFFのタイプがwavかを判断する
    if(std::string("WAVE").find(header.riff_type, 0, riff_chnk_name_len) == std::string::npos){
        return false;
    }

    return true;
}

/**
 * WAVのfmtチャンクかを判断する
 * @param fmt 調べるチャンク
 * @return 判断結果
 */
static const bool is_fmt_wav_header(const riff_fmt& fmt){
    //ファイルがfmtかを判断する
    if(std::string("fmt ").find(fmt.chunck_info.chunk_type, 0, riff_chnk_name_len) == std::string::npos){
        return false;
    }

    //フォーマットのサイズを確かめる
    if(fmt.chunck_info.chunk_data_size < (sizeof(fmt) - sizeof(fmt.chunck_info))){
        return false;
    }

    //タイプがwavかを判断する
    if(fmt.fmt_id != 1){
        return false;
    }

    //量子化ビット数が8の倍数かを判断する
    if(fmt.bit_depth % 8 != 0){
        return false;
    }

    //データ速度が正しいかを判断する
    if(fmt.data_size_per_sec != (fmt.sampling_rate * fmt.ch * fmt.bit_depth / 8)){
        return false;
    }

    //データ速度が正しいかを判断する
    if(fmt.sample_size != (fmt.ch * fmt.bit_depth / 8)){
        return false;
    }

    //チャンネル数を確かめる
    return true;
}

/**
 * WAVEを持つRIFFのdataチャンクかを判断する
 * @param d_chunck 調べるチャンク
 * @return 判断結果
 */
static const bool is_data_wav_header(const riff_data& d_chunck){
    //dataチャンクかを判断する
    if(std::string("data").find(d_chunck.chunck_info.chunk_type, 0, riff_chnk_name_len) == std::string::npos){
        return false;
    }

    //データサイズは合っているかを判断する
    if(d_chunck.chunck_info.chunk_data_size != d_chunck.wave_data.size()){
        return false;
    }

    return true;
}

/**
 * RIFFチャンクのヘッダを読み込む
 * @param fp FILEポインタ
 * @return 読み込んだRIFFヘッダ
 */
static const riff_head read_riff_head(FILE* fp){
    assert(fp != NULL);

    //シークの位置を先頭に動かす
    int res = fseek(fp, 0, SEEK_SET);
    assert(res == 0);

    //RIFFのヘッド情報
    riff_head riff_header = { 0 };
    //RIFFのヘッド情報を読み込む
    const std::size_t read_size = fread(&riff_header, sizeof(riff_header), 1, fp);
    assert(read_size == 1);

    //読み込んだファイルデータを返す
    return riff_header;
}

/**
 * fmtチャンクを読み込む
 * @param fp FILEポインタ
 * @param offset fmtチャンクの頭
 * @return 読み込んだfmtチャンク
 */
static const riff_fmt read_fmt_chunck(FILE* fp, const long offset){
    assert(fp != NULL);

    //シークの位置をfmtの先頭に動かす
    int res = fseek(fp, offset, SEEK_SET);
    assert(res == 0);

    //RIFFのfmt情報
    riff_fmt fmt_data = { 0 };
    //RIFFのヘッド情報を読み込む
    const std::size_t read_size = fread(&fmt_data, sizeof(fmt_data), 1, fp);
    assert(read_size == 1);

    //読み込んだファイルデータを返す
    return fmt_data;
}

/**
 * dataチャンクの情報を読み込む
 * @param fp FILEポインタ
 * @param offset fmtチャンクの頭
 * @return 読み込んだdataチャンク
 */
static const riff_data read_data_chunck(FILE* fp, const long offset){
    assert(fp != NULL);

    //シークの位置をdataの先頭に動かす
    int res = fseek(fp, offset, SEEK_SET);
    assert(res == 0);

    //RIFFのfmt情報
    riff_data data_info = { 0 };
    //dataチャンクの情報を読み込む
    std::size_t read_size = fread(&data_info.chunck_info, sizeof(data_info.chunck_info), 1, fp);
    assert(read_size == 1);

    //波形データを用意する
    data_info.wave_data = wav_buff(data_info.chunck_info.chunk_data_size, 0);
    //波形データを読み込む
    read_size = fread(data_info.wave_data.data(), sizeof(wav_buff::value_type), data_info.wave_data.size(), fp);
    assert(read_size == data_info.wave_data.size());

    //読み込んだファイルデータを返す
    return data_info;
}

/**
 * 現在の位置から目的のチャンクに移動する
 * @param fp FILEポインタ
 * @param chunck_names 探すチャンク名のリスト
 * @return 最初に見つかったチャンクの名前
 */
static const std::string skip_riff_sub_chunck(FILE* fp, const std::vector<std::string>& chunck_names){
    assert(fp != NULL);
    assert(!chunck_names.empty());

    //チャンク情報
    riff_chunck_info ch_info = { 0 };

    //現在のファイルの位置を取得する
    const long cur_pos = ftell(fp);

    ////チャンク情報が取得できなくなるまで探す
    while(fread(&ch_info, sizeof(ch_info), 1, fp)){
        //目的のチャンクを探す
        const auto ch_name = std::find_if(chunck_names.begin(), chunck_names.end(),
            [&ch_info](const std::string& name){
                return name.find(ch_info.chunk_type, 0, riff_chnk_name_len) != std::string::npos;
        });

        //目的のチャンクを取得したかを判断する
        if(ch_name != chunck_names.end()){
            //読み込んだ分だけ元に戻す
            fseek(fp, -static_cast<long int>(sizeof(ch_info)), SEEK_CUR);

            //読み込んだ名前を返す
            return *ch_name;
        }else{
            //読み込んだチャンクを飛ばす
            fseek(fp, ch_info.chunk_data_size, SEEK_CUR);
        }
    }

    //ファイルのカーソル位置を元に戻す
    fseek(fp, cur_pos, SEEK_SET);

    //空文字列を返す
    return "";
}

wav_buff decode_wav(const std::string& path, riff_fmt& fmt){
    assert(!path.empty());

    //ファイルを読み込む
    FILE* fp = fopen(path.c_str(), "rb");
    //ファイルのオープンに失敗したかを判断する
    if(fp == NULL){
        //空バッファを返す
        return {};
    }

    //RIFFのヘッダを読み込む
    const riff_head rif_header = read_riff_head(fp);
    //ファイルを読み込めたかを判断する
    if(!is_riff_wav_header(rif_header)){
        return {};
    }

    //読み込むサブフォーマットの一覧
    std::vector<std::string> read_chnk_names = {"fmt ", "data"};
    //dataチャンク情報の格納先
    riff_data data_chunck = { 0 };
    
    /*サブチャンクを読み込む*/

    //発見されたチャンク名
    std::string chnk_name = "";
    //目的のチャンクが発見されるまで読み込みを行う
    while(!(chnk_name = skip_riff_sub_chunck(fp, read_chnk_names)).empty()){
        //目的のチャンク毎に読み込む
        if(chnk_name == read_chnk_names[0]){
            //フォーマット情報
            fmt = read_fmt_chunck(fp, ftell(fp));
            //ファイルを読み込めたかを判断する
            if(!is_fmt_wav_header(fmt)){
                return {};
            }
        }else if(chnk_name == read_chnk_names[1]){
            //dataチャンクの情報を取得する
            data_chunck = read_data_chunck(fp, ftell(fp));
            //ファイルを読み込めたかを判断する
            if(!is_data_wav_header(data_chunck)){
                return {};
            }
        }
    };

    //データを返す
    return data_chunck.wave_data;
}