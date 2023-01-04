 
  if (!!window.EventSource) {
    let source = new EventSource('/events');
          
    source.addEventListener('error', function(e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
      }
    }, false);
       
    source.addEventListener('new_readings', function(e) {
      console.log("new_readings", e.data);
      let myObj = JSON.parse(e.data);
      let voltValue = document.querySelector('.voltage');
      let currentValue = document.querySelector('.current');
      let powerValue = document.querySelector('.power');
      Power = myObj.power;
      Volt = myObj.voltage;
      Current = myObj.current;
      Energy = myObj.energy;
      voltValue.innerHTML = Volt;
      currentValue.innerHTML = Current;
      powerValue.innerHTML = Power;
      setTimeout(function(){
        odometer.innerHTML = Energy;
        }, 1000);
    }, false);
  }


