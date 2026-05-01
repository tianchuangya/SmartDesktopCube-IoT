import requests
import csv
import time
from datetime import datetime

# ==================== 配置区 ====================
LAT = 29.86          # ⚠️ 改成你宿舍的纬度（百度/高德地图都能查）
LON = 121.55027        # ⚠️ 改成你宿舍的经度
INTERVAL = 300      # 测试阶段10秒，正式跑记得改回300
OUTPUT_FILE = r"C:\Users\ASUS\Desktop\outdoor_data.csv"

# Open-Meteo 免费 API（无需密钥）
WEATHER_URL = f"https://api.open-meteo.com/v1/forecast?latitude={LAT}&longitude={LON}&current=temperature_2m,relative_humidity_2m"
AIR_URL = f"https://air-quality-api.open-meteo.com/v1/air-quality?latitude={LAT}&longitude={LON}&current=pm2_5"
# =================================================

def fetch_all():
    """一次抓取温度、湿度、PM2.5"""
    try:
        w_resp = requests.get(WEATHER_URL, timeout=10).json()
        temp = w_resp["current"]["temperature_2m"]
        humidity = w_resp["current"]["relative_humidity_2m"]

        a_resp = requests.get(AIR_URL, timeout=10).json()
        pm25 = a_resp["current"]["pm2_5"]

        return temp, humidity, pm25
    except Exception as e:
        print(f"抓取失败: {e}")
        return None, None, None

def main():
    print(f"开始采集室外数据，每 {INTERVAL} 秒一次，写入 {OUTPUT_FILE} ...")
    with open(OUTPUT_FILE, "w", newline="", encoding="utf-8-sig") as f:
        writer = csv.writer(f)
        writer.writerow(["timestamp", "outdoor_temp", "outdoor_humidity", "outdoor_pm25"])

    while True:
        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        temp, humidity, pm25 = fetch_all()

        if temp is not None:
            print(f"{now} | 温度: {temp}°C | 湿度: {humidity}% | PM2.5: {pm25} μg/m³")
            with open(OUTPUT_FILE, "a", newline="", encoding="utf-8-sig") as f:
                writer = csv.writer(f)
                writer.writerow([now, temp, humidity, pm25])
        else:
            print("数据不完整，跳过本次写入")

        time.sleep(INTERVAL)

if __name__ == "__main__":
    main()