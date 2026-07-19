"""
SmartDesktopCube-IoT OTA 后端 (单文件)
=====================================
功能：
  1. MQTT 客户端：监听设备上报的 version_check，自动下发 OTA 指令
  2. 固件信息管理：保存最新固件的 url/version/md5
  3. 手动推送：无视 version_check，立即向设备下发 OTA 指令
  4. MD5 计算：算固件 .bin 的 MD5，便于上传 Gitee 后填入

固件 .bin 文件托管在 Gitee Releases（不要放本地，否则 ESP32 受手机热点
客户端隔离影响连不上你 PC）。流程：本地编译 → 上传 Gitee Releases →
拿到下载 URL → 用 push 命令保存到本脚本 → 启动 serve 监听设备。

依赖：
  pip install paho-mqtt

MQTT broker：broker.emqx.io:1883（设备已配置好的公共 broker）
设备 ID：CUBE001（src/DataPool.cpp 里硬编码）
"""

import os
import sys
import json
import hashlib
import paho.mqtt.client as mqtt

# ==================== 配置 ====================
DEVICE_ID    = "CUBE001"
MQTT_BROKER   = "broker.emqx.io"
MQTT_PORT     = 1883
FIRMWARE_INFO = os.path.join(os.path.dirname(os.path.abspath(__file__)), "firmware_info.json")

# 主题（与 src/MqttCom.cpp 完全对齐）
TOPIC_STATUS  = f"cube2026/device/{DEVICE_ID}/status"    # 设备 → 后端（订阅）
TOPIC_CONTROL = f"cube2026/server/{DEVICE_ID}/control"  # 后端 → 设备（发布）


# ==================== 版本比较 ====================
def compare_version(v1, v2):
    """返回 v1 - v2：正数=v1新, 负数=v2新, 0=相等"""
    s1 = v1.lstrip("v").split(".")
    s2 = v2.lstrip("v").split(".")
    for i in range(3):
        a = int(s1[i]) if i < len(s1) else 0
        b = int(s2[i]) if i < len(s2) else 0
        if a != b:
            return a - b
    return 0


# ==================== 固件信息读写 ====================
def load_firmware_info():
    if not os.path.exists(FIRMWARE_INFO):
        return None
    with open(FIRMWARE_INFO, "r", encoding="utf-8") as f:
        return json.load(f)


def save_firmware_info(url, version, md5):
    info = {"url": url, "version": version, "md5": md5}
    with open(FIRMWARE_INFO, "w", encoding="utf-8") as f:
        json.dump(info, f, indent=2, ensure_ascii=False)
    print(f"已保存固件信息到 {FIRMWARE_INFO}")


# ==================== MQTT 回调 ====================
def make_client(client_id):
    """兼容 paho-mqtt 1.x 和 2.x"""
    try:
        return mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id=client_id)
    except (AttributeError, TypeError):
        return mqtt.Client(client_id=client_id)


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"[MQTT] 已连接 {MQTT_BROKER}:{MQTT_PORT}")
        client.subscribe(TOPIC_STATUS)
        print(f"[MQTT] 订阅: {TOPIC_STATUS}")
        print(f"[MQTT] 等待设备 version_check ... (Ctrl+C 退出)\n")
    else:
        print(f"[MQTT] 连接失败 code={rc}")


def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
    except Exception as e:
        print(f"[MQTT] JSON 解析失败: {e}")
        return

    msg_type = payload.get("type")

    if msg_type == "version_check":
        current = payload.get("current_version", "?")
        did = payload.get("device_id", "?")
        print(f"\n[MQTT] 收到 version_check")
        print(f"  设备 ID: {did}")
        print(f"  当前版本: {current}")

        info = load_firmware_info()
        if not info:
            print("  [!] 未配置固件信息，请先用 push 命令")
            return

        latest = info["version"]
        print(f"  最新版本: {latest}")
        if compare_version(latest, current) > 0:
            ota_payload = json.dumps({
                "type": "ota_update",
                "url": info["url"],
                "version": info["version"],
                "md5": info["md5"]
            })
            client.publish(TOPIC_CONTROL, ota_payload)
            print(f"  [>] 已下发 OTA 指令:")
            print(f"      URL:  {info['url']}")
            print(f"      版本: {info['version']}")
            print(f"      MD5:  {info['md5']}")
        else:
            print(f"  [i] 设备已是最新版本，跳过")

    elif msg_type == "handshake":
        print(f"[MQTT] 收到握手: device_id={payload.get('device_id')}")

    elif msg_type == "ota_status":
        print(f"[MQTT] OTA 状态: {payload}")

    elif msg_type == "control_ack":
        print(f"[MQTT] 控制 ACK: {payload}")

    # heartbeat 静默，避免刷屏


# ==================== 命令实现 ====================
def cmd_serve():
    """启动 MQTT 客户端，监听设备的 version_check 并自动下发 OTA"""
    print("=" * 60)
    print("SmartDesktopCube-IoT OTA 后端")
    print(f"  设备 ID: {DEVICE_ID}")
    print(f"  MQTT:    {MQTT_BROKER}:{MQTT_PORT}")
    print(f"  订阅:    {TOPIC_STATUS}")
    print(f"  发布:    {TOPIC_CONTROL}")
    print("=" * 60)

    info = load_firmware_info()
    if info:
        print(f"当前固件:")
        print(f"  版本: {info['version']}")
        print(f"  URL:  {info['url']}")
        print(f"  MD5:  {info['md5']}")
    else:
        print("[!] 未配置固件，请先执行: python ota_backend.py push <url> <version> <md5>")
    print()

    client = make_client(f"cube_backend_{os.getpid()}")
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    try:
        client.loop_forever()
    except KeyboardInterrupt:
        print("\n退出")
        client.disconnect()


def cmd_push(url, version, md5):
    """保存固件信息到 firmware_info.json"""
    save_firmware_info(url, version, md5)
    print(f"\n固件信息已保存:")
    print(f"  版本: {version}")
    print(f"  URL:  {url}")
    print(f"  MD5:  {md5}")
    print(f"\n下一步: python ota_backend.py serve")


def cmd_publish():
    """立即手动下发 OTA 指令（无需等 version_check，前提是设备 MQTT 在线）"""
    info = load_firmware_info()
    if not info:
        print("[!] 无固件信息，请先用 push 命令")
        return

    client = make_client(f"cube_backend_pub_{os.getpid()}")
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    payload = json.dumps({
        "type": "ota_update",
        "url": info["url"],
        "version": info["version"],
        "md5": info["md5"]
    })
    # 等连接建立
    import time
    time.sleep(0.5)
    client.loop_start()
    time.sleep(0.5)
    client.publish(TOPIC_CONTROL, payload)
    time.sleep(1)
    client.loop_stop()
    client.disconnect()
    print(f"[>] 已手动下发 OTA 指令到 {TOPIC_CONTROL}")
    print(f"  版本: {info['version']}")
    print(f"  URL:  {info['url']}")
    print(f"  MD5:  {info['md5']}")


def cmd_md5(filepath):
    """计算固件 .bin 的 MD5"""
    if not os.path.exists(filepath):
        print(f"[!] 文件不存在: {filepath}")
        return
    h = hashlib.md5()
    size = 0
    with open(filepath, "rb") as f:
        while True:
            chunk = f.read(8192)
            if not chunk:
                break
            h.update(chunk)
            size += len(chunk)
    print(f"文件: {filepath}")
    print(f"大小: {size} 字节 ({size/1048576:.2f} MB)")
    print(f"MD5:  {h.hexdigest()}")


def cmd_info():
    """显示当前固件信息和主题"""
    info = load_firmware_info()
    print(f"设备 ID: {DEVICE_ID}")
    print(f"MQTT:    {MQTT_BROKER}:{MQTT_PORT}")
    print(f"订阅:    {TOPIC_STATUS}")
    print(f"发布:    {TOPIC_CONTROL}")
    print()
    if info:
        print(f"当前固件:")
        print(f"  版本: {info['version']}")
        print(f"  URL:  {info['url']}")
        print(f"  MD5:  {info['md5']}")
    else:
        print("未配置固件")


# ==================== 入口 ====================
USAGE = """
SmartDesktopCube-IoT OTA 后端

用法:
  python ota_backend.py serve                      启动 MQTT 监听，设备 version_check 触发自动 OTA
  python ota_backend.py push <url> <ver> <md5>     保存固件信息
  python ota_backend.py publish                     立即手动下发 OTA 指令
  python ota_backend.py md5 <bin_file>             计算固件 .bin 的 MD5
  python ota_backend.py info                        查看当前配置

示例:
  python ota_backend.py md5 .pio\\build\\esp32-s3-devkitm-1\\firmware.bin
  python ota_backend.py push https://gitee.com/yourname/cube-ota/releases/download/v1.0.1/firmware.bin v1.0.1 a1b2c3d4e5f6...
  python ota_backend.py serve
"""

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(USAGE)
        sys.exit(0)

    cmd = sys.argv[1]
    if cmd == "serve":
        cmd_serve()
    elif cmd == "push":
        if len(sys.argv) < 5:
            print("用法: python ota_backend.py push <url> <version> <md5>")
            sys.exit(1)
        cmd_push(sys.argv[2], sys.argv[3], sys.argv[4])
    elif cmd == "publish":
        cmd_publish()
    elif cmd == "md5":
        if len(sys.argv) < 3:
            print("用法: python ota_backend.py md5 <bin_file>")
            sys.exit(1)
        cmd_md5(sys.argv[2])
    elif cmd == "info":
        cmd_info()
    else:
        print(f"未知命令: {cmd}")
        print(USAGE)
