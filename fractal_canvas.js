function renderCanvas() {
    const canvas = document.getElementById('canvas');
    // Canvas resizing from http://stackoverflow.com/a/43364730/2142626
    const width = canvas.clientWidth;
    const height = canvas.clientHeight;
    console.log("w="+width+" h="+height);
    if (canvas.width !== width || canvas.height !== height) {
      canvas.width = width;
      canvas.height = height;
    }
    const context = canvas.getContext('2d');
    const tile_w = 32;
    const tile_h = 32;
    const x = parseFloat(document.getElementById('x').value);
    const y = parseFloat(document.getElementById('y').value);
    const diameter = parseFloat(document.getElementById('diameter').value);
    const m = width/tile_w;
    const n = height/tile_h;
    //big picture
    const big_w = Math.ceil(width/tile_w);
    const big_h = Math.ceil(height/tile_h);
    const fractal = Module.fractal(big_w,big_h,32, x, y, diameter);
    const bigArray = new Uint8ClampedArray(fractal);
    // console.log("big_w="+big_w+" big_h="+big_h);
    for (i = 0;i < big_w;i++) {
      for (j=0;j<big_h;j++) {
        var c= bigArray[(i+j*big_w)*4];
        context.fillStyle = 'rgb('+c+','+c+','+c+')';
        //   console.log("bigArray"+context.fillStyle);
        context.fillRect(i*tile_w,j*tile_h,tile_w,tile_h);
      }
    }

    //details
    function drawTile(i,j, tile_x, tile_y, tile_d) {
      console.log("start "+i+" "+j);
      console.time('mandelbrot'+i+' '+j);
      const fractal = Module.fractal(tile_w,tile_h,32, tile_x, tile_y, tile_d);
      const imageData = new ImageData(new Uint8ClampedArray(fractal), tile_w, tile_h);
      context.putImageData(imageData, i*tile_w, j*tile_h);
      console.timeEnd('mandelbrot'+i+' '+j);
    }
    for (i = 0;i<m;i++) {
      for (j=0;j<n;j++) {
        (function(i0,j0) {
          // setTimeout(function() {

          const tile_x = x + diameter*i0/m;
          const tile_y = y + diameter*height/width*j0/n;
          const tile_d = diameter/m;
          setTimeout(function() {
            drawTile(i0,j0, tile_x, tile_y, tile_d)
            //});
          },0);
        })(i,j);
        //window.requestAnimationFrame(drawTile);
      }
    }
  
}
Module.addOnPostRun(() => {
 renderCanvas();
});
