using System;

namespace SolarPotential._3_Class
{
    /// <summary>
    /// 集計範囲
    /// </summary>
    class ClassAnalyzeHTML
    {
        public static readonly string MapUrl = "http://cyberjapandata.gsi.go.jp/xyz/std/{z}/{x}/{y}.png";

        public string GetHTMLText()
        {
            string text = "";

            text += "<!DOCTYPE html />\r\n";
            text += "<html>\r\n";
            text += "<head>\r\n";
            text += "   <title>GetMapTile</title>\r\n";
            text += "\r\n";
            text += "   <meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" />\r\n";
            text += "   <meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\" />\r\n";
            text += "   <meta http-equiv=\"content-style-type\" content=\"text/css\" />\r\n";
            text += "   <meta http-equiv=\"content-script-type\" content=\"text/javascript\" />\r\n";
            text += "   <meta http-equiv=\"X-UA-Compatible\" content=\"IE=10\" />\r\n";
            text += "   <link rel=\"stylesheet\" href=\"" + System.IO.Directory.GetCurrentDirectory() + "/Assets/Map/ol.css\" type=\"text/css\" />\r\n";
            text += "   <script src=\"" + System.IO.Directory.GetCurrentDirectory() + "/Assets/Map/ol.js\" type=\"text/javascript\"></script>\r\n";
            text += "   <style>\r\n";
            text += "       html, body, .map\r\n";
            text += "       {\r\n";
            text += "           margin: 0;\r\n";
            text += "           padding: 0;\r\n";
            text += "           width: 100%;\r\n";
            text += "           height: 100%;\r\n";
            text += "           background-color: #DDDDDD;\r\n";
            text += "       }\r\n";
            text += "   </style>\r\n";
            text += "   <script type=\"text/javascript\">\r\n";
            text += "       var Zoom = new ol.control.Zoom();\r\n";
            text += "       var RectBtn;\r\n";
            text += "       var PolyBtn;\r\n";
            text += "       var SelBtn;\r\n";
            text += "       var DelBtn;\r\n";
            text += "       var m_map;\r\n";
            text += "       var m_layer;\r\n";
            text += "       var m_vector;\r\n";
            text += "       var m_source;\r\n";
            text += "       var m_shpLayer = Array(3);\r\n";
            text += "       var m_view;\r\n";
            text += "       var featureId = 0;\r\n";
            text += "\r\n";

            // 読み込み
            text += "window.onload = function () {\r\n";
            text += "   init();\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 描画された図形に適用されるスタイル
            text += "var styleFunction = function(feature) {\r\n";
            text += "  var geometry = feature.getGeometry();\r\n";
            text += "  var shape = feature.get('shape');\r\n";
            text += "  var styles = [];\r\n";
            text += "  if (shape == 'Text') {\r\n";
            text += "    styles.push(new ol.style.Style({\r\n";
            text += "      stroke: new ol.style.Stroke({\r\n";
            text += "        color: 'rgba(0,0,0,1)',\r\n";
            text += "        width: 1\r\n";
            text += "      }),\r\n";
            text += "      text: new ol.style.Text({\r\n";
            text += "        font: 'bold 20px \"Open Sans\", \"Arial Unicode MS\", \"sans-serif\"',\r\n";
            text += "        fill: new ol.style.Fill({\r\n";
            text += "          color: 'black'\r\n";
            text += "        }),\r\n";
            text += "        overflow: true,\r\n";
            text += "        offsetX: getTextOffset(feature)[0],\r\n";
            text += "        offsetY: getTextOffset(feature)[1],\r\n";
            text += "        textAlign: 'left',\r\n";
            text += "        text: feature.get('text')\r\n";
            text += "      })\r\n";
            text += "      \r\n";
            text += "    }));\r\n";
            text += "  } else {\r\n";
            text += "    styles.push(new ol.style.Style({\r\n";
            text += "      stroke: new ol.style.Stroke({\r\n";
            text += "        color: '#ff3333',\r\n";
            text += "        width: 3\r\n";
            text += "      }),\r\n";
            text += "      fill: new ol.style.Fill({\r\n";
            text += "        color: 'rgba(255, 255, 255, 0.5)',\r\n";
            text += "      }),\r\n";
            text += "      text: new ol.style.Text({\r\n";
            text += "        font: 'bold 11px \"Open Sans\", \"Arial Unicode MS\", \"sans-serif\"',\r\n";
            text += "        fill: new ol.style.Fill({\r\n";
            text += "          color: 'black'\r\n";
            text += "        }),\r\n";
            text += "        overflow: true,\r\n";
            text += "        offsetX: 0,\r\n";
            text += "        offsetY: 0,\r\n";
            text += "        textAlign: 'center',\r\n";
            text += "        text: feature.get('text')\r\n";
            text += "      })\r\n";
            text += "    }));\r\n";
            text += "  }\r\n";
            text += "\r\n";
            text += "  return styles;\r\n";
            text += "};\r\n";
            text += "\r\n";

            // 読み取り専用の図形に適用されるスタイル
            text += "var readOnlyStyleFunction = function(feature) {\r\n";
            text += "  var geometry = feature.getGeometry();\r\n";
            text += "  var shape = feature.get('shape');\r\n";
            text += "  var styles = [];\r\n";
            text += "  if (shape == 'Text') {\r\n";
            text += "    styles.push(new ol.style.Style({\r\n";
            text += "      stroke: new ol.style.Stroke({\r\n";
            text += "        color: 'rgba(0,0,0,1)',\r\n";
            text += "        width: 1\r\n";
            text += "      }),\r\n";
            text += "      text: new ol.style.Text({\r\n";
            text += "        font: 'bold 11px \"Open Sans\", \"Arial Unicode MS\", \"sans-serif\"',\r\n";
            text += "        fill: new ol.style.Fill({\r\n";
            text += "          color: 'black'\r\n";
            text += "        }),\r\n";
            text += "        overflow: true,\r\n";
            text += "        offsetX: getTextOffset(feature)[0],\r\n";
            text += "        offsetY: getTextOffset(feature)[1],\r\n";
            text += "        textAlign: 'left',\r\n";
            text += "        text: feature.get('text')\r\n";
            text += "      })\r\n";
            text += "      \r\n";
            text += "    }));\r\n";
            text += "  } else {\r\n";
            text += "    styles.push(new ol.style.Style({\r\n";
            text += "      stroke: new ol.style.Stroke({\r\n";
            text += "        color: '#00ff00',\r\n";
            text += "        width: 2\r\n";
            text += "      }),\r\n";
            text += "      text: new ol.style.Text({\r\n";
            text += "        font: 'bold 11px \"Open Sans\", \"Arial Unicode MS\", \"sans-serif\"',\r\n";
            text += "        fill: new ol.style.Fill({\r\n";
            text += "          color: 'black'\r\n";
            text += "        }),\r\n";
            text += "        overflow: true,\r\n";
            text += "        offsetX: 0,\r\n";
            text += "        offsetY: 0,\r\n";
            text += "        textAlign: 'center',\r\n";
            text += "        text: feature.get('text')\r\n";
            text += "      })\r\n";
            text += "    }));\r\n";
            text += "  }\r\n";
            text += "\r\n";
            text += "  return styles;\r\n";
            text += "};\r\n";
            text += "\r\n";

            // 初期化
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
            text += "    m_source = new ol.source.Vector({ wrapX: false });\r\n"; // ※ここ変更
            text += "    m_vector = new ol.layer.Vector({\r\n";
            text += "        source: m_source,\r\n";
            text += "        style: styleFunction\r\n";
            text += "    });\r\n";
            text += "\r\n";
            text += "    m_map = new ol.Map({\r\n";
            text += "        controls: ol.control.defaults({\r\n";
            text += "           zoom: false,\r\n";
            text += "           attributionOptions: { collapsible: false } \r\n"; // 地図タイル出典を表示(折りたたみなし)
            text += "        }).extend([\r\n";
            text += "          RectBtn = new RectangleControl(),\r\n";  // 矩形ボタン
            text += "          PolyBtn = new PolygonControl(),\r\n";    // 多角形ボタン
            text += "          SelBtn = new SelectControl(),\r\n";      // 選択ボタン
            text += "          DelBtn = new DeleteControl()\r\n";       // 消去ボタン
            text += "        ]),\r\n";
            text += "        target: 'map',\r\n";
            text += "        layers: [m_layer, m_vector],\r\n";
            text += "        view: m_view,\r\n";
            text += "        interactions:m_interaction,\r\n";
            text += "        logo: false,\r\n";
            text += "    });\r\n";
            text += "    \r\n";
            text += "    \r\n";
            text += "    m_map.on(\"moveend\", MoveEnd);\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 矩形ボタン
            text += "var RectangleControl = (function(Control) {\r\n";
            text += "   function RectangleControl(opt_options)\r\n";
            text += "   {\r\n";
            text += "       var options = opt_options || { };\r\n";
            text += "       var button = document.createElement('button');\r\n";
            text += "       button.innerHTML = 'ロ';\r\n";
            text += "       button.name = 'Rectangle';\r\n";
            text += "       button.classList.add('map-button');\r\n";
            text += "       var element = document.createElement('div');\r\n";
            text += "       element.className = 'ol-unselectable ol-control';\r\n";
            text += "       element.style.top = '100px';\r\n";
            text += "       element.style.left = '5px';\r\n";
            text += "       element.appendChild(button);\r\n";
            text += "       Control.call(this, {\r\n";
            text += "           element: element,\r\n";
            text += "           target: options.target\r\n";
            text += "       });\r\n";
            text += "       button.addEventListener('click', this.drawRectangle.bind(this), false);\r\n";
            text += "   }\r\n";
            text += "   if (Control) RectangleControl.__proto__ = Control;\r\n";
            text += "   RectangleControl.prototype = Object.create(Control && Control.prototype);\r\n";
            text += "   RectangleControl.prototype.constructor = RectangleControl;\r\n";
            text += "   RectangleControl.prototype.drawRectangle = function drawRectangle() {\r\n";
            text += "       addInteractions('Rectangle');\r\n";
            text += "   };\r\n";
            text += "   return RectangleControl;\r\n";
            text += "}(ol.control.Control));\r\n";
            text += "\r\n";

            // 多角形ボタン
            text += "var PolygonControl = (function(Control) {\r\n";
            text += "   function PolygonControl(opt_options)\r\n";
            text += "   {\r\n";
            text += "       var options = opt_options || { };\r\n";
            text += "       var button = document.createElement('button');\r\n";
            text += "       button.innerHTML = '△';\r\n";
            text += "       button.name = 'Polygon';\r\n";
            text += "       button.classList.add('map-button');\r\n";
            text += "       var element = document.createElement('div');\r\n";
            text += "       element.className = 'ol-unselectable ol-control';\r\n";
            text += "       element.style.top = '135px';\r\n";
            text += "       element.style.left = '5px';\r\n";
            text += "       element.appendChild(button);\r\n";
            text += "       Control.call(this, {\r\n";
            text += "       element: element,\r\n";
            text += "       target: options.target\r\n";
            text += "       });\r\n";
            text += "       button.addEventListener('click', this.drawPolygon.bind(this), false);\r\n";
            text += "   }\r\n";
            text += "   if (Control) PolygonControl.__proto__ = Control;\r\n";
            text += "   PolygonControl.prototype = Object.create(Control && Control.prototype);\r\n";
            text += "   PolygonControl.prototype.constructor = PolygonControl;\r\n";
            text += "   PolygonControl.prototype.drawPolygon = function drawPolygon()\r\n";
            text += "   {\r\n";
            text += "       addInteractions('Polygon');\r\n";
            text += "   };\r\n";
            text += "   return PolygonControl;\r\n";
            text += "}(ol.control.Control));\r\n";
            text += "\r\n";

            text += "var select;\r\n";
            text += "var selectedFeatures;\r\n";
            // 選択ボタン
            text += "var SelectControl = (function (Control) {\r\n";
            text += "    function SelectControl(opt_options) {\r\n";
            text += "        var options = opt_options || {};\r\n";
            text += "        var button = document.createElement('button');\r\n";
            text += "        button.innerHTML = '選';\r\n";
            text += "        button.name = 'Select';\r\n";
            text += "        button.classList.add('map-button');\r\n";
            text += "        var element = document.createElement('div');\r\n";
            text += "        element.className = 'ol-unselectable ol-control';\r\n";
            text += "        element.style.top = '100px';\r\n";
            text += "        element.style.left = '40px';\r\n";
            text += "        element.appendChild(button);\r\n";
            text += "        Control.call(this, {\r\n";
            text += "            element: element,\r\n";
            text += "            target: options.target\r\n";
            text += "        });\r\n";
            text += "        button.addEventListener('click', this.doSelect.bind(this), false);\r\n";
            text += "    }\r\n";
            text += "    if (Control) SelectControl.__proto__ = Control;\r\n";
            text += "    SelectControl.prototype = Object.create(Control && Control.prototype);\r\n";
            text += "    SelectControl.prototype.constructor = SelectControl;\r\n";
            text += "    SelectControl.prototype.doSelect = function doSelect() {\r\n";
            // 選択モードと描画モードの切り替え
            text += "       if (!select) {\r\n";
            text += "           changeMode();\r\n";
            text += "       }\r\n";
            text += "    };\r\n";
            text += "    return SelectControl;\r\n";
            text += "}(ol.control.Control));\r\n";
            text += "\r\n";

            // 消去ボタン
            text += "var DeleteControl = (function (Control) {\r\n";
            text += "   function DeleteControl(opt_options) {\r\n";
            text += "       var options = opt_options || {};\r\n";
            text += "       var button = document.createElement('button');\r\n";
            text += "       button.innerHTML = '消';\r\n";
            text += "       button.name = 'Delete';\r\n";
            text += "       button.classList.add('map-button');\r\n";
            text += "       var element = document.createElement('div');\r\n";
            text += "       element.className = 'ol-unselectable ol-control';\r\n";
            text += "       element.style.top = '135px';\r\n";
            text += "       element.style.left = '40px';\r\n";
            text += "       element.appendChild(button);\r\n";
            text += "       Control.call(this, {\r\n";
            text += "           element: element,\r\n";
            text += "           target: options.target\r\n";
            text += "       });\r\n";
            text += "       button.addEventListener('click', this.doDelete.bind(this), false);\r\n";
            text += "   }\r\n";
            text += "   if (Control) DeleteControl.__proto__ = Control;\r\n";
            text += "   DeleteControl.prototype = Object.create(Control && Control.prototype);\r\n";
            text += "   DeleteControl.prototype.constructor = DeleteControl;\r\n";
            text += "   DeleteControl.prototype.doDelete = function doDelete() {\r\n";
            text += "       selectedFeatures.forEach(function (feature) {\r\n";
            text += "           feature_delete(feature);\r\n";
            text += "           m_source.removeFeature(feature);\r\n";
            text += "       });\r\n";
            text += "       selectedFeatures.clear();\r\n";
            text += "   };\r\n";
            text += "   return DeleteControl;\r\n";
            text += "}(ol.control.Control));\r\n";
            text += "\r\n";

            // 図形描画処理
            text += "var draw, snap;\r\n";
            text += "function addInteractions(type) {\r\n";
            // 描画モードの切り替え
            text += "   if (draw) {\r\n";
            text += "       m_map.removeInteraction(draw);\r\n";
            text += "       draw = null;\r\n";
            text += "   }\r\n";
            text += "   if (select) {\r\n";
            text += "       changeMode();\r\n";
            text += "   }\r\n";
            text += "   var shape = type;\r\n";
            text += "   var geometryFunction;\r\n";
            text += "   if (type == 'Rectangle') {\r\n";
            text += "       type = 'Circle';\r\n";
            text += "       geometryFunction = ol.interaction.Draw.createBox();\r\n";
            text += "   }\r\n";
            text += "   draw = new ol.interaction.Draw({\r\n";
            text += "       source: m_source,\r\n";
            text += "       type: type,\r\n";
            text += "       geometryFunction: geometryFunction,\r\n";
            text += "       geometryName: shape\r\n";
            text += "   });\r\n";
            // 選択範囲座標取得時のイベント
            text += "   draw.on('drawend', function(e){ ";
            text += "       e.feature.setId(++featureId);\r\n"; // IDを設定する
            text += "       setIdText(e.feature);\r\n";
            text += "       feature_add(e.feature);\r\n";
            text += "   });\r\n";
            text += "   m_map.addInteraction(draw);\r\n";
            // マウスカーソルが図形の座標に近づいたときにカーソルを合わせる
            text += "   if (!snap) {\r\n";
            text += "       snap = new ol.interaction.Snap({ source: m_source });\r\n";
            text += "       m_map.addInteraction(snap);\r\n";
            text += "   }\r\n";
            text += "}\r\n";
            text += "\r\n";

            // キー押下イベントを設定(drawstart時だとうまくいかないので暫定対応)
            text += "document.addEventListener('keydown', function(e) {\r\n";
            text += "   if (draw) {\r\n";
            text += "       if (e.key == 'Backspace') {\r\n";
            text += "           draw.removeLastPoint();\r\n";
            text += "       } else if (e.keyCode == 27) {\r\n";    // Esc
            text += "           m_map.removeInteraction(draw);\r\n";
            text += "           draw = null;\r\n";
            text += "       }\r\n";
            text += "   }\r\n";
            text += "});\r\n";

            // IDを描画する
            text += "function setIdText(feature) {\r\n";
            text += "   var id = feature.getId();\r\n";
            text += "   var text = \"A\" + ('000' + id).slice(-3);\r\n";
            text += "   feature.set('text', text);\r\n";
            text += "}\r\n";
            text += "\r\n";

            // モード切替
            text += "var translate;\r\n";
            text += "var modify;\r\n";
            text += "var dragbox;\r\n";
            text += "function changeMode()\r\n";
            text += "{\r\n";
            text += "   if (select) {\r\n";
            // 選択モードから描画モードに切り替え
            text += "       m_map.removeInteraction(select);\r\n";
            text += "       select = null;\r\n";
            text += "       m_map.removeInteraction(translate);\r\n";
            text += "       translate = null;\r\n";
            text += "       m_map.removeInteraction(modify);\r\n";
            text += "       modify = null;\r\n";
            text += "       m_map.removeInteraction(dragbox);\r\n";
            text += "       dragbox = null;\r\n";
            text += "       selectedFeatures.clear();\r\n";
            text += "   } else {\r\n";
            // 描画モードから選択モードへの切り替え
            text += "       if (draw) {\r\n";
            text += "           m_map.removeInteraction(draw);\r\n";
            text += "           draw = null;\r\n";
            text += "       }\r\n";
            text += "       select = new ol.interaction.Select({\r\n";
            text += "           layers: [m_vector],\r\n";
            text += "           style: new ol.style.Style({\r\n";
            text += "               stroke: new ol.style.Stroke({\r\n";
            text += "                   color: 'rgba(0, 153, 255, 1)',\r\n";
            text += "                   width: 3\r\n";
            text += "               }),\r\n";
            text += "               fill: new ol.style.Fill({\r\n";
            text += "                   color: 'rgba(255, 255, 255, 0.3)',\r\n";
            text += "               }),\r\n";
            text += "           }),\r\n";
            text += "           features: selectedFeatures\r\n";
            text += "       });\r\n";
            text += "       selectedFeatures = select.getFeatures();\r\n";
            text += "       m_map.addInteraction(select);\r\n";
            text += "\r\n";
            text += "       dragbox = new ol.interaction.DragBox({\r\n";
            text += "           condition: ol.events.condition.platformModifierKeyOnly\r\n";
            text += "       });\r\n";
            text += "       dragbox.on('boxend', function() {\r\n";
            text += "           var extent = dragbox.getGeometry().getExtent();\r\n";
            text += "           m_source.forEachFeatureIntersectingExtent(extent, function(feature) {\r\n";
            text += "               var exists = false;\r\n";
            text += "               selectedFeatures.forEach(function(e) {\r\n";
            text += "                   if (feature.getId() == e.getId()) {\r\n";
            text += "                       exists = true;\r\n";
            text += "                   }\r\n";
            text += "               });\r\n";
            text += "               if (!exists) {\r\n";
            text += "                   selectedFeatures.push(feature);\r\n";
            text += "               }\r\n";
            text += "           });\r\n";
            text += "       });\r\n";
            text += "       m_map.addInteraction(dragbox);\r\n";
            text += "\r\n";
            text += "       modify = new ol.interaction.Modify({\r\n";
            text += "           features: selectedFeatures\r\n";
            text += "       });\r\n";
            text += "\r\n";
            text += "       var modifyingFeatures = [];\r\n";
            text += "       var rectangleInteraction;\r\n";
            text += "       modify.on('modifystart', function(a) {\r\n";
            // 変形前の座標を保持
            text += "           modifyingFeatures = a.features;\r\n";
            text += "           var extent;\r\n";
            text += "           modifyingFeatures.forEach(function(b) {\r\n";
            text += "               if (b.get('shape') != 'Rectangle') {\r\n";
            // 長方形でない場合、スキップ
            text += "                   return;\r\n";
            text += "               }\r\n";
            text += "               b.set('coordinates', b.getGeometry().getCoordinates());\r\n";
            text += "               extent = ol.extent.boundingExtent(b.getGeometry().getCoordinates()[0]);\r\n";
            text += "           });\r\n";
            // ドラッグ中のイベントを取得
            text += "           document.addEventListener('pointermove', modifying);\r\n";
            text += "\r\n";
            text += "           rectangleInteraction = new ol.interaction.Extent({\r\n";
            text += "              boxStyle: new ol.style.Style({\r\n";
            text += "                  stroke: new ol.style.Stroke({\r\n";
            text += "                      color: 'rgba(0, 153, 255, 1)',\r\n";
            text += "                      width: 3\r\n";
            text += "                  }),\r\n";
            text += "                  fill: new ol.style.Fill({\r\n";
            text += "                      color: 'rgba(255, 255, 255, 0.3)',\r\n";
            text += "                  }),\r\n";
            text += "              })\r\n";
            text += "           });\r\n";
            text += "           rectangleInteraction.setActive(false);\r\n";
            text += "           if (extent) {\r\n";
            text += "               rectangleInteraction.setExtent(extent);\r\n";
            text += "           }\r\n";
            text += "           m_map.addInteraction(rectangleInteraction);\r\n";
            text += "       });\r\n";
            text += "\r\n";
            text += "       modify.on('modifyend', function(a) {\r\n";
            // 変形終了時にドラッグイベントリスナーを削除
            text += "           document.removeEventListener('pointermove', modifying);\r\n";
            text += "           modifyingFeatures.forEach(function(b) {\r\n";
            text += "               feature_edit(b);\r\n";
            text += "               if (!b.get('coordinates')) {\r\n";
            text += "                   return;\r\n";
            text += "               }\r\n";
            // rectangleInteractionと同じ長方形を変形された図形に適用する
            text += "               var extent = rectangleInteraction.getExtent();\r\n";
            text += "               var polygon = new ol.Feature(ol.geom.Polygon.fromExtent(extent));\r\n";
            text += "               b.getGeometry().setCoordinates(polygon.getGeometry().getCoordinates());\r\n";
            text += "               b.unset('coordinates');\r\n";
            text += "           });\r\n";
            text += "           modifyingFeatures = [];\r\n";
            text += "           m_map.removeInteraction(rectangleInteraction);\r\n";
            text += "           rectangleInteraction = null;\r\n";
            text += "       });\r\n";
            text += "\r\n";
            text += "       var modifying = function(c) {\r\n";
            text += "           modifyingFeatures.forEach(function (d) {\r\n";
            text += "               var oldCoordinates = d.get('coordinates');\r\n";
            text += "               if (!oldCoordinates) {\r\n";
            text += "                   return;\r\n";
            text += "               }\r\n";
            text += "               var newCoordinates = d.getGeometry().getCoordinates();\r\n";
            // 新たな長方形を導出する
            text += "               var newExtent = getRectangleExtent(oldCoordinates, newCoordinates);\r\n";
            text += "               rectangleInteraction.setExtent(newExtent);\r\n";
            text += "           });\r\n";
            text += "       };\r\n";
            text += "       m_map.addInteraction(modify);\r\n";
            text += "\r\n";
            // 図形移動のためのinteraction
            text += "       translate = new ol.interaction.Translate({\r\n";
            text += "           features: selectedFeatures\r\n";
            text += "       });\r\n";
            // 図形移動後の処理
            text += "       translate.on('translateend', function(a){\r\n";
            text += "           a.features.forEach(function(b){\r\n";
            text += "               feature_edit(b);\r\n";
            text += "           });\r\n";
            text += "       });\r\n";
            text += "       m_map.addInteraction(translate);\r\n";
            text += "   }\r\n";
            text += "}\r\n";

            // 画面キャプチャ前にコントロールを非表示にする
            text += "function MapCaptureBefore() {\r\n";
            text += "    m_map.removeControl(Zoom);\r\n";
            text += "    m_map.removeControl(RectBtn);\r\n";
            text += "    m_map.removeControl(PolyBtn);\r\n";
            text += "    m_map.removeControl(SelBtn);\r\n";
            text += "    m_map.removeControl(DelBtn);\r\n";
            text += "}\r\n";
            text += "\r\n";

            text += "function MapAddControl() {\r\n";
            text += "    m_map.addControl(Zoom);\r\n";
            text += "    m_map.addControl(RectBtn);\r\n";
            text += "    m_map.addControl(PolyBtn);\r\n";
            text += "    m_map.addControl(SelBtn);\r\n";
            text += "    m_map.addControl(DelBtn);\r\n";
            text += "}\r\n";
            text += "\r\n";

            // フィーチャ範囲に合わせて表示する
            text += "function FitArea() {\r\n";
            text += "   m_map.getView().fit(m_source.getExtent());\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 全体位置に合わせて表示する
            text += "function FitEntireArea(top, bottom, left, right) {\r\n";
            text += "   var extent = ol.proj.transformExtent([parseFloat(left), parseFloat(bottom), parseFloat(right), parseFloat(top)], 'EPSG:4326', 'EPSG:3857');\r\n";
            text += "   m_map.getView().fit(extent, m_map.getSize());\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 画面描画時に地図表示するパターン
            text += "function MapDisp(top, bottom, left, right) {\r\n";
            // zoomレベルの設定（最大最小緯度経度により設定）
            text += "   var extent = ol.proj.transformExtent([parseFloat(left), parseFloat(bottom), parseFloat(right), parseFloat(top)], 'EPSG:4326', 'EPSG:3857');\r\n";
            text += "   m_map.getView().fit(extent, m_map.getSize());\r\n";
            // 指定の緯度経度から適切なズームレベルをセット（最大ズームレベルとする）
            text += "   var maxzoom = m_map.getView().getZoom();\r\n";
            text += "   m_view.setMinZoom(maxzoom);\r\n";
            // 引数で受け取った座標をviewにセットする
            text += "   m_map.setView(new ol.View({\r\n";
            text += "       extent:ol.proj.transformExtent([parseFloat(left), parseFloat(bottom), parseFloat(right), parseFloat(top)], 'EPSG:4326', 'EPSG:3857'),\r\n";
            text += "       zoom: maxzoom,\r\n";
            text += "       maxZoom: 18,\r\n";
            text += "   }));\r\n";
            text += "   m_map.removeLayer(m_layer);\r\n";
            text += "   m_map.removeLayer(m_vector);\r\n";
            text += "   m_map.addControl(Zoom);\r\n";
            text += "   m_layer = new ol.layer.Tile({\r\n";
            text += "       source: new ol.source.XYZ({\r\n";
            text += "           attributions: [\r\n";
            text += "               new ol.Attribution({collapsible: true, collapsed: true,\r\n";
            text += "                   html: \" <a href='https://maps.gsi.go.jp/development/ichiran.html' target='_blank'>出典：国土地理院タイル</a> \"\r\n";
            text += "               })\r\n";
            text += "           ],\r\n";
            text += $"          url: '{MapUrl}',\r\n";
            text += "           projection: \"EPSG:3857\"\r\n";
            text += "       })\r\n";
            text += "   });\r\n";
            text += "   m_map.addLayer(m_layer);\r\n";
            text += "   m_map.addLayer(m_vector);\r\n";
            // 対象地域の中心座標を取得
            text += "   var center_coordinate = getcenter_coordinate(parseFloat(top), parseFloat(bottom), parseFloat(left), parseFloat(right));\r\n";
            text += "\r\n";
            text += "   MoveCenter(center_coordinate[0], center_coordinate[1], Number(maxzoom));\r\n";         // 選択範囲の中心座標を設定
            text += "   m_map.getView().setMinZoom(maxzoom)\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 中央に移動
            text += "function MoveCenter(coordinateLat, coordinateLon, zoomLevel) {\r\n";
            text += "   m_map.getView().setCenter(ol.proj.transform([coordinateLon, coordinateLat], \"EPSG:4326\", \"EPSG:3857\"));\r\n";
            text += "   m_map.getView().setZoom(zoomLevel);\r\n";
            text += "}\r\n";
            text += "\r\n";

            /* フィーチャの描画を解除します。 */
            text += "function removeDrawInteraction() {\r\n";
            text += "   m_vector.getSource().removeFeature(m_vector.getSource().getFeatures()[0]);";
            text += "}\r\n";
            text += "\r\n";

            text += "function MoveEnd(evt) {\r\n";
            text += "    var coordinate = ol.proj.transform(m_map.getView().getCenter(), \"EPSG:3857\", \"EPSG:4326\");\r\n";
            text += "    coordinate[0] = Math.round(coordinate[0] * 1000000) / 1000000;\r\n";
            text += "    coordinate[1] = Math.round(coordinate[1] * 1000000) / 1000000;\r\n";
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

            // 範囲を追加
            text += "function feature_add(feature) {\r\n";
            text += "   var coordinates = feature.getGeometry().transform('EPSG:3857', 'EPSG:4326').getCoordinates();\r\n";
            text += "   window.external.AddExtent(feature.getId(), coordinates);\r\n";
            text += "   feature.getGeometry().transform('EPSG:4326', 'EPSG:3857');\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 範囲を編集
            text += "function feature_edit(feature) {\r\n";
            text += "   var coordinates = feature.getGeometry().transform('EPSG:3857', 'EPSG:4326').getCoordinates();\r\n";
            text += "   window.external.EditExtent(feature.getId(), coordinates);\r\n";
            text += "   feature.getGeometry().transform('EPSG:4326', 'EPSG:3857');\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 範囲を削除
            text += "function feature_delete(feature) {\r\n";
            text += "   window.external.DeleteExtent(feature.getId());\r\n";
            text += "}\r\n";
            text += "\r\n";

            // フィーチャIDを初期値に設定
            text += "function InitId() {\r\n";
            text += "   featureId = 0;\r\n";
            text += "}\r\n";
            text += "\r\n";

            // フィーチャIDを更新
            text += "function UpdateId(id) {\r\n";
            text += "   featureId = id;\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 指定座標範囲を描画する
            text += "function feature_draw(strcoord, id) {\r\n";
            text += "   var strcoords = strcoord.split(',');\r\n";
            text += "   var dblcoords = [];\r\n";
            text += "   var coordinates = [];\r\n";
            text += "   for (i = 0; i < (strcoords.length - 1); (i+=2)) {\r\n";
            text += "       dblcoords.push([parseFloat(strcoords[i]), parseFloat(strcoords[i+1])]);\r\n";
            text += "   };\r\n";
            text += "   coordinates.push(dblcoords);\r\n";
            text += "   var polygon = new ol.geom.Polygon([]);\r\n";
            text += "   polygon.setCoordinates(coordinates)\r\n";
            text += "   var feature = new ol.Feature(polygon.transform('EPSG:4326', 'EPSG:3857'));\r\n";
            text += "   feature.setId(id);\r\n";
            text += "   setIdText(feature);\r\n";
            text += "   m_source.addFeature(feature);\r\n";         // ソースにフィーチャを追加する  
            text += "   m_vector.setSource(m_source);\r\n";         // レイヤにソースを追加する
            text += "}\r\n";
            text += "\r\n";

            // ソースを全部削除する
            text += "function feature_deleteAll() {\r\n";
            text += "   m_source = null;\r\n";
            text += "   m_source = new ol.source.Vector({ wrapX: false });\r\n";
            text += "   m_vector.setSource(m_source);\r\n";         // レイヤにソースを追加する
            text += "}\r\n";
            text += "\r\n";

            // ユーザ指定SHPを描画する
            text += "function feature_add_usershp(strcoord, num) {\r\n";
            text += "   var strcoords = strcoord.split(',');\r\n";
            text += "   var dblcoords = [];\r\n";
            text += "   var coordinates = [];\r\n";
            text += "   for (i = 0; i < (strcoords.length - 1); (i+=2)) {\r\n";
            text += "       dblcoords.push([parseFloat(strcoords[i]), parseFloat(strcoords[i+1])]);\r\n";
            text += "   };\r\n";
            text += "   coordinates.push(dblcoords);\r\n";
            text += "   var polygon = new ol.geom.Polygon([]);\r\n";
            text += "   polygon.setCoordinates(coordinates)\r\n";
            text += "   var feature = new ol.Feature(polygon.transform('EPSG:4326', 'EPSG:3857'));\r\n";
            text += "   var source;\r\n";
            text += "   var index = parseInt(num);\r\n";
            text += "   if (!m_shpLayer[index]) {\r\n";
            text += "       source = new ol.source.Vector({ wrapX: false });\r\n";
            text += "       m_shpLayer[index] = new ol.layer.Vector({\r\n";
            text += "           source: source,\r\n";
            text += "           style: readOnlyStyleFunction\r\n";
            text += "       });\r\n";
            text += "       m_map.addLayer(m_shpLayer[index]);\r\n";
            text += "   } else {\r\n";
            text += "       source = m_shpLayer[index].getSource();\r\n";
            text += "   };\r\n";
            text += "   source.addFeature(feature)\r\n";            // ソースにフィーチャを追加する  
            text += "   m_shpLayer[index].setSource(source);\r\n";  // レイヤにソースを追加する
            text += "}\r\n";
            text += "\r\n";

            // ユーザ指定SHPを非表示にする
            text += "function feature_delete_usershp(num) {\r\n";
            text += "   var index = parseInt(num);\r\n";
            text += "   if (m_shpLayer[index]) {\r\n";
            text += "       var source = m_shpLayer[index].getSource();\r\n";
            text += "       var features = source.getFeatures();\r\n";
            text += "       features.forEach(function (feature) {\r\n";
            text += "           source.removeFeature(feature);\r\n";
            text += "       });\r\n";
            text += "       m_map.removeLayer(m_shpLayer[index]);\r\n";
            text += "       m_shpLayer[index] = null;\r\n";
            text += "   };\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 指定したIDの図形を選択する
            text += "function feature_select(id) {\r\n";
            text += "   if (!select) {\r\n";
            text += "       changeMode();\r\n";
            text += "   };\r\n";
            text += "   selectedFeatures.clear();\r\n";
            text += "   var features = m_source.getFeatures();\r\n";
            text += "   features.forEach(function (feature) {\r\n";
            text += "       if (feature.getId() == id) {\r\n";
            text += "           selectedFeatures.push(feature);\r\n";
            text += "       }\r\n";
            text += "   });\r\n";
            text += "}\r\n";
            text += "\r\n";

            // 指定の座標を中心座標を取得する関数
            text += " function getcenter_coordinate(top, bottom, left, right, geometry) {\r\n";
            text += "   var dbl_top = parseFloat(top)\r\n";
            text += "   var dbl_bottom = parseFloat(bottom)\r\n";
            text += "   var dbl_left = parseFloat(left)\r\n";
            text += "   var dbl_right = parseFloat(right)\r\n";
            text += "   var x = (dbl_top - dbl_bottom) / 2  + dbl_bottom; \r\n";       // 中心座標：x
            text += "   var y = (dbl_right - dbl_left) / 2 + dbl_left; \r\n";          // 中心座標：y
            text += "   var x_coordinate  = Math.round(x * 1000000) / 1000000;\r\n";
            text += "   var y_coordinate  = Math.round(y * 1000000) / 1000000;\r\n";
            // 算出した中心座標を返却する
            text += "   return [x_coordinate, y_coordinate];\r\n";
            text += "};\r\n";
            text += "\r\n";

            // 指定の座標をセットする関数
            text += " function set_coordinate(top, bottom, left, right, geometry) {\r\n";
            text += "   var dbl_top = parseFloat(top)\r\n";
            text += "   var dbl_bottom = parseFloat(bottom)\r\n";
            text += "   var dbl_left = parseFloat(left)\r\n";
            text += "   var dbl_right = parseFloat(right)\r\n";
            text += "   if (!geometry) {\r\n";
            text += "       geometry = new ol.geom.Polygon(null);\r\n";
            text += "   }\r\n";
            text += "   var start = ol.proj.transform([dbl_left, dbl_top], \"EPSG:4326\", \"EPSG:3857\");\r\n";
            text += "   var end = ol.proj.transform([dbl_right, dbl_bottom], \"EPSG:4326\", \"EPSG:3857\");\r\n";
            text += "   geometry.setCoordinates([\r\n";
            text += "       [start, [start[0], end[1]], end, [end[0], start[1]], start]\r\n";
            text += "   ]);\r\n";
            text += "   return geometry;\r\n";
            text += "};\r\n";

            text += "</script>\r\n";
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
