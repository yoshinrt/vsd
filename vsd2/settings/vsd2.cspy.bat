@REM このバッチファイルはIAR Embedded WorkbenchC-SPYデバッガによって
@REM  適切な設定を使用するcspybatコマンドライン
@REMユーティリティを実行するためのコマンドラインの準備を支援するものとして生成されています。
@REM
@REM このファイルは新しいデバッグセッションが初期化されるたびに生成されるため、
@REM ファイルを名称変更または移動してから
@REM 変更を行うことを推奨します。
@REM
@REM cspybatは、このバッチファイル名に続いてデバッグファイル名(通常はELF/DWARFまたはUBROFファイル)を入力することにより、
@REM 起動できます。
@REM
@REM 使用可能なコマンドラインのパラメータについては、C-SPYデバッグガイドを参照してください。
@REM 特定の場合に役立つコマンドラインパラメータに関する他のヒント
@REM :
@REM   --download_only   後にデバッグセッションを起動せずにコードのイメージをダウンロードします
@REM                     。
@REM   --silent          サインオンのメッセージを省略します。
@REM   --timeout         実行時間の上限を設定します。
@REM 


"C:\Program Files\IAR Systems\Embedded Workbench 6.5\common\bin\cspybat" "C:\Program Files\IAR Systems\Embedded Workbench 6.5\arm\bin\armproc.dll" "C:\Program Files\IAR Systems\Embedded Workbench 6.5\arm\bin\armsim2.dll"  %1 --plugin "C:\Program Files\IAR Systems\Embedded Workbench 6.5\arm\bin\armbat.dll" --macro "C:\Program Files\IAR Systems\Embedded Workbench 6.5\arm\config\debugger\ST\Trace_STM32F1xx.dmac" --backend -B "--endian=little" "--cpu=Cortex-M3" "--fpu=None" "-p" "C:\Program Files\IAR Systems\Embedded Workbench 6.5\arm\CONFIG\debugger\ST\STM32F103xB.ddf" "--semihosting" "--device=STM32F103xB" 


