# ESP32
Wifi, Bluetooth 内蔵の組み込み向け SoC。
Espressif Systems (Shanghai, China) の製品、TSMC 製造。
CPU (ISA) は Tensilica の Xtensa LX6。32 bit RISC。


## esp-idf (Espressif IoT Development Framework)
https://github.com/espressif/esp-idf

Espressif が出している公式 SDK。
FreeRTOS を ESP32 にポーティングして色々付け加えたもの。
C 言語製。
libc は newlib が入っているっぽい。

FreeRTOS はオープンソースの組み込みリアルタイム OS。
様々なアーキテクチャに移植されている。
Amazon に買収されたらしい。

これを clone したディレクトリを環境変数 IDF_PATH に設定して使う。
ツールチェインはバイナリを別途ダウンロードして展開、PATH を通す。

ビルドは `$(IDF_PATH)/make/project.mk` を Makefike から include すると
main/ や components/ 以下を再帰走査し component.mk があるディレクトリを
自動的に全部入れる感じ。
`make menuconfig` で設定変更可能。(Linux のぱ〇り)
`sdk config` ファイルができる。
cmake へは v4.0 で移行予定。らしい。


## arduino-esp32 (Arduino core for the ESP32)
https://github.com/espressif/arduino-esp32

Arduino IDE で ESP32 の開発をできるようにするスタック。
デフォルトだと esp-idf で作成した1コアに固定したスレッド(Task)で
arduino メインループを回す感じ。
これも Espressif のリポジトリにある。
C++ 製。
sdkconfig はこの中にあるものをベースにするとよい。


# M5Stack
https://m5stack.com/

Shenzhen 深セン, China のスタートアップ。
ESP32 に液晶ディスプレイ、ボタン、microSD などを付けた感じ。


## M5Stack
https://github.com/m5stack/M5Stack

M5Stack の Arduino 用ライブラリ。
arduino-esp32 に追加で M5Stack のハードウェア操作を提供する感じ。
C++ 製。


## M5StickC
https://github.com/m5stack/M5StickC

姉妹製品。ハードウェアが異なる場合はこのように別のライブラリが用意されているようだ。


## M5Stack-IDF
https://github.com/m5stack/M5Stack-IDF

Auduino を使わずに make (cmake に移行中) でビルドするサンプル。
git submodule で components/ 以下に arduino-esp32 と M5Stack を入れて
esp-idf ビルドをすればよいようだ。
