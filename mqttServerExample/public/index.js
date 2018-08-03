const socket = io();
const checkboxes = document.querySelectorAll("input[type=checkbox]");
const sliders = document.querySelectorAll("input[type=range]");
const colorSelectors = document.querySelectorAll("input[type=color]");

const inputs = document.querySelectorAll("input");
const controls = document.querySelectorAll(".controls");
const clientName = socket.id;

function toggleLight() {
  let data = {
    name : clientName,
    message : `${this.parentElement.parentElement.getAttribute('name')},${this.name},${this.checked}`
  };

  socket.emit('aioEvent', data);
}

function alterLight() {
  let data = {
    name : socket.id,
    message : `${this.parentElement.parentElement.getAttribute('name')},${this.name},${(this.type === "checkbox") ? this.checked : this.value}`
  };

  socket.emit('aioEvent', data);
};

function resizeDivToChildren(d) {
  let w = 0;
  let h = 0;
  for(child of d.children) {
    w += child.offsetWidth;
    h += child.offsetHeight;      
  }
  [d.style.width, d.style.height] = [(`${w}px`), (`${h}px`)];
};
    
controls.forEach((control) => {
  resizeDivToChildren(control);
  control.style.backgroundSize = (`${control.offsetWidth}px, ${control.offsetHeight}px`);
});

checkboxes.forEach(checkbox => checkbox.addEventListener('change', toggleLight));

sliders.forEach(slider => slider.addEventListener('change', alterLight));

colorSelectors.forEach(colorSelector => colorSelector.addEventListener('input', alterLight));