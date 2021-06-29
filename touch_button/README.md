## 测试项

请在以下不同环境下以 `1` 秒时间间隔单击 touch 按键，同时串口工具将 LOG 信息保存为文件，每个 LOG 文件保存 `15` 秒左右。

1. 工作环境下运行 LOG，关闭 `OUTPUT_FALTER_DATA` 宏定义（默认输出原始数据）
2. 工作环境下运行 LOG，开启 `OUTPUT_FALTER_DATA` 宏定义（输出滤波器数据）
3. C/S 测试环境下运行 LOG，关闭 `OUTPUT_FALTER_DATA` 宏定义（默认输出原始数据）
4. C/S 测试环境下运行 LOG，开启 `OUTPUT_FALTER_DATA` 宏定义（输出滤波器数据）