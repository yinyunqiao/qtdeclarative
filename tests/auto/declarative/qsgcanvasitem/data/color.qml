import QtQuick 2.0

Canvas {
   id:canvas
   property string fillStyle:"back"
   property string strokeStyle:"white"
   onStrokeStyleChanged: {
      var ctx = canvas.getContext('2d');
      if (ctx) {
        ctx.strokeStyle = strokeStyle;
      }
   }
   onFillStyleChanged: {
      var ctx = canvas.getContext('2d');
      if (ctx) {
        ctx.fillStyle = fillStyle;
      }
   }
}