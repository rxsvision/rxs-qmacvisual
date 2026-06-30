# 焦距合成 SDK 文档

> 焦距合成（Focus Fusion / Depth from Focus）算子，用于将多张不同焦距拍摄的图像合成为一张全清晰的图像。

## 1. 算法概述

焦距合成（又称焦点堆叠、景深合成）通过采集多张在不同对焦距离下拍摄的图像，从每张图像中提取清晰区域，再将这些清晰区域融合为一张全画面清晰的合成图像。该算法广泛应用于工业检测中需要大景深成像的场景。

核心算法实现在共享库 `czxDependence/czxTool2D` 中，函数名为 `fuseImages`。插件入口位于 `QPlugins/ImageProcess/FocusFusion_D`。

## 2. 函数说明

| 函数名 | 类型 | 说明 |
|--------|------|------|
| `fuseImages` | 函数 | 合成多焦距图像为全清晰图像 |

## 3. SDK 接口说明

### 函数签名

```cpp
cv::Mat fuseImages(const std::vector<cv::Mat>& images);
```

### 函数描述

对多张不同焦距的输入图像进行清晰度评估和融合，生成一张全清晰的合成图像。

### 参数说明

| 参数名 | 类型 | 方向 | 说明 |
|--------|------|------|------|
| `images` | `const std::vector<cv::Mat>&` | 输入 | 多张不同焦距拍摄的图像序列 |
| **返回值** | `cv::Mat` | 输出 | 全清晰的合成图像 |

### 调用约束

- 输入图像数量须 >= 2 张。
- 所有输入图像的尺寸和类型须相同。
- 图像应在不同焦距下拍摄，保证各区域至少在某一张图像中是清晰的。

## 4. 性能特征

- **时间复杂度**：O(n)，其中 n 为图像像素总数。
- **基准性能**：合成 2 张 1200 万像素的图像，耗时约 127 ms。

## 5. 插件结构

```
QPlugins/ImageProcess/FocusFusion_D/
├── CMakeLists.txt                   # 插件构建配置
├── FocusFusion_D.h                  # 插件入口（MainInterface 实现）
├── FocusFusionParameter.h/.ui       # 参数配置面板
├── frmFocusFusion/
│   ├── frmFocusFusion.h/.cpp        # 算子主界面与执行逻辑
│   └── frmFocusFusion.ui            # 界面布局
├── dependence/                      # 共享依赖
└── focusfusion_d_global.h           # 全局导出宏定义
```

## 6. 使用示例

### 在 QMacVisual 平台中使用

1. 在流程图中添加焦距合成算子（FocusFusion_D）。
2. 点击 "+" 号创建图像输入窗口。
3. 链接多张不同焦距的图像。
4. 点击"执行"开始合成 ROI 内的图像。

### 在代码中直接调用

```cpp
#include "czxTool2D.h"

// 准备多张不同焦距的图像
std::vector<cv::Mat> images;
images.push_back(cv::imread("focus_near.png"));
images.push_back(cv::imread("focus_mid.png"));
images.push_back(cv::imread("focus_far.png"));

// 执行焦距合成
cv::Mat fusedResult = fuseImages(images);
```

## 7. 应用场景

| 场景 | 说明 |
|------|------|
| 工业外观检测 | 大景深产品的表面缺陷检测 |
| 半导体检测 | 芯片引脚和焊点的全面检查 |
| 精密制造 | 高倍率显微成像的全清晰拼接 |

## 8. 效果对比

合成结果能有效融合各焦距下的清晰区域，输出全画面清晰的结果图像。
