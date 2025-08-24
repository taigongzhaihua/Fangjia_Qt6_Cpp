' 文件重组脚本 - VBScript版本
Option Explicit

Dim fso, shell, currentPath
Set fso = CreateObject("Scripting.FileSystemObject")
Set shell = CreateObject("WScript.Shell")

' 获取当前路径
currentPath = fso.GetParentFolderName(WScript.ScriptFullName)
shell.CurrentDirectory = currentPath

WScript.Echo "开始重组文件结构..."
WScript.Echo "当前目录: " & currentPath

' 创建目录结构
Sub CreateFolder(path)
    Dim fullPath
    fullPath = fso.BuildPath(currentPath, path)
    If Not fso.FolderExists(fullPath) Then
        CreateParentFolder fso.GetParentFolderName(fullPath)
        fso.CreateFolder fullPath
        WScript.Echo "创建目录: " & path
    End If
End Sub

Sub CreateParentFolder(path)
    If Not fso.FolderExists(path) And path <> "" Then
        CreateParentFolder fso.GetParentFolderName(path)
        If Not fso.FolderExists(path) Then
            fso.CreateFolder path
        End If
    End If
End Sub

' 移动文件
Sub MoveFile(source, destination)
    Dim sourcePath, destPath, destFolder
    sourcePath = fso.BuildPath(currentPath, source)
    destPath = fso.BuildPath(currentPath, destination)
    destFolder = fso.GetParentFolderName(destPath)
    
    If fso.FileExists(sourcePath) Then
        ' 确保目标文件夹存在
        If Not fso.FolderExists(destFolder) Then
            CreateFolder Replace(destFolder, currentPath & "\", "")
        End If
        
        ' 如果目标文件已存在，先删除
        If fso.FileExists(destPath) Then
            fso.DeleteFile destPath, True
        End If
        
        ' 移动文件
        fso.MoveFile sourcePath, destPath
        WScript.Echo "移动: " & source & " -> " & destination
    Else
        WScript.Echo "警告: 文件不存在 - " & source
    End If
End Sub

' 主程序
On Error Resume Next

' 创建目录结构
CreateFolder "components\base"
CreateFolder "components\containers"
CreateFolder "components\widgets"
CreateFolder "views"

WScript.Echo vbCrLf & "移动基础接口文件..."
' 移动基础接口
MoveFile "UiComponent.hpp", "components\base\UiComponent.hpp"
MoveFile "UiContent.hpp", "components\base\UiContent.hpp"
MoveFile "UiButton.hpp", "components\base\UiButton.hpp"

WScript.Echo vbCrLf & "移动容器类文件..."
' 移动容器类
MoveFile "UiRoot.h", "components\containers\UiRoot.h"
MoveFile "UiRoot.cpp", "components\containers\UiRoot.cpp"
MoveFile "UiBoxLayout.h", "components\containers\UiBoxLayout.h"
MoveFile "UiBoxLayout.cpp", "components\containers\UiBoxLayout.cpp"
MoveFile "UiPage.h", "components\containers\UiPage.h"
MoveFile "UiPage.cpp", "components\containers\UiPage.cpp"

WScript.Echo vbCrLf & "移动复合控件文件..."
' 移动复合控件
MoveFile "UiTopBar.h", "components\widgets\UiTopBar.h"
MoveFile "UiTopBar.cpp", "components\widgets\UiTopBar.cpp"
MoveFile "UiNav.h", "components\widgets\UiNav.h"
MoveFile "UiNav.cpp", "components\widgets\UiNav.cpp"
MoveFile "UiTabView.h", "components\widgets\UiTabView.h"
MoveFile "UiTabView.cpp", "components\widgets\UiTabView.cpp"
MoveFile "UiTreeList.h", "components\widgets\UiTreeList.h"
MoveFile "UiTreeList.cpp", "components\widgets\UiTreeList.cpp"

WScript.Echo vbCrLf & "移动业务视图文件..."
' 移动业务视图
MoveFile "UiFormulaView.h", "views\UiFormulaView.h"
MoveFile "UiFormulaView.cpp", "views\UiFormulaView.cpp"
MoveFile "UiFormulaDetail.h", "views\UiFormulaDetail.h"
MoveFile "UiFormulaDetail.cpp", "views\UiFormulaDetail.cpp"

If Err.Number = 0 Then
    WScript.Echo vbCrLf & "==================================="
    WScript.Echo "文件重组完成！"
    WScript.Echo "==================================="
Else
    WScript.Echo vbCrLf & "重组过程中出现错误: " & Err.Description
End If

WScript.Echo vbCrLf & "按任意键退出..."
WScript.StdIn.ReadLine

' 清理
Set fso = Nothing
Set shell = Nothing