taskkill /f /t /im devenv.exe
taskkill /f /t /im MSBuild.exe
taskkill /f /t /im mspdbsrv.exe
del vsd_filter.sdf z_gpslog_*.txt

start "D:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\devenv.exe" "D:\DDS\vsd\vsd_filter\vsd_filter.sln"
