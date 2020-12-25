# win32api_wave_out_ex

## はじめに
ここは某サークルの新刊にのせた記事のおまけ(追加)です。
ここに記載されているソースコードを使用したことによる損害等は一切責任を持ちません。

## 参考元
* [WAVEFORMATEXTENSIBLE構造体](https://docs.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatextensible)
* [WAVEFORMATEX構造体](https://docs.microsoft.com/en-us/previous-versions/dd757713(v=vs.85))

## WAVEFORMATEXTENSIBLEの解説
WAVEFORMATEXTENSIBLE構造体は再生される波形データの形式に関する情報を持ちます。
各メンバ変数が持つ情報は下記の様になっています。

* Format: 波形の基本情報
* Samples: サンプルの形式情報
* dwChannelMask: 使用されるチャンネルの情報
* SubFormat: 波形の形式番号

上記のメンバ変数の内Samplesは共用体で、メンバ変数に入る値は下記の様です。

* wValidBitsPerSample: １サンプルに使用されるビット数
* wSamplesPerBlock: 圧縮された１ブロック内のサンプル数
* wReserved: 必ず0にする

またWAVEFORMATEXTENSIBLEのFormatはwaveOutOpen関数が使用します。

## WAVEFORMATEXTENSIBLEの設定方法
今回は非圧縮のPCM向けの設定を行います。

### SubFormat
ここでは再生したい波形の形式を表す数値を設定します。
設定できる形式はPCMやIEEEのfloatなどがあります。
設定可能な形式の一覧はKsmedia.hに記載されています。

### Samples
まずwReservedに0を設定します。
次にwValidBitsPerSampleに１サンプルに使用されるビット数を設定します。

### dwChannelMask
ここでは再生時のチャンネルをフラグの組み合わせて設定します。
フラグの組み合わせはチャンネルの種類を表すビット値のORから求められます。
各フラグのビット値は[MSDNに記載されています](https://docs.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatextensible#remarks)。

ここで再生される波形データにおけるチャンネルの並びはdwChannelMaskの値で決まります。
dwChannelMaskの最下位ビットに最も近い1を先頭にした並びとなります。

### Format
ここではwaveOutOpen関数で生成する出力デバイスの情報を設定します。
設定の際上記３つのメンバ変数の値を元にして各メンバ変数の値を下記のように決めます。

* wFormatTag: 定数"WAVE_FORMAT_EXTENSIBLE"
* nChannels: dwChannelMaskの立っているビット数
* nSamplesPerSec: 再生時の周波数
* nAvgBytesPerSec: nSamplesPerSec * nBlockAlign
* nBlockAlign: nChannels * (nChannels / 8)
* wBitsPerSample: Samples.wValidBitsPerSampleに最も近い8の倍数
* cbSize: 22以上の整数

## テスト環境
作成したコードのコンパイル及び実行は[MSYS2](https://www.msys2.org/)のMINGW64環境で行いました。
また再生には[JVCのハイレゾ音源を解説するページ](https://www3.jvckenwood.com/audio_w/high-resolution/guidance.html)に添付されていたWAVファイルを使用しました。