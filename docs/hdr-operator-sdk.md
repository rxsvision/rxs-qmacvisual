# HDR 算子 SDK 文档

> HDR（高动态范围）图像合成算子，用于合并多幅不同曝光的图像，获取细节更丰富的高动态范围图像。

## 1. 算法概述

HDR 算子通过融合多张不同曝光时间的图像，生成一张涵盖更宽亮度范围的合成图像。该算法适用于场景中存在过曝或欠曝区域、单张图像无法同时保留高光和暗部细节的检测场景。

核心算法实现在共享库 `czxDependence/czxTool2D` 中，插件入口位于 `QPlugins/ImageProcess/HDR`。

## 2. 函数说明

| 函数名 | 类型 | 说明 |
|--------|------|------|
| `createHDR` | 函数 | 合并多幅不同曝光的图像 |

## 3. SDK 接口说明

### 函数签名

```cpp
cv::Mat createHDR(const std::vector<cv::Mat>& images, std::vector<float>& times);
```

### 函数描述

合并多幅不同曝光的图像，生成一张 HDR 合成图像。

### 参数说明

| 参数名 | 类型 | 方向 | 说明 |
|--------|------|------|------|
| `images` | `const std::vector<cv::Mat>&` | 输入 | 多张不同曝光的输入图像序列 |
| `times` | `std::vector<float>&` | 输入 | 每张图像对应的曝光时间，单位：秒（s） |
| **返回值** | `cv::Mat` | 输出 | HDR 合成后的图像 |

### 调用约束

- 输入图像数量须 >= 2 张。
- `images` 和 `times` 的大小必须一致（一一对应）。
- 所有输入图像的尺寸和类型须相同。

## 4. 性能特征

- **时间复杂度**：O(n)，其中 n 为图像像素总数。
- **基准性能**：融合 5 幅 2500 万像素的图片，耗时约 2 秒。

## 5. 插件结构

HDR 算子以 Qt 插件形式集成到 QMacVisual 平台：

```
QPlugins/ImageProcess/HDR/
├── CMakeLists.txt            # 插件构建配置
├── HDR.h                     # 插件入口（MainInterface 实现）
├── HDRParameter.h/.ui        # 参数配置面板
├── frmHDR/
│   ├── frmHDR.h/.cpp         # 算子主界面与执行逻辑
│   └── frmHDR.ui             # 界面布局
└── dependence/               # 插件共享依赖（接口定义、图形视图等）
```

插件类继承 `QObject` 和 `MainInterface`，通过 `Q_PLUGIN_METADATA` 注册为 Qt 插件。

## 6. 使用示例

### 在 QMacVisual 平台中使用

1. 在流程图中添加 HDR 算子。
2. 点击 "+" 创建新的参数页。
3. 链接多张输入图像，并设置每张图像对应的曝光时间。
4. 点击"执行"按钮即可运行 HDR 合成。

### 在代码中直接调用

```cpp
#include "czxTool2D.h"

// 准备输入图像和曝光时间
std::vector<cv::Mat> images;
std::vector<float> times;

images.push_back(cv::imread("exposure_0.01s.png"));
images.push_back(cv::imread("exposure_0.1s.png"));
images.push_back(cv::imread("exposure_1.0s.png"));

times = {0.01f, 0.1f, 1.0f};

// 执行 HDR 合成
cv::Mat hdrResult = createHDR(images, times);
```

## 7. 效果对比

合成结果与 Halcon 的 HDR 合成效果相近，能够有效保留不同曝光条件下的图像细节。

| 项目 | 说明 |
|------|------|
| 输入 | 多张不同曝光的图像 |
| 输出 | HDR 合成图像 |
| 对标 | Halcon HDR 合成算子 |
