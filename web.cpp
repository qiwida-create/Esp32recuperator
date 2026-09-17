#include "web.h"
#include "config.h"
#include "sensors.h"
#include "storage.h"
#include "relays.h"
#include "display.h"

#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

// ----- HTML главной страницы -----
const char PAGE_INDEX[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>MeteoStation</title>
<style>
 *{box-sizing:border-box}
 body{font-family:system-ui,-apple-system,sans-serif;background:#0b0d12;color:#e6e8ec;
      margin:0;padding:12px;font-size:14px}
 h1{font-size:16px;color:#4dd;letter-spacing:2px;text-align:center;margin:4px 0 14px}
 .grid{display:grid;grid-template-columns:1fr 1fr;gap:8px}
 .tile{background:#161a22;border:1px solid #232734;border-radius:12px;padding:12px 10px;text-align:center}
 .tile .tl{font-size:11px;color:#6a7386;letter-spacing:.5px;text-transform:uppercase}
 .tile .tv{font-size:24px;font-weight:700;color:#fd0;margin-top:6px;
           font-variant-numeric:tabular-nums}
 .tile .tv.err{color:#f55;font-size:16px}
 .relay{background:#161a22;border-radius:12px;padding:12px;margin:10px 0;
        border-left:4px solid #333}
 .relay.on{border-left-color:#3d5}
 .relay.off{border-left-color:#444}
 .relay.blk{border-left-color:#f33;background:#201318}
 .rhead{display:flex;justify-content:space-between;align-items:center;margin-bottom:8px}
 .rnum{color:#6a7386;font-size:12px;text-transform:uppercase;letter-spacing:1px}
 .rstate{font-size:16px;font-weight:700}
 .rstate.on{color:#3d5}.rstate.off{color:#888}
 .rbadge{font-size:11px;padding:2px 8px;border-radius:10px;margin-left:6px;font-weight:600}
 .b-auto{background:#0a4;color:#fff}
 .b-win{background:#069;color:#fff}
 .b-man{background:#a40;color:#fff}
 .b-blk{background:#a00;color:#fff}
 .rinfo{font-size:13px;color:#9aa6bb;line-height:1.6}
 .rinfo b{color:#cde}
 .btns{display:flex;gap:6px;margin-top:10px}
 .btns a{flex:1;text-align:center;padding:8px 0;border-radius:8px;
         text-decoration:none;font-size:12px;font-weight:600;
         background:#1e2330;color:#aab;border:1px solid #2a3040}
 .btns a.act{background:#4dd;color:#000;border-color:#4dd}
 .outbox{background:#101828;border:1px solid #1f3050;border-radius:12px;
         padding:12px;margin:10px 0;border-left:4px solid #36c}
 .outrow{display:flex;justify-content:space-between;padding:4px 0}
 .outrow .k{color:#7a8699}.outrow .v{color:#9df;font-weight:700}
 .alert{background:#3a0000;color:#faa;padding:10px;border-radius:8px;
        margin:8px 0;text-align:center;font-weight:600;font-size:13px;
        border-left:4px solid #f33}
 a.act{display:block;text-align:center;background:#4dd;color:#000;padding:12px;
       border-radius:10px;text-decoration:none;font-weight:700;margin-top:10px}
 .dht-grid{margin-top:8px}
 .tile.dht .tv{font-size:22px}
 .tile.dht .dht-h{margin-top:6px;font-size:13px;color:#8cf;
                  font-variant-numeric:tabular-nums}
 .tile.dht .dht-h.err{color:#f55}
 .tile.dht.out{border-color:#1f3050}
 .tile.dht.out .tl{color:#7da5e0}
</style></head><body>
<h1>МЕТЕОСТАНЦИЯ</h1>
<div id='alert'></div>

<div class='grid'>
 <div class='tile'><div class='tl'>Вытяжка · дом</div><div class='tv' id='s0'>--</div></div>
 <div class='tile'><div class='tl'>Вытяжка · улица</div><div class='tv' id='s1'>--</div></div>
 <div class='tile'><div class='tl'>Приточка · улица</div><div class='tv' id='s2'>--</div></div>
 <div class='tile'><div class='tl'>Приточка · дом</div><div class='tv' id='s3'>--</div></div>
</div>

<div class='grid dht-grid'>
 <div class='tile dht'>
   <div class='tl'>DHT11 · внутри дома</div>
   <div class='tv' id='dInT'>--</div>
   <div class='dht-h'>💧 <span id='dInH'>--</span> %</div>
 </div>
 <div class='tile dht out'>
   <div class='tl'>DHT22 · улица</div>
   <div class='tv' id='dOutT'>--</div>
   <div class='dht-h'>💧 <span id='dOutH'>--</span> %</div>
 </div>
</div>

<div class='relay' id='r0'>
 <div class='rhead'><span class='rnum'>Реле 1</span>
   <span><span class='rstate' id='r0st'>--</span><span class='rbadge' id='r0md'>--</span></span></div>
 <div class='rinfo' id='r0inf'>--</div>
 <div class='btns' id='r0bt'>
   <a data-m='0' href='/setmode?r=0&m=0'>Авто</a>
   <a data-m='3' href='/setmode?r=0&m=3'>Окно</a>
   <a data-m='1' href='/setmode?r=0&m=1'>ВКЛ</a>
   <a data-m='2' href='/setmode?r=0&m=2'>ВЫКЛ</a>
 </div>
</div>

<div class='relay' id='r1'>
 <div class='rhead'><span class='rnum'>Реле 2</span>
   <span><span class='rstate' id='r1st'>--</span><span class='rbadge' id='r1md'>--</span></span></div>
 <div class='rinfo' id='r1inf'>--</div>
 <div class='btns' id='r1bt'>
   <a data-m='0' href='/setmode?r=1&m=0'>Авто</a>
   <a data-m='3' href='/setmode?r=1&m=3'>Окно</a>
   <a data-m='1' href='/setmode?r=1&m=1'>ВКЛ</a>
   <a data-m='2' href='/setmode?r=1&m=2'>ВЫКЛ</a>
 </div>
</div>

<div class='outbox'>
 <div class='outrow'><span class='k'>Уставка по улице</span>
   <span class='v' id='outT'>--</span></div>
 <div class='outrow'><span class='k'>Защита от холода</span>
   <span class='v' id='outE'>--</span></div>
</div>

<a class='act' href='/settings'>⚙ Настройки</a>
<script>
const NAMES=['Вытяжка дом','Вытяжка улица','Приточка улица','Приточка дом'];
const MCLS={0:'b-auto',3:'b-win',1:'b-man',2:'b-man'};
const MNAME={0:'АВТО',3:'ОКНО',1:'РУЧН ВКЛ',2:'РУЧН ВЫКЛ'};
async function refresh(){
 try{
  const d = await (await fetch('/api/data')).json();
  for(let i=0;i<4;i++){
    const el=document.getElementById('s'+i);
    if(d.ds[i]===null){el.textContent='ошибка'; el.className='tv err';}
    else {el.textContent=d.ds[i]+'°'; el.className='tv';}
  }
  const inT = d.dhtT[1], inH = d.dhtH[1];
  const elInT = document.getElementById('dInT');
  const elInH = document.getElementById('dInH');
  if(inT===null){elInT.textContent='ошибка'; elInT.className='tv err';}
  else {elInT.textContent=inT+'°'; elInT.className='tv';}
  elInH.textContent = (inH===null?'--':inH);
  elInH.parentElement.className = (inH===null)?'dht-h err':'dht-h';
  const outT = d.dhtT[0], outH = d.dhtH[0];
  const elOutT = document.getElementById('dOutT');
  const elOutH = document.getElementById('dOutH');
  if(outT===null){elOutT.textContent='ошибка'; elOutT.className='tv err';}
  else {elOutT.textContent=outT+'°'; elOutT.className='tv';}
  elOutH.textContent = (outH===null?'--':outH);
  elOutH.parentElement.className = (outH===null)?'dht-h err':'dht-h';

  document.getElementById('alert').innerHTML = d.blocked
    ? "<div class='alert'>❄ Защита: улица "+d.dhtT[0]+"°C &lt; "+d.outdoorMinT+"°C — все реле ВЫКЛ</div>"
    : "";
  for(let i=0;i<2;i++){
    const R=d.relay[i];
    const box=document.getElementById('r'+i);
    const st =document.getElementById('r'+i+'st');
    const md =document.getElementById('r'+i+'md');
    box.className = 'relay '+(d.blocked?'blk':(R.state?'on':'off'));
    st.textContent = R.state?'ВКЛ':'ВЫКЛ';
    st.className = 'rstate '+(R.state?'on':'off');
    md.textContent = MNAME[R.mode];
    md.className = 'rbadge '+(d.blocked?'b-blk':MCLS[R.mode]);
    if(d.blocked) md.textContent+=' ❄';
    let inf='<b>'+NAMES[R.sensor]+'</b><br>';
    if(R.mode===3) inf += 'Диапазон окна: <b>'+R.tOn+'…'+R.tOff+'°C</b>';
    else if(R.mode===0){
      if(R.tOn>R.tOff) inf += 'Охлаждение: ВКЛ ≥ <b>'+R.tOn+'°C</b>, ВЫКЛ ≤ <b>'+R.tOff+'°C</b>';
      else inf += 'Нагрев: ВКЛ ≤ <b>'+R.tOn+'°C</b>, ВЫКЛ ≥ <b>'+R.tOff+'°C</b>';
    }
    else if(R.mode===1) inf += 'Принудительно ВКЛ';
    else inf += 'Принудительно ВЫКЛ';
    document.getElementById('r'+i+'inf').innerHTML = inf;
    document.querySelectorAll('#r'+i+'bt a').forEach(a=>{
      a.className = (parseInt(a.dataset.m)===R.mode)?'act':'';});
  }
  document.getElementById('outT').textContent = d.outdoorMinT+' °C';
  document.getElementById('outE').textContent =
    d.outdoorEnabled? 'включена' : 'отключена';
 }catch(e){console.log(e);}
}
refresh(); setInterval(refresh,2000);
</script></body></html>
)HTML";

// ----- HTML настроек -----
const char PAGE_SETTINGS[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Настройки</title>
<style>
 *{box-sizing:border-box}
 body{font-family:system-ui,-apple-system,sans-serif;background:#0b0d12;color:#e6e8ec;
      margin:0;padding:14px;font-size:14px}
 h1{font-size:16px;color:#4dd;letter-spacing:2px;text-align:center;margin:4px 0 14px}
 .card{background:#161a22;border:1px solid #232734;border-radius:12px;
       padding:12px;margin:10px 0}
 .card h3{font-size:13px;color:#6a7386;letter-spacing:1px;
          text-transform:uppercase;margin:0 0 8px;font-weight:600}
 label{display:block;margin:10px 0 4px;color:#9aa6bb;font-size:13px}
 input,select{width:100%;padding:10px;border-radius:8px;border:1px solid #2a3040;
              background:#0b0d12;color:#4dd;font-size:16px}
 button{width:100%;padding:14px;margin-top:16px;background:#4dd;color:#000;
        border:none;border-radius:10px;font-size:16px;font-weight:700}
 a.btn{display:block;text-align:center;color:#4dd;margin-top:14px;
       text-decoration:none;font-weight:600}
 .hint{font-size:12px;color:#5a6478;margin-top:4px;line-height:1.4}
</style></head><body>
<h1>НАСТРОЙКИ</h1>
<form method='POST' action='/save'>
 <div class='card'>
  <h3>Реле 1</h3>
  <label>Режим</label>
  <select name='r1_mode'>
    <option value='0'>Авто (охлаждение/нагрев)</option>
    <option value='3'>Окно (ВКЛ внутри диапазона)</option>
    <option value='1'>Всегда ВКЛ</option>
    <option value='2'>Всегда ВЫКЛ</option>
  </select>
  <label>Датчик</label>
  <select name='r1_s'>
    <option value='0'>Вытяжка дом</option>
    <option value='1'>Вытяжка улица</option>
    <option value='2'>Приточка улица</option>
    <option value='3'>Приточка дом</option>
  </select>
  <label>Порог включения / нижняя граница (°C)</label>
  <input type='number' step='0.5' name='r1_on' value='%R1_ON%'>
  <label>Порог выключения / верхняя граница (°C)</label>
  <input type='number' step='0.5' name='r1_off' value='%R1_OFF%'>
  <div class='hint'>Для "Окно": ВКЛ при T внутри [порог_вкл, порог_выкл].</div>
 </div>
 <div class='card'>
  <h3>Реле 2</h3>
  <label>Режим</label>
  <select name='r2_mode'>
    <option value='0'>Авто (охлаждение/нагрев)</option>
    <option value='3'>Окно (ВКЛ внутри диапазона)</option>
    <option value='1'>Всегда ВКЛ</option>
    <option value='2'>Всегда ВЫКЛ</option>
  </select>
  <label>Датчик</label>
  <select name='r2_s'>
    <option value='0'>Вытяжка дом</option>
    <option value='1'>Вытяжка улица</option>
    <option value='2'>Приточка улица</option>
    <option value='3'>Приточка дом</option>
  </select>
  <label>Порог включения / нижняя граница (°C)</label>
  <input type='number' step='0.5' name='r2_on' value='%R2_ON%'>
  <label>Порог выключения / верхняя граница (°C)</label>
  <input type='number' step='0.5' name='r2_off' value='%R2_OFF%'>
  <div class='hint'>Для "Окно": ВКЛ при T внутри [порог_вкл, порог_выкл].</div>
 </div>
 <div class='card'>
  <h3>Уличный датчик (DHT22)</h3>
  <label>Защита от холода</label>
  <select name='o_en'>
    <option value='0'>Отключена</option>
    <option value='1'>Включена</option>
  </select>
  <label>Порог, °C (ниже — все реле ВЫКЛ)</label>
  <input type='number' step='1.0' name='o_min' value='%O_MIN%'>
  <div class='hint'>DHT22 измеряет от -40°C. Пример: -25 — при T улицы ниже -25°C все реле ВЫКЛ.</div>
 </div>
 <button type='submit'>Сохранить</button>
</form>
<a class='btn' href='/'>← На главную</a>
<script>
 document.querySelector("select[name=r1_s]").value    = %R1_S%;
 document.querySelector("select[name=r2_s]").value    = %R2_S%;
 document.querySelector("select[name=r1_mode]").value = %R1_MODE%;
 document.querySelector("select[name=r2_mode]").value = %R2_MODE%;
 document.querySelector("select[name=o_en]").value    = %O_EN%;
</script>
</body></html>
)HTML";

// ============================================================
void handleIndex() { server.send_P(200, "text/html", PAGE_INDEX); }

void handleSettingsPage() {
  String page = FPSTR(PAGE_SETTINGS);
  page.replace("%R1_ON%",   String(relays[0].tOn, 1));
  page.replace("%R1_OFF%",  String(relays[0].tOff, 1));
  page.replace("%R2_ON%",   String(relays[1].tOn, 1));
  page.replace("%R2_OFF%",  String(relays[1].tOff, 1));
  page.replace("%R1_S%",    String(relays[0].sensorIdx));
  page.replace("%R2_S%",    String(relays[1].sensorIdx));
  page.replace("%R1_MODE%", String(relays[0].mode));
  page.replace("%R2_MODE%", String(relays[1].mode));
  page.replace("%O_MIN%",   String(outdoorMinT, 1));
  page.replace("%O_EN%",    String(outdoorEnabled ? 1 : 0));
  server.send(200, "text/html", page);
}

void handleApiData() {
  String j = "{";
  j += "\"dhtT\":[";
  j += isnan(dhtT[0]) ? "null" : String(dhtT[0], 1);  j += ",";
  j += isnan(dhtT[1]) ? "null" : String(dhtT[1], 1);  j += "],";
  j += "\"dhtH\":[";
  j += isnan(dhtH[0]) ? "null" : String(dhtH[0], 0);  j += ",";
  j += isnan(dhtH[1]) ? "null" : String(dhtH[1], 0);  j += "],";
  j += "\"ds\":[";
  for (int i = 0; i < DS_COUNT; i++) {
    if (i) j += ",";
    j += isnan(dsT[i]) ? "null" : String(dsT[i], 1);
  }
  j += "],";
  j += "\"outdoorMinT\":" + String(outdoorMinT, 1) + ",";
  j += "\"outdoorEnabled\":" + String(outdoorEnabled ? "true" : "false") + ",";
  j += "\"blocked\":" + String(systemBlocked ? "true" : "false") + ",";
  j += "\"relay\":[";
  for (int i = 0; i < 2; i++) {
    if (i) j += ",";
    j += "{\"sensor\":" + String(relays[i].sensorIdx);
    j += ",\"tOn\":"   + String(relays[i].tOn, 1);
    j += ",\"tOff\":"  + String(relays[i].tOff, 1);
    j += ",\"mode\":"  + String(relays[i].mode);
    j += ",\"state\":" + String(relays[i].state ? "true" : "false") + "}";
  }
  j += "]}";
  server.send(200, "application/json", j);
}

void handleSave() {
  if (server.hasArg("r1_mode")) {
    int m = server.arg("r1_mode").toInt();
    if (m >= 0 && m <= 3) relays[0].mode = (uint8_t)m;
  }
  if (server.hasArg("r1_s"))   relays[0].sensorIdx = server.arg("r1_s").toInt();
  if (server.hasArg("r1_on"))  relays[0].tOn       = server.arg("r1_on").toFloat();
  if (server.hasArg("r1_off")) relays[0].tOff      = server.arg("r1_off").toFloat();

  if (server.hasArg("r2_mode")) {
    int m = server.arg("r2_mode").toInt();
    if (m >= 0 && m <= 3) relays[1].mode = (uint8_t)m;
  }
  if (server.hasArg("r2_s"))   relays[1].sensorIdx = server.arg("r2_s").toInt();
  if (server.hasArg("r2_on"))  relays[1].tOn       = server.arg("r2_on").toFloat();
  if (server.hasArg("r2_off")) relays[1].tOff      = server.arg("r2_off").toFloat();

  if (server.hasArg("o_en"))  outdoorEnabled = (server.arg("o_en").toInt() != 0);
  if (server.hasArg("o_min")) outdoorMinT    = server.arg("o_min").toFloat();
  if (outdoorMinT < -60.0f) outdoorMinT = -60.0f;
  if (outdoorMinT > 60.0f)  outdoorMinT = 60.0f;

  if (relays[0].sensorIdx > 3) relays[0].sensorIdx = 0;
  if (relays[1].sensorIdx > 3) relays[1].sensorIdx = 0;

  saveSettings();
  updateRelays();

  server.sendHeader("Location", "/");
  server.send(303, "text/plain", "Saved");
}

void handleSetMode() {
  int r = server.arg("r").toInt();
  int m = server.arg("m").toInt();
  if (r >= 0 && r < 2 && m >= 0 && m <= 3) {
    if (relays[r].mode != (uint8_t)m) {
      relays[r].mode = (uint8_t)m;
      saveSettings();
      updateRelays();
      if (currentScreen == SCREEN_MAIN) drawMainScreen();
    }
  }
  server.sendHeader("Location", "/");
  server.send(303, "text/plain", "OK");
}

void setupWebServer() {
  server.on("/",          handleIndex);
  server.on("/settings",  handleSettingsPage);
  server.on("/api/data",  handleApiData);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/setmode",   handleSetMode);
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  server.begin();
  Serial.println("HTTP server started");
}

void handleWebServer() {
  server.handleClient();
}