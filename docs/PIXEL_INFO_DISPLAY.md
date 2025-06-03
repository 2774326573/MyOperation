# 🎯 实时像素信息显示功能 | Real-time Pixel Information Display

## 📖 功能概述 | Feature Overview

实时像素信息显示功能允许用户在Halcon图像窗口中实时查看鼠标位置的像素值信息，支持灰度图像和彩色图像的不同显示格式。

Real-time pixel information display feature allows users to view pixel value information at mouse position in Halcon image window in real-time, supporting different display formats for grayscale and color images.

## ✨ 主要特性 | Key Features

### 🎨 多图像格式支持 | Multi-format Support
- **灰度图像**：显示灰度值和亮度等级描述
- **彩色图像(RGB)**：显示RGB三原色值、亮度和主色调
- **多通道图像**：显示通道数信息

### 📍 实时坐标显示 | Real-time Coordinate Display
- 显示鼠标在图像中的精确坐标位置
- 自动检测坐标是否在图像范围内
- 超出范围时显示图像尺寸信息

### 🎯 智能界面设计 | Smart UI Design
- 半透明绿色主题，符合机器视觉风格
- 左下角固定位置显示，不遮挡重要内容
- 鼠标离开窗口自动隐藏
- 窗口大小变化时自动调整位置

## 🚀 使用方法 | How to Use

### 方法一：通过快捷操作面板
1. 在视觉处理界面中，点击工具栏中的 **"⚡ 快捷操作"** 按钮
2. 在弹出的快捷操作对话框中找到 **"🎯 像素信息显示"** 按钮
3. 点击该按钮可开启/关闭像素信息显示功能
   - 🟢 **开启状态**：按钮显示绿色，文本为 "🎯 像素信息显示 (开启)"
   - ⚪ **关闭状态**：按钮显示灰色，文本为 "🎯 像素信息显示 (关闭)"

### 方法二：程序接口控制
```cpp
// 开启像素信息显示
halWin->setPixelInfoDisplayEnabled(true);

// 关闭像素信息显示  
halWin->setPixelInfoDisplayEnabled(false);

// 查询当前状态
bool isEnabled = halWin->isPixelInfoDisplayEnabled();
```

## 📊 显示信息说明 | Display Information

### 🔘 灰度图像显示格式
```
📍 位置(X, Y)
🔘 灰度值: 128
🌓 中亮区域
```

### 🌈 彩色图像显示格式
```
📍 位置(X, Y)
🔴 红色(R): 255
🟢 绿色(G): 128
🔵 蓝色(B): 64
🎨 RGB: (255,128,64)
💡 亮度: 180
🔴 偏红色调
```

### ⚠️ 超出范围显示格式
```
📍 位置(X, Y)
⚠️ 超出图像范围
📏 图像尺寸: 1024×768
```

## 🎨 亮度等级说明 | Brightness Level Description

| 灰度值范围 | 等级描述 | 图标 |
|------------|----------|------|
| 0-63       | 暗色区域 | 🌑   |
| 64-127     | 中暗区域 | 🌒   |
| 128-191    | 中亮区域 | 🌓   |
| 192-255    | 明亮区域 | 🌕   |

## 🎯 色调判断说明 | Color Tone Analysis

- **🔴 偏红色调**：红色分量最大
- **🟢 偏绿色调**：绿色分量最大  
- **🔵 偏蓝色调**：蓝色分量最大
- **⚪ 中性色调**：RGB分量相近

## ⚙️ 技术特性 | Technical Features

### 🔧 性能优化
- 避免重复更新相同像素信息
- 异常安全处理，不影响主程序运行
- 智能边界检测和位置调整

### 🎨 样式特性
- CSS样式化的现代界面设计
- 等宽字体确保信息对齐
- 圆角边框和阴影效果

### 📐 坐标转换
- 自动将窗口坐标转换为图像坐标
- 考虑图像缩放和平移状态
- 精确的亚像素级定位

## 🐛 故障排除 | Troubleshooting

### 问题：像素信息不显示
**解决方法**：
1. 检查功能是否已开启
2. 确认图像已正确加载
3. 鼠标是否在图像窗口内

### 问题：显示位置不正确
**解决方法**：
1. 调整窗口大小让系统重新计算位置
2. 重新开启功能
3. 检查图像显示区域设置

### 问题：RGB值显示异常
**解决方法**：
1. 确认图像格式为3通道彩色图像
2. 检查图像是否正确加载
3. 验证图像数据完整性

## 💡 使用建议 | Usage Tips

1. **适用场景**：图像分析、颜色检测、像素级检查
2. **性能考虑**：大图像时建议适当缩放后使用
3. **精度要求**：需要精确像素值时建议放大图像
4. **视觉舒适**：长时间使用时可临时关闭功能

## 🔄 版本更新 | Version Updates

- **v1.0** (2024-12): 初始版本，支持基本像素信息显示
- 支持灰度和彩色图像
- 实时坐标转换
- 智能UI布局

---

## 🤝 技术支持 | Technical Support

如果您在使用过程中遇到任何问题，请参考以上故障排除指南，或联系技术支持团队。

For any issues during usage, please refer to the troubleshooting guide above or contact the technical support team. 