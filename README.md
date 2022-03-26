# SAST M5

### SEM-IT Agriculture　Support　Tools for M5Stack
SASTのM5Stack版です。
SASTは主として水稲育苗ハウスの温度通知システムです。ハウス内の温度が危なくなるとLINE通知を行ってくれます。<br/>
M5Stack版は内部でPARTを用いて、Lazrurite mini 920Jと接続してホストし、他方のLazrurite mini 920をノード（センサー部）に使用することでセンサー設置距離を100m程度まで離すことが出来ます。ノードはスイッチサイス社製Lazrurite環境センサーを利用し、温湿度、気圧、照度を取得できます。SASTではそのうち温湿度照度を記録できます。ノードは最大6個まで接続可能です。<br/>
指定された温度や湿度を超えた場合にLINE Notifyを利用してメッセージ通知が可能となります。<br/>
これら一連の設定内容は、M5StackのSDカードにini形式にて保存し、読み出すことで設定可能です。

# ハードウエア
## 本体側
- M5Stack BASIC（無線LAN接続必須、時刻および通知のため）
- Lazrurite min 920J（UART接続 RX16, TX17）
- ユニバーサルボード
- 外部アンテナ 
- 専用アンテナケーブル 15cm （902J-SAM）


## センサー側
- Lazurite mini 902J or Lazurite MJ2001
- Lazurite環境センサ基盤（スイッチサイエンス）
- ケース（タカチ CS75N等） 

にて構成可能です。<br/>

# 仕様
To　Be Determine

