; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CVsdwinDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "vsdwin.h"
LastPage=0

ClassCount=3
Class1=CVsdwinApp
Class2=CAboutDlg
Class3=CVsdwinDlg

ResourceCount=4
Resource1=IDR_MENU1
Resource2=IDD_VSDWIN_DIALOG
Resource3=IDD_ABOUTBOX
Resource4=IDR_ACCELERATOR1

[CLS:CVsdwinApp]
Type=0
BaseClass=CWinApp
HeaderFile=vsdwin.h
ImplementationFile=vsdwin.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=vsdwinDlg.cpp
ImplementationFile=vsdwinDlg.cpp
LastObject=CAboutDlg

[CLS:CVsdwinDlg]
Type=0
BaseClass=CDialog
HeaderFile=vsdwinDlg.h
ImplementationFile=vsdwinDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=ID_MENU_OPEN_FIRM

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_VSDWIN_DIALOG]
Type=1
Class=CVsdwinDlg
ControlCount=19
Control1=IDC_STATIC_TIME,static,1342308352
Control2=IDC_EDIT_CLOCK,edit,1342179456
Control3=IDC_STATIC_BEST,static,1342308352
Control4=IDC_EDIT_BEST,edit,1342179456
Control5=IDC_EDIT_DIFF,edit,1342179456
Control6=IDC_LIST_LAP,SysListView32,1350647809
Control7=IDC_STATIC_METER,static,1342177294
Control8=IDC_PROGRESS_TACO,msctls_progress32,1342177285
Control9=IDC_PROGRESS_SPEED,msctls_progress32,1342177285
Control10=IDC_PROGRESS_WTEMP,msctls_progress32,1342177285
Control11=IDC_PROGRESS_OTEMP,msctls_progress32,1342177285
Control12=IDC_PROGRESS_FUEL,msctls_progress32,1342177285
Control13=IDC_STATIC_TACO,static,1342308354
Control14=IDC_STATIC_SPEED,static,1342308354
Control15=IDC_STATIC_WTEMP,static,1342308354
Control16=IDC_STATIC_OTEMP,static,1342308354
Control17=IDC_STATIC_FUEL,static,1342308354
Control18=IDC_SLIDER_PLAY_POS,msctls_trackbar32,1342242840
Control19=IDC_STATIC_GEAR,static,1342312449

[MNU:IDR_MENU1]
Type=1
Class=?
Command1=ID_MENU_OPEN_FIRM
Command2=ID_MENU_LAPTIME
Command3=ID_MENU_GYMKHANA
Command4=ID_MENU_ZERO_FOUR
Command5=ID_MENU_ZERO_ONE
Command6=ID_MENU_SPEED
Command7=ID_MENU_TACO
Command8=ID_MENU_W_TEMP
Command9=ID_MENU_SHIFT_WARN
Command10=ID_MENU_LED_REVERSE
Command11=ID_MENU_BEEP
Command12=ID_MENU_REPLAY
Command13=ID_MENU_EXIT
CommandCount=13

[ACL:IDR_ACCELERATOR1]
Type=1
Class=?
Command1=ID_MENU_ZERO_ONE
Command2=ID_MENU_ZERO_FOUR
Command3=ID_MENU_BEEP
Command4=ID_MENU_GYMKHANA
Command5=ID_MENU_LAPTIME
Command6=ID_MENU_EXIT
Command7=ID_MENU_LED_REVERSE
Command8=ID_MENU_SPEED
Command9=ID_MENU_TACO
Command10=ID_MENU_SETTING
Command11=ID_MENU_SHIFT_WARN
CommandCount=11

