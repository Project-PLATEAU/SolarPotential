# 環境構築手順書

# 1 本書について

本書では、カーボンニュートラル施策推進支援システム(以下「本システム」という。)の利用環境構築手順について記載しています。本システムの構成や仕様の詳細については以下も参考にしてください。

[技術検証レポート](https://www.mlit.go.jp/plateau/file/libraries/doc/plateau_tech_doc_0030_ver01.pdf)

# 2 動作環境

本システムの動作環境は以下のとおりです。

| 項目 | 最小動作環境 | 推奨動作環境 | 
| - | - | - | 
| OS | Microsoft Windows 10 または 11 | 同左 | 
| CPU | Intel Core i3以上 | Intel Core i5以上 | 
| メモリ | 4GB以上 | 8GB以上 | 
| ディスプレイ解像度 | 1024×768以上 |  同左  | 
| ネットワーク       | 範囲選択機能を使用する場合、以下のURLを閲覧できる環境が必要。<br>・地理院地図(国土地理院)<br>http://cyberjapandata.gsi.go.jp<br>地図表示のため標準地図を参照する。 | 同左 | 


# 3 インストール手順

[こちら](https://github.com/Project-PLATEAU/UC22-013-SolarPotential/releases/)
からアプリケーションをダウンロードします。

ダウンロード後、zipファイルを右クリックし、「すべて展開」を選択することで、zipファイルを展開します。

展開されたフォルダ内の「SolarPotential.exe」をダブルクリックすることで、アプリケーションが起動します。

![](../resources/devMan/tutorial_033.png)

# 4 ビルド手順

自身でソースファイルをダウンロードしビルドを行うことで、実行ファイルを生成することができます。\
ソースファイルは
[こちら](https://github.com/Project-PLATEAU/SolarPotential/)
からダウンロード可能です。

## 4-1 フォルダ構成

GitHubからダウンロードしたソースファイルの構成は以下のようになっています。

![](../resources/devMan/tutorial_028.png)

## 4-2 ビルド前準備

LIB\\CommonUtil\\libフォルダにある「opencv.zip」を解凍し、以下の構成になるように配置します。

![](../resources/devMan/tutorial_034.png)

## 4-3 ビルド実行

(1) 本システムのソリューションファイル(SolarPotential.sln)をVisualStudio2019で開きます。

ソリューションファイルはSRC\\EXE\\SolarPotentialに格納されています。

(2) SolarPotential.slnをVisualStudio2019で開くと、ソリューション'SolarPotential'に6つのプロジェクトが表示されます。

以下の赤枠部分のように、ソリューション構成を【Release】に、ソリューションプラットフォームを【x64】に設定します。

![](../resources/devMan/tutorial_029.png)

(3) \[ビルド\]>\[ソリューションのビルド\]を選択し、ソリューション全体をビルドします。

![](../resources/devMan/tutorial_030.png)

(4) ビルドが正常に終了すると、ソリューションファイルと同じフォルダにあるbin\\Releaseフォルダに実行ファイルが生成されます。

![](../resources/devMan/tutorial_031.png)


## 4-4 参考情報

### 4-4-1 セキュリティによるビルドエラー

ダウンロードしたソリューションをビルドする際に、ビルドエラーとなり、次のメッセージが出力されるケースがあります。

**「(ファイル名)を処理できませんでした。インターネットまたは制限付きゾーン内にあるか、ファイルにWeb のマークがあるためです。
これらのファイルを処理するには、Webのマークを削除してください。」**

この場合は該当するファイルのプロパティを開き、全般タブ内の「セキュリティ」の項目について\[許可する\]にチェックを入れてください。

![](../resources/devMan/tutorial_032.png)


### 4-4-2 ソースファイル構成

ソースファイルの構成と機能は以下のようになっています。コードを修正する際の参考としてください。

|フォルダ名|詳細|
|---|---|
|AggregateData|パネル設置適地判定のデータ管理|
|AnalyzeData|解析・シミュレーションのデータ管理|
|Analyzer|解析・シミュレーションの処理|
|JudgeSuitablePlace|パネル設置適地判定の処理|
|CommonUtil|SHPやGeoTIFFのデータ処理やファイル操作|
|SolarPotential|GUI|

# 5 準備物一覧

アプリケーションを利用するために以下のデータを入手します。

|     | データ名              | 用途                 | 入手先                                                                                                               | 入力方法           |
| --- | --------------------- | -------------------- | -------------------------------------------------------------------------------------------------------------------- | ------------------ |
| ①  | 3D都市モデル(CityGML) | 全般                 | G空間情報センターから取得します。<br> https://front.geospatial.jp/                                                   | 格納フォルダパス指定                             |
| ②  | 可照時間              | 日射量の推計         | 国立天文台 こよみの計算Webページから取得します。<br> https://eco.mtk.nao.ac.jp/cgi-bin/koyomi/koyomix.cgi            | CSVファイルを手動作成しファイルパス指定          |
| ③  | 平均日照時間          |                      | 気象庁 過去の気象データ・ダウンロードから取得します。<br> https://www.data.jma.go.jp/gmd/risk/obsdl/index.php        | CSVファイルをダウンロードしファイルパス指定      |
| ④  | 積雪深                |                      | NEDO 日射量データベース閲覧システム METPV-20から取得します。<br> https://appww2.infoc.nedo.go.jp/appww/index.html    | CSVファイルを手動作成しファイルパス指定          |
| ⑤  | 土地範囲指定データ    | 解析エリア指定       | 解析したい土地のポリゴンデータを任意で用意します。                                                                   | シェープファイルパス指定                         |
| ⑥  | 制限区域データ        | パネル設置適地判定   | 景観整備区域など、規制がある区域のポリゴンデータを任意で用意します。                                                 | シェープファイルパス指定                         |
| ⑦  | 気象関連データ(積雪)  |                      | 国土数値情報の平均値(気候)メッシュから取得<br>https://nlftp.mlit.go.jp/ksj/gml/datalist/KsjTmplt-G02-v3_0.html <br>※データ利用条件に留意してご使用ください。| シェープファイルをダウンロードしファイルパス指定 |


本システムでは、3D都市モデルの建築物モデル(LOD1、LOD2)、地形(LOD1)、道路(LOD1)、災害リスク(LOD1)を活用します。

| 地物       | 地物型            | 属性区分 | 属性名                                 | 内容                 |
| ---------- | ----------------- | -------- | -------------------------------------- | -------------------- |
| 建築物LOD2 | bldg:Building     | 空間属性 | bldg:RoofSurface                       | 建築物のLOD2の屋根面 |
|            |                   |          | bldg:WallSurface                       | 建築物のLOD2の壁面   |
|            |                   | 主題属性 | bldg:measuredHeight                    | 計測高さ             |
|            |                   |          | uro:buildingDisasterRiskAttribute      | 災害リスク           |
|            |                   |          | uro:buildingID                         | 建物ID               |
|            |                   |          | uro:buildingStructureType              | 構造種別             |
|            |                   |          | uro:buildingStructureOrgType           | 構造種別(独自)     |
|            |                   |          | uro:BuildingRiverFloodingRiskAttribute | 洪水浸水リスク       |
|            |                   |          | uro:depth                              | 浸水深               |
|            |                   |          | uro:BuildingTsunamiRiskAttribute       | 津波浸水リスク       |
|            |                   |          | uro:depth                              | 浸水深               |
|            |                   |          | uro:BuildingLandSlideRiskAttribute     | 土砂災害リスク       |
| 建築物LOD1 | bldg:Building     | 空間属性 | bldg:lod1Solid                         | 建築物のLOD1の立体   |
| 地形LOD1   | dem:ReliefFeature | 空間属性 | dem:tin                                | 地形LOD1の面         |
| 道路LOD1   | tran:Road | 空間属性 | tran:lod1MultiSurface                                | 道路LOD1の面         |
| 災害リスク(浸水)LOD1 | wtr:WaterBody | 空間属性 | wtr:lod1MultiSurface                   | LOD1の面         |
| 災害リスク(津波)LOD1 | wtr:WaterBody | 空間属性 | wtr:lod1MultiSurface                   | LOD1の面         |
| 災害リスク(土砂災害)LOD1 | urf:SedimentDisasterProneArea | 空間属性 | urf:lod1MultiSurface | LOD1の面         |
