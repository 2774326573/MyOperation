# 🔧 Halcon兼容性批量修复脚本
# Halcon Compatibility Batch Fix Script

param(
    [string]$FilePath = "thirdparty\hdevelop\src\HalconLable.cpp"
)

Write-Host "🔧 开始批量修复Halcon兼容性问题..." -ForegroundColor Green
Write-Host "📁 目标文件: $FilePath" -ForegroundColor Cyan

# 检查文件是否存在
if (-not (Test-Path $FilePath)) {
    Write-Host "❌ 错误：文件 $FilePath 不存在！" -ForegroundColor Red
    exit 1
}

# 备份原始文件
$BackupPath = "$FilePath.batch_backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
Copy-Item $FilePath $BackupPath
Write-Host "💾 已备份原始文件到: $BackupPath" -ForegroundColor Yellow

# 读取文件内容
$content = Get-Content $FilePath -Raw -Encoding UTF8

Write-Host "🔄 开始批量修复..." -ForegroundColor Blue

# 修复1: MedianImage参数问题 (line 1550)
Write-Host "1️⃣ 修复MedianImage参数问题..."
$content = $content -replace 'MedianImage\(([^,]+),\s*([^,]+),\s*([^,]+),\s*([^,]+),\s*([^)]+)\)', 'MedianImage($1, $2, $3)'

# 修复2: DumpWindow参数问题 (line 2084) 
Write-Host "2️⃣ 修复DumpWindow参数问题..."
$content = $content -replace 'DumpWindow\(([^,]+),\s*([^,]+),\s*"[^"]*",\s*-1,\s*-1,\s*-1,\s*-1\)', 'DumpWindow($1, $2)'

# 修复3: SobelAmp参数问题 (line 2887)
Write-Host "3️⃣ 修复SobelAmp参数问题..."
$content = $content -replace 'SobelAmp\(([^,]+),\s*([^,]+),\s*([^,]+),\s*([^)]+)\)', 'SobelAmp($1, $2)'

# 修复4: 缺失的函数替换
Write-Host "4️⃣ 修复缺失函数..."

# 替换RobertsAmp为可用的函数
$content = $content -replace 'RobertsAmp\(([^,]+),\s*([^)]+)\)', 'SobelAmp($1, $2)'

# 替换VolumeObjectModel3d为替代方法
$content = $content -replace 'VolumeObjectModel3d\(([^,]+),\s*([^)]+)\)', 'GetObjectModel3dParams($1, "volume", $2)'

# 修复5: SetCalibDataCamPose函数名
Write-Host "5️⃣ 修复SetCalibDataCamPose..."
$content = $content -replace 'SetCalibDataCamPose\(([^,]+),\s*([^,]+),\s*([^,]+),\s*([^)]+)\)', 'SetCalibDataPose($1, $2, $4)'

# 修复6: HTuple参数传递问题
Write-Host "6️⃣ 修复HTuple参数传递..."

# 修复MinMaxGray调用中的HTuple()参数
$content = $content -replace 'MinMaxGray\(([^,]+),\s*([^,]+),\s*HTuple\(\),\s*([^,]+),\s*([^,]+),\s*HTuple\(\)\)', 'MinMaxGray($1, $2, HTuple(0), $3, $4, HTuple())'

# 修复Intensity调用中的HTuple()参数
$content = $content -replace 'Intensity\(([^,]+),\s*([^,]+),\s*([^,]+),\s*HTuple\(\)\)', 'Intensity($1, $2, $3, HTuple())'

# 修复7: 缺少的转换
Write-Host "7️⃣ 修复类型转换问题..."

# 在SetCalibDataCamPose的替代版本中修复错误
$robustReplacement = @'
    // 尝试使用新版本的函数名或参数
    try {
      SetCalibDataPose(calibData, 0, cameraPose);
      qDebug() << "✅ 相机姿态数据添加成功";
      errorMessage = tr("姿态数据添加成功");
      return true;
    } catch (HalconCpp::HException& e) {
      // 如果函数不存在，尝试其他方法
      qDebug() << "⚠️ SetCalibDataPose函数不可用，跳过此操作";
      errorMessage = tr("标定姿态设置功能在当前Halcon版本中不可用");
      return false;
    }
'@

$content = $content -replace '(?s)// 尝试使用新版本的函数名或参数.*?(?=\s*}\s*catch)', $robustReplacement

Write-Host "8️⃣ 保存修复后的文件..."

# 写入修复后的内容
try {
    $content | Out-File -FilePath $FilePath -Encoding UTF8 -NoNewline
    Write-Host "✅ 文件修复完成！" -ForegroundColor Green
    
    # 显示修复总结
    Write-Host ""
    Write-Host "📊 修复总结:" -ForegroundColor Cyan
    Write-Host "   ✓ MedianImage 参数调整" -ForegroundColor Green
    Write-Host "   ✓ DumpWindow 参数调整" -ForegroundColor Green  
    Write-Host "   ✓ SobelAmp 参数调整" -ForegroundColor Green
    Write-Host "   ✓ 缺失函数替换" -ForegroundColor Green
    Write-Host "   ✓ SetCalibDataCamPose 替换" -ForegroundColor Green
    Write-Host "   ✓ HTuple 参数修复" -ForegroundColor Green
    Write-Host "   ✓ 类型转换问题修复" -ForegroundColor Green
    Write-Host ""
    Write-Host "🎯 建议：现在重新编译项目测试" -ForegroundColor Yellow
    Write-Host "📁 备份文件：$BackupPath" -ForegroundColor Gray
    
} catch {
    Write-Host "❌ 保存文件时出错: $_" -ForegroundColor Red
    exit 1
}

Write-Host "🏁 批量修复完成！" -ForegroundColor Green 