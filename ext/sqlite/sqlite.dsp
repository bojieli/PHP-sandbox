# Microsoft Developer Studio Project File - Name="sqlite" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sqlite - Win32 Debug_TS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sqlite.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sqlite.mak" CFG="sqlite - Win32 Debug_TS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sqlite - Win32 Release_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sqlite - Win32 Debug_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sqlite - Win32 Release_TS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_TS"
# PROP BASE Intermediate_Dir "Release_TS"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_TS"
# PROP Intermediate_Dir "Release_TS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NETOOLS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\php4" /I "..\..\..\php4\main" /I "..\..\..\php4\Zend" /I "..\..\..\php4\TSRM" /I "..\..\..\php4\win32" /I "..\..\..\php_build" /I "..\..\..\php_build\include\lcrzo" /D ZEND_DEBUG=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "COMPILE_DL_NETOOLS" /D ZTS=1 /D "ZEND_WIN32" /D "PHP_WIN32" /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 php4ts.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"..\..\Release_TS\php_sqlite.dll" /libpath:"..\..\..\php4\Release_TS" /libpath:"..\..\..\php4\Release_TS_Inline" /libpath:"..\..\..\php_build\release" /libpath:"..\..\..\php_build\lib\lcrzo"

!ELSEIF  "$(CFG)" == "sqlite - Win32 Debug_TS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_TS"
# PROP BASE Intermediate_Dir "Debug_TS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_TS"
# PROP Intermediate_Dir "Debug_TS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NETOOLS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\php4" /I "..\..\..\php4\main" /I "..\..\..\php4\Zend" /I "..\..\..\php4\TSRM" /I "..\..\..\php4\win32" /I "..\..\..\php_build" /I "..\..\..\php_build\include\lcrzo" /D ZEND_DEBUG=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "COMPILE_DL_NETOOLS" /D ZTS=1 /D "ZEND_WIN32" /D "PHP_WIN32" /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 php4ts_debug.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\..\Debug_TS\php_sqlite.dll" /pdbtype:sept /libpath:"..\..\..\php4\Debug_TS" /libpath:"..\..\..\php_build\release" /libpath:"..\..\..\php_build\lib\lcrzo"

!ENDIF 

# Begin Target

# Name "sqlite - Win32 Release_TS"
# Name "sqlite - Win32 Debug_TS"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "libsqlite"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\libsqlite\src\auth.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\btree.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\build.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\delete.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\encode.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\expr.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\func.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\hash.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\insert.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\main.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\opcodes.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\os.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\pager.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\parse.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\printf.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\random.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\select.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\sqlite.w32.h

!IF  "$(CFG)" == "sqlite - Win32 Release_TS"

# Begin Custom Build
InputDir=.\libsqlite\src
InputPath=.\libsqlite\src\sqlite.w32.h

"$(InputDir)\sqlite.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(InputDir)\sqlite.h

# End Custom Build

!ELSEIF  "$(CFG)" == "sqlite - Win32 Debug_TS"

# Begin Custom Build
InputDir=.\libsqlite\src
InputPath=.\libsqlite\src\sqlite.w32.h

"$(InputDir)\sqlite.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(InputDir)\sqlite.h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\sqlite_config.w32.h

!IF  "$(CFG)" == "sqlite - Win32 Release_TS"

# Begin Custom Build
InputDir=.\libsqlite\src
InputPath=.\libsqlite\src\sqlite_config.w32.h

"$(InputDir)\config.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(InputDir)\config.h

# End Custom Build

!ELSEIF  "$(CFG)" == "sqlite - Win32 Debug_TS"

# Begin Custom Build
InputDir=.\libsqlite\src
InputPath=.\libsqlite\src\sqlite_config.w32.h

"$(InputDir)\config.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(InputDir)\config.h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\table.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\tokenize.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\trigger.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\update.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\util.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\vdbe.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# Begin Source File

SOURCE=.\libsqlite\src\where.c
# ADD CPP /D HAVE_SQLITE=1 /D "PHP_SQLITE_EXPORTS"
# SUBTRACT CPP /D HAVE_NETOOLS=1 /D "PHP_NETOOLS_EXPORTS"
# End Source File
# End Group
# Begin Source File

SOURCE=.\sqlite.c
# ADD CPP /I "libsqlite\src"
# SUBTRACT CPP /I "..\..\..\php_build\include\lcrzo"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\php_sqlite.h
# End Source File
# End Group
# End Target
# End Project
