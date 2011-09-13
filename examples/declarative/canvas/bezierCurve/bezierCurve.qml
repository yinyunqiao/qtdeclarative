import QtQuick 2.0
import "../contents"
Item {
  id:container
  width:360
  height:600

  Column {
    spacing:5
    anchors.fill:parent
    Text { font.pointSize:25; text:"Bezier Curve"; anchors.horizontalCenter:parent.horizontalCenter}

    Canvas {
      id:canvas
      width:360
      height:360
      property string strokeStyle:"red"
      property string fillStyle:"red"
      property int lineWidth:lineWidthCtrl.value
      property bool fill:true
      property bool stroke:true
      property real alpha:alphaCtrl.value
      property real scaleX : scaleXCtrl.value
      property real scaleY : scaleYCtrl.value
      property real rotate : rotateCtrl.value
      smooth:true
      renderTarget:Canvas.Image
      threadRendering:false


      Behavior on scaleX { SpringAnimation { spring: 2; damping: 0.2; loops:Animation.Infinite } }
      Behavior on scaleY { SpringAnimation { spring: 2; damping: 0.2; loops:Animation.Infinite} }
      Behavior on rotate { SpringAnimation { spring: 2; damping: 0.2; loops:Animation.Infinite} }

    onLineWidthChanged:requestPaint();
    onFillChanged:requestPaint();
    onStrokeChanged:requestPaint();
    onAlphaChanged:requestPaint();
    onScaleXChanged:requestPaint();
    onScaleYChanged:requestPaint();
    onRotateChanged:requestPaint();

    onPaint: {
      var ctx = canvas.getContext('2d');
      ctx.reset();
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      ctx.globalAlpha = canvas.alpha;
      ctx.strokeStyle = canvas.strokeStyle;
      ctx.fillStyle = canvas.fillStyle;
      ctx.lineWidth = canvas.lineWidth;
      ctx.scale(canvas.scaleX, canvas.scaleY);
      ctx.rotate(canvas.rotate);
      ctx.beginPath();
      ctx.moveTo(75,40);
      ctx.bezierCurveTo(75,37,70,25,50,25);
      ctx.bezierCurveTo(20,25,20,62.5,20,62.5);
      ctx.bezierCurveTo(20,80,40,102,75,120);
      ctx.bezierCurveTo(110,102,130,80,130,62.5);
      ctx.bezierCurveTo(130,62.5,130,25,100,25);
      ctx.bezierCurveTo(85,25,75,37,75,40);
      ctx.closePath();
      if (canvas.fill)
         ctx.fill();
      if (canvas.stroke)
         ctx.stroke();
    }
  }

    Rectangle {
        id:controls
        width:360
        height:160
        Column {
          spacing:3
          Slider {id:lineWidthCtrl; width:300; height:30; min:1; max:10; init:2; name:"Line width"}
          Slider {id:scaleXCtrl; width:300; height:30; min:0.1; max:10; init:1; name:"ScaleX"}
          Slider {id:scaleYCtrl; width:300; height:30; min:0.1; max:10; init:1; name:"ScaleY"}
          Slider {id:rotateCtrl; width:300; height:30; min:0; max:Math.PI*2; init:0; name:"Rotate"}
          Slider {id:alphaCtrl; width:300; height:30; min:0; max:1; init:1; name:"Alpha"}
        }
    }
  }
}