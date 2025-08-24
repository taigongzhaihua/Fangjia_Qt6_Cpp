' Fangjia_Qt6_Cpp ��Ŀ�ṹ����ű�
' ʹ�÷���: cscript reorganize.vbs

Option Explicit

Dim fso, shell, currentDir
Set fso = CreateObject("Scripting.FileSystemObject")
Set shell = CreateObject("WScript.Shell")
currentDir = shell.CurrentDirectory

WScript.Echo "=========================================="
WScript.Echo "Fangjia_Qt6_Cpp ��Ŀ�ṹ����ű�"
WScript.Echo "��ǰĿ¼: " & currentDir
WScript.Echo "=========================================="

' �����µ�Ŀ¼�ṹ
Sub CreateDirectoryStructure()
    WScript.Echo vbCrLf & "�����µ�Ŀ¼�ṹ..."
    
    ' ����ģ��
    CreateFolder "src\core\rendering"
    CreateFolder "src\core\platform\windows"
    CreateFolder "src\core\config"
    CreateFolder "src\core\di"
    
    ' ���ģ��
    CreateFolder "src\framework\base"
    CreateFolder "src\framework\containers"
    CreateFolder "src\framework\widgets"
    
    ' ����ģ��
    CreateFolder "src\models"
    
    ' ҵ����ͼ
    CreateFolder "src\views\formula"
    
    ' Ӧ�ò�
    CreateFolder "src\app"
    
    ' ��Դ�ļ�
    CreateFolder "resources\icons"
    
    ' ����
    CreateFolder "tests\framework"
    CreateFolder "tests\models"
    CreateFolder "tests\views"
    
    ' �ĵ�
    CreateFolder "docs"
    
    WScript.Echo "Ŀ¼�ṹ������ɣ�"
End Sub

' �����ļ��У���������ڣ�
Sub CreateFolder(path)
    Dim fullPath
    fullPath = currentDir & "\" & path
    
    If Not fso.FolderExists(fullPath) Then
        CreateFolderRecursive fullPath
        WScript.Echo "  ����: " & path
    Else
        WScript.Echo "  �Ѵ���: " & path
    End If
End Sub

' �ݹ鴴���ļ���
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

' �ƶ��ļ�
Sub MoveFile(source, destination)
    Dim sourcePath, destPath, destFolder
    sourcePath = currentDir & "\" & source
    destPath = currentDir & "\" & destination
    
    If fso.FileExists(sourcePath) Then
        destFolder = fso.GetParentFolderName(destPath)
        If Not fso.FolderExists(destFolder) Then
            CreateFolderRecursive destFolder
        End If
        
        ' ���Ŀ���ļ��Ѵ��ڣ���ɾ��
        If fso.FileExists(destPath) Then
            fso.DeleteFile destPath, True
        End If
        
        fso.MoveFile sourcePath, destPath
        WScript.Echo "  �ƶ�: " & source & " -> " & destination
    Else
        WScript.Echo "  �����������ڣ�: " & source
    End If
End Sub

' �ƶ�����ļ���ʹ��ͨ�����
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

' �ƶ�������Ⱦ�ļ�
Sub MoveRenderingFiles()
    WScript.Echo vbCrLf & "�ƶ�������Ⱦ�ļ�..."
    
    MoveFile "RenderData.hpp", "src\core\rendering\RenderData.hpp"
    MoveFile "Renderer.h", "src\core\rendering\Renderer.h"
    MoveFile "Renderer.cpp", "src\core\rendering\Renderer.cpp"
    MoveFile "IconLoader.h", "src\core\rendering\IconLoader.h"
    MoveFile "IconLoader.cpp", "src\core\rendering\IconLoader.cpp"
End Sub

' �ƶ�ƽ̨�ض��ļ�
Sub MovePlatformFiles()
    WScript.Echo vbCrLf & "�ƶ�ƽ̨�ض��ļ�..."
    
    MoveFile "WinWindowChrome.h", "src\core\platform\windows\WinWindowChrome.h"
    MoveFile "WinWindowChrome.cpp", "src\core\platform\windows\WinWindowChrome.cpp"
End Sub

' �ƶ�����ļ�
Sub MoveFrameworkFiles()
    WScript.Echo vbCrLf & "�ƶ�����ļ�..."
    
    ' �����ӿ�
    MoveFile "components\base\UiComponent.hpp", "src\framework\base\UiComponent.hpp"
    MoveFile "components\base\UiContent.hpp", "src\framework\base\UiContent.hpp"
    MoveFile "components\base\UiButton.hpp", "src\framework\base\UiButton.hpp"
    
    ' ����
    MoveFile "components\containers\UiRoot.h", "src\framework\containers\UiRoot.h"
    MoveFile "components\containers\UiRoot.cpp", "src\framework\containers\UiRoot.cpp"
    MoveFile "components\containers\UiBoxLayout.h", "src\framework\containers\UiBoxLayout.h"
    MoveFile "components\containers\UiBoxLayout.cpp", "src\framework\containers\UiBoxLayout.cpp"
    MoveFile "components\containers\UiPage.h", "src\framework\containers\UiPage.h"
    MoveFile "components\containers\UiPage.cpp", "src\framework\containers\UiPage.cpp"
    
    ' �ؼ�
    MoveFile "components\widgets\UiTopBar.h", "src\framework\widgets\UiTopBar.h"
    MoveFile "components\widgets\UiTopBar.cpp", "src\framework\widgets\UiTopBar.cpp"
    MoveFile "components\widgets\UiNav.h", "src\framework\widgets\UiNav.h"
    MoveFile "components\widgets\UiNav.cpp", "src\framework\widgets\UiNav.cpp"
    MoveFile "components\widgets\UiTabView.h", "src\framework\widgets\UiTabView.h"
    MoveFile "components\widgets\UiTabView.cpp", "src\framework\widgets\UiTabView.cpp"
    MoveFile "components\widgets\UiTreeList.h", "src\framework\widgets\UiTreeList.h"
    MoveFile "components\widgets\UiTreeList.cpp", "src\framework\widgets\UiTreeList.cpp"
End Sub

' �ƶ�����ģ���ļ�
Sub MoveModelFiles()
    WScript.Echo vbCrLf & "�ƶ�����ģ���ļ�..."
    
    MoveFile "ThemeManager.h", "src\models\ThemeManager.h"
    MoveFile "ThemeManager.cpp", "src\models\ThemeManager.cpp"
    MoveFile "NavViewModel.h", "src\models\NavViewModel.h"
    MoveFile "NavViewModel.cpp", "src\models\NavViewModel.cpp"
    MoveFile "TabViewModel.h", "src\models\TabViewModel.h"
    MoveFile "TabViewModel.cpp", "src\models\TabViewModel.cpp"
    MoveFile "FormulaViewModel.h", "src\models\FormulaViewModel.h"
    MoveFile "FormulaViewModel.cpp", "src\models\FormulaViewModel.cpp"
End Sub

' �ƶ���ͼ�ļ�
Sub MoveViewFiles()
    WScript.Echo vbCrLf & "�ƶ�ҵ����ͼ�ļ�..."
    
    MoveFile "views\UiFormulaView.h", "src\views\formula\UiFormulaView.h"
    MoveFile "views\UiFormulaView.cpp", "src\views\formula\UiFormulaView.cpp"
    MoveFile "views\UiFormulaDetail.h", "src\views\formula\UiFormulaDetail.h"
    MoveFile "views\UiFormulaDetail.cpp", "src\views\formula\UiFormulaDetail.cpp"
End Sub

' �ƶ�Ӧ���ļ�
Sub MoveAppFiles()
    WScript.Echo vbCrLf & "�ƶ�Ӧ���ļ�..."
    
    MoveFile "main.cpp", "src\app\main.cpp"
    MoveFile "MainOpenGlWindow.h", "src\app\MainOpenGlWindow.h"
    MoveFile "MainOpenGlWindow.cpp", "src\app\MainOpenGlWindow.cpp"
End Sub

' �ƶ���Դ�ļ�
Sub MoveResourceFiles()
    WScript.Echo vbCrLf & "�ƶ���Դ�ļ�..."
    
    MoveFile "resources.qrc", "resources\resources.qrc"
    
    ' �ƶ�ͼ���ļ�
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

' �ƶ��ĵ��ļ�
Sub MoveDocFiles()
    WScript.Echo vbCrLf & "�ƶ��ĵ��ļ�..."
    
    If fso.FolderExists(currentDir & "\doc") Then
        MoveFile "doc\DESIGN.zh-CN.md", "docs\DESIGN.zh-CN.md"
    End If
End Sub

' �����Ŀ¼
Sub CleanupOldDirectories()
    WScript.Echo vbCrLf & "�����Ŀ¼..."
    
    DeleteFolder "components"
    DeleteFolder "views"
    DeleteFolder "doc"
    DeleteFolder "icons"
End Sub

' ɾ���ļ���
Sub DeleteFolder(path)
    Dim fullPath
    fullPath = currentDir & "\" & path
    
    If fso.FolderExists(fullPath) Then
        fso.DeleteFolder fullPath, True
        WScript.Echo "  ɾ��: " & path
    End If
End Sub

' �����µ� CMakeLists.txt
Sub CreateNewCMakeFile()
    WScript.Echo vbCrLf & "�����µ� CMakeLists.txt..."
    
    Dim cmakeContent, file
    cmakeContent = GetCMakeContent()
    
    Set file = fso.CreateTextFile(currentDir & "\CMakeLists.txt", True)
    file.Write cmakeContent
    file.Close
    
    WScript.Echo "  CMakeLists.txt �Ѹ���"
End Sub

' ��ȡ CMake ����
Function GetCMakeContent()
    Dim content
    content = "cmake_minimum_required(VERSION 3.16)" & vbCrLf & _
              "" & vbCrLf & _
              "# MSVC ������֧��" & vbCrLf & _
              "if (POLICY CMP0141)" & vbCrLf & _
              "  cmake_policy(SET CMP0141 NEW)" & vbCrLf & _
              "  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT ""$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"")" & vbCrLf & _
              "endif()" & vbCrLf & _
              "" & vbCrLf & _
              "project(Fangjia_Qt6_Cpp LANGUAGES CXX)" & vbCrLf & _
              "" & vbCrLf & _
              "# Qt ����" & vbCrLf & _
              "set(CMAKE_AUTOMOC ON)" & vbCrLf & _
              "set(CMAKE_AUTOUIC ON)" & vbCrLf & _
              "set(CMAKE_AUTORCC ON)" & vbCrLf & _
              "" & vbCrLf & _
              "# C++ ��׼" & vbCrLf & _
              "set(CMAKE_CXX_STANDARD 23)" & vbCrLf & _
              "set(CMAKE_CXX_STANDARD_REQUIRED ON)" & vbCrLf & _
              "" & vbCrLf & _
              "# ���� Qt6" & vbCrLf & _
              "find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL Widgets Svg)" & vbCrLf & _
              "" & vbCrLf & _
              "# Դ�����Ŀ¼" & vbCrLf & _
              "set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)" & vbCrLf
    
    ' ����ʡ���������� CMake ���ݣ�����Ը�����Ҫ���
    GetCMakeContent = content
End Function

' ��ִ�к���
Sub Main()
    On Error Resume Next
    
    ' ȷ��ִ��
    Dim answer
    answer = MsgBox("����������Ŀ�ṹ��" & vbCrLf & vbCrLf & _
                    "�˲�������" & vbCrLf & _
                    "1. �����µ�Ŀ¼�ṹ" & vbCrLf & _
                    "2. �ƶ�����Դ�ļ�����λ��" & vbCrLf & _
                    "3. ���� CMakeLists.txt" & vbCrLf & _
                    "4. �����Ŀ¼" & vbCrLf & vbCrLf & _
                    "�Ƿ������", vbYesNo + vbQuestion, "Fangjia ��Ŀ����")
    
    If answer <> vbYes Then
        WScript.Echo "������ȡ��"
        Exit Sub
    End If
    
    ' ִ������
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
        WScript.Echo vbCrLf & "����: " & Err.Description
        Err.Clear
    Else
        WScript.Echo vbCrLf & "=========================================="
        WScript.Echo "��Ŀ�ṹ������ɣ�"
        WScript.Echo "=========================================="
        WScript.Echo vbCrLf & "��һ����"
        WScript.Echo "1. ����ļ��Ƿ�����ȷ�ƶ�"
        WScript.Echo "2. ���� #include ·��"
        WScript.Echo "3. �������� CMake ��Ŀ"
        WScript.Echo "4. �������"
    End If
End Sub

' ִ��������
Call Main()