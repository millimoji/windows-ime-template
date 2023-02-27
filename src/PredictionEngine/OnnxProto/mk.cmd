rem memo

set PROTOC=F:\Git\vcpkg\packages\protobuf_x64-windows-static\tools\protobuf\protoc.exe
set ONNXSRCPATH=F:\Git\onnx\onnx

%PROTOC% ^
    --cpp_out . ^
    -I %ONNXSRCPATH% ^
    onnx.proto3
