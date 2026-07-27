#include "WebServerTask.h"
#include "LocalIntelligence.h"
#include "focus_mode.h"
#include "OTAManager.h"
#include "WiFiManager.h"
#include "DataPool.h"
#include "lv_port_disp.h"
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ==================== mDNS 配置 ====================
#define MDNS_HOSTNAME "cube"  // 访问地址: http://cube.local/

// ==================== Web 服务器实例 ====================
static WebServer webServer(80);
static TaskHandle_t webServerTaskHandle = nullptr;

// ==================== 主页（HTML 界面）====================
static void handleRoot() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Desktop Cube</title>
    <style>
        * { box-sizing: border-box; }
        body { font-family: -apple-system, Arial, sans-serif; max-width: 480px; margin: 20px auto; padding: 16px; background: #1a1a2e; color: #eee; }
        h1 { text-align: center; font-size: 1.3em; margin-bottom: 16px; color: #fff; }
        .card { background: #16213e; border-radius: 14px; padding: 16px; margin: 12px 0; }
        .card h2 { font-size: 0.9em; color: #888; margin: 0 0 12px 0; letter-spacing: 1px; }
        .sensor { display: flex; justify-content: space-between; padding: 8px 0; border-bottom: 1px solid #1a1a3e; }
        .sensor:last-child { border-bottom: none; }
        .value { font-weight: bold; color: #4fc3f7; }
        .status { text-align: center; padding: 8px; border-radius: 8px; margin: 6px 0; font-size: 0.85em; }
        .status.ok { background: #1b5e20; color: #a5d6a7; }
        .status.warn { background: #e65100; color: #ffcc80; }
        /* Toggle Switch */
        .toggle-row { display: flex; justify-content: space-between; align-items: center; padding: 12px 0; border-bottom: 1px solid #1a1a3e; }
        .toggle-row:last-child { border-bottom: none; }
        .toggle-label { font-size: 0.95em; }
        .toggle { position: relative; width: 52px; height: 28px; border-radius: 14px; background: #333; cursor: pointer; transition: background 0.3s; flex-shrink: 0; }
        .toggle.on { background: #4caf50; }
        .toggle::after { content: ''; position: absolute; top: 3px; left: 3px; width: 22px; height: 22px; border-radius: 11px; background: #fff; transition: transform 0.3s; }
        .toggle.on::after { transform: translateX(24px); }
        /* Button */
        .btn { display: block; width: 100%; padding: 12px; border: none; border-radius: 10px; font-size: 1em; cursor: pointer; transition: background 0.3s; margin: 8px 0; }
        .btn-focus { background: #7c4dff; color: #fff; }
        .btn-focus.active { background: #ff5252; }
        /* Slider */
        .slider-row { padding: 12px 0; }
        .slider-row label { font-size: 0.9em; display: block; margin-bottom: 8px; }
        input[type=range] { width: 100%; height: 6px; border-radius: 3px; background: #333; outline: none; -webkit-appearance: none; }
        input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 20px; height: 20px; border-radius: 10px; background: #4fc3f7; cursor: pointer; }
        .slider-val { float: right; color: #4fc3f7; font-weight: bold; }
    </style>
</head>
<body>
    <h1>Smart Desktop Cube</h1>
    <div class="card">
        <h2>传感器数据</h2>
        <div class="sensor"><span>温度</span><span class="value" id="temp">--</span></div>
        <div class="sensor"><span>湿度</span><span class="value" id="humi">--</span></div>
        <div class="sensor"><span>光照</span><span class="value" id="light">--</span></div>
        <div class="sensor"><span>CO2</span><span class="value" id="eco2">--</span></div>
        <div class="sensor"><span>TVOC</span><span class="value" id="tvoc">--</span></div>
        <div class="sensor"><span>PM2.5</span><span class="value" id="pm25">--</span></div>
        <div class="sensor"><span>人体存在</span><span class="value" id="human">--</span></div>
        <div class="sensor"><span>AQI</span><span class="value" id="aqi">--</span></div>
    </div>
    <div class="card">
        <h2>状态</h2>
        <div id="alerts"></div>
    </div>
    <div class="card">
        <h2>控制</h2>
        <button class="btn btn-focus" id="btn-focus" onclick="toggleFocus()">进入专注模式</button>
        <div class="toggle-row"><span class="toggle-label">自动进入专注</span><div class="toggle" id="tg-autofocus" onclick="sendCtrl('auto_focus')"></div></div>
        <div class="toggle-row"><span class="toggle-label">自动亮度</span><div class="toggle" id="tg-bright" onclick="sendCtrl('auto_brightness')"></div></div>
        <div class="toggle-row"><span class="toggle-label">免打扰模式</span><div class="toggle" id="tg-silent" onclick="sendCtrl('silent_mode')"></div></div>
        <div class="slider-row">
            <label>屏幕亮度 <span class="slider-val" id="br-val">100%</span></label>
            <input type="range" id="br-slider" min="10" max="100" value="100" oninput="setBrightness(this.value)">
        </div>
    </div>
    <div class="card">
        <h2>固件更新</h2>
        <div class="sensor"><span>当前版本</span><span class="value" id="fw-ver">--</span></div>
        <div id="ota-section">
            <button class="btn" style="background:#0288d1;color:#fff;" id="btn-check" onclick="checkUpdate()">检测更新</button>
            <div id="ota-available" style="display:none;margin-top:10px;">
                <div class="status warn" id="ota-info">发现新版本</div>
                <button class="btn" style="background:#4caf50;color:#fff;" onclick="confirmUpdate()">确认更新</button>
                <button class="btn" style="background:#555;color:#ccc;" onclick="dismissUpdate()">暂不更新</button>
            </div>
            <div id="ota-progress" style="display:none;margin-top:10px;">
                <div class="status ok" id="ota-prog-text">更新中...</div>
            </div>
        </div>
    </div>
    <div class="card">
        <h2>WiFi 配置</h2>
        <div class="sensor"><span>当前网络</span><span class="value" id="wifi-ssid">--</span></div>
        <div class="sensor"><span>连接状态</span><span class="value" id="wifi-status">--</span></div>
        <button class="btn" style="background:#ff9800;color:#fff;" id="btn-scan" onclick="scanWifi()">扫描 WiFi</button>
        <div id="wifi-list" style="display:none;margin:10px 0;">
            <select id="wifi-select" style="width:100%;padding:10px;border-radius:8px;background:#1a1a2e;color:#eee;border:1px solid #333;font-size:0.9em;">
                <option value="">-- 选择网络 --</option>
            </select>
        </div>
        <div id="wifi-form" style="display:none;margin-top:10px;">
            <input type="password" id="wifi-pwd" placeholder="输入 WiFi 密码" style="width:100%;padding:10px;border-radius:8px;background:#1a1a2e;color:#eee;border:1px solid #333;font-size:0.9em;margin-bottom:8px;">
            <button class="btn" style="background:#4caf50;color:#fff;" onclick="connectWifi()">连接</button>
        </div>
        <div id="wifi-result" style="display:none;margin-top:8px;"></div>
    </div>
    <script>
        function updateData() {
            fetch('/api/data').then(r=>r.json()).then(d=>{
                document.getElementById('temp').textContent = d.temp.toFixed(1)+' °C';
                document.getElementById('humi').textContent = d.humi.toFixed(0)+' %';
                document.getElementById('light').textContent = d.light.toFixed(0)+' lx';
                document.getElementById('eco2').textContent = d.eco2.toFixed(0)+' ppm';
                document.getElementById('tvoc').textContent = d.tvoc.toFixed(0)+' ppb';
                document.getElementById('pm25').textContent = d.pm25.toFixed(0)+' µg/m³';
                document.getElementById('human').textContent = d.human_exist?'有人':'无人';
                document.getElementById('aqi').textContent = d.aqi;
                let bf=document.getElementById('btn-focus');
                if(d.focus_mode){bf.textContent='退出专注模式';bf.classList.add('active');}
                else{bf.textContent='进入专注模式';bf.classList.remove('active');}
                setTg('tg-autofocus', d.auto_focus);
                setTg('tg-bright', d.auto_brightness);
                setTg('tg-silent', d.silent_mode);
                document.getElementById('br-slider').value = d.screen_brightness;
                document.getElementById('br-val').textContent = d.screen_brightness+'%';
                let a='';
                if(d.air_quality_alert) a+='<div class="status warn">空气质量差，建议通风</div>';
                if(d.temp_comfort_alert) a+='<div class="status warn">舒适度偏低，注意调节</div>';
                if(d.silent_mode) a+='<div class="status ok">免打扰模式已开启</div>';
                if(!a) a='<div class="status ok">一切正常</div>';
                document.getElementById('alerts').innerHTML=a;
                // WiFi 状态
                document.getElementById('wifi-ssid').textContent = d.wifi_ssid || '--';
                document.getElementById('wifi-status').textContent = d.wifi_connected ? '已连接 ✅' : '未连接 ❌';
                document.getElementById('wifi-status').style.color = d.wifi_connected ? '#a5d6a7' : '#ff8a80';
                // OTA 状态 (ota_check_status: 0=idle 1=checking 2=update_available 3=latest 4=failed 5=timeout)
                document.getElementById('fw-ver').textContent = d.fw_version || '--';
                var cs = d.ota_check_status;
                var btnCheck = document.getElementById('btn-check');
                if(d.ota_in_progress){
                    btnCheck.style.display='none';
                    document.getElementById('ota-available').style.display='none';
                    document.getElementById('ota-progress').style.display='block';
                    document.getElementById('ota-prog-text').textContent='正在更新... '+d.ota_progress+'%';
                    document.getElementById('ota-prog-text').className='status ok';
                } else if(cs==2 || d.ota_update_available){
                    btnCheck.style.display='none';
                    document.getElementById('ota-available').style.display='block';
                    document.getElementById('ota-progress').style.display='none';
                    document.getElementById('ota-info').textContent='发现新版本: '+d.ota_pending_version;
                    document.getElementById('ota-info').className='status warn';
                } else if(cs==1){
                    btnCheck.style.display='block';
                    btnCheck.textContent='检测中...';
                    btnCheck.disabled=true;
                    btnCheck.style.opacity='0.5';
                    document.getElementById('ota-available').style.display='none';
                    document.getElementById('ota-progress').style.display='none';
                } else if(cs==3){
                    btnCheck.style.display='block';
                    btnCheck.textContent='检测更新';
                    btnCheck.disabled=false;
                    btnCheck.style.opacity='1';
                    document.getElementById('ota-available').style.display='none';
                    document.getElementById('ota-progress').style.display='block';
                    document.getElementById('ota-prog-text').textContent='已是最新版本';
                    document.getElementById('ota-prog-text').className='status ok';
                } else if(cs==4){
                    btnCheck.style.display='block';
                    btnCheck.textContent='检测更新';
                    btnCheck.disabled=false;
                    btnCheck.style.opacity='1';
                    document.getElementById('ota-available').style.display='none';
                    document.getElementById('ota-progress').style.display='block';
                    document.getElementById('ota-prog-text').textContent='检测失败：MQTT未连接或认证未完成';
                    document.getElementById('ota-prog-text').className='status warn';
                } else if(cs==5){
                    btnCheck.style.display='block';
                    btnCheck.textContent='检测更新';
                    btnCheck.disabled=false;
                    btnCheck.style.opacity='1';
                    document.getElementById('ota-available').style.display='none';
                    document.getElementById('ota-progress').style.display='block';
                    document.getElementById('ota-prog-text').textContent='检测超时（5s无响应），请重试';
                    document.getElementById('ota-prog-text').className='status warn';
                } else {
                    btnCheck.style.display='block';
                    btnCheck.textContent='检测更新';
                    btnCheck.disabled=false;
                    btnCheck.style.opacity='1';
                    document.getElementById('ota-available').style.display='none';
                    document.getElementById('ota-progress').style.display='none';
                }
            });
        }
        function setTg(id,on){document.getElementById(id).classList.toggle('on',on);}
        function toggleFocus(){
            fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({command:'focus',value:'toggle'})}).then(()=>setTimeout(updateData,300));
        }
        function sendCtrl(cmd){
            fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({command:cmd,value:'toggle'})}).then(()=>setTimeout(updateData,300));
        }
        let brTimer=null;
        function setBrightness(v){
            document.getElementById('br-val').textContent=v+'%';
            clearTimeout(brTimer);
            brTimer=setTimeout(()=>{
                fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({command:'brightness',value:String(v)})});
            },300);
        }
        function checkUpdate(){
            fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({command:'check_update',value:'1'})}).then(()=>setTimeout(updateData,300));
        }
        function confirmUpdate(){
            fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({command:'confirm_update',value:'1'})}).then(()=>setTimeout(updateData,500));
        }
        function dismissUpdate(){
            fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({command:'dismiss_update',value:'1'})}).then(()=>setTimeout(updateData,300));
        }
        // ===== WiFi 配置 =====
        function scanWifi(){
            var btn=document.getElementById('btn-scan');
            btn.textContent='扫描中...';btn.disabled=true;
            fetch('/api/wifi/scan').then(r=>r.json()).then(d=>{
                btn.textContent='扫描 WiFi';btn.disabled=false;
                var sel=document.getElementById('wifi-select');
                sel.innerHTML='<option value="">-- 选择网络 --</option>';
                d.networks.forEach(function(n){
                    var opt=document.createElement('option');
                    opt.value=n.ssid;
                    opt.textContent=n.ssid+' ('+n.rssi+'dBm)'+(n.secure?' 🔒':' 📶');
                    sel.appendChild(opt);
                });
                document.getElementById('wifi-list').style.display='block';
                document.getElementById('wifi-form').style.display='none';
                document.getElementById('wifi-result').style.display='none';
            }).catch(function(){btn.textContent='扫描 WiFi';btn.disabled=false;});
        }
        document.addEventListener('DOMContentLoaded',function(){
            var sel=document.getElementById('wifi-select');
            if(sel) sel.addEventListener('change',function(){
                document.getElementById('wifi-form').style.display=this.value?'block':'none';
            });
        });
        function connectWifi(){
            var ssid=document.getElementById('wifi-select').value;
            var pwd=document.getElementById('wifi-pwd').value;
            if(!ssid){alert('请选择 WiFi');return;}
            var res=document.getElementById('wifi-result');
            res.style.display='block';
            res.textContent='';
            var d1=document.createElement('div');d1.className='status warn';d1.textContent='正在连接 '+ssid+' ...';res.appendChild(d1);
            fetch('/api/wifi/connect',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:ssid,password:pwd})})
            .then(r=>r.json()).then(d=>{
                res.textContent='';
                var d2=document.createElement('div');
                if(d.success){
                    d2.className='status ok';d2.textContent='已连接 '+ssid+'，IP: '+d.ip;
                }else{
                    d2.className='status warn';d2.textContent='连接失败：'+d.msg;
                }
                res.appendChild(d2);
                setTimeout(updateData,1000);
            }).catch(function(){res.textContent='';var d3=document.createElement('div');d3.className='status warn';d3.textContent='请求失败';res.appendChild(d3);});
        }
        setInterval(updateData,2000);
        updateData();
    </script>
</body>
</html>
)rawliteral";
    
    webServer.send(200, "text/html", html);
}

// ==================== API: 获取传感器数据 ====================
static void handleApiData() {
    JsonDocument doc;
    
    doc["temp"] = sensorData.temp;
    doc["humi"] = sensorData.humi;
    doc["light"] = sensorData.light;
    doc["eco2"] = sensorData.eco2;
    doc["tvoc"] = sensorData.tvoc;
    doc["pm25"] = sensorData.pm25;
    doc["aqi"] = sensorData.aqi;
    doc["human_exist"] = status.is_human_exist;
    doc["human_duration"] = sensorData.human_duration;
    
    doc["focus_mode"] = status.focus_mode;
    doc["auto_focus"] = focusConfig.auto_enter_enabled;
    doc["auto_brightness"] = status.auto_brightness_enabled;
    doc["silent_mode"] = status.silent_mode;
    
    doc["air_quality_alert"] = status.air_quality_alert;
    doc["temp_comfort_alert"] = status.temp_comfort_alert;
    
    doc["wifi_connected"] = status.wifi_connected;
    doc["wifi_ssid"] = WiFiManager_GetSSID();
    doc["mqtt_connected"] = status.mqtt_connected;
    doc["screen_brightness"] = status.screen_brightness;
    
    // OTA 状态
    doc["fw_version"] = FW_VERSION;
    doc["ota_in_progress"] = status.ota_in_progress;
    doc["ota_progress"] = status.ota_progress;
    doc["ota_update_available"] = status.ota_update_available;
    doc["ota_pending_version"] = status.ota_pending_version;
    doc["ota_check_status"] = status.ota_check_status;
    
    String response;
    serializeJson(doc, response);
    webServer.send(200, "application/json", response);
}

// ==================== API: 获取历史数据 ====================
static void handleApiHistory() {
    int count = GetHistoryCount();
    int max_count = (count > 60) ? 60 : count;
    
    float temp[60], humi[60], light[60], eco2[60];
    GetHistoryData(temp, humi, light, eco2, max_count);
    
    JsonDocument doc;
    doc["count"] = max_count;
    
    JsonArray tempArr = doc["temp"].to<JsonArray>();
    JsonArray humiArr = doc["humi"].to<JsonArray>();
    JsonArray lightArr = doc["light"].to<JsonArray>();
    JsonArray eco2Arr = doc["eco2"].to<JsonArray>();
    
    for (int i = 0; i < max_count; i++) {
        tempArr.add(temp[i]);
        humiArr.add(humi[i]);
        lightArr.add(light[i]);
        eco2Arr.add(eco2[i]);
    }
    
    String response;
    serializeJson(doc, response);
    webServer.send(200, "application/json", response);
}

// ==================== API: 控制指令 ====================
static void setToast(const char* msg) {
    strncpy(status.pending_toast, msg, sizeof(status.pending_toast) - 1);
    status.pending_toast[sizeof(status.pending_toast) - 1] = '\0';
    status.toast_pending = true;
}

static void handleApiControl() {
    if (!webServer.hasArg("plain")) {
        webServer.send(400, "application/json", "{\"error\":\"No body\"}");
        return;
    }
    
    String body = webServer.arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
        webServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    const char* command = doc["command"];
    const char* value = doc["value"] | "";

    if (!command) {
        webServer.send(400, "application/json", "{\"error\":\"No command\"}");
        return;
    }

    // 处理控制指令
    if (strcmp(command, "focus") == 0) {
        bool was_focus = status.focus_mode;
        if (strcmp(value, "toggle") == 0) {
            status.focus_mode = !status.focus_mode;
            status.request_focus_screen = status.focus_mode;
        } else if (strcmp(value, "on") == 0) {
            status.focus_mode = true;
            status.request_focus_screen = true;
        } else if (strcmp(value, "off") == 0) {
            status.focus_mode = false;
        }
        // 专注会话生命周期管理
        if (!was_focus && status.focus_mode) {
            sensorData.focus_duration = 0;
            focusMode_notifyManualEnter();  // 开始环境数据记录
        } else if (was_focus && !status.focus_mode) {
            FocusSession_End();             // 生成总结 + 触发弹窗
            sensorData.focus_duration = 0;
        }
        setToast(status.focus_mode ? "Focus ON" : "Focus OFF");
    }
    else if (strcmp(command, "auto_focus") == 0) {
        if (strcmp(value, "toggle") == 0) {
            focusConfig.auto_enter_enabled = !focusConfig.auto_enter_enabled;
        } else if (strcmp(value, "on") == 0) {
            focusConfig.auto_enter_enabled = true;
        } else if (strcmp(value, "off") == 0) {
            focusConfig.auto_enter_enabled = false;
        }
        setToast(focusConfig.auto_enter_enabled ? "Auto Focus ON" : "Auto Focus OFF");
    }
    else if (strcmp(command, "auto_brightness") == 0) {
        if (strcmp(value, "toggle") == 0) {
            status.auto_brightness_enabled = !status.auto_brightness_enabled;
        } else if (strcmp(value, "on") == 0) {
            status.auto_brightness_enabled = true;
        } else if (strcmp(value, "off") == 0) {
            status.auto_brightness_enabled = false;
        }
        setToast(status.auto_brightness_enabled ? "Auto Bright ON" : "Auto Bright OFF");
    }
    else if (strcmp(command, "silent_mode") == 0) {
        if (strcmp(value, "toggle") == 0) {
            status.silent_mode = !status.silent_mode;
        } else if (strcmp(value, "on") == 0) {
            status.silent_mode = true;
        } else if (strcmp(value, "off") == 0) {
            status.silent_mode = false;
        }
        setToast(status.silent_mode ? "Silent ON" : "Silent OFF");
    }
    else if (strcmp(command, "brightness") == 0) {
        int val = atoi(value);
        if (val < 10) val = 10;
        if (val > 100) val = 100;
        status.screen_brightness = val;
        status.auto_brightness_enabled = false;  // 手动调节时关闭自动亮度
        lv_port_disp_set_backlight((uint8_t)val);
        char msg[32];
        snprintf(msg, sizeof(msg), "Brightness %d%%", val);
        setToast(msg);
    }
    else if (strcmp(command, "check_update") == 0) {
        if (!status.mqtt_connected) {
            status.ota_check_status = 4;  // failed - no MQTT
        } else if (!security.token_ok) {
            status.ota_check_status = 4;  // failed - token not ready (handshake pending)
            Serial.println("[OTA] Web请求检测，但Token未就绪");
        } else {
            status.ota_check_requested = true;
            status.ota_check_status = 1;  // checking
            status.ota_check_time = millis();
        }
    }
    else if (strcmp(command, "confirm_update") == 0) {
        if (status.ota_update_available && strlen(status.ota_pending_url) > 0) {
            bool ok = otaTrigger(status.ota_pending_url, status.ota_pending_version, status.ota_pending_md5);
            if (ok) {
                status.ota_update_available = false;
                setToast("OTA started!");
            } else {
                setToast("OTA trigger failed");
            }
        }
    }
    else if (strcmp(command, "dismiss_update") == 0) {
        status.ota_update_available = false;
        status.ota_check_status = 0;
        status.ota_pending_url[0] = '\0';
        status.ota_pending_version[0] = '\0';
        status.ota_pending_md5[0] = '\0';
    }
    
    webServer.send(200, "application/json", "{\"status\":\"ok\"}");
    Serial.printf("[Web] Control: %s = %s\n", command, value ? value : "null");
}

// ==================== API: WiFi 扫描 ====================
static void handleWifiScan() {
    Serial.println("[Web] WiFi 扫描请求");
    String result = WiFiManager_Scan();
    webServer.send(200, "application/json", result);
}

// ==================== API: WiFi 连接 ====================
static void handleWifiConnect() {
    if (!webServer.hasArg("plain")) {
        webServer.send(400, "application/json", "{\"success\":false,\"msg\":\"No body\"}");
        return;
    }
    String body = webServer.arg("plain");
    JsonDocument doc;
    if (deserializeJson(doc, body)) {
        webServer.send(400, "application/json", "{\"success\":false,\"msg\":\"Invalid JSON\"}");
        return;
    }
    const char* ssid = doc["ssid"] | "";
    const char* password = doc["password"] | "";

    if (strlen(ssid) == 0) {
        webServer.send(400, "application/json", "{\"success\":false,\"msg\":\"SSID为空\"}");
        return;
    }

    Serial.printf("[Web] WiFi 连接请求: SSID=%s\n", ssid);
    bool ok = WiFiManager_ConnectTo(ssid, password);

    JsonDocument resp;
    resp["success"] = ok;
    if (ok) {
        resp["ip"] = WiFi.localIP().toString();
        resp["msg"] = "连接成功";
        setToast("WiFi Connected!");
    } else {
        resp["msg"] = "连接超时，请检查密码或距离";
    }
    String out;
    serializeJson(resp, out);
    webServer.send(200, "application/json", out);
}

// ==================== Captive Portal 检测端点 ====================
// Android: 访问 /generate_204，期望收到 204 或重定向
// iOS: 访问 /hotspot-detect.html，期望收到特定 HTML
// Windows: 访问 /ncsi.txt
static void handleCaptivePortal() {
    // 重定向到控制页面（触发手机弹出 Captive Portal 窗口）
    String ip = WiFi.softAPIP().toString();
    webServer.sendHeader("Location", "http://" + ip + "/");
    webServer.send(302, "text/plain", "");
}

static void handleCaptiveApple() {
    // iOS 检测：返回非标准内容触发弹窗
    webServer.send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
}

// ==================== CORS 支持（允许跨域请求）====================
static void handleCors() {
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    webServer.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    webServer.send(204);
}

// ==================== Web 服务器任务 ====================
static void WebServerTask(void* pvParameters) {
    // 等待 WiFi/TCP-IP 协议栈就绪（AP 启动 = lwip 已初始化）
    // 不等待直接 begin() 会触发 "Invalid mbox" 断言崩溃
    Serial.println("[Web] 等待网络就绪...");
    while (WiFi.softAPIP() == IPAddress(0, 0, 0, 0)) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    Serial.printf("[Web] 网络就绪，AP IP: %s\n", WiFi.softAPIP().toString().c_str());

    // 启动 mDNS（让 ESP32 可通过 cube.local 访问）
    if (MDNS.begin(MDNS_HOSTNAME)) {
        Serial.printf("[Web] mDNS started: http://%s.local/\n", MDNS_HOSTNAME);
    } else {
        Serial.println("[Web] mDNS failed, use IP only");
    }
    
    // 注册路由
    webServer.on("/", HTTP_GET, handleRoot);
    webServer.on("/api/data", HTTP_GET, handleApiData);
    webServer.on("/api/history", HTTP_GET, handleApiHistory);
    webServer.on("/api/control", HTTP_POST, handleApiControl);
    webServer.on("/api/control", HTTP_OPTIONS, handleCors);
    webServer.on("/api/wifi/scan", HTTP_GET, handleWifiScan);
    webServer.on("/api/wifi/connect", HTTP_POST, handleWifiConnect);
    // Captive Portal 检测（手机连 AP 后自动弹出页面）
    webServer.on("/generate_204", HTTP_GET, handleCaptivePortal);       // Android
    webServer.on("/hotspot-detect.html", HTTP_GET, handleCaptiveApple); // iOS
    webServer.on("/library/test/success.html", HTTP_GET, handleCaptiveApple); // iOS alt
    webServer.on("/ncsi.txt", HTTP_GET, handleCaptivePortal);           // Windows
    webServer.on("/connecttest.txt", HTTP_GET, handleCaptivePortal);    // Windows 11
    
    // 启动服务器
    webServer.begin();
    Serial.printf("[Web] Web server started\n");
    Serial.printf("[Web]    mDNS: http://%s.local/\n", MDNS_HOSTNAME);
    Serial.printf("[Web]    IP:   http://%s/\n", WiFi.localIP().toString().c_str());
    
    uint8_t last_ap_clients = 0;
    uint32_t last_ap_toast = 0;

    while (1) {
        webServer.handleClient();

        // Captive Portal DNS 处理
        WiFiManager_DNSLoop();

        // 检测 AP 客户端 → 持续显示 IP 浮窗，断开后自动消失
        uint8_t ap_clients = WiFiManager_GetAPClientCount();
        if (ap_clients > 0) {
            // 有设备连着：每 5 秒刷新一次 toast 保持可见
            if (millis() - last_ap_toast > 5000) {
                last_ap_toast = millis();
                char msg[48];
                if (status.wifi_connected) {
                    snprintf(msg, sizeof(msg), "Ctrl: %s", WiFi.localIP().toString().c_str());
                } else {
                    snprintf(msg, sizeof(msg), "Ctrl: %s", WiFi.softAPIP().toString().c_str());
                }
                setToast(msg);
            }
        }
        last_ap_clients = ap_clients;

        vTaskDelay(pdMS_TO_TICKS(10));  // 让出 CPU
    }
}

// ==================== 初始化 ====================
void WebServerTask_Init(void) {
    xTaskCreatePinnedToCore(
        WebServerTask,
        "WebServer",
        8192,       // 栈大小（Web 服务器需要较大栈）
        NULL,
        1,          // 优先级（低）
        &webServerTaskHandle,
        0           // 核心 0（网络层）
    );
}
