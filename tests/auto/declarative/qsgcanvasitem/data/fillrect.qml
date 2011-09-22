import QtQuick 2.0

Canvas {
   id:canvas
   width:20
   height:20
   property bool ok:false
   property string dataURL:"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAAN1wAADWEB8LeSRgAAACJJREFUOI1j/M/A8J+BioCJmoaNGjhq4KiBowaOGjiUDAQAHAQCJm0Xf5UAAAAASUVORK5CYII=" 
   renderInThread:false
   renderTarget:Canvas.Image
   onPaint: {
     context.reset();
     context.fillStyle = "red";
     context.fillRect(0, 0, canvas.width, canvas.height);
     var data = canvas.toDataURL();
     if (data == canvas.dataURL) {
       canvas.ok = true;
     } else {
       console.log("[" + canvas.dataURL + "]");
       console.log("[" + canvas.toDataURL() + "]");
     }
   }
}
