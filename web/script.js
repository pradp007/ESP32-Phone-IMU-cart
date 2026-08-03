let running = false;

function toggleCar() {
  if (!running) {
    fetch("/start");
    document.getElementById("startStop").innerText = "STOP";
    document.getElementById("startStop").style.background = "#d50000";
  } else {
    fetch("/stop");
    document.getElementById("startStop").innerText = "START";
    document.getElementById("startStop").style.background = "#00c853";
  }
  running = !running;
}

function brake() { 
  fetch("/brake"); 
}

function horn() { 
  fetch("/horn"); 
}

if (window.DeviceOrientationEvent) {
  window.addEventListener('deviceorientation', function(event) {
    let pitch = event.beta || 0;
    let roll = event.gamma || 0;

    document.getElementById("pitch").innerText = pitch.toFixed(1);
    document.getElementById("roll").innerText = roll.toFixed(1);

    fetch(`/imu?pitch=${pitch}&roll=${roll}`);
  });
}

setInterval(() => {
  fetch("/distance")
    .then(res => res.json())
    .then(data => {
      document.getElementById("distance").innerText = data.distance + " cm";

      let status = "SAFE";
      let color = "#00e676";

      if (data.distance < 5) {
        status = "STOP!";
        color = "#ff1744";
      } else if (data.distance < 20) {
        status = "WARNING";
        color = "#ff9100";
      }

      document.getElementById("distStatus").innerText = status;
      document.getElementById("distStatus").style.color = color;
    });
}, 200);
