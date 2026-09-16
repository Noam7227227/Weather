import os

try:
    Import("env")  # type: ignore # noqa: F821
except NameError:
    pass

env_path = os.path.join(env.get("PROJECT_DIR"), ".env") if "env" in globals() else ".env"  # type: ignore # noqa: F821

wifi_ssid = "YOUR_WIFI_NAME"
wifi_password = "YOUR_WIFI_PASSWORD"

if os.path.exists(env_path):
    with open(env_path, "r") as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith("#") and "=" in line:
                key, value = line.split("=", 1)
                key = key.strip()
                value = value.strip().strip('"').strip("'")
                if key == "WIFI_SSID":
                    wifi_ssid = value
                elif key == "WIFI_PASSWORD":
                    wifi_password = value

if "env" in globals():
    env.Append(CPPDEFINES=[  # type: ignore # noqa: F821
        ("ENV_WIFI_SSID", f'\\"{wifi_ssid}\\"'),
        ("ENV_WIFI_PASSWORD", f'\\"{wifi_password}\\"')
    ])
