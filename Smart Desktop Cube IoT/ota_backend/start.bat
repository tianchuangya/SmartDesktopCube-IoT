@echo off
chcp 65001 >nul
cd /d "%~dp0"

echo ============================================
echo  SmartDesktopCube-IoT OTA 后端一键启动
echo ============================================
echo.

if not exist "firmware_info.json" (
    echo [!] 未配置固件信息，请先执行：
    echo     python ota_backend.py push ^<url^> ^<version^> ^<md5^>
    echo.
    echo 示例：
    echo     python ota_backend.py push https://gitee.com/xxx/cube-ota/releases/download/v1.0.1/firmware.bin v1.0.1 abcdef1234567890...
    echo.
    pause
    exit /b 1
)

python ota_backend.py serve
pause
