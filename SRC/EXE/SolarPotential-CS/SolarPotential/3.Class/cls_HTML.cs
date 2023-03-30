using System;

namespace SolarPotential._3.Class
{
    class cls_HTML
    {
        private string url = "http://cyberjapandata.gsi.go.jp/xyz/std/{z}/{x}/{y}.png";

        public String GetHTMLText()
        {
            String text = "";

            text += "<!DOCTYPE html />\r\n";
            text += "<html>\r\n";
            text += "<head>\r\n";
            text += "    <title>GetMapTile</title>\r\n";
            text += "\r\n";
            text += "    <meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" />\r\n";
            text += "    <meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\" />\r\n";
            text += "    <meta http-equiv=\"content-style-type\" content=\"text/css\" />\r\n";
            text += "    <meta http-equiv=\"content-script-type\" content=\"text/javascript\" />\r\n";
            text += "    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=10\" />\r\n";
            text += "    <link rel=\"stylesheet\" href=\"" + System.IO.Directory.GetCurrentDirectory() + "/9.Others/ol.css\" type=\"text/css\" />\r\n";
            text += "    <script src=\"" + System.IO.Directory.GetCurrentDirectory() + "/9.Others/ol.js\" type=\"text/javascript\"></script>\r\n";
            text += "    <style>\r\n";
            text += "        html, body, .map\r\n";
            text += "        {\r\n";
            text += "            margin: 0;\r\n";
            text += "            padding: 0;\r\n";
            text += "            width: 100%;\r\n";
            text += "            height: 100%;\r\n";
            text += "            background-color: #DDDDDD;\r\n";
            text += "        }\r\n";
            text += "    </style>\r\n";
            text += "    <script type=\"text/javascript\">\r\n";
            text += "  var  Zoom = new ol.control.Zoom();\r\n";

            text += "var m_map;\r\n";
            text += "var m_layer;\r\n";
            text += "var m_vector;\r\n";
            text += "\r\n";
            text += "var m_extent = 0;\r\n";
            text += "var m_extentInteraction;\r\n";
            text += "\r\n";
            text += "var source;\r\n";
            text += "var m_view;\r\n";
            text += "function init() {\r\n";
            text += "    m_view = new ol.View({\r\n";
            text += "        projection: \"EPSG:3857\",\r\n";
            text += "        maxZoom: 18,\r\n";
            text += "    });\r\n";
            text += "\r\n";
            text += "    var m_interaction = new ol.Collection();\r\n";
            text += "    m_interaction.push(new ol.interaction.DragPan());\r\n";
            text += "    m_interaction.push(new ol.interaction.DoubleClickZoom());\r\n";
            text += "    m_interaction.push(new ol.interaction.KeyboardPan());\r\n";
            text += "    m_interaction.push(new ol.interaction.KeyboardZoom());\r\n";
            text += "    m_interaction.push(new ol.interaction.MouseWheelZoom());\r\n";
            text += "\r\n";
            text += "    m_layer = new ol.layer.Tile({\r\n";
            text += "        source: new ol.source.XYZ({\r\n";
            text += "            projection: \"EPSG:3857\"\r\n";
            text += "        })\r\n";
            text += "    });\r\n";
            text += "\r\n";
            text += "    source = new ol.source.Vector({ wrapX: false });\r\n"; // ※ここ変更
            text += "    m_vector = new ol.layer.Vector({\r\n";
            text += "        source: source,\r\n";
            text += "        style: new ol.style.Style({\r\n";
            text += "            fill: new ol.style.Fill({\r\n";
            text += "                color: 'rgba(255, 255, 255, 0.5)'\r\n";
            text += "            }),\r\n";
            text += "            stroke: new ol.style.Stroke({\r\n";
            text += "                color: '#ff3333',\r\n";
            text += "                width: 3\r\n";
            text += "            })\r\n";
            text += "        })\r\n";
            text += "    });\r\n";
            text += "\r\n";
            text += "    var geometryFunction = function (coordinates, geometry) {\r\n";
            text += "        if (!geometry) {\r\n";
            text += "            geometry = new ol.geom.Polygon(null);\r\n";
            text += "        }\r\n";
            text += "        var start = coordinates[0];\r\n";
            text += "        var end = coordinates[1];\r\n";
            text += "        geometry.setCoordinates([\r\n";
            text += "            [start, [start[0], end[1]], end, [end[0], start[1]], start]\r\n";
            text += "        ]);\r\n";
            text += "        return geometry;\r\n";
            text += "    };\r\n";
            text += "\r\n";
            text += "    m_extentInteraction = new ol.interaction.Draw({\r\n";
            text += "        source: source,\r\n";
            text += "        type: 'LineString',\r\n";
            text += "        geometryFunction: geometryFunction,\r\n";
            text += "        maxPoints: 2\r\n";
            text += "    });\r\n";
            text += "\r\n";

            text += "    m_extentInteraction.on('drawstart', function(e){ ";
            text += "        m_vector.getSource().clear();\r\n";
            text += "    });\r\n";
            // 選択範囲座標取得時のイベント
            text += "    m_extentInteraction.on('drawend', function(e){ ";
            text += "        feature_add(e.feature);\r\n";
            text += "        source = new ol.source.Vector();\r\n";
            text += "        source.addFeature(e.feature);\r\n";
            text += "        m_vector.setSource(source);\r\n";      // レイヤにソースを追加する
            text += "    });\r\n";
            text += "\r\n";
            text += "    m_map = new ol.Map({\r\n";
            text += "        target: 'map',\r\n";
            text += "        layers: [m_layer,m_vector],\r\n";
            text += "        view: m_view,\r\n";
            text += "        controls: ol.control.defaults({\r\n";
            text += "        zoom: false,\r\n";
            text += "        attributionOptions: { collapsible: false } \r\n"; // 地図タイル出典を表示(折りたたみなし)
            text += "        }),\r\n";
            text += "        interactions:m_interaction,\r\n";
            text += "        logo: false,\r\n";
            text += "    });\r\n";
            text += "    \r\n";
            text += "    \r\n";
            text += "    m_map.on(\"moveend\", MoveEnd);\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 画面キャプチャ前にコントロールを非表示にする
            text += "function MapCaptureBefore() {\r\n";
            text += "    m_map.removeControl(Zoom);\r\n";
            text += "}\r\n";

            // 画面キャプチャ前にコントロールを非表示にする
            text += "function MapAddControl() {\r\n";
            text += "    m_map.addControl(Zoom);\r\n";
            text += "}\r\n";

            // 画面描画時に地図表示するパターン
            text += "function MapDisp(top, bottom, left, right) {\r\n";
            text += "\r\n";
            // zoomレベルの設定（最大最小緯度経度により設定）
            text += $"   var extent = ol.proj.transformExtent([parseFloat(left), parseFloat(bottom), parseFloat(right), parseFloat(top)], 'EPSG:4326', 'EPSG:3857');\r\n";
            text += "    m_map.getView().fit(extent, m_map.getSize());\r\n";
            // 指定の緯度経度から適切なズームレベルをセット（最大ズームレベルとする）
            text += "    var maxzoom = m_map.getView().getZoom();\r\n";
            text += "    m_view.setMinZoom(maxzoom);\r\n";
            // 引数で受け取った座標をviewにセットする
            text += "    m_map.setView(new ol.View({\r\n";
            text += "       extent:ol.proj.transformExtent([parseFloat(left), parseFloat(bottom), parseFloat(right), parseFloat(top)], 'EPSG:4326', 'EPSG:3857'),\r\n";
            text += "       zoom: maxzoom,\r\n";
            text += "        maxZoom: 18,\r\n";
            text += "    }));\r\n";
            text += "    m_map.removeLayer(m_layer);\r\n";
            text += "    m_map.removeLayer(m_vector);\r\n";
            text += "    \r\n";
            text += "    MapAddControl();\r\n";
            text += "    m_layer = new ol.layer.Tile({\r\n";
            text += "            source: new ol.source.XYZ({\r\n";
            text += "            attributions: [\r\n";
            text += "                     new ol.Attribution({collapsible: true, collapsed: true,\r\n";
            text += "                     html: \" <a href='https://maps.gsi.go.jp/development/ichiran.html' target='_blank'>出典：国土地理院タイル</a> \"\r\n";
            text += "                     })\r\n";
            text += "            ],\r\n";
            text += $"           url: '{url}',\r\n";
            text += "            projection: \"EPSG:3857\"\r\n";
            text += "        })\r\n";
            text += "    });\r\n";
            text += "\r\n";
            text += "    m_map.addLayer(m_layer);\r\n";
            text += "    m_map.addLayer(m_vector);\r\n";
            // 対象地域の中心座標を取得
            text += "    var center_coordinate = getcenter_coordinate(parseFloat(top), parseFloat(bottom),  parseFloat(left),  parseFloat(right));\r\n";
            text += "\r\n";

            text += "    MoveCenter(center_coordinate[0], center_coordinate[1], Number(maxzoom));\r\n";         // 選択範囲の中心座標を設定
            text += "    m_map.getView().setMinZoom(maxzoom)\r\n";
            text += "}\r\n";

            text += "function ChangeExtent(mode) {\r\n";
            text += "    m_extent = mode\r\n";
            text += "     \r\n";
            text += "    if (m_extent == 0) {\r\n";
            text += "        m_map.removeLayer(m_vector);\r\n";                             // 全範囲の場合はレイヤ非表示
            text += "        m_map.removeInteraction(m_extentInteraction);\r\n";
            text += "        m_vector.getSource().clear();\r\n";
            text += "    } else {\r\n";
            text += "        m_map.addLayer(m_vector);\r\n";                                // 選択範囲の場合はレイヤ表示
            text += "        m_map.addInteraction(m_extentInteraction);\r\n";
            text += "    }\r\n";
            text += "}\r\n";
            text += "\r\n";
            text += "window.onload = function () {\r\n";
            text += "    init();\r\n";
            text += "\r\n";
            text += "}\r\n";


            text += "function MoveCenter(coordinateLat, coordinateLon, zoomLevel) {\r\n";
            text += "    m_map.getView().setCenter(ol.proj.transform([coordinateLon, coordinateLat], \"EPSG:4326\", \"EPSG:3857\"));\r\n";
            text += "    m_map.getView().setZoom(zoomLevel);\r\n";
            text += "}\r\n";
            text += "\r\n";
            /** フィーチャの描画を解除します。 */
            text += "   function removeDrawInteraction() {\r\n";
            text += "   m_vector.getSource().removeFeature(m_vector.getSource().getFeatures()[0]);";
            text += "}\r\n";

            text += "function MoveEnd(evt) {\r\n";
            text += "\r\n";
            text += "    var coordinate = ol.proj.transform(m_map.getView().getCenter(), \"EPSG:3857\", \"EPSG:4326\");\r\n";
            text += "\r\n";
            text += "    coordinate[0] = Math.round(coordinate[0] * 1000000) / 1000000;\r\n";
            text += "    coordinate[1] = Math.round(coordinate[1] * 1000000) / 1000000;\r\n";
            text += "\r\n";
            text += "    window.external.MoveEnd(coordinate[1], coordinate[0], m_map.getView().getZoom());\r\n";
            text += "}\r\n";
            text += "\r\n";


            text += "function GetExtent() {\r\n";
            text += "    var extent;\r\n";
            text += "    extent = m_vector.getSource().getFeatures()[0].getGeometry().getExtent();\r\n";
            text += "\r\n";
            text += "    var leftTop = ol.proj.transform(ol.extent.getTopLeft(extent), \"EPSG:3857\", \"EPSG:4326\");\r\n";
            text += "    var rightBottom = ol.proj.transform(ol.extent.getBottomRight(extent), \"EPSG:3857\", \"EPSG:4326\");\r\n";
            text += "\r\n";
            text += "    window.external.GetExtent(leftTop[1], rightBottom[1], wrapLon(leftTop[0]), wrapLon(rightBottom[0]));\r\n";
            text += "}\r\n";
            text += "\r\n";


            text += "function wrapLon(value) {\r\n";
            text += "    var worlds = Math.floor((value + 180) / 360);\r\n";
            text += "    return value - (worlds * 360);\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 最大ズームレベルをhiddenに保持しておく（Ｍａｐ表示後に読みだす想定）
            text += "function setDefaultZoomlevel(zoom) {\r\n";
            text += "    document.getElementById(\"default_zoom_level\").value  = (zoom);\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 選択範囲の座標取得
            text += "function feature_add(feature) {\r\n";
            text += "    var extent;\r\n";
            text += "    extent = feature.getGeometry().getExtent();\r\n";
            text += "\r\n";
            text += "    var leftTop = ol.proj.transform(ol.extent.getTopLeft(extent), \"EPSG:3857\", \"EPSG:4326\");\r\n";
            text += "    var rightBottom = ol.proj.transform(ol.extent.getBottomRight(extent), \"EPSG:3857\", \"EPSG:4326\");\r\n";
            text += "\r\n";
            text += "    window.external.GetExtent(leftTop[1], rightBottom[1], wrapLon(leftTop[0]), wrapLon(rightBottom[0]));\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 更新ボタン押下時の処理
            text += "function feature_update(top, bottom, left, right) {\r\n";
            text += "    var center_coordinate = getcenter_coordinate(top, bottom, left, right); \r\n";
            text += "    var pos = [center_coordinate[1], center_coordinate[0]];\r\n";
            text += "    var coord = ol.proj.transform(pos, 'EPSG:4326', 'EPSG:3857');\r\n";
            text += "    var point = new ol.geom.Point(coord, 'XY');\r\n";  // ポイントを作成する(描画前を考慮し、仮で作成→指定の選択範囲で上書き)
            text += "    var feature = new ol.Feature(point);\r\n";         // フィーチャを作成する  
            text += "    source = new ol.source.Vector();\r\n";             // ソースを作成する   
            text += "    source.addFeature(feature);\r\n";                  // ソースにフィーチャを追加する  
            text += "    m_vector.setSource(source);\r\n";                  // レイヤにソースを追加する
            text += "    var newGeometry = set_coordinate(top, bottom, left, right,new ol.geom.Polygon([]));\r\n";  // 入力値を元に変更した座標を作成
            text += "    m_vector.getSource().getFeatures()[0].setGeometry(newGeometry);";                          // 変更した内容をセット
            text += "}\r\n";
            text += "\r\n";

            // 指定の座標を中心座標を取得する関数
            text += " function getcenter_coordinate(top, bottom, left, right, geometry) {\r\n";
            text += "        var dbl_top = parseFloat(top)\r\n";
            text += "        var dbl_bottom = parseFloat(bottom)\r\n";
            text += "        var dbl_left = parseFloat(left)\r\n";
            text += "        var dbl_right = parseFloat(right)\r\n";
            text += "        var x = (dbl_top - dbl_bottom) / 2  + dbl_bottom; \r\n";       // 中心座標：x
            text += "        var y = (dbl_right - dbl_left) / 2 + dbl_left; \r\n";          // 中心座標：y
            text += "        var x_coordinate  = Math.round(x * 1000000) / 1000000;\r\n";
            text += "        var y_coordinate  = Math.round(y * 1000000) / 1000000;\r\n";
            // 算出した中心座標を返却する
            text += "        return [x_coordinate, y_coordinate];\r\n";
            text += "    };\r\n";

            // 指定の座標をセットする関数
            text += " function set_coordinate(top, bottom, left, right, geometry) {\r\n";
            text += "        var dbl_top = parseFloat(top)\r\n";
            text += "        var dbl_bottom = parseFloat(bottom)\r\n";
            text += "        var dbl_left = parseFloat(left)\r\n";
            text += "        var dbl_right = parseFloat(right)\r\n";
            text += "        if (!geometry) {\r\n";
            text += "            geometry = new ol.geom.Polygon(null);\r\n";
            text += "        }\r\n";
            text += "        var start = ol.proj.transform([dbl_left, dbl_top], \"EPSG:4326\", \"EPSG:3857\");\r\n";
            text += "        var end = ol.proj.transform([dbl_right, dbl_bottom], \"EPSG:4326\", \"EPSG:3857\");\r\n";
            text += "        geometry.setCoordinates([\r\n";
            text += "            [start, [start[0], end[1]], end, [end[0], start[1]], start]\r\n";
            text += "        ]);\r\n";
            text += "        return geometry;\r\n";
            text += "    };\r\n";
            text += "    </script>\r\n";
            text += "</head>\r\n";
            text += "<body>\r\n";
            text += "    <div id=\"map\" class=\"map\">\r\n";
            text += "    </div>\r\n";
            text += "</body>\r\n";
            text += "</html>\r\n";

            return text;
        }
    }
}
