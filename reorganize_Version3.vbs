' Fangjia_Qt6_Cpp 项目结构重组脚本
' 使用方法: cscript reorganize.vbs

Option Explicit

Dim fso, shell, currentDir
Set fso = CreateObject("Scripting.FileSystemObject")
Set shell = CreateObject("WScript.Shell")
currentDir = shell.CurrentDirectory

WScript.Echo "=========================================="
WScript.Echo "Fangjia_Qt6_Cpp 项目结构重组脚本"
WScript.Echo "当前目录: " & currentDir
WScript.Echo "=========================================="

' 创建新的目录结构
Sub CreateDirectoryStructure()
    WScript.Echo vbCrLf & "创建新的目录结构..."
    
    ' 核心模块
    CreateFolder "src\core\rendering"
    CreateFolder "src\core\platform\windows"
    CreateFolder "src\core\config"
    CreateFolder "src\core\di"
    
    ' 框架模块
    CreateFolder "src\framework\base"
    CreateFolder "src\framework\containers"
    CreateFolder "src\framework\widgets"
    
    ' 数据模型
    CreateFolder "src\models"
    
    ' 业务视图
    CreateFolder "src\views\formula"
    
    ' 应用层
    CreateFolder "src\app"
    
    ' 资源文件
    CreateFolder "resources\icons"
    
    ' 测试
    CreateFolder "tests\framework"
    CreateFolder "tests\models"
    CreateFolder "tests\views"
    
    ' 文档
    CreateFolder "docs"
    
    WScript.Echo "目录结构创建完成！"
End Sub

' 创建文件夹（如果不存在）
Sub CreateFolder(path)
    Dim fullPath
    fullPath = currentDir & "\" & path
    
    If Not fso.FolderExists(fullPath) Then
        CreateFolderRecursive fullPath
        WScript.Echo "  创建: " & path
    Else
        WScript.Echo "  已存在: " & path
    End If
End Sub

' 递归创建文件夹
Sub CreateFolderRecursive(path)
    Dim parentPath
    parentPath = fso.GetParentFolderName(path)
    
    If Not fso.FolderExists(parentPath) Then
        CreateFolderRecursive parentPath
    End If
    
    If Not fso.FolderExists(path) Then
        fso.CreateFolder path
    End If
End Sub

' 移动文件
Sub MoveFile(source, destination)
    Dim sourcePath, destPath, destFolder
    sourcePath = currentDir & "\" & source
    destPath = currentDir & "\" & destination
    
    If fso.FileExists(sourcePath) Then
        destFolder = fso.GetParentFolderName(destPath)
        If Not fso.FolderExists(destFolder) Then
            CreateFolderRecursive destFolder
        End If
        
        ' 如果目标文件已存在，先删除
        If fso.FileExists(destPath) Then
            fso.DeleteFile destPath, True
        End If
        
        fso.MoveFile sourcePath, destPath
        WScript.Echo "  移动: " & source & " -> " & destination
    Else
        WScript.Echo "  跳过（不存在）: " & source
    End If
End Sub

' 移动多个文件（使用通配符）
Sub MoveFiles(pattern, destFolder)
    Dim sourcePath, file, destPath
    sourcePath = currentDir & "\" & pattern
    
    If fso.GetFolder(fso.GetParentFolderName(sourcePath & ".")).Files.Count > 0 Then
        For Each file In fso.GetFolder(fso.GetParentFolderName(sourcePath & ".")).Files
            If fso.GetFileName(file.Path) Like fso.GetFileName(pattern) Then
                destPath = currentDir & "\" & destFolder & "\" & fso.GetFileName(file.Path)
                MoveFile Replace(file.Path, currentDir & "\", ""), destFolder & "\" & fso.GetFileName(file.Path)
            End If
        Next
    End If
End Sub

' 移动核心渲染文件
Sub MoveRenderingFiles()
    WScript.Echo vbCrLf & "移动核心渲染文件..."
    
    MoveFile "RenderData.hpp", "src\core\rendering\RenderData.hpp"
    MoveFile "Renderer.h", "src\core\rendering\Renderer.h"
    MoveFile "Renderer.cpp", "src\core\rendering\Renderer.cpp"
    MoveFile "IconLoader.h", "src\core\rendering\IconLoader.h"
    MoveFile "IconLoader.cpp", "src\core\rendering\IconLoader.cpp"
End Sub

' 移动平台特定文件
Sub MovePlatformFiles()
    WScript.Echo vbCrLf & "移动平台特定文件..."
    
    MoveFile "WinWindowChrome.h", "src\core\platform\windows\WinWindowChrome.h"
    MoveFile "WinWindowChrome.cpp", "src\core\platform\windows\WinWindowChrome.cpp"
End Sub

' 移动框架文件
Sub MoveFrameworkFiles()
    WScript.Echo vbCrLf & "移动框架文件..."
    
    ' 基础接口
    MoveFile "components\base\UiComponent.hpp", "src\framework\base\UiComponent.hpp"
    MoveFile "components\base\UiContent.hpp", "src\framework\base\UiContent.hpp"
    MoveFile "components\base\UiButton.hpp", "src\framework\base\UiButton.hpp"
    
    ' 容器
    MoveFile "components\containers\UiRoot.h", "src\framework\containers\UiRoot.h"
    MoveFile "components\containers\UiRoot.cpp", "src\framework\containers\UiRoot.cpp"
    MoveFile "components\containers\UiBoxLayout.h", "src\framework\containers\UiBoxLayout.h"
    MoveFile "components\containers\UiBoxLayout.cpp", "src\framework\containers\UiBoxLayout.cpp"
    MoveFile "components\containers\UiPage.h", "src\framework\containers\UiPage.h"
    MoveFile "components\containers\UiPage.cpp", "src\framework\containers\UiPage.cpp"
    
    ' 控件
    MoveFile "components\widgets\UiTopBar.h", "src\framework\widgets\UiTopBar.h"
    MoveFile "components\widgets\UiTopBar.cpp", "src\framework\widgets\UiTopBar.cpp"
    MoveFile "components\widgets\UiNav.h", "src\framework\widgets\UiNav.h"
    MoveFile "components\widgets\UiNav.cpp", "src\framework\widgets\UiNav.cpp"
    MoveFile "components\widgets\UiTabView.h", "src\framework\widgets\UiTabView.h"
    MoveFile "components\widgets\UiTabView.cpp", "src\framework\widgets\UiTabView.cpp"
    MoveFile "components\widgets\UiTreeList.h", "src\framework\widgets\UiTreeList.h"
    MoveFile "components\widgets\UiTreeList.cpp", "src\framework\widgets\UiTreeList.cpp"
End Sub

' 移动数据模型文件
Sub MoveModelFiles()
    WScript.Echo vbCrLf & "移动数据模型文件..."
    
    MoveFile "ThemeManager.h", "src\models\ThemeManager.h"
    MoveFile "ThemeManager.cpp", "src\models\ThemeManager.cpp"
    MoveFile "NavViewModel.h", "src\models\NavViewModel.h"
    MoveFile "NavViewModel.cpp", "src\models\NavViewModel.cpp"
    MoveFile "TabViewModel.h", "src\models\TabViewModel.h"
    MoveFile "TabViewModel.cpp", "src\models\TabViewModel.cpp"
    MoveFile "FormulaViewModel.h", "src\models\FormulaViewModel.h"
    MoveFile "FormulaViewModel.cpp", "src\models\FormulaViewModel.cpp"
End Sub

' 移动视图文件
Sub MoveViewFiles()
    WScript.Echo vbCrLf & "移动业务视图文件..."
    
    MoveFile "views\UiFormulaView.h", "src\views\formula\UiFormulaView.h"
    MoveFile "views\UiFormulaView.cpp", "src\views\formula\UiFormulaView.cpp"
    MoveFile "views\UiFormulaDetail.h", "src\views\formula\UiFormulaDetail.h"
    MoveFile "views\UiFormulaDetail.cpp", "src\views\formula\UiFormulaDetail.cpp"
End Sub

' 移动应用文件
Sub MoveAppFiles()
    WScript.Echo vbCrLf & "移动应用文件..."
    
    MoveFile "main.cpp", "src\app\main.cpp"
    MoveFile "MainOpenGlWindow.h", "src\app\MainOpenGlWindow.h"
    MoveFile "MainOpenGlWindow.cpp", "src\app\MainOpenGlWindow.cpp"
End Sub

' 移动资源文件
Sub MoveResourceFiles()
    WScript.Echo vbCrLf & "移动资源文件..."
    
    MoveFile "resources.qrc", "resources\resources.qrc"
    
    ' 移动图标文件
    Dim iconFiles, iconFile
    iconFiles = Array( _
        "sun.svg", "moon.svg", _
        "follow_on.svg", "follow_off.svg", _
        "home_light.svg", "home_dark.svg", _
        "data_light.svg", "data_dark.svg", _
        "explore_light.svg", "explore_dark.svg", _
        "fav_light.svg", "fav_dark.svg", _
        "settings_light.svg", "settings_dark.svg", _
        "sys_min.svg", "sys_max.svg", "sys_close.svg", _
        "nav_toggle_expand.svg", "nav_toggle_collapse.svg" _
    )
    
    For Each iconFile In iconFiles
        MoveFile "icons\" & iconFile, "resources\icons\" & iconFile
    Next
End Sub

' 移动文档文件
Sub MoveDocFiles()
    WScript.Echo vbCrLf & "移动文档文件..."
    
    If fso.FolderExists(currentDir & "\doc") Then
        MoveFile "doc\DESIGN.zh-CN.md", "docs\DESIGN.zh-CN.md"
    End If
End Sub

' 清理旧目录
Sub CleanupOldDirectories()
    WScript.Echo vbCrLf & "清理旧目录..."
    
    DeleteFolder "components"
    DeleteFolder "views"
    DeleteFolder "doc"
    DeleteFolder "icons"
End Sub

' 删除文件夹
Sub DeleteFolder(path)
    Dim fullPath
    fullPath = currentDir & "\" & path
    
    If fso.FolderExists(fullPath) Then
        fso.DeleteFolder fullPath, True
        WScript.Echo "  删除: " & path
    End If
End Sub

' 创建新的 CMakeLists.txt
Sub CreateNewCMakeFile()
    WScript.Echo vbCrLf & "创建新的 CMakeLists.txt..."
    
    Dim cmakeContent, file
    cmakeContent = GetCMakeContent()
    
    Set file = fso.CreateTextFile(currentDir & "\CMakeLists.txt", True)
    file.Write cmakeContent
    file.Close
    
    WScript.Echo "  CMakeLists.txt 已更新"
End Sub

' 获取 CMake 内容
Function GetCMakeContent()
    Dim content
    content = "cmake_minimum_required(VERSION 3.16)" & vbCrLf & _
              "" & vbCrLf & _
              "# MSVC 热重载支持" & vbCrLf & _
              "if (POLICY CMP0141)" & vbCrLf & _
              "  cmake_policy(SET CMP0141 NEW)" & vbCrLf & _
              "  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT ""$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"")" & vbCrLf & _
              "endif()" & vbCrLf & _
              "" & vbCrLf & _
              "project(Fangjia_Qt6_Cpp LANGUAGES CXX)" & vbCrLf & _
              "" & vbCrLf & _
              "# Qt 设置" & vbCrLf & _
              "set(CMAKE_AUTOMOC ON)" & vbCrLf & _
              "set(CMAKE_AUTOUIC ON)" & vbCrLf & _
              "set(CMAKE_AUTORCC ON)" & vbCrLf & _
              "" & vbCrLf & _
              "# C++ 标准" & vbCrLf & _
              "set(CMAKE_CXX_STANDARD 23)" & vbCrLf & _
              "set(CMAKE_CXX_STANDARD_REQUIRED ON)" & vbCrLf & _
              "" & vbCrLf & _
              "# 查找 Qt6" & vbCrLf & _
              "find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL Widgets Svg)" & vbCrLf & _
              "" & vbCrLf & _
              "# 源代码根目录" & vbCrLf & _
              "set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)" & vbCrLf
    
    ' 这里省略了完整的 CMake 内容，你可以根据需要添加
    GetCMakeContent = content
End Function

' 主执行函数
Sub Main()
    On Error Resume Next
    
    ' 确认执行
    Dim answer
    answer = MsgBox("即将重组项目结构。" & vbCrLf & vbCrLf & _
                    "此操作将：" & vbCrLf & _
                    "1. 创建新的目录结构" & vbCrLf & _
                    "2. 移动所有源文件到新位置" & vbCrLf & _
                    "3. 更新 CMakeLists.txt" & vbCrLf & _
                    "4. 清理旧目录" & vbCrLf & vbCrLf & _
                    "是否继续？", vbYesNo + vbQuestion, "Fangjia 项目重组")
    
    If answer <> vbYes Then
        WScript.Echo "操作已取消"
        Exit Sub
    End If
    
    ' 执行重组
    CreateDirectoryStructure
    MoveRenderingFiles
    MovePlatformFiles
    MoveFrameworkFiles
    MoveModelFiles
    MoveViewFiles
    MoveAppFiles
    MoveResourceFiles
    MoveDocFiles
    CreateNewCMakeFile
    CleanupOldDirectories
    
    If Err.Number <> 0 Then
        WScript.Echo vbCrLf & "错误: " & Err.Description
        Err.Clear
    Else
        WScript.Echo vbCrLf & "=========================================="
        WScript.Echo "项目结构重组完成！"
        WScript.Echo "=========================================="
        WScript.Echo vbCrLf & "下一步："
        WScript.Echo "1. 检查文件是否都已正确移动"
        WScript.Echo "2. 更新 #include 路径"
        WScript.Echo "3. 重新配置 CMake 项目"
        WScript.Echo "4. 编译测试"
    End If
End Sub

' 执行主函数
Call Main()