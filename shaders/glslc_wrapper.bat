@echo off
REM Usage: glslc_wrapper.bat input.vert output.spv
glslc %1 -o %2
