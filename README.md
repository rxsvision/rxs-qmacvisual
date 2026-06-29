# RXS QMacVisual

> 2D 视觉检测平台 — 基于 Qt5 + OpenCV

## 来源

- D:\2_4\QMacVisual — 2D 视觉检测平台
- 含 flowchart2d 流程图引擎
- 含 60+ 视觉算子 (FindCircle, TemplateMatch, ShapeMatch, BlobDetector 等)

## 构建系统 (CMake)

### 依赖

| 依赖 | 版本 | 说明 |
|------|------|------|
| Qt5 | 5.15.2 (msvc2019_64) | Core, Gui, Widgets, Network, SerialPort, Sql, Xml, Script |
| OpenCV | 4.6.0 | opencv_world460 |
| PCL | 1.10.1 (可选) | 仅 3D 检测算子需要 (Flatness, Flatness3D, Gap3D) |
| CMake | >= 3.16 | |
| 编译器 | MSVC v142 (VS2019) | /utf-8 /MP /permissive- |

### 构建方法

```bash
# 配置
cmake -B build -S src \
    -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64" \
    -DOpenCV_DIR="C:/opencv/build" \
    -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build build --config Release --parallel

# 输出
#   build/bin/QMacVisual.exe      — 主程序
#   build/Plugins/*.dll           — 60个视觉算子插件
```

### 项目结构

```
src/
├── CMakeLists.txt                    # 顶层 CMake (Qt5 查找, 插件宏)
├── cmake/
│   └── rxs_qmacvisual_plugin.cmake   # 共享插件宏 (rxs_add_qmacvisual_plugin)
├── QMainPro/
│   ├── CMakeLists.txt                # 桥接
│   └── QMacVisual/
│       └── CMakeLists.txt            # 主程序 (Application)
├── QPlugins/
│   ├── CalibrationTools/             # 标定工具 (4 算子)
│   ├── CommunicateTools/             # 通信工具 (5 算子)
│   ├── DetectIdentify/               # 检测识别 (8 算子)
│   ├── ExtLibraryCases/              # 外部库 (1 算子)
│   ├── GeometryMeasure/              # 几何测量 (6 算子)
│   ├── GeometryTools/                # 几何工具 (7 算子)
│   ├── ImageProcess/                 # 图像处理 (15 算子)
│   ├── LogicalTools/                 # 逻辑工具 (5 算子)
│   ├── ParaTools/                    # 参数工具 (4 算子)
│   ├── SystemTools/                  # 系统工具 (2 算子)
│   └── ThreeDInspection/             # 3D 检测 (3 算子)
└── czxDependence/                    # 共享 2D/3D 算法库
```

### 插件宏用法

每个插件 CMakeLists.txt 仅需 3-5 行：

```cmake
rxs_add_qmacvisual_plugin(FitCircle
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  FITCIRCLE_LIB
)
```

宏自动处理：
- Qt5 模块链接 (Core, Gui, Widgets, Network, SerialPort, Script)
- AUTOMOC / AUTOUIC / AUTORCC
- OpenCV 链接
- PCL 链接 (3D 检测算子自动检测)
- XXX_LIB 预处理定义
- 插件输出到 `build/Plugins/` 目录
- MSVC /utf-8 /MP 编译选项

### 算子列表 (60 个)

| 类别 | 算子 | Qt 模块 |
|------|------|---------|
| 标定工具 | DistortionCalibration, ERTCalibration, MeasureCalibration, SignalCircleCalibration | Core Gui Widgets |
| 通信工具 | GeneralIo, PlcCommunicate, SerialPort, SocketTcpClient, SocketTcpServer | +Network/SerialPort |
| 检测识别 | BarcodeIdentify, BlobDetector, Brightness, Z_Classifier, ColorIdentify, ImageClarity, QRcodeIdentify, SelectShape | Core Gui Widgets |
| 外部库 | ExtLibraryCase | Core Gui Widgets |
| 几何测量 | Z_EdgeWidthMeasure, LineCircle, LineLineI, PointLine, PointPoint, RoundedCorners | Core Gui Widgets |
| 几何工具 | FindCircle, FindLine, FindSlot, FitCircle, FitEllipse, FitLine, GetContourPoints | Core Gui Widgets |
| 图像处理 | CreateRoi, CropImage, Z_ExportImage, FocusFusion_D, HDR, ImageFlip, ImageMorphology, ImageRepair, ImageRotate, A_ImageSource, ImageSplice, ImageView, PerspectiveTransform, Skeleton, photometricStero | Core Gui Widgets |
| 逻辑工具 | ExtensionLibrary, LogicGoto, LogicJudge, LogicJudgeEnd, ScriptEdit | +Script |
| 参数工具 | CamShiftTrack, LinearCalculation, ShapeMatch, TemplateMatch | Core Gui Widgets |
| 系统工具 | DelayTool, ExportCsv | Core Gui Widgets |
| 3D 检测 | Flatness, Flatness3D, Gap3D | Core Gui Widgets (+PCL) |

## 功能模块

- 图像处理 (Brightness, ImageRotate, ImageRepair, ImageView 等)
- 几何检测 (FindCircle, FindLine, FitLine, FitEllipse, FitCircle 等)
- 模板匹配 (TemplateMatch, ShapeMatch)
- 条码识别 (BarcodeIdentify, QRcodeIdentify)
- 逻辑控制 (LogicJudge, LogicGoto, DelayTool)
- 通信 (SerialPort, SocketTcpServer, SocketTcpClient, PlcCommunicate)
- 3D 检测 (Flatness, Flatness3D, Gap3D) — 需 PCL

## 许可证

BSL 1.1 (Change Date: 2030-01-01, Change License: GPLv2)

## 公司

苏州锐新视科技有限公司
