# 光度立体法 SDK 文档

> 光度立体法（Photometric Stereo）算子，通过多张不同光照方向的图像重建表面法线场和高度场，用于表面缺陷检测和三维形貌重建。

## 1. 算法概述

光度立体法是一种经典的三维重建方法，通过在不同光源方向下拍摄同一场景的多张图像，利用 Lambertian 反射模型求解表面法向量，进而积分获取表面高度信息。

核心算法实现在 `czxDependence/2D/photometricStero` 中，类名为 `czx::PhotometricSteroCZX`。插件入口位于 `QPlugins/ImageProcess/photometricStero`。

## 2. 函数与类说明

| 名称 | 类型 | 说明 |
|------|------|------|
| `PhotometricSteroCZX` | 类 | 封装了光度立体法各个流程（法线计算、高度重建、反照率计算等） |
| `photometricStero` | 函数 | 使用光度立体法获取合成图像（插件层调用入口） |

## 3. SDK 接口说明

### PhotometricSteroCZX 类

#### 构造函数

```cpp
// 方式一：通过图像路径 + 光源方向向量构造
PhotometricSteroCZX(
    const std::vector<std::string>& img_paths,
    const std::vector<cv::Vec3f>& lights
);

// 方式二：通过图像路径 + 倾斜角/俯仰角构造
PhotometricSteroCZX(
    const std::vector<std::string>& img_paths,
    const std::vector<float>& slants,
    const std::vector<float>& tilts
);

// 方式三：通过图像矩阵 + 倾斜角/俯仰角构造
PhotometricSteroCZX(
    const std::vector<cv::Mat>& imgs,
    const std::vector<float>& slants,
    const std::vector<float>& tilts
);
```

#### 参数说明

| 参数名 | 类型 | 说明 |
|--------|------|------|
| `img_paths` | `vector<string>` | 输入图像文件路径列表 |
| `imgs` | `vector<cv::Mat>` | 输入图像矩阵列表 |
| `lights` | `vector<cv::Vec3f>` | 光源方向向量列表（三维向量） |
| `slants` | `vector<float>` | 光源倾斜角列表（slant angle） |
| `tilts` | `vector<float>` | 光源俯仰角列表（tilt angle） |

#### 成员函数

| 函数名 | 返回类型 | 说明 |
|--------|----------|------|
| `getNormal()` | `cv::Mat` | 获取表面法向量场（三通道浮点矩阵） |
| `getNormalGray()` | `cv::Mat` | 获取法向量场的灰度表示 |
| `getMean()` | `cv::Mat` | 获取多张输入图像的平均图像 |
| `getAlbedo()` | `cv::Mat` | 获取表面反照率（albedo）图 |
| `globalHeights()` | `cv::Mat` | 通过全局积分获取表面高度场 |
| `globalHeightsMLS()` | `VectorXf` | 通过移动最小二乘法（MLS）获取高度场 |
| `getCloud(float scale)` | `CP` | 获取点云数据，`scale` 为缩放系数 |

### 插件层调用示例

```cpp
// 使用倾斜角和俯仰角构造
std::vector<cv::Mat> imgs = { img1, img2, img3, img4 };
std::vector<float> slants = { 45.0f, 135.0f, 225.0f, 315.0f };
std::vector<float> tilts  = { 30.0f, 30.0f, 30.0f, 30.0f };

czx::PhotometricSteroCZX ps(imgs, slants, tilts);

cv::Mat normalMap = ps.getNormalGray();   // 法线图
cv::Mat albedo    = ps.getAlbedo();        // 反照率图
cv::Mat heights   = ps.globalHeights();    // 高度场
```

## 4. 性能特征

性能瓶颈在计算法线场，合成图片需要 O(mn) 复杂度，其中 m、n 为图像的宽和高。理论上还与图片数目有关，但数目太小时可忽略。

**测试条件**：处理 4 张 4096 x 3000 大小的图片。

| 方式 | 耗时 |
|------|------|
| 单线程 CPU | ~8538 ms |
| 多线程 CPU | ~1514 ms |
| GPU（32x32 块尺寸） | ~273 ms |

## 5. 插件结构

```
QPlugins/ImageProcess/photometricStero/
├── CMakeLists.txt                    # 插件构建配置
├── frmPhotometricStero.h/.cpp        # 算子主界面与执行逻辑
├── frmPhotometricStero.ui            # 界面布局
├── ImageParameter.h/.ui              # 图像参数配置面板
├── photometricSteroUI.h/.cpp         # 插件 UI 封装
├── photometricstero_global.h         # 全局导出宏定义
└── dependence/                       # 共享依赖
```

## 6. 使用示例

### 在 QMacVisual 平台中使用

1. 在流程图中添加光度立体法算子。
2. 新增输入图像，点击加号创建新窗口。
3. 点击"链接图像"，在下方输入框中输入每张图像对应的倾斜角（slant）和俯仰角（tilt）。
4. 设置输出图像类型（光照图和/或法线图）。
5. 点击执行。

### 输出类型

| 输出类型 | 说明 |
|----------|------|
| 光照图（Albedo） | 表面反照率图像，消除光照影响后的表面纹理 |
| 法线图（Normal Map） | 表面法向量编码为 RGB 的图像，用于可视化表面朝向 |

## 7. 效果对比

合成结果与 Halcon 和 2.5D Master 的光度立体法效果对标，能够准确重建表面法线和高度信息。
