cmake_minimum_required(VERSION 3.0.0)
project(Cocoa VERSION 0.1.0)

set(executable_name "Cocoa")
set(deps_dir ${CMAKE_HOME_DIRECTORY}/deps)
set(third_party_libs_dir ${deps_dir}/v8/out.gn/x64.release.sample/obj
                         ${deps_dir}/skia/out/Shared)
set(sys_libs_dir /opt/rocm/lib)
set(src_dir ${CMAKE_HOME_DIRECTORY}/src)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(COCOA_PROJECT=1 COCOA_USE_OPENCL=1 V8_COMPRESS_POINTERS)
add_compile_options("-Wall")

include_directories(
    ${CMAKE_HOME_DIRECTORY}/src
    ${deps_dir}/skia)

add_executable(
    ${executable_name}

    ${src_dir}/moe/Allocation.cc
    ${src_dir}/moe/GCHeap.cc
    ${src_dir}/moe/Visitor.cc

    ${src_dir}/ciallo/GrBase.cc
    ${src_dir}/ciallo/GrUtils.cc
    ${src_dir}/ciallo/SkpSnapshot.cc
    ${src_dir}/ciallo/DDR/GrLayerResult.cc
    ${src_dir}/ciallo/DDR/GrTargetSurface.cc
    ${src_dir}/ciallo/DDR/GrGpuCompositor.cc
    ${src_dir}/ciallo/DDR/GrCpuCompositor.cc
    ${src_dir}/ciallo/DDR/GrOpenCLCompositor.cc
    ${src_dir}/ciallo/DDR/GrBaseCompositor.cc
    ${src_dir}/ciallo/DDR/GrBaseRenderLayer.cc
    ${src_dir}/ciallo/DDR/GrGpuRenderLayer.cc
    ${src_dir}/ciallo/DDR/GrCpuRenderLayer.cc
    ${src_dir}/ciallo/DDR/GrOpenCLRenderLayer.cc
    ${src_dir}/ciallo/DDR/GrBasePlatform.cc
    ${src_dir}/ciallo/DDR/GrXcbPlatform.cc
    ${src_dir}/ciallo/DIR/BaseNode.cc
    ${src_dir}/ciallo/DIR/CompositeNode.cc
    ${src_dir}/ciallo/DIR/RenderNode.cc
    ${src_dir}/ciallo/DIR/PaintNode.cc
    ${src_dir}/ciallo/Thread.cc
    ${src_dir}/ciallo/GraphicsContext.cc

    ${src_dir}/komorebi/KEvent.cc
    ${src_dir}/komorebi/KWidget.cc
    ${src_dir}/komorebi/KTDLParser.cc
    ${src_dir}/komorebi/css/CSSStylesheetEnvironment.cc
    ${src_dir}/komorebi/css/CSSStylesheet.cc
    ${src_dir}/komorebi/css/CSSSelectionContext.cc
    ${src_dir}/komorebi/css/CSSDOMVisitor.cc
    ${src_dir}/komorebi/css/CSSComputedStyle.cc
    ${src_dir}/komorebi/css/CSSCalculator.cc

    ${src_dir}/core/Journal.cc
    ${src_dir}/core/Exception.cc
    ${src_dir}/core/Configurator.cc
    ${src_dir}/core/Configurations.cc
    ${src_dir}/core/ObjectHolder.cc
    ${src_dir}/core/IODispatcher.cc
    ${src_dir}/core/QObject.cc

    ${src_dir}/Main.cc)

target_link_options(${executable_name} PRIVATE "-rdynamic")
target_link_directories(${executable_name} PRIVATE ${third_party_libs_dir} ${sys_libs_dir})
target_link_libraries(
    ${executable_name}
    PRIVATE
    dl skia vulkan xcb xcb-image xcb-render-util xcb-render
    PocoFoundation PocoXML PocoJSON
    pthread ffi OpenCL
    css parserutils wapcaplet)
