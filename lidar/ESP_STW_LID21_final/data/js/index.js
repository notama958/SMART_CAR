/**
 * global variables
 */
var Lidar = 0; // holding lidar distance
var Degree = 0; // holding rotation degree
let car = document.getElementById('car');
var valx = document.getElementById('rd_slider_1').value;

/**
 * UI interaction variables
 */
var dire = 0; // direction
var dist = 0; // distance
var mode = 0; // mode 1 = direction value 2=distance vale

// scale 3:1
// container dimension
const speed = 3; // 3cm:10px
// change w and h on your simulation area
const w = 1.2; // m
const h = 0.8; // m
let container_title = document.getElementById('text-info');
container_title.innerHTML = `Dimension: ${speed}px:1cm - ${w * 1000}cmx${
  h * 1000
}cm`;
let container = document.getElementById('right-box');
container.style.width = (w * 1000) / speed + 'px';
container.style.height = (h * 1000) / speed + 'px';
const containerWidth = container.offsetWidth;
const containerHeight = container.offsetHeight;
let ox = document.getElementById('ox');
ox.style.width = containerWidth + 'px';
let oy = document.getElementById('oy');
oy.style.height = containerHeight + 'px';

// props car object
const carWidth = car.offsetWidth;

///////////////////////////////////// JS start /////////////////////
// get lidar va direction value in interval time
// should match with arduino file
setInterval(function () {
  let xhttp2 = getLidar();
}, 1000);
setInterval(function () {
  let xhttp1 = getDegree();
}, 1500);

// Getting rotation degree
function getDegree() {
  let xhttp = new XMLHttpRequest();
  xhttp.open('GET', '/Deg_dist', true);
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      console.log('degree> ' + this.responseText);
      Degree = parseInt(this.responseText);
    }
  };
  xhttp.send();
}
// Getting lidar distance
function getLidar() {
  let xhttp = new XMLHttpRequest();
  xhttp.open('GET', '/Lid_dist', true);
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      console.log('lidar> ' + this.responseText);
      Lidar = parseInt(this.responseText);
      document.getElementById('Box_lidar_dist').innerHTML = Lidar;
      movingCar(Lidar, Degree);
    }
  };
  xhttp.send();
}

// slider
$(document).ready(function () {
  $('#rd_slider_1').roundSlider({
    radius: 85,
    min: -180,
    max: 180,
    circleShape: 'pie',
    sliderType: 'min-range',
    value: 0,
    startAngle: 315,
    change: function (args) {
      dire = args.value; // direction variable set

      send_srw_val(dire); // call type_sl_val
    },
  });
});

function send_srw_val(val) {
  // send stw value
  mode = 1; // Steering wheel = mode 1
  httpGetAsync(tryme);
}

function FRW5() {
  // Send drive command 5cm
  mode = 2; // drive motors = mode 2
  dist = 5; // drive distance
  httpGetAsync(tryme);
}

function RET10() {
  mode = 2;
  dist = -10;
  httpGetAsync(tryme);
}

function httpGetAsync(callback) {
  // HTTP GET to send stw or drive values to C++
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.onreadystatechange = function () {
    if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
      callback(xmlHttp.responseText);
  };
  if (mode == 1) {
    xmlHttp.open('GET', '/?dire=' + dire, true);
  } // true for asynchronous
  if (mode == 2) {
    xmlHttp.open('GET', '/?dist=' + dist, true);
  } // true for asynchronous
  xmlHttp.send(null);
}
function tryme(param1) {}

// moving car and rotate
function movingCar(lidar_value, direction) {
  // get transform of car object
  let style = window.getComputedStyle(car);
  let matrix = style.transform || style.MozTransform;
  // turn matrix string to array
  let matrixValues = matrix.match(/matrix.*\((.+)\)/)[1].split(', ');
  let topEdge = matrixValues[5];
  let leftEdge = matrixValues[4];
  let degree = direction; // 0-360
  let desiredX, ox, desiredY, oy;
  if (degree != null) {
    car.style.transform = `matrix(1,0,0,1,${topEdge},${leftEdge})`;
    car.style.transform += `rotate(${degree}deg) `;

    // console.log('L ' + leftEdge + ',T ' + topEdge + ',D ' + degree);
  }
  if (lidar_value != null) {
    if (degree >= 0 && degree <= 90) {
      // first quarter of circle in CW PASSED
      if (degree <= 45) {
        ox = Math.floor(
          Math.tan((degree * Math.PI) / 180) * (containerHeight / 2) -
            (Math.sin((degree * Math.PI) / 180) * lidar_value) / speed
        );
        desiredX = containerWidth / 2 + ox;
        desiredY = Math.floor(
          (Math.cos((degree * Math.PI) / 180) * lidar_value) / speed
        );
      } else {
        degree = 90 - degree;
        ox = Math.floor(
          (Math.cos((degree * Math.PI) / 180) * lidar_value) / speed
        );
        console.log(ox);
        desiredX = containerWidth - ox - carWidth;
        desiredY = Math.floor(
          containerHeight / 2 -
            Math.tan((degree * Math.PI) / 180) * (containerWidth / 2 - ox)
        );
      }
    } else if (degree > 90 && degree < 180) {
      // second quarter of circle CW PASSED
      degree = degree % 90;
      if (degree <= 45) {
        desiredX =
          containerWidth -
          Math.floor(
            (Math.cos((degree * Math.PI) / 180) * lidar_value) / speed
          ) -
          carWidth;
        desiredY =
          Math.floor(
            Math.tan((degree * Math.PI) / 180) * (containerWidth / 2) -
              (Math.sin((degree * Math.PI) / 180) * lidar_value) / speed
          ) +
          containerHeight / 2 -
          carWidth;
      } else {
        degree = 90 - degree;
        desiredX =
          containerWidth / 2 +
          Math.floor(
            Math.tan((degree * Math.PI) / 180) * (containerWidth / 2) -
              (Math.sin((degree * Math.PI) / 180) * lidar_value) / speed
          );
        desiredY =
          containerHeight -
          Math.floor(
            (Math.cos((degree * Math.PI) / 180) * lidar_value) / speed
          ) -
          carWidth;
      }
    } else if (degree >= 180 && degree <= 270) {
      // third quater of circle CW PASSED
      degree = degree % 180;
      if (degree <= 45) {
        desiredX =
          containerWidth / 2 -
          Math.floor(
            Math.tan((degree * Math.PI) / 180) * (containerWidth / 2) -
              (Math.sin((degree * Math.PI) / 180) * lidar_value) / speed
          );
        desiredY =
          containerHeight -
          Math.floor(
            (Math.cos((degree * Math.PI) / 180) * lidar_value) / speed
          ) -
          carWidth;
      } else {
        degree = 90 - degree;
        desiredY =
          containerHeight / 2 +
          Math.floor(
            Math.tan((degree * Math.PI) / 180) * (containerWidth / 2) -
              Math.sin((degree * Math.PI) / 180) * lidar_value * speed
          );
        desiredX = Math.floor(
          (Math.cos((degree * Math.PI) / 180) * lidar_value) / speed
        );
      }
    }
    //OKAY
    else {
      // fourth quarter of circle CW PASSED
      degree = degree % 270;
      if (degree <= 45) {
        desiredX = Math.floor(
          (Math.cos((degree * Math.PI) / 180) * lidar_value) / speed
        );
        desiredY =
          containerHeight / 2 -
          Math.floor(
            Math.tan((degree * Math.PI) / 180) * (containerWidth / 2) -
              (Math.sin((degree * Math.PI) / 180) * lidar_value) / speed
          );
      } else {
        degree = 90 - degree;
        desiredY = Math.floor(
          (Math.cos((degree * Math.PI) / 180) * lidar_value) / speed
        );
        desiredX =
          containerWidth / 2 -
          Math.floor(
            Math.tan((degree * Math.PI) / 180) * (containerHeight / 2) -
              (Math.sin((degree * Math.PI) / 180) * lidar_value) / speed
          );
      }
    }
    if (desiredX > containerWidth) desiredX = containerWidth;
    else if (desiredX < 0) desiredX = 0;
    if (desiredY < 0) desiredY = 0;
    else if (desiredY > containerHeight) desiredY = containerHeight;
    console.log(desiredX, desiredY);
    car.style.transform = `translate(${Math.abs(desiredX)}px,${Math.abs(
      desiredY
    )}px)`;

    car.style.transform += `rotate(${direction}deg) `;
    console.log(car.style.transform);
  }
}
// movingCar(63, 179);
// movingCar(10, 270
//   );
// car.style.transform+=
// movingCar(10, 80);
// car.style.transform += `rotate(${degree}deg) `;
/**
 * Drive input
 */
const goButton = document.getElementById('addDistance');
goButton.addEventListener('click', function () {
  const distanceAmount = document.getElementById('distanceAmount').value;
  dist = parseFloat(distanceAmount);
  httpGetAsync(tryme);
});
/**
 * Update for status bar
 */
i = 0;

function tstm() {
  i -= 10;
  RET10();
  // console.log(dist);
  update();
}

function tst() {
  i += 5;
  FRW5();
  // console.log(dist);
  update();
}

function update() {
  var element = document.getElementById('myprogressBar');
  element.style.width = i > 100 ? 105 : i + '%';
  element.innerText = i + 'cm';
}
///////////////////////////////////////////// JS END
