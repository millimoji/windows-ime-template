rem This is memo for how to create onnx.proto3.pb.{h,cc}

set PROTOC=F:\Git\vcpkg\packages\protobuf_x64-windows-static\tools\protobuf\protoc.exe
set ONNXSRCPATH=F:\Git\onnx\onnx

%PROTOC% ^
    --cpp_out . ^
    -I %ONNXSRCPATH% ^
    onnx.proto3
