// ==================== WEBSOCKET ====================

// Dùng host (có cả port) để WebSocket luôn đúng với port 8080
var gateway = `ws://${window.location.host}/ws`;
var websocket = null;

window.addEventListener('load', function () {
    initWebSocket();
    initGauges();
    loadCoreIOTConfig();
    pollSensors();
});

function initWebSocket() {
    console.log('🔌 Mở kết nối WebSocket...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('✅ WebSocket đã kết nối');
}

function onClose(event) {
    console.log('⚠️ WebSocket đóng, thử lại sau 2s...');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    console.log("📩 Nhận:", event.data);
    try {
        var data = JSON.parse(event.data);
        // Cập nhật gauge nếu có dữ liệu
        if (data.temp !== undefined && window.gaugeTemp) {
            window.gaugeTemp.refresh(data.temp);
        }
        if (data.humi !== undefined && window.gaugeHumi) {
            window.gaugeHumi.refresh(data.humi);
        }
    } catch (e) {
        console.warn("Dữ liệu không phải JSON:", event.data);
    }
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        var payload = (typeof data === "string") ? data : JSON.stringify(data);
        websocket.send(payload);
        console.log("📤 Gửi:", payload);
    } else {
        console.warn("⚠️ WebSocket chưa sẵn sàng, không thể gửi!");
    }
}

// ==================== UI NAVIGATION ====================
let relayList = [];
let deleteTarget = null;

function showSection(id, event) {
    // Ẩn tất cả section
    document.querySelectorAll('.section').forEach(sec => sec.style.display = 'none');

    // Hiện section được chọn
    document.getElementById(id).style.display = (id === 'settings') ? 'flex' : 'block';

    // Active menu
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    event.currentTarget.classList.add('active');

    // Khi mở Settings thì load config
    if (id === 'settings') {
        loadCoreIOTConfig();
    }
}

// ==================== HOME GAUGES ====================
function initGauges() {
    window.gaugeTemp = new JustGage({
        id: "gauge_temp",
        value: 26,
        min: -10,
        max: 50,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"]
    });

    window.gaugeHumi = new JustGage({
        id: "gauge_humi",
        value: 60,
        min: 0,
        max: 100,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#42A5F5", "#00BCD4", "#0288D1"]
    });
}

// ==================== DEVICE FUNCTIONS ====================
function openAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'flex';
}

function closeAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'none';
}

function saveRelay() {
    const name = document.getElementById('relayName').value.trim();
    const gpio = document.getElementById('relayGPIO').value.trim();

    if (!name || !gpio) {
        alert("⚠️ Vui lòng điền đầy đủ tên và GPIO!");
        return;
    }

    relayList.push({ id: Date.now(), name, gpio, state: false });
    renderRelays();
    closeAddRelayDialog();
}

function renderRelays() {
    const container = document.getElementById('relayContainer');
    container.innerHTML = "";

    relayList.forEach(r => {
        const card = document.createElement('div');
        card.className = 'device-card';
        card.innerHTML = `
            <i class="fa-solid fa-bolt device-icon"></i>
            <h3>${r.name}</h3>
            <p>GPIO: ${r.gpio}</p>
            <button class="toggle-btn ${r.state ? 'on' : ''}" onclick="toggleRelay(${r.id})">
                ${r.state ? 'ON' : 'OFF'}
            </button>
            <i class="fa-solid fa-trash delete-icon" onclick="showDeleteDialog(${r.id})"></i>
        `;
        container.appendChild(card);
    });
}

function toggleRelay(id) {
    const relay = relayList.find(r => r.id === id);
    if (relay) {
        relay.state = !relay.state;

        Send_Data({
            page: "device",
            value: {
                name: relay.name,
                status: relay.state ? "ON" : "OFF",
                gpio: relay.gpio
            }
        });

        renderRelays();
    }
}

function showDeleteDialog(id) {
    deleteTarget = id;
    document.getElementById('confirmDeleteDialog').style.display = 'flex';
}

function closeConfirmDelete() {
    document.getElementById('confirmDeleteDialog').style.display = 'none';
}

function confirmDelete() {
    relayList = relayList.filter(r => r.id !== deleteTarget);
    renderRelays();
    closeConfirmDelete();
}

// ==================== COREIOT CONFIG API ====================

// Load config từ ESP32
async function loadCoreIOTConfig() {
    try {
        const response = await fetch('/api/coreiot/config');
        if (!response.ok) {
            console.warn("⚠️ Không load được CoreIOT config");
            return;
        }

        const data = await response.json();

        // Điền vào form (WiFi phần này ESP32 không dùng, chỉ MQTT)
        if (data.server)    document.getElementById('server').value = data.server;
        if (data.port)      document.getElementById('port').value = data.port;
        if (data.client_id) document.getElementById('client_id').value = data.client_id;
        if (data.username)  document.getElementById('mqtt_username').value = data.username;

        // Nếu đã có password trên ESP32 thì báo cho người dùng
        const mqttPassInput = document.getElementById('mqtt_password');
        mqttPassInput.value = "";
        if (data.password_set) {
            mqttPassInput.placeholder = "Mật khẩu đã lưu (để trống = giữ nguyên)";
        } else {
            mqttPassInput.placeholder = "Password (MQTT)";
        }

        console.log("✅ Đã load CoreIOT config");
    } catch (error) {
        console.error("❌ Lỗi load config:", error);
    }
}

// Lưu config lên ESP32
document.getElementById("settingsForm").addEventListener("submit", async function (e) {
    e.preventDefault();

    const ssid          = document.getElementById("ssid").value.trim();      // hiện tại ESP32 chưa dùng
    const wifiPassword  = document.getElementById("password").value.trim();  // hiện tại ESP32 chưa dùng
    const server        = document.getElementById("server").value.trim();
    const portValue     = document.getElementById("port").value.trim();
    const client_id     = document.getElementById("client_id").value.trim();
    const mqtt_username = document.getElementById("mqtt_username").value.trim();
    const mqtt_password = document.getElementById("mqtt_password").value.trim();

    // Kiểm tra port
    const port = parseInt(portValue, 10);
    if (!Number.isInteger(port) || port <= 0 || port > 65535) {
        alert("⚠️ Port không hợp lệ! (1-65535)");
        return;
    }

    // Kiểm tra field bắt buộc
    if (!server || !client_id || !mqtt_username) {
        alert("⚠️ Vui lòng điền đủ: Server, Client ID, Username!");
        return;
    }

    // Payload gửi cho ESP32 (WiFi tạm thời chỉ gửi kèm, backend đang bỏ qua)
    const config = {
        server    : server,
        port      : port,
        client_id : client_id,
        username  : mqtt_username,
        password  : mqtt_password || "***"  // rỗng = giữ lại password cũ
        // ssid: ssid,
        // wifi_password: wifiPassword
    };

    try {
        const response = await fetch('/api/coreiot/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(config)
        });

        const result = await response.json();

        if (result.success) {
            alert("✅ Đã lưu cấu hình! MQTT sẽ kết nối lại.");
            setTimeout(loadCoreIOTConfig, 500);
        } else {
            alert("❌ Lỗi: " + (result.message || "Không rõ nguyên nhân"));
        }
    } catch (error) {
        console.error("❌ Lỗi gửi config:", error);
        alert("❌ Không thể kết nối đến ESP32!");
    }
});

// ==================== SENSOR POLLING ====================
async function pollSensors() {
    try {
        const res = await fetch('/sensor');
        if (!res.ok) return;
        const data = await res.json();
        if (data.error) return;
        if (window.gaugeTemp) window.gaugeTemp.refresh(data.temperature ?? 0);
        if (window.gaugeHumi) window.gaugeHumi.refresh(data.humidity ?? 0);
        if (window.gaugeRain) window.gaugeRain.refresh(data.rain ?? 0);
    } catch (err) {
        console.warn('Sensor poll failed', err);
    }
}

setInterval(pollSensors, 5000);

