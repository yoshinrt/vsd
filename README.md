VSD プロジェクト
======================
VSD プロジェクトは，自動車のサーキット走行などにおけるデータロガー環境を自作することを目指し，車両情報の取得，車両情報のリアルタイム表示，
オフラインでの車両情報と車載動画との合成，を実現するプロジェクトです．

以下の成果物を含んでいます．  
+ `vsd_filter`: GPS ログを車載動画に合成する [VSD for GPS](https://sites.google.com/site/vsdforgps/home/vsd-for-gps)
+ `vsd2`: ARM マイコンボード (DesignWave 2008/5月号付属) を，車両データ取得用データロガーとするための Firmware
+ `vsdroid`: 上記 ARM マイコンの UI として動作するための Android アプリ
+ `seri2eth`: Fonera+ (FON 2201) を UART⇔Ether コンバータとして使用するためのプログラム
+ `scripts`: 開発過程で作成した多雑なスクリプト

以下は過去の産物であり，すでに未メンテ
+ `vsd`: 秋月 H8 3664 マイコンボード (すでに廃盤?) 用 Firmware
+ `vsd_psp`: PSP 用 LUA playre で動作する，UI アプリ
+ `vsdwin`: Windows 用 UI アプリ
 
使い方
------
### コンパイル ###
追々コンパイル方法等は公開していきます．
