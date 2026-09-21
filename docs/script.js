// ============================================================
// Настройки MQTT
// ============================================================
const PREFIX = 'esp32_recuperator';
const BROKER = 'wss://broker.emqx.io:8084/mqtt';
const USER   = 'emqx';
const PASS   = 'public';

// ============================================================
// Подключение к брокеру через WebSocket
// ============================================================
const client = mqtt.connect(BROKER, {
  clientId: 'web_' + Math.random().toString(16).substr(2, 8),
  username: USER,
  password: PASS,
  clean: true,
  reconnectPeriod: 3000
});

const statusEl = document.getElementById('mqtt-status');

client.on('connect', () => {
  console.log('MQTT connected');
  statusEl.textContent = '✅ Подключено к MQTT';
  statusEl.className = 'status ok';

  client.subscribe(`${PREFIX}/telemetry`);
  client.subscribe(`${PREFIX}/relay/0/state`);
  client.subscribe(`${PREFIX}/relay/1/state`);
  client.subscribe(`${PREFIX}/config/state`);
  client.subscribe(`${PREFIX}/status`);
});

client.on('reconnect', () => {
  statusEl.textContent = 'Переподключение...';
  statusEl.className = 'status';
});

client.on('error', (err) => {
  console.error('MQTT error:', err);
  statusEl.textContent = '❌ Ошибка MQTT';
  statusEl.className = 'status err';
});

client.on('close', () => {
  statusEl.textContent = '❌ Соединение потеряно';
  statusEl.className = 'status err';
});

// ============================================================
// Обработка входящих сообщений
// ============================================================
const NAMES = ['Вытяжка дом', 'Вытяжка улица', 'Приточка улица', 'Приточка дом'];
const MCLS  = { 0: 'b-auto', 3: 'b-win', 1: 'b-man', 2: 'b-man' };
const MNAME = { 0: 'АВТО', 3: 'ОКНО', 1: 'РУЧН ВКЛ', 2: 'РУЧН ВЫКЛ' };
const MCMD  = { 0: 'AUTO', 3: 'WINDOW', 1: 'ON', 2: 'OFF' };

let telemetry = null;
let relayState = [null, null];
let config = null;
let espOnline = false;

client.on('message', (topic, message) => {
  let data;
  try { data = JSON.parse(message.toString()); }
  catch (e) { return; }

  if (topic.endsWith('/telemetry')) {
    telemetry = data;
    renderTelemetry();
    renderRelays();
  }
  else if (topic.endsWith('/relay/0/state')) { relayState[0] = data; renderRelay(0); }
  else if (topic.endsWith('/relay/1/state')) { relayState[1] = data; renderRelay(1); }
  else if (topic.endsWith('/config/state'))  { config = data; renderConfig(); }
  else if (topic.endsWith('/status')) {
    espOnline = (message.toString() === 'online');
    if (!espOnline) {
      statusEl.textContent = '⚠ ESP32 offline';
      statusEl.className = 'status err';
    } else if (client.connected) {
      statusEl.textContent = '✅ ESP32 online';
      statusEl.className = 'status ok';
    }
  }
});

// ============================================================
// Отрисовка телеметрии
// ============================================================
function renderTelemetry() {
  if (!telemetry) return;

  // DS18B20
  for (let i = 0; i < 4; i++) {
    const el = document.getElementById('s' + i);
    const v = telemetry.ds[i];
    if (v === null || v === undefined) {
      el.textContent = 'ошибка';
      el.className = 'tv err';
    } else {
      el.textContent = v + '°';
      el.className = 'tv';
    }
  }

  // DHT11 (indoor) — индекс 1
  const inT = telemetry.dhtT[1], inH = telemetry.dhtH[1];
  const elInT = document.getElementById('dInT');
  const elInH = document.getElementById('dInH');
  if (inT === null) { elInT.textContent = 'ошибка'; elInT.className = 'tv err'; }
  else { elInT.textContent = inT + '°'; elInT.className = 'tv'; }
  elInH.textContent = (inH === null ? '--' : inH);
  elInH.parentElement.className = (inH === null) ? 'dht-h err' : 'dht-h';

  // DHT22 (outdoor) — индекс 0
  const outT = telemetry.dhtT[0], outH = telemetry.dhtH[0];
  const elOutT = document.getElementById('dOutT');
  const elOutH = document.getElementById('dOutH');
  if (outT === null) { elOutT.textContent = 'ошибка'; elOutT.className = 'tv err'; }
  else { elOutT.textContent = outT + '°'; elOutT.className = 'tv'; }
  elOutH.textContent = (outH === null ? '--' : outH);
  elOutH.parentElement.className = (outH === null) ? 'dht-h err' : 'dht-h';

  // Alert
  document.getElementById('alert').innerHTML = telemetry.blocked
    ? `<div class="alert">❄ Защита: улица ${telemetry.dhtT[0]}°C &lt; ${telemetry.outdoorMinT}°C — все реле ВЫКЛ</div>`
    : '';

  // Outdoor box
  document.getElementById('outT').textContent = telemetry.outdoorMinT + ' °C';
  document.getElementById('outE').textContent =
    telemetry.outdoorEnabled ? 'включена' : 'отключена';
}

// ============================================================
// Отрисовка реле
// ============================================================
function renderRelays() {
  renderRelay(0);
  renderRelay(1);
}

function renderRelay(i) {
  // Используем либо отдельное retained-сообщение, либо данные из telemetry
  const R = relayState[i] || (telemetry ? telemetry.relay[i] : null);
  if (!R) return;

  const blocked = telemetry ? telemetry.blocked : false;
  const box  = document.getElementById('r' + i);
  const st   = document.getElementById('r' + i + 'st');
  const md   = document.getElementById('r' + i + 'md');
  const info = document.getElementById('r' + i + 'inf');

  box.className = 'relay ' + (blocked ? 'blk' : (R.state ? 'on' : 'off'));
  st.textContent = R.state ? 'ВКЛ' : 'ВЫКЛ';
  st.className = 'rstate ' + (R.state ? 'on' : 'off');

  md.textContent = MNAME[R.mode];
  md.className = 'rbadge ' + (blocked ? 'b-blk' : MCLS[R.mode]);
  if (blocked) md.textContent += ' ❄';

  let inf = `<b>${NAMES[R.sensor]}</b><br>`;
  if (R.mode === 3) {
    inf += `Диапазон окна: <b>${R.tOn}…${R.tOff}°C</b>`;
  } else if (R.mode === 0) {
    if (R.tOn > R.tOff)
      inf += `Охлаждение: ВКЛ ≥ <b>${R.tOn}°C</b>, ВЫКЛ ≤ <b>${R.tOff}°C</b>`;
    else
      inf += `Нагрев: ВКЛ ≤ <b>${R.tOn}°C</b>, ВЫКЛ ≥ <b>${R.tOff}°C</b>`;
  } else if (R.mode === 1) {
    inf += 'Принудительно ВКЛ';
  } else {
    inf += 'Принудительно ВЫКЛ';
  }
  info.innerHTML = inf;

  // Подсветка активной кнопки
  document.querySelectorAll('#r' + i + 'bt a').forEach(a => {
    a.className = (parseInt(a.dataset.m) === R.mode) ? 'act' : '';
  });
}

// ============================================================
// Отрисовка конфигурации (outdoor из retained)
// ============================================================
function renderConfig() {
  if (!config) return;
  document.getElementById('outT').textContent = config.outdoorMinT + ' °C';
  document.getElementById('outE').textContent =
    config.outdoorEnabled ? 'включена' : 'отключена';
}

// ============================================================
// Отправка команд на ESP32
// ============================================================
function sendRelay(idx, cmd) {
  client.publish(`${PREFIX}/relay/${idx}/cmd`, cmd);
  console.log(`Sent: relay/${idx}/cmd = ${cmd}`);
}

// Экспорт в глобальную область (для onclick в HTML)
window.sendRelay = sendRelay;
