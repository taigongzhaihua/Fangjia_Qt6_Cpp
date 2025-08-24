' �ļ�����ű� - VBScript�汾
Option Explicit

Dim fso, shell, currentPath
Set fso = CreateObject("Scripting.FileSystemObject")
Set shell = CreateObject("WScript.Shell")

' ��ȡ��ǰ·��
currentPath = fso.GetParentFolderName(WScript.ScriptFullName)
shell.CurrentDirectory = currentPath

WScript.Echo "��ʼ�����ļ��ṹ..."
WScript.Echo "��ǰĿ¼: " & currentPath

' ����Ŀ¼�ṹ
Sub CreateFolder(path)
    Dim fullPath
    fullPath = fso.BuildPath(currentPath, path)
    If Not fso.FolderExists(fullPath) Then
        CreateParentFolder fso.GetParentFolderName(fullPath)
        fso.CreateFolder fullPath
        WScript.Echo "����Ŀ¼: " & path
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

' �ƶ��ļ�
Sub MoveFile(source, destination)
    Dim sourcePath, destPath, destFolder
    sourcePath = fso.BuildPath(currentPath, source)
    destPath = fso.BuildPath(currentPath, destination)
    destFolder = fso.GetParentFolderName(destPath)
    
    If fso.FileExists(sourcePath) Then
        ' ȷ��Ŀ���ļ��д���
        If Not fso.FolderExists(destFolder) Then
            CreateFolder Replace(destFolder, currentPath & "\", "")
        End If
        
        ' ���Ŀ���ļ��Ѵ��ڣ���ɾ��
        If fso.FileExists(destPath) Then
            fso.DeleteFile destPath, True
        End If
        
        ' �ƶ��ļ�
        fso.MoveFile sourcePath, destPath
        WScript.Echo "�ƶ�: " & source & " -> " & destination
    Else
        WScript.Echo "����: �ļ������� - " & source
    End If
End Sub

' ������
On Error Resume Next

' ����Ŀ¼�ṹ
CreateFolder "components\base"
CreateFolder "components\containers"
CreateFolder "components\widgets"
CreateFolder "views"

WScript.Echo vbCrLf & "�ƶ������ӿ��ļ�..."
' �ƶ������ӿ�
MoveFile "UiComponent.hpp", "components\base\UiComponent.hpp"
MoveFile "UiContent.hpp", "components\base\UiContent.hpp"
MoveFile "UiButton.hpp", "components\base\UiButton.hpp"

WScript.Echo vbCrLf & "�ƶ��������ļ�..."
' �ƶ�������
MoveFile "UiRoot.h", "components\containers\UiRoot.h"
MoveFile "UiRoot.cpp", "components\containers\UiRoot.cpp"
MoveFile "UiBoxLayout.h", "components\containers\UiBoxLayout.h"
MoveFile "UiBoxLayout.cpp", "components\containers\UiBoxLayout.cpp"
MoveFile "UiPage.h", "components\containers\UiPage.h"
MoveFile "UiPage.cpp", "components\containers\UiPage.cpp"

WScript.Echo vbCrLf & "�ƶ����Ͽؼ��ļ�..."
' �ƶ����Ͽؼ�
MoveFile "UiTopBar.h", "components\widgets\UiTopBar.h"
MoveFile "UiTopBar.cpp", "components\widgets\UiTopBar.cpp"
MoveFile "UiNav.h", "components\widgets\UiNav.h"
MoveFile "UiNav.cpp", "components\widgets\UiNav.cpp"
MoveFile "UiTabView.h", "components\widgets\UiTabView.h"
MoveFile "UiTabView.cpp", "components\widgets\UiTabView.cpp"
MoveFile "UiTreeList.h", "components\widgets\UiTreeList.h"
MoveFile "UiTreeList.cpp", "components\widgets\UiTreeList.cpp"

WScript.Echo vbCrLf & "�ƶ�ҵ����ͼ�ļ�..."
' �ƶ�ҵ����ͼ
MoveFile "UiFormulaView.h", "views\UiFormulaView.h"
MoveFile "UiFormulaView.cpp", "views\UiFormulaView.cpp"
MoveFile "UiFormulaDetail.h", "views\UiFormulaDetail.h"
MoveFile "UiFormulaDetail.cpp", "views\UiFormulaDetail.cpp"

If Err.Number = 0 Then
    WScript.Echo vbCrLf & "==================================="
    WScript.Echo "�ļ�������ɣ�"
    WScript.Echo "==================================="
Else
    WScript.Echo vbCrLf & "��������г��ִ���: " & Err.Description
End If

WScript.Echo vbCrLf & "��������˳�..."
WScript.StdIn.ReadLine

' ����
Set fso = Nothing
Set shell = Nothing