# Microsoft Developer Studio Project File - Name="gtest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=gtest - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "gtest.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "gtest.mak" CFG="gtest - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "gtest - Win32 Release" ("Win32 (x86) Static Library" 用)
!MESSAGE "gtest - Win32 Debug" ("Win32 (x86) Static Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gtest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../include" /I "../" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /TP /c
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "gtest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /I "../" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /TP /c
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\gtestd.lib"

!ENDIF 

# Begin Target

# Name "gtest - Win32 Release"
# Name "gtest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\src\gtest-death-test.cc"
# End Source File
# Begin Source File

SOURCE="..\src\gtest-filepath.cc"
# End Source File
# Begin Source File

SOURCE="..\src\gtest-port.cc"
# End Source File
# Begin Source File

SOURCE="..\src\gtest-test-part.cc"
# End Source File
# Begin Source File

SOURCE="..\src\gtest-typed-test.cc"
# End Source File
# Begin Source File

SOURCE=..\src\gtest.cc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="..\include\gtest\internal\gtest-death-test-internal.h"

!IF  "$(CFG)" == "gtest - Win32 Release"

!ELSEIF  "$(CFG)" == "gtest - Win32 Debug"

# Begin Custom Build
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\include\gtest\gtest-death-test.h"

!IF  "$(CFG)" == "gtest - Win32 Release"

!ELSEIF  "$(CFG)" == "gtest - Win32 Debug"

# Begin Custom Build
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\include\gtest\internal\gtest-filepath.h"

!IF  "$(CFG)" == "gtest - Win32 Release"

!ELSEIF  "$(CFG)" == "gtest - Win32 Debug"

# Begin Custom Build
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\src\gtest-internal-inl.h"

!IF  "$(CFG)" == "gtest - Win32 Release"

!ELSEIF  "$(CFG)" == "gtest - Win32 Debug"

# Begin Custom Build
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\include\gtest\internal\gtest-internal.h"

!IF  "$(CFG)" == "gtest - Win32 Release"

!ELSEIF  "$(CFG)" == "gtest - Win32 Debug"

# Begin Custom Build
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\include\gtest\gtest-message.h"

!IF  "$(CFG)" == "gtest - Win32 Release"

!ELSEIF  "$(CFG)" == "gtest - Win32 Debug"

# Begin Custom Build
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\include\gtest\internal\gtest-port.h"

!IF  "$(CFG)" == "gtest - Win32 Release"

!ELSEIF  "$(CFG)" == "gtest - Win32 Debug"

# Begin Custom Build
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\include\gtest\gtest-spi.h"

!IF  "$(CFG)" == "gtest - Win32 Release"

!ELSEIF  "$(CFG)" == "gtest - Win32 Debug"

# Begin Custom Build
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\include\gtest\internal\gtest-string.h"

!IF  "$(CFG)" == "gtest - Win32 Release"

!ELSEIF  "$(CFG)" == "gtest - Win32 Debug"

# Begin Custom Build
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\gtest\gtest.h

!IF  "$(CFG)" == "gtest - Win32 Release"

!ELSEIF  "$(CFG)" == "gtest - Win32 Debug"

# Begin Custom Build
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\gtest\gtest_pred_impl.h
# End Source File
# Begin Source File

SOURCE=..\include\gtest\gtest_prod.h
# End Source File
# End Group
# End Target
# End Project
