・SRAM ルーチンを割込みハンドラに設定
・割り込みを使用したシリアル入出力
・ここら辺が FIX したら FLASH に焼いて，「SD カード上のファームを SRAM にロード」を運用開始
・割り込み? DMA? を利用した SD カード R/W
・SD カード挿入検出
・USB 再初期化
----完了----
・FLASH のルーチンを SRAM プログラムからコールする仕組みを確立

----------------------------
車速・タコ -> 5V-tolerant	4pin
アクセル→抵抗分割		6pin
G センサ→不要
lap shot -> open drain		Φ1.5 jack
wifi→もとから3.3V		4pin?

・その他 I/F
SD カード
リセット SW
input sw * 1
led output * 4

●
・TSC→RTC 32bit
・tacho/spd → 入力キャプチャで行ける? TIM2,3
・磁気センサー → TSC+GPIO 割込み or タイマx2 入力キャプチャ
・AD と G はメインループで加算平均
・SD から FW ロード (できないかも)
  PIO でもいいかも
・SD 書き込み (できないかも)
  バッファ分たまったら (4KB くらい?) DMA
・1/16 秒ごとにシリアル出力 UART or SPI(WiFi)
----------------------------

メモ:
vsd2.eww の Release が Flash 用
            exec_sram が SRAM 実行用
vsd2_sram.eww の Release は動かないっぽい
                 exec_sram が Flash リンク SRAM 実行用

●sram リンク時のシンボル二重定義エラーの回避
\exec_sram\List\vsd2.map を消す
bin/make_entry2.pl を実行する
ビルドすると，二重定義シンボルがすべてリストアップされる
もう一度 bin/make_entry2.pl を実行する

※rom_entry.s 内の symbol を weak にする方法がわからない．
(PUBWKEAK だと EQU で怒られる)
