# QMacVisual 插件开发指南

> 本文档介绍 QMacVisual 2D 视觉检测平台的插件体系架构，以及如何基于 CMake 构建系统创建、编译和调试新的视觉算子插件。

## 目录

- [1. 插件系统架构](#1-插件系统架构)
- [2. 插件类别与算子列表](#2-插件类别与算子列表)
- [3. 插件接口规范](#3-插件接口规范)
- [4. 创建新插件](#4-创建新插件)
- [5. 编译新插件](#5-编译新插件)
- [6. 调试插件](#6-调试插件)
- [7. 常见问题](#7-常见问题)

---

## 1. 插件系统架构

### 1.1 总体架构

QMacVisual 采用 **QPlugin** 动态库插件体系。每个视觉算子编译为独立的 `.dll` 文件，运行时由主程序 `QMacVisual.exe` 动态加载。

```
QMacVisual.exe（主程序）
    |
    +-- 加载 Plugins/*.dll
    |       +-- FindCircle.dll
    |       +-- TemplateMatch.dll
    |       +-- HDR.dll
    |       +-- ... (共 60 个算子)
    |
    +-- flowchart2d 流程图引擎
            +-- 用户通过流程图编排算子执行顺序
```

### 1.2 核心接口

插件体系基于两个核心接口：

**MainInterface** — 插件元信息接口（`maininterface.h`）：

```cpp
class MainInterface
{
public:
    virtual ~MainInterface() {}
    virtual QString name() = 0;         // 返回算子名称
    virtual QString information() = 0;  // 返回算子描述信息
};

#define MainInterface_iid "com.Interface.MainInterface"
Q_DECLARE_INTERFACE(MainInterface, MainInterface_iid)
```

**Toolnterface** — 算子执行接口（`Toolnterface.h`）：

```cpp
class Toolnterface : public QDialog
{
public:
    virtual int Execute(const QString toolname);  // 核心执行函数
    virtual int ExecuteLink(...);                  // 处理链接输入
    virtual int InitSetToolData(const QVariant data);  // 加载保存的参数
    virtual QVariant InitGetToolData();                 // 保存当前参数
    // ... 其他链接执行函数（通用 IO、PLC、串口等）
};
```

### 1.3 插件导出函数

每个插件 DLL 必须导出以下三个 C 函数：

```cpp
extern "C" Q_DECL_EXPORT QDialog* showDialog(QString toolName, QToolBase* toolBase);
extern "C" Q_DECL_EXPORT int ShowFormState();
extern "C" Q_DECL_EXPORT void SetFormState();
```

| 函数 | 说明 |
|------|------|
| `showDialog` | 创建并返回算子的参数配置对话框 |
| `ShowFormState` | 查询对话框当前状态（是否打开） |
| `SetFormState` | 设置对话框状态 |

### 1.4 共享依赖

插件共享以下依赖模块：

| 模块 | 路径 | 说明 |
|------|------|------|
| `dependence/` | 各插件内部 | 包含 `Toolnterface`、`MainInterface`、`QToolBase`、图形视图组件等 |
| `czxDependence/` | `src/czxDependence/` | 共享 2D/3D 算法库（`czxTool2D`、`czxTool` 等） |
| OpenCV | 系统安装 | 4.6.0 版本，提供图像处理基础 |
| PCL | 系统安装（可选） | 1.10.1 版本，仅 3D 检测算子需要 |

---

## 2. 插件类别与算子列表

当前共 60 个算子，分布在 11 个类别中：

| 类别 | 目录名 | 算子数量 | 算子列表 |
|------|--------|----------|----------|
| 标定工具 | `CalibrationTools` | 4 | DistortionCalibration, ERTCalibration, MeasureCalibration, SignalCircleCalibration |
| 通信工具 | `CommunicateTools` | 5 | GeneralIo, PlcCommunicate, SerialPort, SocketTcpClient, SocketTcpServer |
| 检测识别 | `DetectIdentify` | 8 | BarcodeIdentify, BlobDetector, Brightness, Z_Classifier, ColorIdentify, ImageClarity, QRcodeIdentify, SelectShape |
| 外部库 | `ExtLibraryCases` | 1 | ExtLibraryCase |
| 几何测量 | `GeometryMeasure` | 6 | Z_EdgeWidthMeasure, LineCircle, LineLineI, PointLine, PointPoint, RoundedCorners |
| 几何工具 | `GeometryTools` | 7 | FindCircle, FindLine, FindSlot, FitCircle, FitEllipse, FitLine, GetContourPoints |
| 图像处理 | `ImageProcess` | 15 | CreateRoi, CropImage, Z_ExportImage, FocusFusion_D, HDR, ImageFlip, ImageMorphology, ImageRepair, ImageRotate, A_ImageSource, ImageSplice, ImageView, PerspectiveTransform, Skeleton, photometricStero |
| 逻辑工具 | `LogicalTools` | 5 | ExtensionLibrary, LogicGoto, LogicJudge, LogicJudgeEnd, ScriptEdit |
| 参数工具 | `ParaTools` | 4 | CamShiftTrack, LinearCalculation, ShapeMatch, TemplateMatch |
| 系统工具 | `SystemTools` | 2 | DelayTool, ExportCsv |
| 3D 检测 | `ThreeDInspection` | 3 | Flatness, Flatness3D, Gap3D |

---

## 3. 插件接口规范

### 3.1 输入约定

- **图像输入**：通过 `ExecuteLink` 函数接收，以 `QString` 链接名标识上游算子的输出图像。图像数据通过全局变量表（`gVariable`）传递，类型为 `cv::Mat`。
- **参数输入**：通过 UI 界面配置，使用 `InitGetToolData()` / `InitSetToolData()` 进行序列化保存和加载。
- **特殊链接**：通信类算子（PLC、串口、TCP）有专用的 `ExecuteXxxLink()` 函数。

### 3.2 输出约定

- **图像输出**：算子将处理结果写入全局变量表，供下游算子读取。
- **数值输出**：测量类算子输出数值结果（距离、角度、坐标等），同样存入全局变量表。
- **状态输出**：`Execute()` 返回值为 `int`，表示执行状态（0 为成功，非 0 为失败）。

### 3.3 参数序列化

每个算子的参数数据类须支持 `QDataStream` 序列化：

```cpp
class MyPluginData
{
public:
    QVector<QString> img_links;
    qreal x, y, width, height;

    friend QDataStream& operator <<(QDataStream& ds, const MyPluginData& d)
    {
        ds << d.img_links << d.x << d.y << d.width << d.height;
        return ds;
    }

    friend QDataStream& operator >>(QDataStream& ds, MyPluginData& d)
    {
        ds >> d.img_links >> d.x >> d.y >> d.width >> d.height;
        return ds;
    }
};
Q_DECLARE_METATYPE(MyPluginData);
```

### 3.4 ROI 支持

多数图像处理算子支持 ROI（感兴趣区域）操作：
- 用户可在界面上绘制矩形 ROI。
- ROI 数据（x, y, width, height）随参数一起保存。
- 算子在 `Execute()` 中仅处理 ROI 内的图像区域。

---

## 4. 创建新插件

### 4.1 目录结构

以创建一个新的图像处理算子 `MyFilter` 为例，在 `QPlugins/ImageProcess/` 下创建以下目录和文件：

```
QPlugins/ImageProcess/MyFilter/
+-- CMakeLists.txt               # 构建配置（必须）
+-- MyFilter.h                   # 插件入口类（必须）
+-- myfilter_global.h            # DLL 导出宏定义（必须）
+-- maininterface.h              # 主接口定义（复制自其他插件）
+-- QToolBase.h                  # 工具基类（复制自其他插件）
+-- gvariable.h                  # 全局变量定义（复制自其他插件）
+-- Toolnterface.h/.cpp          # 工具接口基类（复制自其他插件）
+-- frmMyFilter.h/.cpp           # 算子主界面与执行逻辑
+-- frmMyFilter.ui               # Qt Designer 界面文件
+-- dependence/                  # 共享依赖（可选，按需复制）
+-- resource/                    # 资源文件（图标等）
```

### 4.2 CMakeLists.txt 配置

每个插件的 CMakeLists.txt 仅需 3-5 行：

```cmake
# MyFilter plugin
if(NOT Qt5_FOUND)
    return()
endif()

rxs_add_qmacvisual_plugin(MyFilter
    QT_MODULES  Core Gui Widgets
    LIB_DEFINE  MYFILTER_LIB
)
```

**`rxs_add_qmacvisual_plugin` 宏参数说明**：

| 参数 | 说明 | 是否必须 |
|------|------|----------|
| 第一个参数（`MyFilter`） | 插件目标名，即生成的 DLL 文件名 | 必须 |
| `QT_MODULES` | 需要的 Qt 模块，默认为 `Core Gui Widgets` | 可选 |
| `LIB_DEFINE` | 预处理宏定义，用于 DLL 导出标记 | 必须 |
| `SOURCES` | 显式指定源文件列表，省略则自动收集 `*.cpp` | 可选 |

**特殊 Qt 模块需求**：

- 通信类算子需额外添加 `Network` 和/或 `SerialPort`：
  ```cmake
  rxs_add_qmacvisual_plugin(SocketTcpServer
      QT_MODULES  Core Gui Widgets Network
      LIB_DEFINE  SOCKETTCPSERVER_LIB
  )
  ```
- 脚本编辑算子需添加 `Script`：
  ```cmake
  rxs_add_qmacvisual_plugin(ScriptEdit
      QT_MODULES  Core Gui Widgets Script
      LIB_DEFINE  SCRIPEDIT_LIB
  )
  ```

### 4.3 插件入口类

```cpp
// MyFilter.h
#pragma once
#pragma execution_character_set("utf-8")

#include "myfilter_global.h"
#include "maininterface.h"
#include "qdialog.h"
#include "QToolBase.h"

class MYFILTER_EXPORT MyFilter
    : public QObject
    , public MainInterface
{
    Q_OBJECT
    Q_INTERFACES(MainInterface)
    Q_PLUGIN_METADATA(IID MainInterface_iid)

public:
    MyFilter();
    virtual QString name() override;
    virtual QString information() override;
};

extern "C" Q_DECL_EXPORT QDialog* showDialog(QString toolName, QToolBase* toolBase);
extern "C" Q_DECL_EXPORT int ShowFormState();
extern "C" Q_DECL_EXPORT void SetFormState();
```

### 4.4 DLL 导出宏

```cpp
// myfilter_global.h
#pragma once

#include <QtCore/qglobal.h>

#if defined(MYFILTER_LIB)
#  define MYFILTER_EXPORT Q_DECL_EXPORT
#else
#  define MYFILTER_EXPORT Q_DECL_IMPORT
#endif
```

### 4.5 算子执行类

```cpp
// frmMyFilter.h
#pragma once
#include <QDialog>
#include "Toolnterface.h"

class frmMyFilter : public Toolnterface
{
    Q_OBJECT

public:
    frmMyFilter(QString toolName, QToolBase* toolBase, QWidget* parent = nullptr);
    ~frmMyFilter();

    virtual int Execute(const QString toolname) override;
    virtual int ExecuteLink(const int int_link, const QString str_link,
        const QMap<QString, gVariable::Global_Var> variable_link) override;
    virtual int InitSetToolData(const QVariant data) override;
    virtual QVariant InitGetToolData() override;

private:
    // 算子参数
    // ...
    cv::Mat dstImage;
};
```

### 4.6 注册到构建系统

将新插件添加到对应类别的 `CMakeLists.txt` 中：

```cmake
# QPlugins/ImageProcess/CMakeLists.txt
# ... 现有插件 ...
add_subdirectory(MyFilter)   # 新增
```

---

## 5. 编译新插件

### 5.1 环境准备

| 依赖 | 版本 | 说明 |
|------|------|------|
| Qt5 | 5.15.2 (msvc2019_64) | Core, Gui, Widgets, Network, SerialPort, Sql, Xml, Script |
| OpenCV | 4.6.0 | `opencv_world460` |
| PCL | 1.10.1（可选） | 仅 3D 检测算子需要 |
| CMake | >= 3.16 | |
| 编译器 | MSVC v142 (VS2019) | 需安装"使用 C++ 的桌面开发"工作负载 |

### 5.2 CMake 配置

```bash
# 在仓库根目录执行
cmake -B build -S src \
    -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64" \
    -DOpenCV_DIR="C:/opencv/build" \
    -DCMAKE_BUILD_TYPE=Release
```

### 5.3 编译

```bash
# 编译全部插件
cmake --build build --config Release --parallel

# 仅编译新插件（以 MyFilter 为例）
cmake --build build --config Release --target MyFilter --parallel
```

### 5.4 编译输出

```
build/
+-- bin/
|   +-- QMacVisual.exe          # 主程序
+-- Plugins/
    +-- MyFilter.dll             # 新插件 DLL
    +-- FindCircle.dll
    +-- TemplateMatch.dll
    +-- ...                      # 所有 60 个算子 DLL
```

### 5.5 CMake 宏自动处理的配置

`rxs_add_qmacvisual_plugin` 宏自动完成以下配置，开发者无需手动设置：

- Qt5 模块查找和链接
- AUTOMOC / AUTOUIC / AUTORCC（自动处理 Q_OBJECT、UI 文件和资源文件）
- OpenCV 查找和链接
- PCL 查找和链接（3D 检测算子根据 `LIB_DEFINE` 自动判断）
- `XXX_LIB` 预处理定义
- 输出目录设置到 `build/Plugins/`
- MSVC 编译选项：`/utf-8`、`/MP`（多核编译）、`/permissive-`（标准一致性）
- 包含路径：自动添加 `src/`、`src/czxDependence/`、`src/QMainPro/QMacVisual/include/`

---

## 6. 调试插件

### 6.1 Visual Studio 调试

1. 在 Visual Studio 中打开 `build/` 目录生成的解决方案。
2. 将 `QMacVisual` 设为启动项目。
3. 在插件源文件中设置断点。
4. 启动调试（F5），主程序启动后在流程图中添加目标算子。
5. 执行算子时断点自动命中。

### 6.2 使用 Visual Studio 附加进程

1. 先正常启动 `QMacVisual.exe`。
2. 在 Visual Studio 中选择"调试 -> 附加到进程"。
3. 选择 `QMacVisual.exe` 进程。
4. 在插件源码中设置断点。
5. 在主程序中触发算子执行。

### 6.3 日志调试

在插件代码中使用 `qDebug()` 输出调试信息：

```cpp
#include <QDebug>

int frmMyFilter::Execute(const QString toolname)
{
    qDebug() << "[MyFilter] Execute called for:" << toolname;
    // ...
    return 0;
}
```

---

## 7. 常见问题

### Q: 编译时报错 "Qt5 not found"

设置 `CMAKE_PREFIX_PATH` 指向 Qt 安装路径：
```bash
cmake -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64" ...
```

### Q: 新插件未出现在主程序算子列表中

确认以下检查项：
1. 插件 DLL 是否已编译到 `build/Plugins/` 目录。
2. `Q_PLUGIN_METADATA(IID MainInterface_iid)` 是否正确声明。
3. `showDialog` 等导出函数是否正确实现。
4. 主程序是否从正确的 `Plugins/` 目录加载。

### Q: 3D 检测算子编译失败，提示 PCL 未找到

安装 PCL 1.10.1 并确保 CMake 能找到 `PCLConfig.cmake`。如果不需要 3D 算子，可以在类别级 `CMakeLists.txt` 中注释掉 `ThreeDInspection`。

### Q: 中文乱码

确保源文件以 UTF-8 with BOM 编码保存，CMake 宏已自动添加 `/utf-8` 编译选项。

### Q: 如何参考现有插件学习

建议按以下顺序阅读代码：
1. **FindCircle**（`QPlugins/GeometryTools/FindCircle/`）— 最基础的几何检测算子，结构清晰。
2. **ImageFlip**（`QPlugins/ImageProcess/ImageFlip/`）— 简单的图像处理算子，参数少。
3. **HDR**（`QPlugins/ImageProcess/HDR/`）— 多图像输入的图像处理算子。
4. **TemplateMatch**（`QPlugins/ParaTools/TemplateMatch/`）— 较复杂的参数匹配算子。

---

## 附录：项目目录结构总览

```
src/
+-- CMakeLists.txt                       # 顶层 CMake 配置
+-- cmake/
|   +-- rxs_qmacvisual_plugin.cmake      # 共享插件宏定义
+-- QMainPro/
|   +-- QMacVisual/
|       +-- CMakeLists.txt               # 主程序构建
+-- QPlugins/
|   +-- CalibrationTools/                # 标定工具（4 算子）
|   +-- CommunicateTools/                # 通信工具（5 算子）
|   +-- DetectIdentify/                  # 检测识别（8 算子）
|   +-- ExtLibraryCases/                 # 外部库（1 算子）
|   +-- GeometryMeasure/                 # 几何测量（6 算子）
|   +-- GeometryTools/                   # 几何工具（7 算子）
|   +-- ImageProcess/                    # 图像处理（15 算子）
|   +-- LogicalTools/                    # 逻辑工具（5 算子）
|   +-- ParaTools/                       # 参数工具（4 算子）
|   +-- SystemTools/                     # 系统工具（2 算子）
|   +-- ThreeDInspection/                # 3D 检测（3 算子）
+-- czxDependence/                       # 共享 2D/3D 算法库
    +-- 2D/                              # 2D 算法（含 photometricStero）
    +-- 3D/                              # 3D 算法
    +-- czxTool2D.h/.cpp                 # 2D 工具集（含 createHDR, fuseImages）
    +-- czxTool.h/.cpp                   # 通用工具集
```
